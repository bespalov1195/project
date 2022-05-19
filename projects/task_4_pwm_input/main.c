#include "main.h"

#define DONE   1
#define IDLE   !DONE
#define F_CLK  142000000

void DMA1_Stream3_IRQHandler(void);
void Reverse(char* str, int len);
void IntToStr(float freq, float dutycycle, char *const str);

volatile uint8_t USART_TX_ReadyToSend; 
double DutyCycle;
double Frequency;
char res[15] = {'\t','\t','\t','\t','\t','\t','\t','\t','\t','\t','\t','\t','\t','\t','\t'};
int IC1Value = 0;
int IC2Value = 0;


int main(void)
{
    USART_TX_ReadyToSend = IDLE; 
    Frequency = 0.0;
    DutyCycle = 0.0;

    SetSysClock_HSE_84();
    Configure_USART3(); 
    Configure_TIM5();
    Configure_DMA1();

    TIM_Cmd(TIM5, ENABLE);

    while(1)
    {
        for (uint32_t i = 0 ; i < 0x4FFFFF; i++){} 

        if (IC1Value != 0)
        {
            DutyCycle = (float) ((IC2Value)* 100) / (IC1Value);
            Frequency = 142000000.0 / (IC1Value+1);
        }
        else
        {
            DutyCycle = 0;
            Frequency = 0;
        }

        IntToStr(Frequency, DutyCycle, res);
        DMA_Cmd(DMA1_Stream3, ENABLE);
        while(!USART_TX_ReadyToSend) {};
        USART_TX_ReadyToSend = IDLE;
    }

    return 0;
}


//**************************************************************************************************
// Procedure DMA1_Stream3_IRQHandler()
//**************************************************************************************************
void DMA1_Stream3_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3) == SET)
    {
        DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);

        USART_TX_ReadyToSend = DONE;
    }
}


void Reverse(char *const str, int len)
{
    int i = 0; 
    int j = len - 1;
    int temp;

    while (i < j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}


void IntToStr(float freq, float dutycycle,  char *const str)
{
    int locFreq = (int) freq;
    int locDutyCycle = (int) dutycycle;

    if (freq - 0.49 > (float) locFreq)
        locFreq ++;
    
    if (dutycycle - 0.49 > (float) locDutyCycle)
        locDutyCycle ++;

    int i = 0;

    str[i++] = '%';
    
    while (locDutyCycle)
    {
        str[i++] = (locDutyCycle % 10) + '0';
        locDutyCycle = locDutyCycle / 10;
    }

    str[i++] = '\t';

    str[i++] = 'z';
    str[i++] = 'H';
   
    while (locFreq)
    {
        str[i++] = (locFreq % 10) + '0';
        locFreq = locFreq / 10;
    }

    Reverse(str, i);

    while(i < 15)
    {
        str[i] = ' ';
        i++;
    }

    str[13] = '\n';
    str[14] = '\r'; 
}