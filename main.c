#include <stm32f0xx.h>
#include <stdint.h>

static void Init3(void)
{
	RCC->AHBENR |= (RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOAEN);
	GPIOC->MODER |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0);

	SystemCoreClockUpdate();
	SysTick->LOAD = SystemCoreClock / 10 - 1;
	SysTick->VAL =  SystemCoreClock / 10 - 1;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk ;//| SysTick_CTRL_TICKINT_Msk;
}

static void task3(void)
{

	GPIOC->BSRR = GPIO_BSRR_BS_6;
	int flag = 0;

	while(1)
	{
		int button = GPIOA->IDR & GPIO_IDR_0;

		int timer = SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk;
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

int main(void)
{
	Init3();
	task3();
}
