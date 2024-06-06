/**
 * @file zmodem.c
 * @brief Zmodem state machine
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

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <zmodem.h>
#include <zmodem_helper.h>

static uint32_t idx = 0;

uint32_t zmodem_recv(uint8_t *out, const size_t len)
{
	uint8_t buf[256];
	uint8_t rzr_buf[4];
	uint16_t count;
	uint32_t received_data_size = 0;
	ZHDR hdr;
	zm_await((char *)"rz\r", (char *)rzr_buf, 4);
	while (true) {
startframe:
		uint16_t result = zm_await_header(&hdr);

		switch (result) {
		case CANCELLED:
			goto cleanup;
		case OK:

			switch (hdr.type) {
			case ZRQINIT:
			case ZEOF:

				result = zm_send_flags_hdr(
					ZRINIT, CANOVIO | CANFC32, 0, 0, 0);

				if (result == CANCELLED) {
					goto cleanup;
				} else if (result == OK) {
				} else if (result == CLOSED) {
					goto cleanup;
				}

				continue;

			case ZFIN:

				result = zm_send_pos_hdr(ZFIN, 0);

				if (result == CANCELLED) {
					goto cleanup;
				} else if (result == OK) {
				} else if (result == CLOSED) {
				}

				goto cleanup;

			case ZFILE:

				switch (hdr.flags.f0) {
				case 0: /* no special treatment - default to ZCBIN */
				case ZCBIN:
					break;
				case ZCNL:
					break;
				case ZCRESUM:
					break;
				default:
					break;
				}

				count = sizeof(buf);
				result = zm_read_data_block(buf, &count);

				if (result == CANCELLED) {
					goto cleanup;
				} else if (!IS_ERROR(result)) {

					result = zm_send_pos_hdr(
						ZRPOS, received_data_size);

					if (result == CANCELLED) {
						goto cleanup;
					} else if (result == CLOSED) {
						goto cleanup;
					}
				}
				continue;

			case ZDATA:

				while (true) {
					count = sizeof(buf);
					result =
						zm_read_data_block(buf, &count);


					if (result == CANCELLED) {
						goto cleanup;
					} else if (!IS_ERROR(result)) {
						memcpy(out+idx, buf, count - 1);
						idx += (count-1);
						received_data_size +=
							(count - 1);

						if (result == GOT_CRCE) {
							// End of frame, header follows, no ZACK expected.
							break;
						} else if (result == GOT_CRCG) {
							// Frame continues, non-stop (another data packet follows)
							continue;
						} else if (result == GOT_CRCQ) {
							// Frame continues, ZACK required
							result = zm_send_pos_hdr(
								ZACK,
								received_data_size);

							if (result ==
							    CANCELLED) {
								goto cleanup;
							} else if (result ==
								   OK) {
							} else if (result ==
								   CLOSED) {
								goto cleanup;
							}

							continue;
						} else if (result == GOT_CRCW) {
							// End of frame, header follows, ZACK expected.

							result = zm_send_pos_hdr(
								ZACK,
								received_data_size);

							if (result ==
							    CANCELLED) {
								goto cleanup;
							} else if (result ==
								   OK) {
							} else if (result ==
								   CLOSED) {
								goto cleanup;
							}

							break;
						}

					} else {
						result = zm_send_pos_hdr(
							ZRPOS,
							received_data_size);

						if (result == CANCELLED) {
							goto cleanup;
						} else if (result == OK) {
							goto startframe;
						} else if (result == CLOSED) {
							goto cleanup;
						}
					}
				}

				continue;

			default:
				continue;
			}

			break;
		default:
			result = zm_send_pos_hdr(ZNAK, received_data_size);

			if (result == CANCELLED) {
				goto cleanup;
			} else if (result == OK) {
			} else if (result == CLOSED) {
				goto cleanup;
			}

			continue;
		}
	}

cleanup:
	return idx;
}
