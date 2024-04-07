/**
 * @file mic.cc
 * @brief Microphone data acquisition
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

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include "mic.h"

extern "C" {
	#include "stm32l4xx_hal.h"
	#include "dma.h"
	#include "dfsdm.h"
	#include "error.h"
}

void speech::mic::init() {
	dma_init();
	dfsdm_init();
	this->buf = (int32_t*)malloc(this->buf_size * sizeof(int32_t));
	if (this->buf == NULL){
		ERR("malloc failed.\n");
	}

	/* Memory test */
	memset(this->buf, 42, this->buf_size * sizeof(int32_t));
	for (size_t i = 0; i < this->buf_size; i++){
		this->buf[i] = 42;
	}

	for (size_t i = 0; i < this->buf_size; i++){
		if (this->buf[i] != 42){
			printf("mismatch on %u\n", i);
		}
		assert(this->buf[i] == 42);
	}

	if (HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter0, this->buf, this->buf_size) !=
		HAL_OK) {
		ERR("Failed to start conversion.\n");
	}
}

void speech::mic::dump_recording(){
	for (size_t i = 0; i < this->buf_size; i++){
		printf("%li\n", this->buf[i]);
	}
}

