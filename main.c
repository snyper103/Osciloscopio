// Gabriel Alexandre de Souza Braga
#include "stm32f1xx.h" // Biblioteca STM
#define MAXTAM 128

/* Variáveis globais */
char rx_dado, podeTransmitir = 0;
unsigned int itx = MAXTAM, iADC = 0;
unsigned int LeituraADC[MAXTAM];

/*
 *##############################################################
 *#################### FUNÇÕES DE USO GERAL ####################
 *##############################################################
*/
void EnviaQuadroIRQ_USART(void)
{
    register int i = 0;

    while ( !(USART1->SR & USART_SR_TXE) ); // Aguarda buffer de Tx estar vazio
    
    itx = 0;
    USART1->CR1 |= USART_CR1_TXEIE; // Hab. interrupção por TXE
}

/*
 *############################################################
 *#################### FUNÇÕES DE HANDLER ####################
 *############################################################
*/
void USART1_IRQHandler(void)
{
    if ( USART1->SR & USART_SR_RXNE )
    {
        /* Recebe dado */
        rx_dado = USART1->DR;
        // Se estiver usando paridade, deve ser verificada aqui (se chegou mesmo  par ou ímpar) 
        if ( rx_dado == 'x' )
            podeTransmitir = 1;
    }
    
    else
        if ( USART1->SR & USART_SR_TXE )
        {
            /* Transmite string */
            if ( itx < MAXTAM )  // Existe algum elemento a ser transmitido?
                USART1->DR = LeituraADC[itx++];
            
            else
                USART1->CR1 &= (uint32_t)(~USART_CR1_TXEIE); // Desab. interrupção por TXE
        }
}
void SysTick_Handler(void)  // Trata IRQ do SysTick
{
    if ( itx >= MAXTAM )
        ADC1->CR2 |= ADC_CR2_SWSTART;   // Dispara a conversão
}
void ADC1_2_IRQHandler(void)    // Trata IRQ do ADC1
{
    if ( ADC1->SR & ADC_SR_EOC )
        LeituraADC[iADC++] = ADC1->DR>>4;  // Ao ler, apaga a flag EOC de forma automática
    
    if ( iADC >= MAXTAM && podeTransmitir )
    {
        EnviaQuadroIRQ_USART();
        podeTransmitir = 0;
    }

    iADC %= 128;
}

/*
 *#################################################################
 *#################### FUNÇÕES DE CONFIGURAÇÃO ####################
 *#################################################################
*/
void gpioConfig(void)   // Configura os registradores
{
    /*
     *  Porta da GPIOA 9 Tx (transmissão de dados)
     *  Porta da GPIOA 10 Rx (Recebimento de dados)
    */
    
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // Habilita clock do barramento APB2 para GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; //  Habilita funções alternativas APB2
    /* Config USART1 para Tx e Rx sem IRQ */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN; // Hab. clock para USART1

    GPIOA->CRH &= ~(GPIO_CRH_CNF9_0); // alternate function output push-pull
    GPIOA->CRH |= GPIO_CRH_CNF9_1; // alternate function output push-pull
    GPIOA->CRH |=  GPIO_CRH_CNF10_0;    // input floating
    GPIOA->CRH &= ~(GPIO_CRH_CNF10_1);  // input floating

    GPIOA->CRH |= GPIO_CRH_MODE9_0 | GPIO_CRH_MODE9_1;    // Configura pino PA9 como saída de 50 MHz
    GPIOA->CRH &= ~(GPIO_CRH_MODE10_0 | GPIO_CRH_MODE10_1);    // Configura pino PA10 como entrada
}
void usart1Config(void) // Config USART1 para Tx e Rx com IRQ
{
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;   // Hab. clock para USART1
    USART1->BRR = (int)8e6/9600;            // Define baudrate = 9600 baud/s (APB2_clock = 8 MHz)
    USART1->CR1 |= (USART_CR1_RE | USART_CR1_TE);   //  Hab. RX e TX
    USART1->CR1 |= USART_CR1_RXNEIE;        // Hab. interrupção por RXNE
    USART1->CR1 |= USART_CR1_UE;            // Hab USART1 
    USART1->CR1 &= ~(USART_CR1_M);            // define 8 bits de resolução 
    // NVIC->IP[USART1_IRQn] = ?;        // Config. prioridade da IRQ USART1 
    NVIC->ISER[1] = (uint32_t)(1 << (USART1_IRQn-32)); // Hab. IRQ da USART1 na NVIC
}
void sysConfig(void)    // Config. SysTick com interrupção a cada 1 ms ->  1 kHz
{     
    SysTick->LOAD = (int)8e3;      // (default BluePill: HSI com 8MHz)
	SysTick->VAL = 0;		    // Limpa o valor da contagem
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk  | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;   // Clock do processador sem dividir, Hab. IRQ e SysTick */
}
void adConfig(void) // Config. ADC1 - modo conversão única do canal 0
{
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;     // Habilita clock do barramento APB2 para ADC1
    ADC1->SQR3 = (0 << ADC_SQR3_SQ1_Pos);   // Define o canal 0 como primeiro a ser lido
    ADC1->SQR1 = (0b000 << ADC_SQR1_L_Pos); // Conversão de um canal somente
    ADC1->SMPR2 = (0b111 << ADC_SMPR2_SMP0_Pos); // Tempo de amostragem de 239.5 cycles
    ADC1->CR1 = ADC_CR1_EOCIE;              // Hab. IRQ por fim de conversão (EOC)
    NVIC->ISER[0] = (uint32_t)(1 << ADC1_IRQn); // Hab. IRQ do ADC na NVIC
    ADC1->CR2 |= (0b111 << ADC_CR2_EXTSEL_Pos); // Seleciona disparo por software
    ADC1->CR2 |= ADC_CR2_EXTTRIG | ADC_CR2_ADON; // Hab. disparo e liga o conversor

    /* Rotina de calibração do ADC */
    ADC1->CR2 |= ADC_CR2_RSTCAL;        // Reseta a calibração
    while (ADC1->CR2 & ADC_CR2_RSTCAL); // aguarda terminar
    ADC1->CR2 |= ADC_CR2_CAL;           // Inicia calibração
    while (ADC1->CR2 & ADC_CR2_CAL);    // aguarda terminar

    ADC1->CR2 |= ADC_CR2_SWSTART;       // Primeiro disparo de conversão
}

/*
 *##############################################
 *#################### MAIN ####################
 *##############################################
*/
int main(void)
{
    gpioConfig();
    usart1Config();
    sysConfig();
    adConfig();
    
    __enable_irq(); // Hab. globalmente as IRQs (PRIMASK), no reset já vem habilitado

    /* Loop infinito */
    while ( 1 );

    return 0;
}
