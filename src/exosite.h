/*****************************************************************************
*
*  exosite.h - Exosite Activator Library
*  Copyright (C) 2015 Exosite LLC
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#ifndef EXOSITE_H
#define EXOSITE_H

#include <stdint.h>
#include "exosite_pal.h"


// DEFINES
#define CIK_LENGTH                              40

// ENUMS
typedef enum exo_error
{
	EXO_OK = 0,
	EXO_GENERAL_ERROR,
	EXO_OUT_OF_SPACE,
	EXO_FATAL_ERROR_PAL,
} exo_error;

typedef enum exo_state
{
	EXO_IDLE,
	EXO_WAITING,
	EXO_BUSY,
	EXO_NEW_RESPONSE,
	EXO_ERROR,
	EXO_CONNECTION_ERROR,
	EXO_DEVICE_NOT_ENABLED,
	EXO_DEVICE_DOES_NOT_EXIST,
} exo_state;

typedef enum exo_data_format
{
	EXO_STRING,
	EXO_FLOAT,
	EXO_INTEGER
} exo_data_format;

typedef enum exo_device_state
{
  EXO_STATE_UNINITIALIZED,
  EXO_STATE_INITIALIZED,
  EXO_STATE_GOOD,
  EXO_STATE_BAD_CIK,
} exo_device_state;

typedef enum exo_request_type
{
  EXO_NULL = 0,
  EXO_WRITE,
  EXO_READ,
  EXO_SUBSCRIBE,
  EXO_ACTIVATE,
} exo_request_type;

typedef enum exo_request_state
{
  EXO_REQUEST_NULL,
  EXO_REQUEST_NEW,
  EXO_REQUEST_PENDING,
  EXO_REQUEST_SUBSCRIBED,
  EXO_REQUEST_SUB_ACK,
  EXO_REQUEST_SUB_ACK_NEW,
  EXO_REQUEST_SUCCESS,
  EXO_REQUEST_ERROR,
} exo_request_state;

typedef struct exo_op
{
	uint64_t token;
	uint64_t timeout;
	size_t value_max;
	uint32_t obs_seq;
	const char * alias;
	char * value;
	exo_request_type type;
	exo_request_state state;
	uint16_t mid;
	uint8_t tkl;
	uint8_t retries;
} exo_op;

// PUBLIC FUNCTIONS
exo_error exo_init(const char * vendor, const char *model, const char *sn);

void exo_write(exo_op *op, const char * alias, const char * value);
void exo_read(exo_op *op, const char * alias, char * value, const size_t value_max);
void exo_subscribe(exo_op *op, const char * alias, char * value, const size_t value_max);

void exo_op_init(exo_op *op);
void exo_op_done(exo_op *op);

uint8_t exo_is_op_valid(exo_op *op);
uint8_t exo_is_op_success(exo_op *op);
uint8_t exo_is_op_finished(exo_op *op);
uint8_t exo_is_op_read(exo_op *op);
uint8_t exo_is_op_subscribe(exo_op *op);
uint8_t exo_is_op_write(exo_op *op);

exo_state exo_operate(exo_op * ops, uint8_t count);


#endif



