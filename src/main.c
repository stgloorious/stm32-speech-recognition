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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

UART_HandleTypeDef huart1;

volatile int flag = 0;
#define BUF_LEN 2048
int32_t *buf;

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

	buf = malloc(BUF_LEN * sizeof(int32_t));
	if (buf == NULL) {
		ERR("malloc() failed.\n");
	}

	printf("Hello World!\n");

	/* Start reading from microphone */
	if (HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter0, buf, BUF_LEN) !=
	    HAL_OK) {
		ERR("Failed to start conversion.");
	}
	while (1) {
		while (!flag)
			;
		flag = 0;
		BSP_LED_Toggle(LED2);
		/*for (uint32_t i = 0; i < BUF_LEN; i++) {
			printf("%li\n", buf[i]);
		}*/
	}
}

void HAL_DFSDM_FilterRegConvCpltCallback(
	DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
	flag = 1;
}
