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

volatile uint32_t lastButton1 = 0 ;
volatile uint32_t lastButton2 = 0;
volatile uint32_t lastButton3 = 0 ;
volatile uint32_t lastButton4 = 0;


typedef struct cross_t {
	uint16_t x1, y1;
	uint16_t x2, y2;
	uint16_t x3, y3;
} cross;

volatile uint8_t crossState = 0;

//normal'niy naming dlya pidorasov
cross heh;
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

void changer(volatile uint8_t* button, volatile int* counter)
{

	if (*counter == BUTTON_FILTER)
	{
		*button = 1;
	}
}

static void usage(uint16_t xx, uint16_t yy)
{

	uint32_t coords = ((uint32_t ) yy<<8) | (uint32_t ) xx;
	SPI2->DR = coords;

}


void SPI2_IRQHandler(void)
{


	if (SPI2->SR & SPI_SR_RXNE)
	{

		//GPIOC->ODR ^= GPIO_ODR_6;
		GPIOA->BSRR = GPIO_BSRR_BS_8; // leech
		volatile uint32_t lol = SPI2->DR;
		switch(crossState)
		{
			case(0):
				usage(heh.x1,heh.y1);
				break;
			case(1):
				usage(heh.x2,heh.y2);
				break;
			default:
				usage(heh.x3,heh.y3);
		}
		crossState = (crossState + 1) % 3;
		GPIOA->BSRR = GPIO_BSRR_BR_8; // leech

	}
}

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
		flag = 1;
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


//аппаратные прерывания
// NVIC контроллер прерываний
//


void Init(void)
{
	RCC->AHBENR |= (RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN);

	//a15 and c12 for buttons
	GPIOA->MODER |= (GPIO_MODER_MODER8_0 | GPIO_MODER_MODER15_0);
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR4_1 | GPIO_PUPDR_PUPDR5_1);
	GPIOC->MODER |= (GPIO_MODER_MODER12_0 | GPIO_MODER_MODER6_0);



	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	//GPIOB->AFR[0] |= Fn << 4 * pinNumber if Pn < 8
	GPIOB->AFR[1] |= (0 << 4 * (13 - 8)) | (0 << 4 * (15 - 8));
	GPIOB->MODER |= GPIO_MODER_MODER13_1 | GPIO_MODER_MODER15_1;


	SystemCoreClockUpdate();
	SysTick->LOAD = SystemCoreClock / 2000 - 1;
	SysTick->VAL =  SystemCoreClock / 2000 - 1;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Pos | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
	heh.x1 = heh.y1 = 0x08;
	heh.x2 = 0x1C;
	heh.y2 = 0x10;
	heh.x3 = 0x08;
	heh.y3 = 0x20;


	//SPI config
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	SPI2->CR2 = SPI_CR2_DS | SPI_CR2_RXNEIE;
	SPI2->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR | SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA;
	SPI2->CR1 |= SPI_CR1_SPE;

	NVIC_EnableIRQ(SPI2_IRQn);
	usage(heh.x1, heh.y1);

}

volatile uint16_t x = 0x08;
volatile uint16_t y = 0x08;






int main(void)
{

	// 6 - red
	// 7 - blue
	// 8 - yellow
	// 9 - green
	Init();

	//я обязательно перестану костылить

	while (1)
	{

		if(pressed2)
		{
			// ненавижу программирование
			if(heh.x1 < 0x40)
			{
				heh.x1 = heh.x1 << 1;
				heh.x2 = heh.x2 << 1;
				heh.x3 = heh.x3 << 1;
			}
			pressed2 = 0 ;
		}
		if(pressed1)
		{
			if(heh.x1 > 0x02)
			{
				heh.x1 = heh.x1 >> 1;
				heh.x2 = heh.x2 >> 1;
				heh.x3 = heh.x3 >> 1;
			}
			pressed1 = 0 ;
		}
		if(pressed4)
		{
			if(heh.y1 < 0x40)
			{

				heh.y1 = heh.y1 << 1;
				heh.y2 = heh.y2 << 1;
				heh.y3 = heh.y3 << 1;
			}
			pressed4 = 0 ;
		}
		if(pressed3)
		{
			if(heh.y1 > 0x01)
			{

				heh.y1 = heh.y1 >> 1;
				heh.y2 = heh.y2 >> 1;
				heh.y3 = heh.y3 >> 1;
			}
			pressed3 = 0 ;
		}
	}

}






