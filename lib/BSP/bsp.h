#ifndef _BSP_H_
#define _BSP_H_

#include "stm32f7xx_hal.h"

typedef enum {  
  BUTTON_WAKEUP = 0
}Button_TypeDef;

typedef enum {  
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
}ButtonMode_TypeDef;

#define BUTTONn                             1
#define WAKEUP_BUTTON_PIN                   GPIO_PIN_11
#define WAKEUP_BUTTON_GPIO_PORT             GPIOI
#define WAKEUP_BUTTON_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOI_CLK_ENABLE()
#define WAKEUP_BUTTON_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOI_CLK_DISABLE()
#define WAKEUP_BUTTON_EXTI_IRQn             EXTI15_10_IRQn 
#define KEY_BUTTON_EXTI_IRQHandler       		EXTI15_10_IRQHandler

#define BUTTONx_GPIO_CLK_ENABLE(__INDEX__)    do { if((__INDEX__) == 0) \
	WAKEUP_BUTTON_GPIO_CLK_ENABLE(); } while(0)	

void
	BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode);
void
	BSP_PB_DeInit(Button_TypeDef Button);
uint32_t
	BSP_PB_GetState(Button_TypeDef Button);

#endif /*_BSP_H_*/