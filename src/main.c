/*
 * main.c
 *
 * Copyright (C) 2023 Stefan Gloor
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
#include "stm32l4xx_nucleo_32.h"

#include "clock.h"
#include "debug_io.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	HAL_Init();

	SystemClock_Config();

	BSP_LED_Init(LED3);

	uart_debug_init();

	printf("\n\n");
	printf("STM32 NUCLEO-K432KC Demo Application Version 1.0\n");
	printf("Build date: %s %s\n", __DATE__, __TIME__);
	printf("GCC %i.%i.%i ", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
	printf("Newlib %s\n\n", _NEWLIB_VERSION);

	while (1) {
		BSP_LED_Toggle(LED3);
		printf("Hello World!\n");

		HAL_Delay(1000);
	}
}
