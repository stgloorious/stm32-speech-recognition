/**
 * @file serial.c
 * @brief Simple serial protocol for transferring preprocessed input data
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

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <serial.h>
#include <checksum.h>

static uint32_t bytes_received = 0;

int serial_recv(char* out, size_t len){
	int i = 0;
	// Wait for transaction to start
	do {
		i += read(STDIN_FILENO, out+i, strlen(SERIAL_CMD_START_TRANSACTION));
	} while ((strstr(out, SERIAL_CMD_START_TRANSACTION) == NULL) && (i < len));
	if (i >= len - 1){
		write(STDOUT_FILENO, SERIAL_CMD_NACK, strlen(SERIAL_CMD_NACK));
		return 0;
	}
	// Send "START" back
	write(STDOUT_FILENO, SERIAL_CMD_START_TRANSACTION, strlen(SERIAL_CMD_START_TRANSACTION));

	// Send expected block size
	char blocksize[5];
	snprintf(blocksize, sizeof(blocksize), "%04u", SERIAL_BLOCK_SIZE);
	write(STDOUT_FILENO, blocksize, 4);

	// Receive file size
	i = 0;
	while (i < 8){
		i += read(STDIN_FILENO, out+i, 8-i);
	}
	out[8] = '\0';
	size_t filesize = atoi(out);
	size_t rounded_filesize = (filesize + SERIAL_BLOCK_SIZE - 1)
		/ SERIAL_BLOCK_SIZE * SERIAL_BLOCK_SIZE;

	if ((rounded_filesize > len) || (filesize == 0)){
		write(STDOUT_FILENO, SERIAL_CMD_NACK, strlen(SERIAL_CMD_NACK));
		return 0;
	}
	write(STDOUT_FILENO, SERIAL_CMD_ACK, strlen(SERIAL_CMD_ACK));

	// Receive data
	bytes_received = 0;
	unsigned char blockbuf[SERIAL_BLOCK_SIZE];
	do {
		int j = 0;
		char checksum[9];
		// 32 bit checksum first
		while (j < 8){
			j += read(STDIN_FILENO, checksum+j, 1);
		}
		checksum[8] = '\0';
		uint32_t expected_crc32 = strtoll(checksum, NULL, 16);

		j = 0;
		// actual data block
		while (j < SERIAL_BLOCK_SIZE){
			size_t len = read(STDIN_FILENO, blockbuf+j, 1);
			// An error occurred (e.g., timeout)
			if (len == 0){
				break;
			}
			else {
				j += len;
			}
		}

		if (crc_32(blockbuf, SERIAL_BLOCK_SIZE) == expected_crc32) {
			write(STDOUT_FILENO, SERIAL_CMD_ACK, strlen(SERIAL_CMD_ACK));
			memcpy(out + bytes_received, blockbuf, SERIAL_BLOCK_SIZE);
			bytes_received += SERIAL_BLOCK_SIZE;
		}
		else {
			write(STDOUT_FILENO, SERIAL_CMD_NACK, strlen(SERIAL_CMD_NACK));
		}
	} while (bytes_received < rounded_filesize);

	return bytes_received;
}
