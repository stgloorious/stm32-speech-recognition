/**
 * @file main.c
 * @brief Demo application
 */

/*
 * Copyright (C) 2024 Stefan Gloor
 *
 * SPDX-License-Identifier: MIT
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 */

#include "stm32l4xx_hal.h"
#include "stm32l475e_iot01.h"

#include "clock.h"
#include "debug_io.h"
#include "dfsdm.h"
#include "dma.h"
#include "error.h"

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_hid.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

volatile int flag = 0;
#define BUF_LEN 1600 // 100 ms @ 16 kS/s
int32_t *buf;

USBD_HandleTypeDef USBD_Device;
extern PCD_HandleTypeDef hpcd;
__IO uint32_t joyready = 0;

int main(void)
{
	/* Disable buffering */
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	/* Initialize the hardware abstraction library */
	if (HAL_Init() != HAL_OK) {
		/* As configuration is not completed yet,
		* this message might not make it out.
		* Try anyway. */
		ERR("HAL_Init() failed.\n");
	}

	/* Configure and start the necessary clocks and PLLs */
	SystemClock_Config();

	/* Initialize peripherals */
	uart_debug_init();
	dma_init();
	dfsdm_init();
	BSP_LED_Init(LED2);
	BSP_LED_Off(LED2);

	/* Enable Power Clock*/
	__HAL_RCC_PWR_CLK_ENABLE();

	/* Enable USB power on Pwrctrl CR2 register */
	HAL_PWREx_EnableVddUSB();

	/* Configure Tamper button for remote wakeup */
	BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

	/* Init Device Library */
	if (USBD_Init(&USBD_Device, &HID_Desc, 0) != USBD_OK) {
		ERR("Failed to initialize usbd driver.\n");
	}

	/* Add Supported Class */
	if (USBD_RegisterClass(&USBD_Device, USBD_HID_CLASS) != USBD_OK) {
		ERR("Failed to register HID class with usbd driver.\n");
	}

	/* Start Device Process */
	if (USBD_Start(&USBD_Device) != USBD_OK) {
		ERR("Failed to start usbd.\n");
	}

	buf = malloc(BUF_LEN * sizeof(int32_t));
	if (buf == NULL) {
		ERR("malloc() failed.\n");
	}

	printf("Hello World!\n\n\n");

	/* Start reading from microphone */
	if (HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter0, buf, BUF_LEN) !=
	    HAL_OK) {
		ERR("Failed to start conversion.");
	}
	long count;
	long oldcount;
	while (1) {
		while (!flag)
			;
		flag = 0;
		BSP_LED_Toggle(LED2);
		//uart_write_bulk((char*)buf, BUF_LEN);
		printf("%lu\n", count - oldcount);
		count = HAL_GetTick();
		if (count - oldcount != 100) {
			printf("\n\n\n\n\n\n");
			printf("Data streaming too slow: %lu/%u\n",
			       count - oldcount, 100);
			while (1)
				;
		}
		oldcount = count;
	}
}

void HAL_DFSDM_FilterRegConvCpltCallback(
	DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
	flag = 1;
}
