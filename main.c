#include <stm32f0xx.h>
#include <stdlib.h>
#define FILTER_MAX_CNT 10
#define DMA_ADC_BUFFER_SIZE 128



typedef struct PatternS{
	unsigned short frames[8];
	char showframe[8];
	char frames_cnt;
} PatternS;

typedef struct Display{
	uint8_t raws[8];
	uint8_t frame;
} Display;

void pushNextFrame(Display* dis){
	unsigned short data = (dis->raws[dis->frame]<<8) | (1 << dis->frame);
	SPI2->DR = data;
	dis->frame++;
	dis->frame%=8;
}


// Programming manual - p. 85+
void STInit(){
	SystemCoreClockUpdate();
	SysTick->LOAD = (SystemCoreClock) / 500 - 1; // 1 ms
	SysTick->VAL =  (SystemCoreClock) / 500 - 1; // initial value of timer
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk; //- razrehit` obrabotchik prerivaniy
}

volatile static int button1 = 0; // vneshnie peremennie
volatile static int button2 = 0;
volatile static int button3 = 0;
volatile static int button4 = 0;
volatile static int stateOfKeyboardChecker = 0;
volatile static int c1 = 0;
volatile static int c2 = 0;
volatile static int c3 = 0;
volatile static int c4 = 0;
volatile static int state1 = 0;
volatile static int state2 = 0;
volatile static int state3 = 0;
volatile static int state4 = 0;
// Display variables
volatile static int frame = 0;
volatile static PatternS patternStruct;
//volatile static int isDataPushedToSPI = 1;
volatile static Display displayS;
volatile static uint32_t ADC_array[DMA_ADC_BUFFER_SIZE];
volatile int half_of_DMA_array;



void ChangeDisplayFrame(){
	frame++;
	frame%=patternStruct.frames_cnt;
}

void PushFrame(int curr_frame, PatternS pat){
		if (!pat.showframe[curr_frame]) SPI2->DR = 0;
		else SPI2->DR = pat.frames[curr_frame];
}

void SysTick_Handler(void){ 
	ButtonsCheker();
}


static void SPI_init(){
	GPIOA->MODER |= GPIO_MODER_MODER8_0;
	
	// podat` taktirovanie na SPI
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	
	//nastraivaem interfeis
	SPI2->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR | SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA;
	SPI2->CR2 = SPI_CR2_DS;
	SPI2->CR1 |= SPI_CR1_SPE;

	// daem taktirovanie na vihody
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	//GPIOB->AFR[0] |= Fn << 4 * Pn; // Pn < 8
	//GPIOB->AFR[1] |= Fn << 4 * (Pn - 8); // Pn >= 8
	GPIOB->AFR[1] |= (0 << 4 * (13 - 8)) | (0 << 4 * (15 - 8));
	
	GPIOB->MODER |= GPIO_MODER_MODER13_1 | GPIO_MODER_MODER15_1;
	
	// Enable interrupts from SPI2 in core
	NVIC_EnableIRQ(SPI2_IRQn);
	
	//Control Register
	SPI2->CR2 |= SPI_CR2_RXNEIE;
	
	SPI2->DR = 0x0111; // to catch first interrupt
}


void SPI2_IRQHandler(){
	if (SPI2->SR & SPI_SR_RXNE){ // Data was received
		GPIOA->BSRR = GPIO_BSRR_BS_8;  // LE = 1
		volatile uint32_t a = SPI2->DR;
		pushNextFrame(&displayS);
		GPIOA->BSRR = GPIO_BSRR_BR_8;
	}
}


void reset_pulse(){
	while((USART1->ISR & USART_ISR_TC) != USART_ISR_TC){
		
	}
	USART1->TDR = 0;
	while ((USART1->ISR & USART_ISR_RXNE) != USART_ISR_RXNE){
		// waiting until data will be received
	}
	char chartoreceive = (uint8_t)(USART1->RDR); /* Receive data, clear flag */
	if (chartoreceive == 0)
		GPIOC->BSRR = GPIO_BSRR_BS_9;
}

