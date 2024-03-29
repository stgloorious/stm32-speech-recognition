/**
 * @file dfsdm.c
 * @brief Interface and filter for PDM microphone
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
#include "dfsdm.h"
#include "dma.h"
#include "error.h"
#include "pins.h"

DFSDM_Filter_HandleTypeDef hdfsdm1_filter0;
DFSDM_Channel_HandleTypeDef hdfsdm1_channel2;

static uint32_t HAL_RCC_DFSDM1_CLK_ENABLED = 0;
static uint32_t DFSDM1_Init = 0;

void dfsdm_init(void)
{
	hdfsdm1_filter0.Instance = DFSDM1_Filter0;
	hdfsdm1_filter0.Init.RegularParam.Trigger = DFSDM_FILTER_SW_TRIGGER;
	hdfsdm1_filter0.Init.RegularParam.FastMode = ENABLE;
	hdfsdm1_filter0.Init.RegularParam.DmaMode = ENABLE;
	hdfsdm1_filter0.Init.FilterParam.SincOrder = DFSDM_FILTER_SINC3_ORDER;
	hdfsdm1_filter0.Init.FilterParam.Oversampling = 200;
	hdfsdm1_filter0.Init.FilterParam.IntOversampling = 1;
	if (HAL_DFSDM_FilterInit(&hdfsdm1_filter0) != HAL_OK) {
		ERR("Failed to initialize DFSDM filter 0\n");
	}
	hdfsdm1_channel2.Instance = DFSDM1_Channel2;
	hdfsdm1_channel2.Init.OutputClock.Activation = ENABLE;
	hdfsdm1_channel2.Init.OutputClock.Selection =
		DFSDM_CHANNEL_OUTPUT_CLOCK_SYSTEM;
	hdfsdm1_channel2.Init.OutputClock.Divider = 25; /* 80 MHz / 25 = 3.2 MHz */
	hdfsdm1_channel2.Init.Input.Multiplexer = DFSDM_CHANNEL_EXTERNAL_INPUTS;
	hdfsdm1_channel2.Init.Input.DataPacking = DFSDM_CHANNEL_STANDARD_MODE;
	hdfsdm1_channel2.Init.Input.Pins = DFSDM_CHANNEL_SAME_CHANNEL_PINS;
	hdfsdm1_channel2.Init.SerialInterface.Type = DFSDM_CHANNEL_SPI_RISING;
	hdfsdm1_channel2.Init.SerialInterface.SpiClock =
		DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
	hdfsdm1_channel2.Init.Awd.FilterOrder = DFSDM_CHANNEL_FASTSINC_ORDER;
	hdfsdm1_channel2.Init.Awd.Oversampling = 1;
	hdfsdm1_channel2.Init.Offset = -190;
	hdfsdm1_channel2.Init.RightBitShift = 8;
	if (HAL_DFSDM_ChannelInit(&hdfsdm1_channel2) != HAL_OK) {
		ERR("Failed to initialize DFSDM channel 2\n");
	}
	if (HAL_DFSDM_FilterConfigRegChannel(&hdfsdm1_filter0, DFSDM_CHANNEL_2,
					     DFSDM_CONTINUOUS_CONV_ON) !=
	    HAL_OK) {
		ERR("Failed to set DFSDM filter 0 to channel 2\n");
	}
}

