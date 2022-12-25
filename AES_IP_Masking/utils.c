/*
	utils.c
*/

#include <stm32f4xx.h>
#include <misc.h>			
#include <stm32f4xx_usart.h> 
#include <stm32f4xx_rcc.h>

#include "config.h"
#include "common.h"
#include "term_io.h"

uint8 RND_FLAG;

void initRandSrc(uint8 selection)
{
	// RNG Peripheral enable
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);
	
	if (selection==0)
		RND_FLAG = 0x00;
	else
		RND_FLAG = 0x01;
}

void genRandByte(uint8 * res)
{
	uint8 a;

	//Wait until one RNG number is ready
	while(RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET);

	//Get a 32bit Random number
	//return RNG_GetRandomNumber();
	a = (uint8)RNG->DR;
	*res = a * ((RND_FLAG+0xFF)>>8);
	
	
	/**** while(RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET);
	 8000276:	2001      	movs	r0, #1
 	 8000278:	f00a f8c6 	bl	800a408 <RNG_GetFlagStatus>
 	 800027c:	4603      	mov	r3, r0
 	 800027e:	2b00      	cmp	r3, #0
 	 8000280:	d0f9      	beq.n	8000276 <TM_RNG_Get+0x6>
	 *
	 *
	 **** return RNG->DR;
	 * ldr	r3, [pc, #8]	; (800028c <TM_RNG_Get+0x1c>)
 	 8000284:	689b      	ldr	r3, [r3, #8]
 	 800028c:	50060800 	.word	0x50060800

	 //equivalent:
 	 ldr r0,=0x50060800
 	 ldr r0, [r0, #8]
	 */

	//#define PERIPH_BASE           ((uint32_t)0x40000000) /*!< Peripheral base address in the alias region                                */
	//#define AHB2PERIPH_BASE       (PERIPH_BASE + 0x10000000)
	//#define RNG_BASE              (AHB2PERIPH_BASE + 0x60800)
	//RNG_BASE = 0x50060800

}

void fetchCoin(uint8 * res)
{
	//Wait until one RNG number is ready
	while(RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET);

	//Get a 32bit Random number
	//return RNG_GetRandomNumber();
	*res = (uint8)RNG->DR & 0x01;
}

void printMaskedByte(mask * mval)
{
	int i;
	
	xprintf("\n{ [");
	for(i=0; i<n-1; i++)
		xprintf("%02x, ", mval->L[i]);
	xprintf("%02x]", mval->L[i]);
	xprintf(", [");
	for(i=0; i<n-1; i++)
		xprintf("%02x, ", mval->R[i]);
	xprintf("%02x] }", mval->R[i]);
}

void PC6_Config(void)
{
        GPIO_InitTypeDef GPIO_InitStructure;

        /// GPIOE clocks enable 
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

        // GPIOE Configuration 
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

        //GPIO_ResetBits(GPIOC,GPIO_Pin_6);
}

void PC6_trigger()
{
        GPIO_ResetBits(GPIOC,GPIO_Pin_6);
        GPIO_SetBits(GPIOC,GPIO_Pin_6);
        GPIO_ResetBits(GPIOC,GPIO_Pin_6);
}

