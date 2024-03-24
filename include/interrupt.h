/**
 * @file interrupt.h
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

/**
 * @brief Non-maskable interrupt handler
 */
void NMI_Handler(void);

/**
 * @brief Hard fault exception handler
 * @note Compile with -DDEBUG for debug information
 */
__attribute__((noreturn)) void HardFault_Handler(void);

/**
 * @brief MemManage exception handler
 * @note Compile with -DDEBUG for debug information
 */
__attribute__((noreturn)) void MemManage_Handler(void);

/**
 * @brief Bus fault exception handler handler
 * @note Compile with -DDEBUG for debug information
 */
__attribute__((noreturn)) void BusFault_Handler(void);

/**
 * @brief Usage fault exception handler
 * @note Compile with -DDEBUG for debug information
 */
__attribute__((noreturn)) void UsageFault_Handler(void);

/**
 * @brief SVC exception handler
 */
void SVC_Handler(void);

/**
 * @brief Debug monitor interrupt handler
 */
void DebugMon_Handler(void);

/**
 * @brief Pendable service request handler
 */
void PendSV_Handler(void);

/**
 * @brief System tick handler
 */
void SysTick_Handler(void);
