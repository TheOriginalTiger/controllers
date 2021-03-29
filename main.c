#include <stm32f0xx.h>
#include <stdint.h>



volatile uint8_t flag = 0 ;

volatile uint8_t pressed1 = 0;
volatile uint8_t pressed2 = 0;
volatile uint8_t pressed3 = 0;
volatile uint8_t pressed4 = 0;

void SysTick_Handler(void)
{

	pressed1 = pressed2 = pressed3 = pressed4 = 0;

	if(flag)
	{
		flag = 0;
		if(GPIOA->IDR & GPIO_IDR_4)
			pressed1 = 1;
		if (GPIOA->IDR & GPIO_IDR_5)
			pressed2 = 1 ;

		GPIOC->ODR |= GPIO_ODR_12;
		GPIOA->ODR &= ~GPIO_ODR_15;

		//GPIOC->BSRR =  GPIO_BSRR_BR_12 ;
		//GPIOA->BSRR =  GPIO_BSRR_BS_15 ;
	}
	else
	{
		flag =1;
		if(GPIOA->IDR & GPIO_IDR_4)
			pressed3 = 1;
		if (GPIOA->IDR & GPIO_IDR_5)
			pressed4 = 1 ;
		//GPIOC->BSRR = GPIO_BSRR_BS_12 ;
		//GPIOA->BSRR = GPIO_BSRR_BR_15 ;
		GPIOC->ODR &= ~GPIO_ODR_12;
		GPIOA->ODR |= GPIO_ODR_15;

	}
}


static void Init3(void)
{
	RCC->AHBENR |= (RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOAEN);
	GPIOC->MODER |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0 | GPIO_MODER_MODER12_0);
	GPIOC->PUPDR |=  (GPIO_PUPDR_PUPDR6_1 | (GPIO_PUPDR_PUPDR7_1) | (GPIO_PUPDR_PUPDR8_1) | (GPIO_PUPDR_PUPDR9_1));
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR4_1 | GPIO_PUPDR_PUPDR5_1);
	GPIOA->MODER |= GPIO_MODER_MODER15_0;

	SystemCoreClockUpdate();
	SysTick->LOAD = SystemCoreClock/1000  - 1;
	SysTick->VAL =  SystemCoreClock/1000 - 1;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
}


/*
 * SysTick_CTRL_TICKINT_Msk enable interrupt

static void task3(void)
{

	GPIOC->BSRR = GPIO_BSRR_BS_6;
	int flag = 0;
	uint32_t oldTimer = 0 ;
	while(1)
	{
		int button = GPIOA->IDR & GPIO_IDR_0;
		uint32_t timer = timerCounter - oldTimer;
		if(timer)
		{
			oldTimer++;
		}
		if (button && flag == 0)
		{
				GPIOC->BSRR = GPIO_BSRR_BR_6 | GPIO_BSRR_BS_8;
				flag = 1;
		}		
		else if (timer && flag == 1)
		{
			GPIOC->BSRR = GPIO_BSRR_BR_8 | GPIO_BSRR_BS_9;
			flag = 2;
		}
		else if (timer && flag == 2)
		{
			
			GPIOC->BSRR = GPIO_BSRR_BR_9 | GPIO_BSRR_BS_6;
			flag = 0 ; 
		}

		if(button)
		{
			GPIOC->ODR |= GPIO_ODR_7;
		}
		GPIOC->ODR &= ~GPIO_ODR_7;
	}
}
*/
int main(void)
{
	Init3();
	while(1)
	{
		uint32_t mask = 0;
		if (pressed1)
			mask |= GPIO_ODR_6;
		if (pressed2)
			mask |= GPIO_ODR_7;;
		if (pressed3)
			mask |= GPIO_ODR_8;;
		if (pressed4)
			mask |= GPIO_ODR_9;;
		GPIOC->ODR |= mask;
		GPIOC->ODR &= ~(GPIO_ODR_6 | GPIO_ODR_7 | GPIO_ODR_8 | GPIO_ODR_9);
	}
}
