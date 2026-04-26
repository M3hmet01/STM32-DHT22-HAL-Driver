/*
 * DHT.c
 * Author: Mehmet Yusuf Alkan
 * Description: STM32 HAL driver for DHT22 Temperature & Humidity sensor.
 * Features microsecond precision using hardware timers and robust timeout handling.
 */

#include "DHT.h"

// External hardware timer reference for microsecond delays
extern TIM_HandleTypeDef htim2;

// Global variables for sensor data and parsing
uint8_t 	Data[5] 		= {0}; 				// 40-bit data buffer (5 bytes)
uint16_t  	Temperature		=  0 ;
uint16_t  	Humidity	 	=  0 ;

GPIO_InitTypeDef GPIO_InitStruct = {0};

/**
 * @brief  Provides a microsecond-level delay using a hardware timer.
 * @param  delay: Time in microseconds to wait.
 */

void Delay_us(uint16_t delay ){
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	while(__HAL_TIM_GET_COUNTER(&htim2) < delay);
}

/**
 * @brief  Configures the sensor data pin as Output (Open-Drain).
 * Open-Drain is used for 1-wire bidirectional communication.
 * @param  GPIOx: GPIO Port
 * @param  GPIO_Pin_x: GPIO Pin number
 */

void Set_Pin_Output	 		(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x){
	GPIO_InitStruct.Pin =	GPIO_Pin_x;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

/**
 * @brief  Configures the sensor data pin as Input.
 * @param  GPIOx: GPIO Port
 * @param  GPIO_Pin_x: GPIO Pin number
 */

void Set_Pin_Input	 		(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x){
	GPIO_InitStruct.Pin  =       GPIO_Pin_x;
	GPIO_InitStruct.Mode = 	GPIO_MODE_INPUT;
	HAL_GPIO_Init  (GPIOx, &GPIO_InitStruct);
}

/**
 * @brief  Sends the start signal to wake up the DHT22 sensor (Pull LOW).
 * @param  GPIOx: GPIO Port
 * @param  GPIO_Pin_x: GPIO Pin number
 * @param  delay: Duration to hold the line low (usually > 1ms)
 */

void DHT_Start_Signal		(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x, uint16_t delay ){
	Set_Pin_Output(GPIOx, GPIO_Pin_x);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin_x, 0);
	Delay_us(delay);
}

/**
 * @brief  Releases the line (Pull HIGH) and waits for the sensor's response.
 * @param  GPIOx: GPIO Port
 * @param  GPIO_Pin_x: GPIO Pin number
 * @param  delay: Duration to wait (usually 20-40us)
 */

void DHT_Wait_Time			(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x, uint16_t delay ){
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin_x, 1);
	Delay_us(delay);
}

/**
 * @brief  Verifies the integrity of the received 40-bit data.
 * @retval 1 if data is valid (checksum matches), 0 if data is corrupted.
 */

int Data_Check_Function(){
	
	uint8_t CheckSum = 0;

	// Calculate sum of the first 4 bytes
	for(int index = 0; index <4; index++){
		CheckSum += Data[index];
	}
	
	// Compare with the 5th byte (parity bit)
	if(CheckSum == Data[4]){
		return 1;
	} else{
		return 0;
	}
}

/**
 * @brief  Main state machine to trigger, read, and parse DHT22 data.
 * @param  sensor: DHT_Sensor struct containing port and pin info.
 * @return DHT_data struct containing parsed humidity and temperature floats.
 */

DHT_data DHT_Read_Data  (DHT_Sensor sensor ){

	int Tim;
	DHT_data 	result = {0.0f, 0.0f};

	// 1. Clear the data buffer before starting a new reading
		for(int i = 0  ; i <=4  ; i++ ){
		Data[i] = 0;
		}
	
	// 2. MCU Wake-up Request Phase
	DHT_Start_Signal		(sensor.DHT_Port, sensor.DHT_Pin, DHT_Start_Time);
	DHT_Wait_Time			(sensor.DHT_Port, sensor.DHT_Pin, DHT_Wait_Response_Time);
	Set_Pin_Input			(sensor.DHT_Port, sensor.DHT_Pin);

	// 3. Sensor Acknowledge Phase (Wait for 80us LOW, then 80us HIGH)
    __HAL_TIM_SET_COUNTER(&htim2, 0);
	while(HAL_GPIO_ReadPin(sensor.DHT_Port, sensor.DHT_Pin) == 0){
	if(__HAL_TIM_GET_COUNTER(&htim2) > DHT_While_Timeout_Control){return result;}
}

    __HAL_TIM_SET_COUNTER(&htim2, 0);
	while(HAL_GPIO_ReadPin(sensor.DHT_Port, sensor.DHT_Pin) == 1 ){
	if(__HAL_TIM_GET_COUNTER(&htim2) > DHT_While_Timeout_Control){return result;}
}

	// 4. Data Reading Phase (40 bits total: 5 bytes x 8 bits)
			for(int index = 0; index <=4 ; index++){
					for(int bit = 7; bit >=0; bit--){

						// Wait out the ~50us LOW signal before every bit
					    __HAL_TIM_SET_COUNTER(&htim2, 0);
						while(HAL_GPIO_ReadPin(sensor.DHT_Port, sensor.DHT_Pin) == 0){
						if(__HAL_TIM_GET_COUNTER(&htim2) > DHT_While_Timeout_Control){return result;}
					}
						// Measure the duration of the HIGH signal
					    __HAL_TIM_SET_COUNTER(&htim2, 0);
						while(HAL_GPIO_ReadPin(sensor.DHT_Port, sensor.DHT_Pin) == 1 ){
						if(__HAL_TIM_GET_COUNTER(&htim2) > DHT_While_Timeout_Control){return result;}
					}

			Tim = __HAL_TIM_GET_COUNTER(&htim2);

			// If HIGH duration > threshold, it's a logical '1' (typically ~70us for '1', ~26us for '0')
			if(  Tim >= DHT_Bit_Check_Tim){
				Data[index] |= (1 << bit);
			}

		}
	}
	
	// 5. Data Verification and Conversion Phase
	if  (Data_Check_Function() == 1) {

		// Calculate Relative Humidity (Bytes 0 and 1)
		Humidity  = (Data[0]<<8 | Data[1]);
		result.hum =  Humidity / 10.0f;

		// Calculate Temperature (Bytes 2 and 3)
		Temperature = (Data[2] <<8 | Data[3]);
		// Check if the MSB of Byte 2 is set (indicates negative temperature)
		if(Data[2]  &  0x80){
		Temperature &= 0x7FFF;	// Mask the sign bit
		result.temp = Temperature / -10.0f;
		}
		else
		{
		result.temp =  Temperature / 10.0f;
		}

	}

	return result;
}

