#include <stm32f0xx.h>
#include <stdlib.h>
#define FILTER_MAX_CNT 10
#define DMA_ADC_BUFFER_SIZE 128
#define FIRST 1



typedef struct DisplayNumber{
	uint8_t raws[8];
} DisplayNumber;


typedef struct Display{
	uint8_t raws[8];
	uint8_t frame;
	int temperature;
	char isChanged;
} Display;

// Display variables
volatile static int frame = 0;
//volatile static int isDataPushedToSPI = 1;
volatile static Display displayS;
volatile static DisplayNumber dNumbers[10];


void pushNextFrame(Display* dis){
	if(dis->isChanged){
		for (int i = 0; i < 8; i++){
			displayS.raws[i] = (dNumbers[dis->temperature % 10].raws[i] << 4) | dNumbers[(dis->temperature / 10)%10].raws[i];
		}
		dis->isChanged = 0;
	}
	
	
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


volatile uint8_t chartoreceive;
volatile uint8_t flg = 0;
volatile uint8_t counter = 0 ;
volatile uint8_t sending;
void USART1_IRQHandler()
{
		
	if ((USART1->ISR & USART_ISR_RXNE) == USART_ISR_RXNE)
	{
		counter++;
		chartoreceive = (uint8_t)(USART1->RDR);
		flg = 1;
		
		//GPIOC->BSRR = GPIO_BSRR_BS_7;
		//for (int i =0 ; i < 10000/2; i++);
		//GPIOC->BSRR = GPIO_BSRR_BR_7;
	}
}


void resetBaudRate(uint32_t br)
{
	if (USART1->CR1 & USART_CR1_UE)
	{
		USART1->CR1 &= (~USART_CR1_UE );
	}
	
	USART1->BRR = (SystemCoreClock /  br ); 

	USART1->CR1 |= USART_CR1_UE;
}

void master_write (int bit){
	if (bit){
		USART1->TDR = 0xFF;
		while((USART1->ISR & USART_ISR_TC) != USART_ISR_TC){}
	}
	else{
		USART1->TDR = 0x00;
		while((USART1->ISR & USART_ISR_TC) != USART_ISR_TC){}
		// while flag probably to read    --  overrun!!!
	}
}

static int master_read (){
	flg = 0 ;
	USART1->TDR = 0xFF;  // initialize read time slot
	//while((USART1->ISR & USART_ISR_TC) != USART_ISR_TC){}
	while (!flg)
	{
		GPIOC->BSRR = GPIO_BSRR_BS_8; //yellow
	}
	GPIOC->BSRR = GPIO_BSRR_BR_8;
	flg = 0 ;
	if (chartoreceive == 0xFF) // RX byte received from a 1-Wire read 1 is always 0xFF
		return 1;
	else return 0; // RX byte received from a 1-Wire read 0 ranges from 0xFE to 0x00 depending on ...
}

int reset_pulse(){
	resetBaudRate(9600); //~104 mks each bit
	
	sending = 0xF0;
	flg = 0 ;
	USART1->TDR = sending;
	while ((USART1->ISR & USART_ISR_TC) != USART_ISR_TC){}
	
	while (!flg)
	{
		GPIOC->BSRR = GPIO_BSRR_BS_7; //blue
	}
	flg = 0 ;
	GPIOC->BSRR = GPIO_BSRR_BR_7;
	resetBaudRate(115200); //~8.5 mks each bit
	if (chartoreceive <= 0b11100000 )
	{
		GPIOC->BSRR = GPIO_BSRR_BS_9; //Green	
		return 0;
	}
	else 
	{
		GPIOC->BSRR = GPIO_BSRR_BS_8; //Joltiy
		return 1;
	}
	
}

uint8_t usart3Flag;
void USART3_4_IRQHandler()
{
	if(USART3->ISR & USART_ISR_RXNE)
	{
		uint8_t data = (uint8_t) USART3->RDR;
		
		GPIOC->BSRR = GPIO_BSRR_BS_7;
		displayS.temperature = data;
		displayS.isChanged = 1;
		GPIOC->BSRR = GPIO_BSRR_BR_7;
		usart3Flag = 1;
	}
}	


void USART3_Init()
{
	GPIOC->AFR[1] |= (1 << (4 * (10 - 8))) | (1 << (4 * (11 - 8)));
	GPIOD->AFR[0] |= (1 << (4 * 2));
	RCC->APB1ENR  |= RCC_APB1ENR_USART3EN;
	SystemCoreClockUpdate();
	USART3->BRR = 4800000 / 9600;
	USART3->CR3 |= USART_CR3_DEM | USART_CR3_OVRDIS;
	
	/*
	if (FIRST)
	{
		USART3->CR1 |=  USART_CR1_TE ;
	}
	else 
	{
		USART3->CR1 |=  USART_CR1_RE ;
	}*/
	USART3->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;
	NVIC_EnableIRQ(USART3_4_IRQn);
}

void USART_Init(){
	
	GPIOA->AFR[1] |= (1 << (4 * (9 - 8))) | (1 << (4 * (10 - 8)));
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;  // Clock on USART1
	SystemCoreClockUpdate();
	/* (3) 8 data bit, 1 start bit, 1 stop bit, no parity, reception and transmission enabled */
	USART1->CR1 |= USART_CR1_TE | USART_CR1_RE  | USART_CR1_RXNEIE	| USART_CR1_UE ; /* (3) */
	
	resetBaudRate(9600);
	NVIC_EnableIRQ(USART1_IRQn);
	
	/* Polling idle frame Transmission */
	while ((USART1->ISR & USART_ISR_TC) != USART_ISR_TC){}

}

static void sendByte(uint8_t byte){
	for(int i = 0; i<8; i++){
		master_write(byte & (1 << i));
	}
}

static uint8_t readByte()
{
	uint8_t res = 0 ; 
	for (int i = 0 ; i < 8; i++)
			res |= (master_read()<<i);
	return res; 
}	

void businessLogic(int* temp){

		// Start temperature conversion
		if(reset_pulse()) return;
		
		sendByte(0xCC); // Skip Rom
		sendByte(0x44); // Convert T
		
		// Wait untill conversion complete		 
		while(!master_read());//TODO pojimalkin
		
		if(reset_pulse()) return;
		
		sendByte(0xCC); // Skip Rom
		 // Read scratchpad! 
		int mem[9];
		for (int i =0 ; i <0; i++)
			mem[i] = 0 ;
		int cc = 0 ; 
		GPIOC->BSRR = GPIO_BSRR_BR_6;
		sendByte(0xBE);
		while (cc <9)
		{
			mem[cc] = readByte();
			cc++;
		}
		int16_t resTemp = mem[0] | (mem[1]<<8);
		resTemp = resTemp >> 4;
		*temp = resTemp;
}

void notBuisnessLogic()
{
	
	uint8_t temp1 = 0;
	while(1)
	{
		if(FIRST)
		{
				
				USART3->TDR = temp1;
				while((USART3->ISR & USART_ISR_TC) != USART_ISR_TC)
				{
					GPIOC->BSRR = GPIO_BSRR_BS_8;
				}
				GPIOC->BSRR = GPIO_BSRR_BR_8;
				while(!usart3Flag);
				usart3Flag = 0;
				businessLogic(&temp1);
		}
		else
		{
			
			while(!usart3Flag);
			usart3Flag = 0;
			USART3->TDR = temp1;
			while((USART3->ISR & USART_ISR_TC) != USART_ISR_TC)
			{
				GPIOC->BSRR = GPIO_BSRR_BS_8;
			}
			GPIOC->BSRR = GPIO_BSRR_BR_8;
			businessLogic(&temp1);
		}
	}
}





static void numbersInit(){
	dNumbers[0].raws[0] = 0x0F;
	dNumbers[0].raws[1] = 0x09;
	dNumbers[0].raws[2] = 0x09;
	dNumbers[0].raws[3] = 0x09;
	dNumbers[0].raws[4] = 0x09;
	dNumbers[0].raws[5] = 0x09;
	dNumbers[0].raws[6] = 0x09;
	dNumbers[0].raws[7] = 0x0F;

	dNumbers[1].raws[0] = 0x04;
	dNumbers[1].raws[1] = 0x06;
	dNumbers[1].raws[2] = 0x04;
	dNumbers[1].raws[3] = 0x04;
	dNumbers[1].raws[4] = 0x04;
	dNumbers[1].raws[5] = 0x04;
	dNumbers[1].raws[6] = 0x04;
	dNumbers[1].raws[7] = 0x04;

	dNumbers[2].raws[0] = 0x06;
	dNumbers[2].raws[1] = 0x09;
	dNumbers[2].raws[2] = 0x08;
	dNumbers[2].raws[3] = 0x08;
	dNumbers[2].raws[4] = 0x04;
	dNumbers[2].raws[5] = 0x02;
	dNumbers[2].raws[6] = 0x01;
	dNumbers[2].raws[7] = 0x0F;

	dNumbers[3].raws[0] = 0x06;
	dNumbers[3].raws[1] = 0x09;
	dNumbers[3].raws[2] = 0x08;
	dNumbers[3].raws[3] = 0x04;
	dNumbers[3].raws[4] = 0x08;
	dNumbers[3].raws[5] = 0x08;
	dNumbers[3].raws[6] = 0x09;
	dNumbers[3].raws[7] = 0x06;
	
	dNumbers[4].raws[0] = 0x09;
	dNumbers[4].raws[1] = 0x09;
	dNumbers[4].raws[2] = 0x09;
	dNumbers[4].raws[3] = 0x0F;
	dNumbers[4].raws[4] = 0x08;
	dNumbers[4].raws[5] = 0x08;
	dNumbers[4].raws[6] = 0x08;
	dNumbers[4].raws[7] = 0x08;
	
	dNumbers[5].raws[0] = 0x0F;
	dNumbers[5].raws[1] = 0x01;
	dNumbers[5].raws[2] = 0x01;
	dNumbers[5].raws[3] = 0x0F;
	dNumbers[5].raws[4] = 0x08;
	dNumbers[5].raws[5] = 0x08;
	dNumbers[5].raws[6] = 0x09;
	dNumbers[5].raws[7] = 0x0F;
	
	dNumbers[6].raws[0] = 0x0F;
	dNumbers[6].raws[1] = 0x01;
	dNumbers[6].raws[2] = 0x01;
	dNumbers[6].raws[3] = 0x0F;
	dNumbers[6].raws[4] = 0x09;
	dNumbers[6].raws[5] = 0x09;
	dNumbers[6].raws[6] = 0x09;
	dNumbers[6].raws[7] = 0x0F;
	
	dNumbers[7].raws[0] = 0x0F;
	dNumbers[7].raws[1] = 0x08;
	dNumbers[7].raws[2] = 0x04;
	dNumbers[7].raws[3] = 0x04;
	dNumbers[7].raws[4] = 0x02;
	dNumbers[7].raws[5] = 0x02;
	dNumbers[7].raws[6] = 0x01;
	dNumbers[7].raws[7] = 0x01;
	
	dNumbers[8].raws[0] = 0x06;
	dNumbers[8].raws[1] = 0x09;
	dNumbers[8].raws[2] = 0x09;
	dNumbers[8].raws[3] = 0x06;
	dNumbers[8].raws[4] = 0x09;
	dNumbers[8].raws[5] = 0x09;
	dNumbers[8].raws[6] = 0x09;
	dNumbers[8].raws[7] = 0x06;

	dNumbers[9].raws[0] = 0x0F;
	dNumbers[9].raws[1] = 0x09;
	dNumbers[9].raws[2] = 0x09;
	dNumbers[9].raws[3] = 0x0F;
	dNumbers[9].raws[4] = 0x08;
	dNumbers[9].raws[5] = 0x08;
	dNumbers[9].raws[6] = 0x09;
	dNumbers[9].raws[7] = 0x0F;
}

int main(void){
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIODEN;
	GPIOC->MODER |= GPIO_MODER_MODER12_0;
	GPIOA->MODER |= GPIO_MODER_MODER15_0;
	GPIOC->MODER |= GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0 | GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1;
	GPIOD->MODER |= GPIO_MODER_MODER2_1;
	GPIOA->MODER |= GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR4_1 | GPIO_PUPDR_PUPDR5_1;
	
	numbersInit();
	displayS.frame = 0;
	for (int i = 0; i < 8; i++){
		displayS.raws[i] = 0;
	}
	
	SPI_init();
	
	
	USART_Init();
	USART3_Init();
	
	notBuisnessLogic();
	
	
}
