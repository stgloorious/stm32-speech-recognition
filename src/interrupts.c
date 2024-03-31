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
#include <stdio.h>

#include "interrupt.h"
#include "stm32l4xx_hal.h"
#include "usbd_def.h"
#include "usbd_hid.h"

#define CURSOR_STEP 5
uint8_t HID_Buffer[4];
extern PCD_HandleTypeDef hpcd;
extern USBD_HandleTypeDef USBD_Device;

static void GetPointerData(uint8_t *pbuf);

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

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	static __IO uint32_t counter = 0;
	HAL_IncTick();

	/* check Joystick state every polling interval (10ms) */
	if (counter++ == USBD_HID_GetPollingInterval(&USBD_Device)) {
		GetPointerData(HID_Buffer);

		/* send data though IN endpoint*/
		if ((HID_Buffer[1] != 0) || (HID_Buffer[2] != 0)) {
			//USBD_HID_SendReport(&USBD_Device, HID_Buffer, 4);
		}
		counter = 0;
	}
}

/******************************************************************************/
/*                 STM32L4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles USB-On-The-Go FS global interrupt request.
  * @param  None
  * @retval None
  */
void OTG_FS_IRQHandler(void)
{
	HAL_PCD_IRQHandler(&hpcd);
}

/**
  * @brief  This function handles External lines 15 to 10 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

/**
  * @brief  Gets Pointer Data.
  * @param  pbuf: Pointer to report
  * @retval None
  */
static void GetPointerData(uint8_t *pbuf)
{
	static int8_t cnt = 0;
	int8_t x = 0, y = 0;

	if (cnt++ > 0) {
		x = CURSOR_STEP;
	} else {
		x = -CURSOR_STEP;
	}

	pbuf[0] = 0;
	pbuf[1] = x;
	pbuf[2] = y;
	pbuf[3] = 0;
}

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */

/**
  * @}
  */
