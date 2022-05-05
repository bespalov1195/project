#include "main.h"
#include <math.h>

#define DONE   1
#define SEND   DONE
#define IDLE   !DONE
#define F_CLK  42000000 


void TIM2_IRQHandler(void);
void Usart1_Send_symbol(uint8_t data);
void Usart1_Send_String(char *const str);
void Reverse(char* str, int len);
void IntToStr(float x, char *const str);

volatile uint8_t USART_TX_ReadyToSend; 
volatile uint8_t State;
volatile uint8_t StateUsart;
uint32_t T1, T2;
uint32_t diffCaputre;
float Frequency;
char res[10];


int main(void)
{
    USART_TX_ReadyToSend = DONE; 
    State = IDLE;
    StateUsart = IDLE;
    T1 = 0;
    T2 = 0;
    diffCaputre = 0;
    Frequency = 0.0;

    for (int i = 0; i < 10; i++)
    {
        res[i] = 0;
    }

    SetSysClock_HSE_84();
    Configure_TIM2();
    Configure_USART3(); 

    while(1)
    {
        if (!USART_TX_ReadyToSend)
        {
            TIM_ITConfig(TIM2, TIM_IT_CC3, DISABLE);

            USART_TX_ReadyToSend = DONE; 

            IntToStr(Frequency, res);
            Usart1_Send_String(res);
            while(!USART_TX_ReadyToSend) {};

            USART_TX_ReadyToSend = DONE; 
            
            TIM_ITConfig(TIM2, TIM_IT_CC3, ENABLE);
        }

        for (uint32_t i = 0 ; i < 0xF000; i++);
    }

    return 0;
}


//**************************************************************************************************
// Procedure TIM_IRQHandler()
//**************************************************************************************************
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_CC3) == SET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC3);

        if (State == IDLE)
        {
            T1 = TIM_GetCapture3(TIM2);
            State = DONE;
        }
        else if (State == DONE)
        {
            T2 = TIM_GetCapture3(TIM2);

            if (T2 > T1)
            {
                diffCaputre = T2 - T1;
                Frequency = F_CLK / diffCaputre;
                T1 = T2;
                USART_TX_ReadyToSend = IDLE;
            }
            else if (T2 < T1)
            {
                diffCaputre = ((0xFFFF - T1) + T2) + 1;
                Frequency = F_CLK / diffCaputre;
                T1 = T2;
                USART_TX_ReadyToSend = IDLE;
            }
            else 
            {
                // Error;
            }
        }
    }
}


void Usart1_Send_symbol(uint8_t data) 
{
    while(!(USART3->SR & USART_SR_TC)); //Status register transmit complate
    USART3->DR = data; //Data register
}


void Usart1_Send_String(char *const str) 
{
    uint8_t i = 0;

    while(str[i]) 
    {
        Usart1_Send_symbol(str[i]);
        i++;
    }
    
    Usart1_Send_symbol('\n');
    Usart1_Send_symbol('\r');
    USART_TX_ReadyToSend = DONE; 
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


void IntToStr(float x, char *const str)
{
    int iPart = (int) x;

    if (x - 0.49 > (float) iPart)
        iPart ++;

    int i = 0;

    while (iPart)
    {
        str[i++] = (iPart % 10) + '0';
        iPart = iPart / 10;
    }

    Reverse(str, i);

    str[i] = '\0';
}