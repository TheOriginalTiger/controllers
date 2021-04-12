#include <stm32f0xx.h>
#include <stdint.h>


#define BUTTON_FILTER 5


volatile uint8_t flag = 0 ;

volatile uint8_t pressed1 = 0;
volatile int counter1 = 0;

volatile uint8_t pressed2 = 0;
volatile int counter2 = 0;

volatile uint8_t pressed3 = 0;
volatile int counter3 = 0;

volatile uint8_t pressed4 = 0;
volatile int counter4 = 0;


void changer(volatile uint8_t* button, volatile int* counter)
{

	if (*counter == BUTTON_FILTER)
	{
		switch (*button){
		case(1):
			*button = 0;
			break;
		default:
			*button = 1;
			break;
		}
	}
}


void checkButton(uint32_t* curr, volatile uint32_t* lastButton, volatile int* counter)
{
	if (*curr != *lastButton)
	{
		*counter = 0 ;
		*lastButton = *curr;
	}
	else
		(*counter)++;

}


volatile uint32_t lastButton1 = 0 ;
volatile uint32_t lastButton2 = 0;
volatile uint32_t lastButton3 = 0 ;
volatile uint32_t lastButton4 = 0;
void SysTick_Handler(void)
{

	uint32_t button1 = GPIOA->IDR & GPIO_IDR_4;
	uint32_t button2 = GPIOA->IDR & GPIO_IDR_5;
	if(flag)
	{
		flag = 0;
		checkButton(&button1, &lastButton1, &counter1);
		checkButton(&button2, &lastButton2, &counter2);

		if(button1)
		{
			changer(&pressed1, &counter1);
		}
		if (button2)
		{
			changer(&pressed2,&counter2);

		}

		GPIOC->BSRR =  GPIO_BSRR_BS_12 ;
		GPIOA->BSRR =  GPIO_BSRR_BR_15 ;
	}
	else
	{
		flag =1;
		checkButton(&button1, &lastButton4, &counter4);
		checkButton(&button2, &lastButton3, &counter3);

		if(button1)
			changer(&pressed4,&counter4);
		if (button2)
			changer(&pressed3,&counter3);

		GPIOC->BSRR = GPIO_BSRR_BR_12 ;
		GPIOA->BSRR = GPIO_BSRR_BS_15 ;

	}
}


void Init(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= GPIO_MODER_MODER8_0;

	//SPI config
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	SPI2->CR1 = SPI_CR1_SSM | SPI_CR1_BR | SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA;

	SPI2->CR2 |= SPI_CR2_DS;
	SPI->CR1 |= SPI_CR1_SRE;

	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	//GPIOB->AFR[0] |= Fn << 4 * pinNumber if Pn < 8
	GPIOB->AFR[1] |= (0 << 4 * (13 - 8)) | (0 << 4 * (15 - 8));
	GPIOB->MODER |= GPIO_MODER_MODER13_1 | GPIO_MODER_MODER15_1;

}

static void usage(void)
{
	SPI->DR = 0x0101;

}
static void Init3(void)
{
	RCC->AHBENR |= (RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOAEN);
	GPIOC->MODER |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0 | GPIO_MODER_MODER12_0);
	//GPIOC->PUPDR |=  (GPIO_PUPDR_PUPDR6_1 | (GPIO_PUPDR_PUPDR7_1) | (GPIO_PUPDR_PUPDR8_1) | (GPIO_PUPDR_PUPDR9_1));
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR4_1 | GPIO_PUPDR_PUPDR5_1);
	GPIOA->MODER |= GPIO_MODER_MODER15_0;

	SystemCoreClockUpdate();
	SysTick->LOAD = SystemCoreClock/1000 - 1;
	SysTick->VAL =  SystemCoreClock/1000 - 1;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
	//PB15 - data line
	//PB13 - CLK
	//PA8 - leech enable

}



int main(void)
{

	// 6 - red
	// 7 - blue
	// 8 - yellow
	// 9 - green
	Init3();
	while(1)
	{
		switch(pressed1) {
		case(1):
			GPIOC->BSRR = GPIO_BSRR_BS_6;
			break;
		default:
			GPIOC->BSRR = GPIO_BSRR_BR_6;
		}

		switch(pressed2) {
		case(1):
			GPIOC->BSRR = GPIO_BSRR_BS_7;
			break;
		default:
			GPIOC->BSRR = GPIO_BSRR_BR_7;

		}
		switch(pressed3){
		case(1):
			GPIOC->BSRR = GPIO_BSRR_BS_8;
			break;
		default:
			GPIOC->BSRR = GPIO_BSRR_BR_8;
		}
		switch(pressed4){
		case(1):
			GPIOC->BSRR = GPIO_BSRR_BS_9;
			break;
		default:
			GPIOC->BSRR = GPIO_BSRR_BR_9;
		}
	}


}

/*
 *
 */





