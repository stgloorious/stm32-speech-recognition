/**
 * @file debug_io.c
 * @brief Initializes UART to be used as a debug UART port.
 */

/* Copyright (C) 2024 Stefan Gloor
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

#include "debug_io.h"
#include "stm32l4xx_hal.h"

#include <unistd.h>

UART_HandleTypeDef uart_hd_debug_uart;

#define UART_RX_FIFO_SIZE 1024
volatile static uint8_t uart_rx_fifo[UART_RX_FIFO_SIZE];
volatile static uint8_t uart_rx_fifo_enqueue = 0;
volatile static uint8_t uart_rx_fifo_dequeue = 1;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uart_hd)
{
	HAL_UART_Receive_IT(uart_hd,
			    (uint8_t *)uart_rx_fifo + uart_rx_fifo_enqueue, 1);
	uart_rx_fifo_enqueue = (uart_rx_fifo_enqueue + 1) % UART_RX_FIFO_SIZE;
}

/* attach the _read,_write syscall to debug UART */
int _write(int fd, const void *buf, int cnt)
{
	if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
		if (HAL_UART_Transmit(&uart_hd_debug_uart, (uint8_t *)buf, cnt,
				      100) != HAL_OK) {
			return -1;
		}
#ifndef CONFIG_DEBUG_NOCR
		/* check whether the last character was a newline,
         * and add a carriage return if it is */
		if (*(uint8_t *)(buf + cnt - 1) == '\n') {
			_write(fd, "\r", 1);
		}
#endif
	}
	return cnt;
}

int _read(int fd, uint8_t *buf, int cnt)
{
	/* wait as long as fifo is empty */
	while (uart_rx_fifo_enqueue == uart_rx_fifo_dequeue)
		;

	int i = 0;
	while (uart_rx_fifo_enqueue != uart_rx_fifo_dequeue) {
		buf[i++] = uart_rx_fifo[uart_rx_fifo_dequeue - 1];
		uart_rx_fifo_dequeue =
			(uart_rx_fifo_dequeue + 1) % UART_RX_FIFO_SIZE;
		if (i == cnt) {
			break;
		}
	}
	return i;
}

int uart_debug_init()
{
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	uart_hd_debug_uart.Instance = USART1;

	uart_hd_debug_uart.Init.BaudRate = 115200;
	uart_hd_debug_uart.Init.WordLength = UART_WORDLENGTH_8B;
	uart_hd_debug_uart.Init.StopBits = UART_STOPBITS_1;
	uart_hd_debug_uart.Init.Parity = UART_PARITY_NONE;
	uart_hd_debug_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uart_hd_debug_uart.Init.Mode = UART_MODE_TX_RX;
	uart_hd_debug_uart.Init.OverSampling = UART_OVERSAMPLING_16;

	if (HAL_UART_DeInit(&uart_hd_debug_uart) != HAL_OK) {
		return -1;
	}
	if (HAL_UART_Init(&uart_hd_debug_uart) != HAL_OK) {
		return -1;
	}

	GPIO_InitTypeDef GPIO_InitStruct;
	/* UART_TX */
	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* UART_RX */
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	__HAL_UART_ENABLE_IT(&uart_hd_debug_uart, UART_IT_RXNE);
	HAL_NVIC_SetPriority(USART1_IRQn, 10, 10);
	HAL_NVIC_EnableIRQ(USART1_IRQn);

	HAL_UART_Receive_IT(&uart_hd_debug_uart,
			    (uint8_t *)uart_rx_fifo + uart_rx_fifo_enqueue, 1);
	return 0;
}

void USART1_IRQHandler()
{
	HAL_UART_IRQHandler(&uart_hd_debug_uart);
}