void USART_Init(){
	GPIOA->AFR[1] |= (1 << 4 * (9 - 8)) | (1 << 4 * (10 - 8));
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;  // Clock on USART1
	//GPIOA->MODER |= GPIO_MODER_MODER9_0;   // Tx pin -> Output mode
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR9_0 | GPIO_PUPDR_PUPDR10_0;   // pull up;  TX-???
	/* (1) Oversampling by 16 */
	/* (2) Single-wire half-duplex mode */
	/* (3) 8 data bit, 1 start bit, 1 stop bit, no parity, reception and
	transmission enabled */
	USART1->BRR = 48 * 6; /* (1) */ /*baud rate = 1(6 * 10**-6)  BRR = 48MHz / baudrate */
	USART1->CR3 = USART_CR3_HDSEL; /* (2) */
	USART1->CR1 = USART_CR1_RE | USART_CR1_UE; /* (3) */
	
	// CHECK START ------------------------
	while ((USART1->ISR & USART_ISR_RXNE) != USART_ISR_RXNE){
		// waiting until data will be received
	}
	char chartoreceive = (uint8_t)(USART1->RDR); /* Receive data, clear flag */
	if (chartoreceive == 0)
		GPIOC->BSRR = GPIO_BSRR_BS_8;
	// CHECK END   ________________________
	
	/* Polling idle frame Transmission */
	USART1->CR1 = USART_CR1_TE;
	while ((USART1->ISR & USART_ISR_TC) != USART_ISR_TC)
	{
	/* add time out here for a robust application */
	}
	USART1->ICR |= USART_ICR_TCCF; /* Clear TC flag */
	
	/*
	USART1->CR2 |= USART_CR2_ABRMODE_0;
	USART1->CR2 |= USART_CR2_ABREN;
	if((USART1->ISR & USART_ISR_TC) == USART_ISR_TC){
		USART1->TDR = 0;
	}*/
	//USART1->CR1 |= USART_CR1_TCIE; /* Enable TC interrupt */
	/*while (~(USART1->ISR) & USART_ISR_ABRF){
		reset_pulse();
	}*/
	while ((USART1->ISR & USART_ISR_RXNE) != USART_ISR_RXNE){
		// waiting until data will be received
	}
	chartoreceive = (uint8_t)(USART1->RDR); /* Receive data, clear flag */
	if (chartoreceive == 0)
		GPIOC->BSRR = GPIO_BSRR_BS_9;
}



void master_write (int bit){
	if (bit){
		while((USART1->ISR & USART_ISR_TC) != USART_ISR_TC){	
		}
		USART1->TDR = 0xFF;
	}
	else{
		while((USART1->ISR & USART_ISR_TC) != USART_ISR_TC){	
		}
		USART1->TDR = 0x00;
	}
}

static int master_read (){
	while ((USART1->ISR & USART_ISR_RXNE) != USART_ISR_RXNE){
		// waiting until data will be received
	}
	char chartoreceive = (uint8_t)(USART1->RDR); /* Receive data, clear flag */
	if (chartoreceive)
		return 1;
	else return 0;
	
}

int main(void){
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOAEN;
	GPIOC->MODER |= GPIO_MODER_MODER12_0;
	GPIOA->MODER |= GPIO_MODER_MODER15_0;
	GPIOC->MODER |= GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR4_1 | GPIO_PUPDR_PUPDR5_1;
	
	displayS.frame = 0;
	for (int i = 0; i < 8; i++){
		displayS.raws[i] = 0;
	}
	
	for(int i = 0; i<DMA_ADC_BUFFER_SIZE; i++){
		ADC_array[0] = 0;
	}
	
	SPI_init();
	
	USART_Init();
	GPIOC->BSRR = GPIO_BSRR_BS_6;  // Red light
	return 0;
	
}
