/**
 * @file interrupts.c
 * @brief Exception handlers
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

#include "interrupt.h"
#include "stm32l4xx_hal.h"

#include <stdio.h>

void NMI_Handler(void)
{
}

void MemManage_Handler(void)
{
#ifdef DEBUG
	printf("==========================================\n");
	printf("Unexpected Interrupt: %s\n", " MemManage");
	printf("==========================================\n");
#endif
	while (1)
		;
}

void BusFault_Handler(void)
{
#ifdef DEBUG
	printf("==========================================\n");
	printf("Unexpected Interrupt: %s\n", "  BusFault");
	printf("==========================================\n");
#endif
	while (1)
		;
}

void UsageFault_Handler(void)
{
#ifdef DEBUG
	printf("==========================================\n");
	printf("Unexpected Interrupt: %s\n", "UsageFault");
	printf("==========================================\n");
#endif
	while (1)
		;
}

void HardFault_Handler(void)
{
#ifdef DEBUG
	printf("==========================================\n");
	printf("Unexpected Interrupt: %s\n", " HardFault");
	printf("==========================================\n");
#endif
	while (1)
		;
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
	HAL_IncTick();
}
