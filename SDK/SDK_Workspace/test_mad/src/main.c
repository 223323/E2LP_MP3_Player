/*
 * Copyright (c) 2009-2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "platform.h"
#include "xparameters.h"
#include "xil_types.h"
#include "xintc.h"

#include "pff.h"
#include "mad.h"
#include "mp3_parsing.h"
#include "LockFreeFifo.h"

#include "delay.h"

#include "dds.h"

#include <stdarg.h>
#include "vga_periph_mem.h"

char tmp_str[100];
char tmp_str2[80];
Xint32 cursor = 0;

char toupper2(char a) {
	if(a >= 'a' && a <= 'z')
		a += 'A'-'a';
	return a;
}

#define LINE_WIDTH 80*4
#define CURSOR_MAX 100000

void set_cursor_line(int line) {
	cursor = line*LINE_WIDTH;
	set_cursor(cursor);
}

int divv(int n, int a) {
	int b = 0;
	while(n > a) {
		n-=a;
		b++;
	}
	return b;
}

void vga_printf(char *fmt, ...)
{
    va_list ap;
    int d;
    char c, *s;

    va_start(ap, fmt);
    int size;
    /*

    set_cursor(cursor);
    strcpy(tmp_str, fmt);
    vsprintf(tmp_str, fmt, ap);
    size = strlen(tmp_str);
    print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, (unsigned char*)tmp_str, size);
    cursor+=4*size;
    return;
    */
   set_cursor(cursor);
   strcpy(tmp_str, fmt);
   int i;
   int len = strlen(tmp_str);
   for(i=0; i < len; i++)
	   tmp_str[i] = toupper2(tmp_str[i]);

   //print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, (unsigned char*)tmp_str, len);

   int a = 0;
   int b = 0;

    while (fmt[b]) {
    	if(fmt[b] == '%') {

    		if(b-a > 0) {
    			size = b-a-1;
				print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, (unsigned char*)tmp_str+a, size-1);
				a = b+2;
				cursor+=size*4;
				set_cursor(cursor);
			}

			switch (fmt[++b]) {
				case 's':
					s = va_arg(ap, char *);

					strcpy(tmp_str2, s);
					size = strlen(tmp_str2);
					for(i=0; i < size; i++) {
						tmp_str2[i] = toupper2(tmp_str2[i]);
					}

					print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, (unsigned char*)tmp_str2, size);
					cursor+=size*4;
					set_cursor(cursor);
					break;
				case 'd':
					d = va_arg(ap, int);
					//sprintf(tmp_str2, "%d", d);
					size = strlen(tmp_str2);
					print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, (unsigned char*)tmp_str2, size);
					cursor+=size*4;
					set_cursor(cursor);
					break;
				case 'c':
					c = (char) va_arg(ap, int);
					break;
			}
    	} else if(fmt[b] == '\n' || fmt[b] == '\r') {

    		if(b-a > 0) {
				size = b-a;
				print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, (unsigned char*)tmp_str+a, size-1);
				a = b+1;
				cursor+=size*4;
				set_cursor(cursor);
			}

    		if(fmt[b] == '\n') {
    			cursor = divv(cursor, LINE_WIDTH) * LINE_WIDTH + LINE_WIDTH;
				//cursor = (cursor / LINE_WIDTH)*LINE_WIDTH + LINE_WIDTH;
				set_cursor(cursor);
    		}

    	}
    	b++;
    }
    va_end(ap);

    if(b-a > 0) {
		size = b-a;
		print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, (unsigned char*)tmp_str+a, size-1);
		cursor+=size*4;
		set_cursor(cursor);
	}

    if(cursor > CURSOR_MAX) cursor = 0;
}

void vga_init() {
	VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x00, 0x0);// direct mode   0
	VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x04, 0x3);// display_mode  1
	VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x08, 0x1);// show frame      2
	VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x0B, 0x1);// font size       3
	VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x10, 0xFF00FF);// foreground 4
	VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x14, 0x0000FF);// background color 5
	VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x18, 0x00ff00);// frame color      6

	clear_text_screen(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR);
	set_cursor(cursor);
}

int main(void) {

	init_platform();

	vga_init();

	XStatus status;


	// SD card stuff.

	FATFS fatfs; /* File system object */
	FRESULT rc;

	xil_printf("Mounting a volume...\r\n");
	rc = pf_mount(&fatfs);
	if (rc) {
		xil_printf("Failed mounting volume with rc = %d!\r\n", (int) rc);
		return 1;
	}

	DIR dir;
	FILINFO fno;
	pf_opendir(&dir, "");


	char buff[128];
	WORD read;
	int read_pos;

	set_cursor_line(10);

	xil_printf("Directory listing:\r\n");
	int size = 0;
	for (;;) {

		rc = pf_readdir(&dir, &fno);	// Read a directory item
		if(strstr(fno.fname, ".MP3") > 0) {
			xil_printf("--------------------------\r\n");
			xil_printf("file: %s\r\n", fno.fname);
			vga_printf("    file: %s\n", fno.fname);

			rc = pf_open(fno.fname);
			if (rc) {
				xil_printf("Failed opening mp3 file with rc = %d!\r\n", (int) rc);
				return 1;
			}
			size = fno.fsize;
			read_pos = size-128;
			xil_printf("file size: %d\r\n", size);
			xil_printf("reading from: %d\r\n", read_pos);
			rc = pf_lseek(read_pos);
			if(rc) {
				xil_printf("Failed seeking mp3 file with rc = %d!\r\n", (int) rc);
				return 1;
			}
			rc = pf_read(buff, 128, &read);
			if(rc) {
				xil_printf("Failed reading mp3 file with rc = %d!\r\n", (int) rc);
				return 1;
			}

			xil_printf("data read: %d\r\n", read);
			if(read == 128) {
				MP3ID3TAG1 header = *(MP3ID3TAG1*)buff;//parse_mp3_header2(buff);

				vga_printf("     name            %s\n", header.name);
				vga_printf("     artist          %s\n", header.artist);
				vga_printf("     album           %s\n", header.album);
				vga_printf("     comment         %s\n", header.comment);
			}
			vga_printf("\n\n");



		}
		if (rc || !fno.fname[0])
			break;	// Error or end of dir
		if (fno.fattrib & AM_DIR) {
			xil_printf("   <dir>  %s\r\n", fno.fname);
		}
		else {
			xil_printf("%8d  %s\r\n", fno.fsize, fno.fname);
		}


	}

	xil_printf("Opening a mp3 file...\r\n");


	// MP3 stuff.

	xil_printf("Decoding...\r\n");





	xil_printf("Exiting...\r\n");

	return 0;
}

/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/

DWORD get_fattime(void) {
	return ((DWORD) (2010 - 1980) << 25) /* Fixed to Jan. 1, 2010 */
	| ((DWORD) 1 << 21) | ((DWORD) 1 << 16) | ((DWORD) 0 << 11)
			| ((DWORD) 0 << 5) | ((DWORD) 0 >> 1);
}