/**
* @brief DFSDM_Filter MSP Initialization
* This function configures the hardware resources used in this example
* @param hdfsdm_filter: DFSDM_Filter handle pointer
* @retval None
*/
void HAL_DFSDM_FilterMspInit(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };
	if (DFSDM1_Init == 0) {
		/** Initializes the peripherals clock
  */
		PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_DFSDM1;
		PeriphClkInit.Dfsdm1ClockSelection = RCC_DFSDM1CLKSOURCE_PCLK;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
			ERR("Failed to setup peripheral clock.\n");
		}

		/* Peripheral clock enable */
		HAL_RCC_DFSDM1_CLK_ENABLED++;
		if (HAL_RCC_DFSDM1_CLK_ENABLED == 1) {
			__HAL_RCC_DFSDM1_CLK_ENABLE();
		}

		__HAL_RCC_GPIOE_CLK_ENABLE();
		/**DFSDM1 GPIO Configuration
		PE7     ------> DFSDM1_DATIN2
		PE9     ------> DFSDM1_CKOUT
		*/
		GPIO_InitStruct.Pin = DFSDM1_DATIN2_Pin | DFSDM1_CKOUT_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF6_DFSDM1;
		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

		DFSDM1_Init++;
	}

	/* DFSDM1 DMA Init */
	/* DFSDM1_FLT0 Init */
	if (hdfsdm_filter->Instance == DFSDM1_Filter0) {
		hdma_dfsdm1_flt0.Instance = DMA1_Channel4;
		hdma_dfsdm1_flt0.Init.Request = DMA_REQUEST_0;
		hdma_dfsdm1_flt0.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_dfsdm1_flt0.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_dfsdm1_flt0.Init.MemInc = DMA_MINC_ENABLE;
		hdma_dfsdm1_flt0.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		hdma_dfsdm1_flt0.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
		hdma_dfsdm1_flt0.Init.Mode = DMA_CIRCULAR;
		hdma_dfsdm1_flt0.Init.Priority = DMA_PRIORITY_HIGH;
		if (HAL_DMA_Init(&hdma_dfsdm1_flt0) != HAL_OK) {
			ERR("Failed to initialize DMA1 for DFSDM1 Filter 0\n");
		}

		/* Several peripheral DMA handle pointers point to the same DMA handle.
		Be aware that there is only one channel to perform all the requested DMAs. */
		__HAL_LINKDMA(hdfsdm_filter, hdmaInj, hdma_dfsdm1_flt0);
		__HAL_LINKDMA(hdfsdm_filter, hdmaReg, hdma_dfsdm1_flt0);
	}
}

/**
* @brief DFSDM_Channel MSP Initialization
* This function configures the hardware resources used in this example
* @param hdfsdm_channel: DFSDM_Channel handle pointer
* @retval None
*/
void HAL_DFSDM_ChannelMspInit(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };
	if (DFSDM1_Init == 0) {
		/** Initializes the peripherals clock */
		PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_DFSDM1;
		PeriphClkInit.Dfsdm1ClockSelection = RCC_DFSDM1CLKSOURCE_PCLK;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
			ERR("Failed to setup peripheral clock.\n");
		}

		/* Peripheral clock enable */
		HAL_RCC_DFSDM1_CLK_ENABLED++;
		if (HAL_RCC_DFSDM1_CLK_ENABLED == 1) {
			__HAL_RCC_DFSDM1_CLK_ENABLE();
		}

		__HAL_RCC_GPIOE_CLK_ENABLE();
		/**DFSDM1 GPIO Configuration
		PE7     ------> DFSDM1_DATIN2
		PE9     ------> DFSDM1_CKOUT */
		GPIO_InitStruct.Pin = DFSDM1_DATIN2_Pin | DFSDM1_CKOUT_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF6_DFSDM1;
		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

		DFSDM1_Init++;
	}
}

/**
* @brief DFSDM_Filter MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hdfsdm_filter: DFSDM_Filter handle pointer
* @retval None
*/
void HAL_DFSDM_FilterMspDeInit(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
	DFSDM1_Init--;
	if (DFSDM1_Init == 0) {
		/* Peripheral clock disable */
		__HAL_RCC_DFSDM1_CLK_DISABLE();

		/**DFSDM1 GPIO Configuration
		PE7     ------> DFSDM1_DATIN2
		PE9     ------> DFSDM1_CKOUT*/
		HAL_GPIO_DeInit(GPIOE, DFSDM1_DATIN2_Pin | DFSDM1_CKOUT_Pin);

		/* DFSDM1 DMA DeInit */
		HAL_DMA_DeInit(hdfsdm_filter->hdmaInj);
		HAL_DMA_DeInit(hdfsdm_filter->hdmaReg);
	}
}

/**
* @brief DFSDM_Channel MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hdfsdm_channel: DFSDM_Channel handle pointer
* @retval None
*/
void HAL_DFSDM_ChannelMspDeInit(DFSDM_Channel_HandleTypeDef *hdfsdm_channel)
{
	DFSDM1_Init--;
	if (DFSDM1_Init == 0) {
		/* Peripheral clock disable */
		__HAL_RCC_DFSDM1_CLK_DISABLE();

		/**DFSDM1 GPIO Configuration
		PE7     ------> DFSDM1_DATIN2
		PE9     ------> DFSDM1_CKOUT
    */
		HAL_GPIO_DeInit(GPIOE, DFSDM1_DATIN2_Pin | DFSDM1_CKOUT_Pin);
	}
}
