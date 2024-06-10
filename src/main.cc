/* Copyright 2023 The TensorFlow Authors. All Rights Reserved.

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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <stm32l4xx_hal.h>

#include <models/model_tflite.h>
#include <tensorflow/lite/core/c/common.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/micro_log.h>
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include <tensorflow/lite/micro/micro_op_resolver.h>
#include <tensorflow/lite/micro/micro_profiler.h>
#include <tensorflow/lite/micro/recording_micro_interpreter.h>
#include <tensorflow/lite/micro/system_setup.h>
#include <tensorflow/lite/schema/schema_generated.h>

#include <dsp/basic_math_functions.h>
#include <dsp/transform_functions.h>
#include <dsp/window_functions.h>
#include <dsp/fast_math_functions.h>

#include <serial.h>

#include "mic.h"
int dfsdm_conversion_done;


const int kTensorArenaSize = 66800;
alignas(16) static uint8_t tensor_arena[kTensorArenaSize];

static uint8_t waveform[16128];
static uint8_t last_ffts[125];

#define DEBUG_PRINTF(...)            \
	{                            \
		printf("[i] ");      \
		printf(__VA_ARGS__); \
	}

#define RAW_PRINTF(...)              \
	{                            \
		printf(__VA_ARGS__); \
	}

#define SUCCESS_PRINTF(...)             \
	{                               \
		printf("\e[0;32m[*] "); \
		printf(__VA_ARGS__);    \
		printf("\e[0m");        \
	}

void PrintModelDetails(const tflite::Model *model)
{
	DEBUG_PRINTF("TFlite schema version: %lu\n", model->version());
	assert(model->version() == TFLITE_SCHEMA_VERSION);

	// Primary subgraph usually at index 0
	const tflite::SubGraph *subgraph = model->subgraphs()->Get(0);

	DEBUG_PRINTF("Number of tensors: %lu\n", subgraph->tensors()->size());
	for (size_t i = 0; i < subgraph->tensors()->size(); i++) {
		const tflite::Tensor *tensor = subgraph->tensors()->Get(i);
		DEBUG_PRINTF(
			"    %u: %s\n", i,
			(char *)tflite::EnumNameTensorType(tensor->type()));
	}
	DEBUG_PRINTF("Number of operators: %lu\n",
		     subgraph->operators()->size());

	// Iterate over operators and print their details
	for (size_t i = 0; i < subgraph->operators()->size(); i++) {
		const tflite::Operator *op = subgraph->operators()->Get(i);
		const tflite::OperatorCode *op_code =
			model->operator_codes()->Get(op->opcode_index());

		const char *op_name = tflite::EnumNameBuiltinOperator(
			static_cast<tflite::BuiltinOperator>(
				op_code->builtin_code()));

		DEBUG_PRINTF("    %u: %s\n", i, op_name);

		DEBUG_PRINTF("        Inputs: ");
		for (size_t j = 0; j < op->inputs()->size(); j++) {
			printf("%lu ", op->inputs()->Get(j));
		}
		printf("\n");

		DEBUG_PRINTF("        Outputs: ");
		for (size_t j = 0; j < op->outputs()->size(); j++) {
			printf("%lu ", op->outputs()->Get(j));
		}
		printf("\n");
	}
}

void print_shape(TfLiteTensor *tensor)
{
	DEBUG_PRINTF("%s bytes = %u\n", tensor->name, tensor->bytes);
	DEBUG_PRINTF("%s shape = (", tensor->name);
	for (int i = 0; i < tensor->dims->size; i++) {
		RAW_PRINTF("%u", tensor->dims->data[i]);
		if (i != tensor->dims->size - 1) {
			RAW_PRINTF(", ");
		}
	}
	RAW_PRINTF(")\n");
}

int main(int argc, char *argv[])
{
	tflite::InitializeTarget();
	speech::mic microphone;
	/*	microphone.init();

	while (!dfsdm_conversion_done)
		;
	microphone.dump_recording();
*/

	const static uint32_t window_size = 256;
	const static uint32_t frame_step = 128;

	arm_rfft_fast_instance_f32 fft;
	if (arm_rfft_fast_init_256_f32(&fft) != ARM_MATH_SUCCESS) {
		assert(!"Failed to init RFFT");
	}

	static float hanning[window_size];
	arm_hanning_f32(hanning, window_size);


	const tflite::Model *model = tflite::GetModel(model_tflite);
	DEBUG_PRINTF("Model architecture:\n");
	DEBUG_PRINTF("==============================================\n");
	PrintModelDetails(model);

	tflite::MicroMutableOpResolver<8> op_resolver;

	// Not really sure which ops to add
	if (op_resolver.AddRelu() != kTfLiteOk) {
		assert(!"Failed to add op");
	}
	if (op_resolver.AddConv2D() != kTfLiteOk) {
		assert(!"Failed to add op");
	}
	if (op_resolver.AddMaxPool2D() != kTfLiteOk) {
		assert(!"Failed to add op");
	}
	if (op_resolver.AddReshape() != kTfLiteOk) {
		assert(!"Failed to add op");
	}
	if (op_resolver.AddFullyConnected() != kTfLiteOk) {
		assert(!"Failed to add op");
	}
	if (op_resolver.AddSoftmax() != kTfLiteOk) {
		assert(!"Failed to add op");
	}
	if (op_resolver.AddResizeBilinear() != kTfLiteOk) {
		assert(!"Failed to add op");
	}
	if (op_resolver.AddQuantize() != kTfLiteOk) {
		assert(!"Failed to add op");
	}

	DEBUG_PRINTF("Added operations to OpsResolver.\n");

	// Interpreter
	tflite::MicroInterpreter interpreter(model, op_resolver, tensor_arena,
						 kTensorArenaSize);
	DEBUG_PRINTF("MicroInterpreter initialized.\n");

	if (interpreter.AllocateTensors() != kTfLiteOk) {
		assert(!"AllocateTensors() failed\n");
	}
	DEBUG_PRINTF("MicroInterpreter tensors allocated.\n");

	while (1) {
		uint32_t waveform_len = serial_recv((char*)waveform, sizeof(waveform));
		if (waveform_len == 0) {
			assert(!"Transfer failed.");
		}

		float min = 999999.0f;
		float max = 0;
		for (uint32_t i = 0; i < waveform_len; i++) {
			float val = (float)(waveform[i]);
			if (val < min) {
				min = val;
			}
			if (val > max) {
				max = val;
			}
		}

		for (uint32_t idx = 0; idx < 124; idx++) {
			float dst[window_size];
			static float mag[window_size + 1];
			double sum = 0;

			static float* signal_chunk = mag;

			for (uint32_t i = 0; i < window_size; i++) {
				signal_chunk[i] =
					(float)((uint8_t)waveform[idx * frame_step + i]);

				// Normalize from -1 to 1
				signal_chunk[i] = (2.0f * (signal_chunk[i] - min) / (max - min)) - 1;
				sum += signal_chunk[i];
			}

			// Remove DC component
			float mean = (float)(sum / (double)window_size);
			for (uint32_t i = 0; i < window_size; i++) {
				signal_chunk[i] = signal_chunk[i] - mean;

				// Apply window function
				signal_chunk[i] *= hanning[i];
			}

			arm_rfft_fast_f32(&fft, signal_chunk, dst, 0);

			// From to the CMSIS documentation:
			// https://arm-software.github.io/CMSIS-DSP/latest/group__RealFFT.html
			//
			// The FFT of a real N-point sequence has even symmetry in the
			// frequency domain. The second half of the data equals the conjugate
			// of the first half flipped in frequency. This conjugate part is not
			// computed by the float RFFT. As consequence, the output of a N point
			// real FFT should be a N//2 + 1 complex numbers so N + 2 floats.

			// It happens that the first complex of number of the RFFT output is
			// actually all real. Its real part represents the DC offset. The value
			// at Nyquist frequency is also real.

			// Those two complex numbers can be encoded with 2 floats rather than
			// using two numbers with an imaginary part set to zero.

			// The implementation is using a trick so that the output buffer can be
			// N float : the last real is packaged in the imaginary part of the
			// first complex (since this imaginary part is not used and is zero).

			// The first "complex" is actually to reals, X[0] and X[N/2]
			float first_real = (dst[0] < 0.0f) ? (-1.0f * dst[0]) : dst[0];
			float second_real = (dst[1] < 0.0f) ? (-1.0f * dst[1]) : dst[1];

			// Take the magnitude for all the complex values in between
			arm_cmplx_mag_f32(dst + 2, mag + 1, window_size / 2);

			// Fill in the two real numbers at 0 and N/2
			mag[0] = first_real;
			mag[128] = second_real;

			// N+1 FFT output, reuse waveform array
			for (uint32_t i = 0; i < 129; i++) {
#ifdef PRINT_SPECTROGRAM
				printf("%08f\n", mag[i]);
#endif

				// We can't override waveform[129 * idx + 128] yet
				// because we need it for the next iteration, so we need to store
				// it separately
				if (i < 128) {
					((uint8_t*)waveform)[128 * idx + i] = (uint8_t)(mag[i] * 8.0f);
				}
				else {
					last_ffts[idx] = (uint8_t)(mag[i] * 8.0f);
				}
			}
		}

		uint8_t* input_tensor = waveform;
		uint32_t input_tensor_len = 124 * 129;

		// We need to append an additional 124 bytes at the end of the spectrogram
		// because we need to transform it from 128 to 129 points
		// and insert the the N+1 points
		for (uint32_t idx = 123; idx > 0; idx--){
			uint8_t tmp[128];
			memcpy(tmp, waveform + (idx * 128), 128);
			memcpy(waveform + (idx * 128 + idx), tmp, 128);
			waveform[idx * 128 + (idx - 1)] = last_ffts[idx - 1];
		}

#ifdef PRINT_SPECTROGRAM
		for (uint32_t i = 0; i < input_tensor_len; i++){
			printf("%u\n", input_tensor[i]);
		}
#endif

		size_t start_time = HAL_GetTick();
		// Prepare input tensor
		TfLiteTensor *input = interpreter.input(0);
		input->dims->size = 4;
		input->dims->data[0] = 1;
		input->dims->data[1] = 124;
		input->dims->data[2] = 129;
		input->dims->data[3] = 1;
		input->bytes = input_tensor_len;
		const char input_name[] = "Input";
		input->name = input_name;
		memcpy(input->data.uint8, input_tensor, input->bytes);
		print_shape(input);

		// Perform inference
		DEBUG_PRINTF("Running inference...\n");
		if (interpreter.Invoke() != kTfLiteOk) {
			assert(!"Inference failed.\n");
		}

		// Get output tensor
		TfLiteTensor *output = interpreter.output(0);
		const char output_name[] = "Output";
		output->name = output_name;
		size_t end_time = HAL_GetTick();

		DEBUG_PRINTF("Time: #%08u\n", end_time - start_time);

		print_shape(output);
		const char* labels[] = {"DOWN", "LEFT", "NO", "RIGHT", "UP", "YES"};

		uint32_t pred = sizeof(labels);
		uint32_t max_val = 0;
		for (int32_t i = 0; i < output->dims->data[1]; i++){
			SUCCESS_PRINTF("Prediction %s: %u\n", labels[i], output->data.uint8[i]);
			if (output->data.uint8[i] > max_val){
				max_val = output->data.uint8[i];
				pred = i;
			}
		}
		SUCCESS_PRINTF("@%s\n", labels[pred]);
	}
}
