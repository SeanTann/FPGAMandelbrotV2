/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"

#include "xparameters.h"
#include "sleep.h"
#include "xgpio.h"

#include "xil_io.h"
#include "float.h"

#include "mathFunctions.h"

//#include "math.h"

void mainLoop(void);
void sendPixel(int, int, int, long long [2], int *, XGpio, int[8]);
int increment(int, char);
int shift(int, char);




int main()
{
	//int error = testFunc();

    init_platform();



    mainLoop();

    cleanup_platform();
    return 0;
}

int increment(int level, char c)
{
	const int LUTx[11] = {1, 2, 2, 4, 4, 8, 8, 16, 16, 32, 32};
	const int LUTy[11] = {2, 2, 4, 4, 8, 8, 16, 16, 32, 32, 32};

	int l = 0;

	for(int i =0;i<11;i++)
	{
		if(level <= 1)
			i = 11;
		level = level>>1;
		l++;
	}
	l--;

	if(c == 'y')
		return LUTy[l];
	else
		return LUTx[l];
}

int shift(int level, char c)
{
	const int LUTx[11] = {0, 1, 0, 2, 0, 4, 0, 8, 0, 16, 0};
	const int LUTy[11] = {1, 0, 2, 0, 4, 0, 8, 0, 16, 0, 0};

	int l = 0;

	for(int i =0;i<11;i++)
	{
		if(level <= 1)
			i = 11;
		level = level>>1;
		l++;
	}
	l--;

	if(c == 'y')
		return LUTy[l];
	else
		return LUTx[l];
}



void mainLoop(){

	XGpio gpio;

	XGpio_Initialize(&gpio, XPAR_GPIO_0_DEVICE_ID);
	XGpio_SetDataDirection(&gpio, 1, 0x0);
	XGpio_SetDataDirection(&gpio, 2, 0xFFFFFFFF);

    XGpio_DiscreteWrite(&gpio, 1, 1);

	uint32_t cData[8] = {0,0,0,0,0,0,0,0};
	uint32_t transmitData[64];


	int watchdog = 0;



	int debug = 0;


	unsigned long long cr[2], ci[2], co[2], *temp, in[2], cursorAdd[2], cursorTemp[2], shiftedXY[2];

	signed int zoom = 0, zoomOutOffset;
	int read = 0, prevRead = 0;
	long long xCursor, yCursor, coordSigned;

	int zoomIn, zoomOut, changedIter;

	uint32_t resLevel = 0x00000800;

	ci[0] = 0;
	ci[1] = 0;

	cr[0] = 0;
	cr[1] = 0;

	cursorTemp[0] = 0;
	cursorTemp[1] = 0;

	unsigned long long x = 0, y = 0;
	unsigned int xInc = 0, yInc = 0;
	int pixCount = 0;
	int finished = 0;
	while(1){
		
		XGpio_DiscreteWrite(&gpio, 1, resLevel);

		if(resLevel != 1)
			resLevel = resLevel>>1;
		else
			finished = 1;
		
		if(debug){
			resLevel = 1;
			zoom = 6;
		}

	
		for(yInc = 0; yInc < 1080; yInc = yInc + increment(resLevel, 'y')){
			y = (long long)(yInc + shift(resLevel, 'y'));

			
			changedIter = ((read & 0x08000000) && !(prevRead & 0x08000000));
			zoomIn = ((read & 0x20000000) && !(prevRead & 0x20000000));
			zoomOut =  ((read & 0x80000000) && !(prevRead & 0x80000000));
			if(zoomIn)
				zoom++;
			else if (zoomOut)
				zoom--;

			xCursor = ((long long)(read & 0x000007FF) & 0x7FF) - 960;
			yCursor = ((long long)((read>>16) & 0x000007FF)& 0x7FF) - 540;

			if(changedIter){
				resLevel = 0x00000400;
				finished = 0;
				yInc = 0;
				xInc = 0;
			}


			if(zoomIn || zoomOut){
				resLevel = 0x00000400;
				finished = 0;
				yInc = 0;
				xInc = 0;

				
				zoomOutOffset = 0;
				if(zoomOut){
					xCursor = -xCursor;
					yCursor = -yCursor;
					zoomOutOffset = 1;
			    }

				if(yCursor < 0)
					cursorTemp[1] = 0xFFFFFFFFFFFFFFFF;
				else
					cursorTemp[1] = 0x0000000000000000;

				cursorTemp[0] = yCursor;
				temp = shiftLeft128(&cursorTemp[0], 69 - zoom -zoomOutOffset , &cursorAdd[0]);

				temp = add128(&ci[0], &cursorAdd[0], &co[0]);
				ci[0] = co[0];
				ci[1] = co[1];

				if(xCursor < 0)
					cursorTemp[1] = 0xFFFFFFFFFFFFFFFF;
				else
					cursorTemp[1] = 0x0000000000000000;

				cursorTemp[0] = xCursor;
				temp = shiftLeft128(&cursorTemp[0], 69 - zoom -zoomOutOffset, &cursorAdd[0]);

				temp = add128(&cr[0], &cursorAdd[0], &co[0]);
				cr[0] = co[0];
				cr[1] = co[1];

			}
			coordSigned = y - 540;
			in[0] = coordSigned;
			if(coordSigned >= 0)
				in[1] = 0;
			else
				in[1] = 0xFFFFFFFFFFFFFFFF;


			shiftLeft128(&in[0], -zoom +69, &shiftedXY[0]);
			add128(&shiftedXY[0], &ci[0], &co[0]);

			cData[3] = (uint32_t)(co[0] & 0x00000000FFFFFFFF);
			cData[4] = (uint32_t)((co[0]>>32) & 0x00000000FFFFFFFF);
			cData[5] = (uint32_t)(co[1] & 0x000000000007FFFF);





			prevRead = read;

			for(xInc = 0; xInc < 1920; xInc= xInc + increment(resLevel, 'x'))
			{
				x = (long long)(xInc+ shift(resLevel, 'x'));

				coordSigned = x - 960;

				in[0] = coordSigned;

				if(coordSigned >= 0)
					in[1] = 0;
				else
					in[1] = 0xFFFFFFFFFFFFFFFF;

				temp = shiftLeft128(&in[0],  -zoom + 69, &shiftedXY[0]);
				
				temp = add128(&shiftedXY[0], &cr[0], &co[0]);


				cData[0] = (uint32_t)(co[0] & 0x00000000FFFFFFFF);
				cData[1] = (uint32_t)((co[0]>>32) & 0x00000000FFFFFFFF);
				cData[2] = (uint32_t)(co[1] & 0x000000000007FFFF);

				cData[7] = (x & 0x00000FFF) | ((y<<12) & 0x00FFF000);

	 
				pixCount++;
				if(pixCount%32 == 0)
					read = XGpio_DiscreteRead(&gpio, 2);

				watchdog = 0;
				while((read & 0x10000000) && watchdog < 100000) {
					watchdog++;
					read = XGpio_DiscreteRead(&gpio, 2);
				}

				if(!finished && (pixCount%8) == 0)
					memcpy(XPAR_ZYNQ_PS_WRAP_ZYNQ_PS_I_M01_AXI_BASEADDR, &transmitData[0], 8*4*8);


				if(!finished){
					for(int i = 0; i < 8; i++){
						transmitData[i+ 8*(pixCount%8)] = cData[7-i];
					}


				}
				//*/

			}
		}



	}

	return;
}
