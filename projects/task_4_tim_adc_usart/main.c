//**************************************************************************************************
// Includes
//**************************************************************************************************

#include "main.h"


#define UREF        3000    //Vref 3V
#define TWELVE_BIT  4096    //12bit: 2^12 = 4096
#define DONE        1
#define IDLE        !DONE


//**************************************************************************************************
// Declarations and definitions
//**************************************************************************************************
void DMA1_Stream6_IRQHandler(void);
void DMA2_Stream0_IRQHandler(void);


//**************************************************************************************************
// Global variable
//**************************************************************************************************
char res[SIZE];
uint16_t resADC1;

volatile uint8_t USART_TX_ReadyToSend; 
volatile uint8_t ADC_Was_Measured; 

uint16_t voltageADC1;
uint16_t meanADC1;
uint8_t gCounter;


//**************************************************************************************************
// Function main()     
//**************************************************************************************************

int main(void)
{
    USART_TX_ReadyToSend = IDLE;
    ADC_Was_Measured = IDLE;
    meanADC1 = 0;
    gCounter = 0;

	SetSysClock_HSE_142(); 
    Configure_ADC1();
	Configure_TIM3();
	Configure_USART2();
    Configure_DMA1();
    Configure_DMA2();

	while (1)
	{
        if (ADC_Was_Measured)
        {

            voltageADC1 = (UREF * meanADC1) / TWELVE_BIT; //mV : (Data_ADC = (Uref * DOR)/ 4096)

            meanADC1 = 0;

            for (uint8_t i = 0; i < SIZE; i++)
            {
                res[i] = ' ';
            }

            uint8_t length = 0;


            while (voltageADC1)
            {        
                res[length++] =  (voltageADC1 % 10) + '0';
                voltageADC1 = voltageADC1 / 10;
            }

            res[length++] = '\n';
            res[length++] = '\r';


            uint8_t i = 0;
            uint8_t j = length - 1;
            uint8_t tmp;

            while (i < j)
            {
                tmp = res[i];
                res[i] = res[j];
                res[j] = tmp;
                i++;
                j--;
            }

            DMA_Cmd(DMA1_Stream6, ENABLE);
            while(!USART_TX_ReadyToSend) {};
            
            ADC_Was_Measured = IDLE;
            USART_TX_ReadyToSend = IDLE;
            gCounter = 0;
        }
	}

	return 0;
}


//**************************************************************************************************
// Procedure DMA2_Stream0_IRQHandler()     
//**************************************************************************************************

//* IRQ Handler DMA2 *//
void DMA2_Stream0_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0) == SET)
    {
        DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);

        if (gCounter < 2)
        {
            meanADC1 += resADC1;
        }
        else if (gCounter == 2)
        {
            meanADC1 /= gCounter;

            ADC_Was_Measured = DONE;
        }

        gCounter++;
   }
}


//**************************************************************************************************
// Procedure DMA1_Stream6_IRQHandler()     
//**************************************************************************************************

//* IRQ Handler DMA1 *//
void DMA1_Stream6_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6) == SET)
    {
        DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);

        USART_TX_ReadyToSend = DONE;
   }
}