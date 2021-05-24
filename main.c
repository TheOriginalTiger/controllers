#include <stm32f0xx.h>
#include <stdint.h>
#include <string.h>

#define BUTTON_FILTER 5
#define AMOUNTOFDATA 128

volatile uint8_t values[8];
volatile uint8_t status;


void usage(uint8_t x , uint8_t yy)
{
	uint8_t xx = 1 << x;
	uint8_t res = yy >> 5;
	uint32_t coords = (((uint32_t ) xx) <<8) | ((1<< res ) );
	SPI2->DR = coords;

}



void addNewData(uint8_t data)
{
	for(int i =0; i < 7; i++)
	{
		values[i] = values[i+1];
	}
	values[7] = data;
}

void SPI2_IRQHandler(void)
{

	if (SPI2->SR & SPI_SR_RXNE)
	{

		//GPIOC->ODR ^= GPIO_ODR_6;
		GPIOA->BSRR = GPIO_BSRR_BS_8; // leech
		volatile uint32_t lol = SPI2->DR;
		usage(status, values[7 - status]);
		status = (status + 1) % 8;
		GPIOA->BSRR = GPIO_BSRR_BR_8; // leech

	}
}

volatile uint8_t FLAG;
volatile uint8_t fromAds[AMOUNTOFDATA];
volatile uint8_t prevValue;
volatile uint8_t newValue;
volatile uint8_t valuesStatus;

void DMA1_Channel1_IRQHandler()
{
	GPIOC->BSRR = GPIO_BSRR_BS_6;
	FLAG =1;
	if(DMA1->ISR & DMA_ISR_TCIF1)
	{
		DMA1->IFCR = DMA_IFCR_CGIF1;
		valuesStatus = 2;
	}
	else if(DMA1->ISR & DMA_ISR_HTIF1)
	{
		DMA1->IFCR = DMA_ISR_HTIF1;
		valuesStatus = 1;
	}
}

//аппаратные прерывания
// NVIC контроллер прерываний
//


void SysTick_Handler()
{
	if (newValue != prevValue)
	{
		//addNewData(newValue);
		prevValue = newValue;
	}
}

void Init(void)
{
	RCC->AHBENR |= (RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN);

	//a15 and c12 for buttons
	GPIOA->MODER |= (GPIO_MODER_MODER8_0 | GPIO_MODER_MODER15_0 | GPIO_MODER_MODER1_0 | GPIO_MODER_MODER1_1); // MODERA_1 - PA ANALOG
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR4_1 | GPIO_PUPDR_PUPDR5_1);
	GPIOC->MODER |= (GPIO_MODER_MODER12_0 | GPIO_MODER_MODER6_0 | GPIO_MODER_MODER9_0 );



	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	//GPIOB->AFR[0] |= Fn << 4 * pinNumber if Pn < 8
	GPIOB->AFR[1] |= (0 << 4 * (13 - 8)) | (0 << 4 * (15 - 8));
	GPIOB->MODER |= GPIO_MODER_MODER13_1 | GPIO_MODER_MODER15_1;


	SystemCoreClockUpdate();
	SysTick->LOAD = SystemCoreClock / 10 - 1;
	SysTick->VAL =  SystemCoreClock / 10 - 1;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Pos | SysTick_CTRL_ENABLE_Msk ;//| SysTick_CTRL_TICKINT_Msk;



	//SPI config
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	SPI2->CR2 = SPI_CR2_DS | SPI_CR2_RXNEIE;
	SPI2->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR | SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA;
	SPI2->CR1 |= SPI_CR1_SPE;

	NVIC_EnableIRQ(SPI2_IRQn);
	usage(1,1);


	//ADC!
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

	ADC1->CFGR1 |=  ADC_CFGR1_DMACFG ;
	ADC1->CR |= ADC_CR_ADCAL;
	while (ADC1->CR & ADC_CR_ADCAL){}

	ADC1->CR |= ADC_CR_ADEN;
	ADC1->ISR |= ADC_ISR_ADRDY;
	while(!(ADC1->ISR & ADC_ISR_ADRDY))
	{
		ADC1->CR |= ADC_CR_ADEN;
	}

	ADC1->CHSELR |= ADC_CHSELR_CHSEL1;
	ADC1->CFGR1 |= ADC_CFGR1_RES_1;
	ADC1->CFGR1 |= ADC_CFGR1_CONT | ADC_CFGR1_OVRMOD;
	ADC1->CR |= ADC_CR_ADSTART;
	//DMA

	RCC->AHBENR |= RCC_AHBENR_DMAEN;
	ADC1->CFGR1 |= ADC_CFGR1_DMAEN ;
	DMA1_Channel1->CCR |= DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_HTIE | DMA_CCR_TCIE | DMA_CCR_PSIZE_0 ;

	DMA1_Channel1->CNDTR = AMOUNTOFDATA;
	DMA1_Channel1->CPAR = (uint32_t) (&ADC1->DR);
	DMA1_Channel1->CMAR = (uint32_t) fromAds;
	DMA1_Channel1->CCR |= DMA_CCR_EN;
	NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}


int main(void)
{

	// 6 - red
	// 7 - blue
	// 8 - yellow
	// 9 - green					GPIOC->BSRR = GPIO_BSRR_BS_6;
	Init();

	//я обязательно перестану костылить
	int counter = 0 ;
	uint32_t result = 0 ;

	while (1)
	{
		if (FLAG)
		{
			switch(valuesStatus)
			{
				case(1):
						GPIOC->BSRR = GPIO_BSRR_BS_6;
						for(int i =0 ; i < AMOUNTOFDATA/2; i++  )
						{
							result += fromAds[i] ;
						}
						break;
				default:
						GPIOC->BSRR = GPIO_BSRR_BS_9;
						for(int i =AMOUNTOFDATA/2 ; i < AMOUNTOFDATA; i++  )
						{
							result += fromAds[i] ;
						}
						break;
			}
			newValue = (uint8_t) (result / (AMOUNTOFDATA / 2));
			result = 0;
			counter++;
			if (counter == 300)
			{
				addNewData(newValue);
				counter =0;
			}
			FLAG = 0;
		}
	}


}






