#include <stm32f4xx.h>
#include <misc.h>			
#include <stm32f4xx_usart.h> 
#include <stm32f4xx_rcc.h>

#include "global.h"
#include "term_io.h"
#include "stdlib.h"

#include "config.h"
#include "common.h"
#include "gf_256.h"
#include "masking.h"
#include "utils.h"
#include "aes.h"


// Global variables
// ---------------------
uint8 key_aes[16] = {
	0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 
	0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};

uint8 pt_aes1[16] = {
	0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 
	0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
};

uint8 rcon[10] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 
	0x20, 0x40, 0x80, 0x1b, 0x36
};

uint8 pt_aes2[16] = {0};
uint8 coin_flips[65536] = {0};

/*
	void printDuration(uint32_t startTime, uint32_t stopTime, int repeatCount, uint64_t offset_cycles)
	{
	  uint32_t total_cycles = (stopTime - startTime) - offset_cycles;
	  uint32_t average_cycles = total_cycles/repeatCount;
	  xprintf("%u cycles\n", average_cycles);
	}
*/

int main (void)
{
	// Allocate variables
	int i, j, k;
	uint16_t num_iter;
	
	uint8 cmd, coin, tmp1, tmp2;
	
	maskState mState, mState1, mState2;
	maskState keyState;
	
	// Communication
	comm_init();
	
	// Trigger
	PC6_Config();
	
	// Set up IP Masking parameters
	uint8 L[n]; uint8 L2[n*n]; 
	L[0] = 0x01;	L[1] = 0x5b;
	if(n==3) 		L[2] = 0xef;
	//for(i=1; i<n; i++)
		//genRandByteNonZero(&(L[i]));	

	for(i=0; i<n; i++)
	{
		for(j=0; j<n; j++)
			gf256Multiply(&(L2[i*n+j]), L[i], L[j]);
	}

	for(i=0;i<16;i++)
	{
		initMasks(&mState.st[i], &L[0], &L2[0]);
		initMasks(&mState1.st[i], &L[0], &L2[0]);
		initMasks(&mState2.st[i], &L[0], &L2[0]);
		initMasks(&keyState.st[i], &L[0], &L2[0]);
	}

	// Speed configuration
	/*
		uint32_t startTime, stopTime;
		CoreDebug->DEMCR = CoreDebug->DEMCR | 0x01000000;
		DWT->CYCCNT=0;
		DWT->CTRL=DWT->CTRL | 1;
		startTime = DWT->CYCCNT; 
		stopTime = DWT->CYCCNT; 
		uint64_t offset_cycles = (uint32_t)(stopTime-startTime);
		xprintf("offset_cycles:%u\n",(uint32_t)offset_cycles);
	*/
	
	// AES Encryption (locked loop)
	// ---------------------------
	while(1)
	{
		// Receive and parse parameters
		cmd = comm_get();
		num_iter = comm_get();
		num_iter = num_iter << 8;
		num_iter += comm_get();
		
		switch(cmd)
		{
			case 0x10: case 0x11:
				for(i=0; i<16; i++)
					key_aes[i] = comm_get();
				for(i=0; i<16; i++)
					pt_aes1[i] = comm_get();
				break;
			case 0x20: case 0x21: case 0x30: case 0x31:
				for(i=0; i<16; i++)
					key_aes[i] = comm_get();
				for(i=0; i<16; i++)
					pt_aes1[i] = comm_get();
				for(i=0; i<16; i++)
					pt_aes2[i] = comm_get();
				break;			
			default: 
				break;
		}
		
		// Random source (masks on/off)
		initRandSrc(cmd&0x01);
			
		// MAIN ITERATION LOOP
		// ---------------------------	
		for(k=0; k<num_iter; k++)
		{		
			// Encode plaintext & key
			// ---------------------------
			for(i=0; i<16; i++)
				encodeByte(&mState1.st[i], pt_aes1[i]);
			for(i=0; i<16; i++)
				encodeByte(&mState2.st[i], pt_aes2[i]);
			for(i=0; i<16; i++)
				encodeByte(&keyState.st[i], key_aes[i]);
		
			// Prepare input
			// ---------------------------
			if(cmd==0x10 || cmd==0x11)
			{
				for(i=0; i<16; i++)
					copyMasks(&mState.st[i], &mState1.st[i]);
			}
			else
			{
				fetchCoin(&coin);
				if(coin==0)
				{
					for(i=0; i<16; i++)
						copyMasks(&mState.st[i], &mState1.st[i]);
				}
				else
				{
					for(i=0; i<16; i++)
						copyMasks(&mState.st[i], &mState2.st[i]);
				}
				coin_flips[k] = coin;	
			}
			
			// Trigger
			// ---------------------------
			PC6_trigger();
			
			// debug josep
			/*
				xprintf("\nIN  %d: ",k);
				for(i=0; i<16; i++)
				{
					decodeByte(&tmp1,&mState.st[i]);
					xprintf("0x%02x,", tmp1);
				}
				printMaskedByte(&mState.st[0]);
			*/
			// end debug josep
			
			/*
				DWT->CYCCNT=0;
				startTime = DWT->CYCCNT; // Get the start time
			*/
			
						
			// KeyAddition 
			// ---------------------------
			for(i=0; i<16; i++)
				maskAddition(&mState.st[i], &mState.st[i], &keyState.st[i]);
			
			// Round iterations
			// ---------------------------
			for(j=0; j<9; j++)
			{
				// SubBytes
				// ---------------------------
				for(i=0; i<16; i++)
					sboxAES(&mState.st[i]);
				
				// ShiftRows
				// ---------------------------
				shiftRowsAES(&mState); 
			
				// MixColumns
				// ---------------------------
				for(i=0; i<4; i++)
					mixColumnsAES(&mState.st[4*i], &mState.st[4*i+1], &mState.st[4*i+2], &mState.st[4*i+3]);
				
				// NextRoundKey
				// ---------------------------
				nextRoundKeyAES(&keyState, rcon[j]);
				
				// KeyAddition
				// ---------------------------
				for(i=0; i<16; i++)
					maskAddition(&mState.st[i], &mState.st[i], &keyState.st[i]);
			}
		
			// SubBytes
			// ---------------------------
			for(i=0; i<16; i++)
				sboxAES(&mState.st[i]);
			
			// ShiftRows
			// ---------------------------
			shiftRowsAES(&mState);

			// NextRoundKey
			// ---------------------------
			nextRoundKeyAES(&keyState, rcon[j]);
		
		
			// KeyAddition 
			// ---------------------------
			for(i=0; i<16; i++)
				maskAddition(&mState.st[i], &mState.st[i], &keyState.st[i]);
			
			/*
				stopTime = DWT->CYCCNT; 
			*/
			
			// debug josep
			/*
				xprintf("\nOUT %d: ",k);
				for(i=0; i<16; i++)
				{
					decodeByte(&tmp1,&mState.st[i]);
					xprintf("0x%02x,", tmp1);
				}
				printMaskedByte(&mState.st[0]);
				xprintf(" Encryption time: ");
				printDuration(startTime, stopTime,1,offset_cycles);
			*/
			// end debug josep
			
			// Parse output
			// ---------------------------
			if(cmd==0x10 || cmd==0x11)
			{
				for(i=0; i<16; i++)
					decodeByte(&pt_aes1[i],&mState.st[i]);
			}
			if(cmd==0x20 || cmd==0x21)
			{
				if(coin==1)
				{
					for(i=0; i<16; i++)
						decodeByte(&pt_aes2[i],&mState.st[i]);
				}
			}
			
			for(i=0; i<10000; i++);
		}
		// ---------------------------
		// END MAIN ITERATION LOOP
		
		
		// Return values
		// ---------------------------
		if(cmd==0x10 || cmd==0x11)
		{
			for(i=0; i<16; i++)
			{
				decodeByte(&tmp1,&mState.st[i]);
				comm_put(tmp1);
			}
		}
		else
		{
			for(i=0; i<num_iter; i++)
				//xprintf("%d,", coin_flips[i]);
				comm_put(coin_flips[i]);
		}
		

	}
	
	return 0;
}

