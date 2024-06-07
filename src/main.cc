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

#include <serial.h>

#include "mic.h"
int dfsdm_conversion_done;

const int kTensorArenaSize = 48 * 1024;
alignas(16) static uint8_t tensor_arena[kTensorArenaSize] = { 0x55 };

static char input_tensor[16500];

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

	size_t input_tensor_len = serial_recv(input_tensor, sizeof(input_tensor));
	DEBUG_PRINTF("Received %u bytes.\n", input_tensor_len);

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

	print_shape(output);
	const char* labels[] = {"NO", "YES"};
	for (int i = 0; i < output->dims->data[output->dims->size - 1]; i++) {
		SUCCESS_PRINTF("Prediction %4s: %6.2f %%\n", labels[i],
			       tflite::GetTensorData<uint8_t>(output)[i]/2.55);
	}


	while (1)
		;
}
