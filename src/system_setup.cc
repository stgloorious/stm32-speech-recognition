/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/lite/micro/system_setup.h"

extern "C" {
#include <stdio.h>
#include "stm32l4xx_hal.h"
#include "stm32l475e_iot01.h"
#include "clock.h"
#include "debug_io.h"
}

extern "C" void init_hw(void);

namespace tflite
{

void InitializeTarget()
{
	init_hw();
}
}

void init_hw(void)
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
		//ERR("HAL_Init() failed.\n");
	}

	/* Configure and start the necessary clocks and PLLs */
	SystemClock_Config();

	BSP_LED_Init(LED2);
	BSP_LED_On(LED2);

	uart_debug_init();

	//BSP_LED_Off(LED2);

	printf("ML on MCU Demo\n");
	printf("Build date: %s %s\n", __DATE__, __TIME__);
	printf("GCC %i.%i.%i ", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
	printf("Newlib %s\n\n", _NEWLIB_VERSION);
}
