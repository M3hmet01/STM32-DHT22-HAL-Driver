#include "main.h"

#define  DHT_While_Timeout_Control					150			// Timeout 	Control
#define  DHT_Start_Time								1500		// Start  	Signal 	Time
#define  DHT_Wait_Response_Time						30			// Response Signal 	Time
#define	 DHT_Onebit_Send_Time						60			// OneBit 	Send 	Time
#define  DHT_Bit_Check_Tim							50			// OneBit 	Value 	Check

	typedef struct{

	float hum ;		//humidity
	float temp;		//temperature

	}DHT_data;


	typedef struct{

	GPIO_TypeDef *DHT_Port;			//	ex.(GPIOA,GPIOB,etc)
	uint16_t	  DHT_Pin;			//	ex( GPIO_PIN_0, GPIO_PIN_1, etc)

	}DHT_Sensor;


	void Delay_us				(uint16_t delay );
	void Set_Pin_Output	 		(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x);
	void Set_Pin_Input	 		(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x);
	void DHT_Start_Signal		(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x, uint16_t delay );
	void DHT_Wait_Time			(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x, uint16_t delay );
	int Data_Check_Function();

	DHT_data DHT_Read_Data( DHT_Sensor sensor);

