#include "main.h"

#define DONE   1
#define SEND   DONE
#define IDLE   !DONE

#define F_CLK  84000000 
// #define F_CLK  83849000
// #define F_CLK  84003419 


void TIM3_IRQHandler(void);
void DMA1_Stream3_IRQHandler(void);
// void Usart1_Send_symbol(uint8_t data);
// void Usart1_Send_String(char *const str);
void Reverse(char* str, int len);
void IntToStr(float freq, float dutycycle, char *const str);

volatile uint8_t USART_TX_ReadyToSend; 
uint32_t IC1Value_int;
float IC1Value_float;
float DutyCycle;
float Frequency;
uint32_t tmp;
float tmp1;

char res[15] = {'\t','\t','\t','\t','\t','\t','\t','\t','\t','\t','\t','\t','\t','\t','\t'}; //Global array

int main(void)
{
    USART_TX_ReadyToSend = DONE; 
    Frequency = 0.0;
    DutyCycle = 0.0;
    IC1Value_int = 0;
    IC1Value_float = 0.0;
    tmp = 0;
    tmp1 = 0.0;


    SetSysClock_HSE_84();
    Configure_USART3(); 
    Configure_DMA1_USART3();
    Configure_TIM3();

    while(1)
    {
        if (!USART_TX_ReadyToSend)
        {
            IC1Value_float = (float) TIM_GetCapture1(TIM3);
            IC1Value_int = (int) IC1Value_float;
            tmp1 = (float) TIM_GetCapture1(TIM3);
            
            if (IC1Value_int != 0)
            {
                tmp = TIM_GetCapture2(TIM3);

                DutyCycle = (float) ((TIM_GetCapture2(TIM3)) * 100) / (IC1Value_int);
            
                Frequency = (float) (F_CLK) / (IC1Value_int);
                
            }
            else
            {
                DutyCycle = 0;
                Frequency = 0;
            }

            IntToStr(Frequency, DutyCycle, res);
            DMA_Cmd(DMA1_Stream3, ENABLE);
            //Usart1_Send_String(res);
            while(!USART_TX_ReadyToSend) {};

            TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);
        }

        for (uint32_t i = 0 ; i < 0xFFFFF; i++);
    }

    return 0;
}


//**************************************************************************************************
// Procedure TIM_IRQHandler()
//**************************************************************************************************
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_CC1) == SET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);

        TIM_ITConfig(TIM3, TIM_IT_CC1, DISABLE);

        USART_TX_ReadyToSend = IDLE;

    }
}

void DMA1_Stream3_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3) == SET)
    {
        DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);

        USART_TX_ReadyToSend = DONE;
    }
}

// void Usart1_Send_symbol(uint8_t data) 
// {
//     while(!(USART3->SR & USART_SR_TC)); //Status register transmit complate
//     USART3->DR = data; //Data register
// }


// void Usart1_Send_String(char *const str) 
// {
//     uint8_t i = 0;

//     while(str[i]) 
//     {
//         Usart1_Send_symbol(str[i]);
//         i++;
//     }

//     Usart1_Send_symbol('\n');
//     Usart1_Send_symbol('\r');

//     USART_TX_ReadyToSend = IDLE; 
// }


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
    
    // if (1/locFreq * ;

    
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