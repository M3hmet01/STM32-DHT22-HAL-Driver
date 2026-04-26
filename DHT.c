#include "DHT.h"
extern TIM_HandleTypeDef htim2;

uint8_t 	Data[5] 		= {0};
uint16_t  	Temperature		=  0 ;
uint16_t  	Humidity	 	=  0 ;

GPIO_InitTypeDef GPIO_InitStruct = {0};


void Delay_us(uint16_t delay ){
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	while(__HAL_TIM_GET_COUNTER(&htim2) < delay);
}
void Set_Pin_Output	 		(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x){
	GPIO_InitStruct.Pin =	GPIO_Pin_x;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}
void Set_Pin_Input	 		(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x){
	GPIO_InitStruct.Pin  =       GPIO_Pin_x;
	GPIO_InitStruct.Mode = 	GPIO_MODE_INPUT;
	HAL_GPIO_Init  (GPIOx, &GPIO_InitStruct);
}
void DHT_Start_Signal		(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x, uint16_t delay ){
	Set_Pin_Output(GPIOx, GPIO_Pin_x);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin_x, 0);
	Delay_us(delay);
}
void DHT_Wait_Time			(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x, uint16_t delay ){
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin_x, 1);
	Delay_us(delay);
}
int Data_Check_Function(){


	uint8_t 	CheckSum   			= 0;
	for(int index  = 0; index <4; index++){
		CheckSum  += Data[index];
	}
	if(CheckSum == Data[4]){
		return 1;
	}
	else{
		return 0;
	}
}

DHT_data DHT_Read_Data  (DHT_Sensor sensor ){

	int Tim;
	DHT_data 	result = {0.0f, 0.0f};

	//Data Clear Loop
		for(int i = 0  ; i <=4  ; i++ ){
		Data[i] = 0;
		}

	DHT_Start_Signal		(sensor.DHT_Port, sensor.DHT_Pin, DHT_Start_Time);
	DHT_Wait_Time			(sensor.DHT_Port, sensor.DHT_Pin, DHT_Wait_Response_Time);
	Set_Pin_Input			(sensor.DHT_Port, sensor.DHT_Pin);

    __HAL_TIM_SET_COUNTER(&htim2, 0);
	while(HAL_GPIO_ReadPin(sensor.DHT_Port, sensor.DHT_Pin) == 0){
	if(__HAL_TIM_GET_COUNTER(&htim2) > DHT_While_Timeout_Control){return result;}
}

    __HAL_TIM_SET_COUNTER(&htim2, 0);
	while(HAL_GPIO_ReadPin(sensor.DHT_Port, sensor.DHT_Pin) == 1 ){
	if(__HAL_TIM_GET_COUNTER(&htim2) > DHT_While_Timeout_Control){return result;}
}


			for(int index = 0; index <=4 ; index++){

					for(int bit = 7; bit >=0; bit--){

					    __HAL_TIM_SET_COUNTER(&htim2, 0);
						while(HAL_GPIO_ReadPin(sensor.DHT_Port, sensor.DHT_Pin) == 0){
						if(__HAL_TIM_GET_COUNTER(&htim2) > DHT_While_Timeout_Control){return result;}
					}

					    __HAL_TIM_SET_COUNTER(&htim2, 0);
						while(HAL_GPIO_ReadPin(sensor.DHT_Port, sensor.DHT_Pin) == 1 ){
						if(__HAL_TIM_GET_COUNTER(&htim2) > DHT_While_Timeout_Control){return result;}
					}

			Tim = __HAL_TIM_GET_COUNTER(&htim2);
			if(  Tim >= DHT_Bit_Check_Tim){
				Data[index] |= (1 << bit);
			}

		}
	}

	if  (Data_Check_Function() == 1) {
		Humidity  = (Data[0]<<8 | Data[1]);
		result.hum =  Humidity / 10.0f;


		Temperature = (Data[2] <<8 | Data[3]);
		if(Data[2]  &  0x80){
		Temperature &= 0x7FFF;
		result.temp = Temperature / -10.0f;
		}
		else
		{
		result.temp =  Temperature / 10.0f;
		}

	}

	return result;
}

