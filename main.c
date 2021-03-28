#include <stm32f0xx.h>
#include <stdint.h>

static void Init1(void)
{
	// 6 == u (red) , d == 7 (blue) , L == 8 (yellow) , r == 9 (green)
	//enable clock on PC registers
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

	//initializing all of our ports as ouput
	GPIOC->MODER |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0);


	//too slow...
	//GPIOC->ODR |= GPIO_ODR_6;

	//GPIOC->PUPDR |= GPIO_PUPDR_PUPDR0_0;
	//GPIOC->OTYPER |= GPIO_OTYPER_OT_6;

	//GPIOC->BSRR = GPIO_BSRR_BS_6;


}

static void task1(void )
{
	while(1)
	{
		GPIOC->BSRR = (GPIO_BSRR_BR_6 |GPIO_BSRR_BS_7);
		for (int i = 0; i < 100000; i++);
		GPIOC->BSRR = (GPIO_BSRR_BR_7 |GPIO_BSRR_BS_8);
		for (int i = 0; i < 100000; i++);
		GPIOC->BSRR = (GPIO_BSRR_BR_8 |GPIO_BSRR_BS_9);
		for (int i = 0; i < 100000; i++);
		GPIOC->BSRR = (GPIO_BSRR_BR_9 |GPIO_BSRR_BS_6);
		for (int i = 0; i < 100000; i++);
	}
}

static void Init2(void)
{
	RCC->AHBENR |= (RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOAEN);
	GPIOC->MODER |= GPIO_MODER_MODER7_0;
}

static void task2(void )
{
	while(1)
	{
		while(GPIO_IDR_0 & GPIOA->IDR)
		{
			GPIOC->BSRR = GPIO_BSRR_BS_7;
		}
		GPIOC->BSRR = GPIO_BSRR_BR_7;
	}
}

static void Init3(void)
{
	RCC->AHBENR |= (RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOAEN);
	GPIOC->MODER |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0);
}


static void buisnessLogic()
{
	int dur = 500000;
	for(int i=0; i < dur; i++)
	{
		while(GPIOA->IDR & GPIO_IDR_0 && i < dur)
		{
			GPIOC->ODR |= GPIO_ODR_7;
			i++;
		}
		GPIOC->ODR &= ~GPIO_BSRR_BS_7;

	}

}

static void task3(void)
{

	GPIOC->BSRR = GPIO_BSRR_BS_6;
	int flag= 0;
	/*
	while(1)
	{
		if(GPIOA->IDR & GPIO_IDR_0 )
		{
			GPIOC->BSRR = GPIO_BSRR_BR_6 | GPIO_BSRR_BS_8;

			buisnessLogic();
			GPIOC->BSRR = GPIO_BSRR_BR_8 | GPIO_BSRR_BS_9;

			buisnessLogic();
			GPIOC->BSRR = GPIO_BSRR_BR_9 | GPIO_BSRR_BS_6  ;
		}
	}*/
	int i = 0 ;
	int dur = 500000;
	while(1)
	{
		if (GPIOA->IDR & GPIO_IDR_0)
		{
			if (flag == 0)
			{
				GPIOC->BSRR = GPIO_BSRR_BR_6 | GPIO_BSRR_BS_8;
				flag = 1;
			}
			GPIOC->ODR |= GPIO_ODR_7;
		}
		if (i == dur && flag == 1)
		{
			GPIOC->BSRR = GPIO_BSRR_BR_8 | GPIO_BSRR_BS_9;
			flag = 2;
		}
		else if (i == dur && flag == 1)
		i++;
	}
}

int main(void)
{
	Init3();
	task3();
}
