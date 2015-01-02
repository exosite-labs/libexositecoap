/*****************************************************************************
*
*  exosite.c - Exosite Activator Library
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
#include "exosite_pal.h"
#include "exosite.h"
#include "coap.h"

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

// Internal Functions

static void exo_process_waiting_datagrams(exo_op *op, uint8_t count);
static void exo_process_active_ops(exo_op *op, uint8_t count);
exo_error exo_build_msg_activate(coap_pdu *pdu, const char *vendor, const char *model, const char *serial_number);
exo_error exo_build_msg_read(coap_pdu *pdu, const char *alias);
exo_error exo_build_msg_observe(coap_pdu *pdu, const char *alias);
exo_error exo_build_msg_write(coap_pdu *pdu, const char *alias, const char *value);
exo_error exo_build_msg_rst(coap_pdu *pdu, const uint16_t mid, const uint64_t token, const uint8_t tkl);
exo_error exo_build_msg_ack(coap_pdu *pdu, const uint16_t mid);
uint8_t exosite_validate_cik(char *cik);

// Local Variables
static char cik[CIK_LENGTH  + 1];
static const char *vendor;
static const char *model;
static const char *serial;
static uint16_t message_id_counter;
static exo_device_state device_state = EXO_STATE_UNINITIALIZED;

/*!
 * \brief  Initializes the Exosite library
 *
 * This **MUST** be called before any other exosite library calls are called.
 *
 * \param[in] *vendor  Pointer to string containing the vendor name
 * \param[in] *model   Pointer to string containing the model name
 * \param[in] *serial  Pointer to string containing the serial number
 *
 * \return EXO_ERROR, EXO_OK on success, else error code
 */
exo_error exo_init(const char *vendor_in, const char *model_in, const char *serial_in)
{
  device_state = EXO_STATE_UNINITIALIZED;

  if (exopal_init() != 0) {
    return EXO_FATAL_ERROR_PAL;
  }

  srand(time(NULL));
  message_id_counter = rand();

  serial = serial_in;
  vendor = vendor_in;
  model = model_in;

  if (exopal_retrieve_cik(cik) > 1){
    return EXO_FATAL_ERROR_PAL;
  } else {
    cik[40] = 0;
  }

  if (exopal_udp_sock() != 0) {
    return EXO_FATAL_ERROR_PAL;
  }

  device_state = EXO_STATE_INITIALIZED;

  return EXO_OK;
}

/*!
 * \brief  Queues a Write to the Exosite One Platform
 *
 * Queues a request to write to a dataport.
 *
 * \param[in] *alias    Alias of dataport to write to, pointer must remain
 *                      valid until the request has been sent.
 * \param[in] callback  Function pointer to call on success
 *
 * \return EXO_ERROR, EXO_OK on success or error code
 *
 */

void exo_write(exo_op *op, const char * alias, const char * value)
{
  op->type = EXO_WRITE;
  op->state = EXO_REQUEST_NEW;
  op->alias = alias;
  op->value = (char *)value; // this is kinda dirty, I know
  op->value_max = 0;
  op->mid = 0;
}

/*!
 * \brief  Queues a Read from the Exosite One Platform
 *
 * Queues a request to read a dataport. If the request is successful, the
 * provided callback will be called with a single parameter, a pointer to the
 * result as a C string.
 *
 * \param[in] *alias    Alias of dataport to read from, pointer must remain
 *                      valid until the request has been sent.
 * \param[in] callback  Function pointer to call on success
 *
 * \return EXO_ERROR, EXO_OK on success or error code
 *
 */
void exo_read(exo_op *op, const char * alias, char * value, const size_t value_max)
{
  op->type = EXO_READ;
  op->state = EXO_REQUEST_NEW;
  op->alias = alias;
  op->value = value;
  op->value_max = value_max;
  op->mid = 0;
}

/*!
 * \brief  Subscribes to a Dataport on the Exosite One Platform
 *
 * Begins a subscription to a dataport  The callback is called any time the
 * value is updated as well as when the subscription is started.
 *
 * \param[in] alias     Alias of dataport to read from
 * \param[in] callback  Function pointer to call on change
 *
 * \return EXO_ERROR, EXO_OK on success or error code
 *
 */
void exo_subscribe(exo_op *op, const char * alias, char * value, const size_t value_max)
{
  op->type = EXO_SUBSCRIBE;
  op->state = EXO_REQUEST_NEW;
  op->alias = alias;
  op->value = value;
  op->value_max = value_max;
  op->mid = 0;
}

/*!
 * \brief  Activates the Device on the Platform
 *
 * Queues an activation request. Usually only used internally.
 *
 */
void exo_activate(exo_op *op)
{
  op->type = EXO_ACTIVATE;
  op->state = EXO_REQUEST_NEW;
  op->alias = NULL;
  op->value = NULL;
  op->value_max = 0;
  op->mid = 0;
}

void exo_op_init(exo_op *op)
{
  op->type = EXO_NULL;
  op->state = EXO_REQUEST_NULL;
  op->alias = NULL;
  op->value = NULL;
  op->value_max = 0;
  op->mid = 0;
  op->obs_seq = 0;
  op->timeout = 0;
}

void exo_op_done(exo_op *op)
{
  if (exo_is_op_subscribe(op)) {
    op->state = EXO_REQUEST_SUBSCRIBED;
  } else {
    exo_op_init(op);
  }
}

uint8_t exo_is_op_valid(exo_op *op)
{
  return op->type != EXO_NULL;
}

uint8_t exo_is_op_success(exo_op *op)
{
  return op->state == EXO_REQUEST_SUCCESS;
}

uint8_t exo_is_op_finished(exo_op *op)
{
  return op->state == EXO_REQUEST_SUCCESS || op->state == EXO_REQUEST_ERROR;
}

uint8_t exo_is_op_read(exo_op *op)
{
  return op->type == EXO_READ;
}

uint8_t exo_is_op_subscribe(exo_op *op)
{
  return op->type == EXO_SUBSCRIBE;
}

uint8_t exo_is_op_write(exo_op *op)
{
  return op->type == EXO_WRITE;
}


/*!
 * \brief Performs queued operations with the Exosite One Platform
 *
 * Queues a request to read a dataport from the Exosite One Platform. If the
 * request is successful the provided callback will be called with a single
 * parameter, a pointer to the result as a C string.
 *
 * \param[in] alias     Alias of dataport to read from
 * \param[in] callback  Function pointer to call on success
 *
 * \return EXO_STATE, EXO_OK on success or error code
 *
 */
exo_state exo_operate(exo_op *op, uint8_t count)
{
  int i;

  switch (device_state){
    case EXO_STATE_UNINITIALIZED:
      return EXO_ERROR;
    case EXO_STATE_INITIALIZED:
    case EXO_STATE_BAD_CIK:
      if (op[0].state == EXO_REQUEST_NULL && op[0].timeout == 0)
        exo_activate(&op[0]);
    case EXO_STATE_GOOD:
      break;
  }

  exo_process_waiting_datagrams(op, count);
  exo_process_active_ops(op, count);

  for (i = 0; i < count; i++) {
    if (op[i].state == EXO_REQUEST_NEW)
      return EXO_BUSY;
  }

  for (i = 0; i < count; i++) {
    if (op[i].state == EXO_REQUEST_PENDING)
      return EXO_WAITING;
  }

  return EXO_IDLE;
}

// Internal Functions

static void exo_process_waiting_datagrams(exo_op *op, uint8_t count)
{
  uint8_t buf[576];
  coap_pdu pdu;
  coap_payload payload;
  int i;

  pdu.buf = buf;
  pdu.max = 576;
  pdu.len = 0;

  // receive a UDP packet if one or more waiting
  while (exopal_udp_recv(pdu.buf, pdu.max, &pdu.len) == 0) {
    if (coap_validate_pkt(&pdu) != CE_NONE)
      continue; //Invalid Packet, Ignore

    for (i = 0; i < count; i++) {
      if(op[i].type == EXO_WRITE) {
        if (op[i].state == EXO_REQUEST_PENDING && op[i].mid == coap_get_mid(&pdu)) {
          if (coap_get_code_class(&pdu) == 2) {
            op[i].state = EXO_REQUEST_SUCCESS;
          } else {
            op[i].state = EXO_REQUEST_ERROR;

            if (coap_get_code(&pdu) == CC_UNAUTHORIZED)
              device_state = EXO_STATE_BAD_CIK;
          }
          break;
        }
      } else if(op[i].type == EXO_READ) {
        if (op[i].state == EXO_REQUEST_PENDING && op[i].mid == coap_get_mid(&pdu)) {
          if (coap_get_code_class(&pdu) == 2) {
            payload = coap_get_payload(&pdu);
            if (payload.len == 0) {
              op[i].value = 0;
            } else if (payload.len+1 > op[i].value_max || op[i].value == 0) {
              op[i].state = EXO_REQUEST_ERROR;
            } else{
              memcpy(op[i].value, payload.val, payload.len);
              op[i].value[payload.len] = 0;
              op[i].state = EXO_REQUEST_SUCCESS;
            }
          } else {
            op[i].state = EXO_REQUEST_ERROR;

            if (coap_get_code(&pdu) == CC_UNAUTHORIZED)
              device_state = EXO_STATE_BAD_CIK;
          }
          break;
        }
      } else if(op[i].type == EXO_SUBSCRIBE) {
        if (op[i].state == EXO_REQUEST_PENDING && op[i].mid == coap_get_mid(&pdu)) {
          if (coap_get_code_class(&pdu) == 2) {
            payload = coap_get_payload(&pdu);
            if (payload.len == 0) {
              op[i].value = 0;
            } else if (payload.len+1 > op[i].value_max || op[i].value == 0) {
              op[i].state = EXO_REQUEST_ERROR;
            } else{
              memcpy(op[i].value, payload.val, payload.len);
              op[i].value[payload.len] = 0;
              op[i].state = EXO_REQUEST_SUCCESS;
              op[i].timeout = exopal_get_time() + 120000000 + (rand() % 15 * 100000);
            }
          } else {
            op[i].state = EXO_REQUEST_ERROR;

            if (coap_get_code(&pdu) == CC_UNAUTHORIZED)
              device_state = EXO_STATE_BAD_CIK;
          }
          break;
        }else if(op[i].state == EXO_REQUEST_SUBSCRIBED && coap_get_token(&pdu) == op[i].token) {
          coap_option opt;
          uint32_t new_seq = 0;
          opt = coap_get_option_by_num(&pdu, CON_OBSERVE, 0);
          for (int j = 0; j < opt.len; j++) {
            new_seq = (new_seq << (8*j)) | opt.val[j];
          }

          payload = coap_get_payload(&pdu);
          if (payload.len == 0) {
            op[i].value = 0;
          } else if (payload.len+1 > op[i].value_max || op[i].value == 0) {
            op[i].state = EXO_REQUEST_ERROR;
          } else{
            memcpy(op[i].value, payload.val, payload.len);
            op[i].value[payload.len] = 0;
            op[i].mid = coap_get_mid(&pdu);
            // TODO: User proper logic to ensure it's a new value not a different, but old one.
            if (op[i].obs_seq != new_seq) {
              op[i].state = EXO_REQUEST_SUB_ACK_NEW;
              op[i].obs_seq = new_seq;
            } else {
              op[i].state = EXO_REQUEST_SUB_ACK;
            }
          }
          break;
        }
      } else if(op[i].type == EXO_ACTIVATE) {
        if (op[i].state == EXO_REQUEST_PENDING && op[i].mid == coap_get_mid(&pdu)) {
          if (coap_get_code_class(&pdu) == 2) {
            payload = coap_get_payload(&pdu);
            if (payload.len == CIK_LENGTH) {
              memcpy(cik, payload.val, CIK_LENGTH);
              cik[CIK_LENGTH] = 0;
              op[i].state = EXO_REQUEST_SUCCESS;
              exopal_store_cik(cik);
              device_state = EXO_STATE_GOOD;
            } else {
              op[i].state = EXO_REQUEST_ERROR;
            }
          } else {
            // May or may not be an error, might just already be activated.
            op[i].state = EXO_REQUEST_ERROR;
            device_state = EXO_STATE_GOOD;
          }

          // We're done with this op now.
          exo_op_init(&op[i]);
          break;
        }
      }
    }

    // if the above loop ends normally we don't recognize message, reply RST
    if (i == count){
      if (coap_get_type(&pdu) == CT_CON) {
        // this can't fail
        exo_build_msg_rst(&pdu, coap_get_mid(&pdu), coap_get_token(&pdu), coap_get_tkl(&pdu));

        // best effort, don't bother checking if it failed, nothing we can do it
        // it did anyway
        exopal_udp_send(pdu.buf, pdu.len);
      }

      break;
    }
  }
}

// process all ops that are in an active state
static void exo_process_active_ops(exo_op *op, uint8_t count)
{
  uint8_t buf[576];
  coap_pdu pdu;
  int i;
  uint64_t now = exopal_get_time();

  pdu.buf = buf;
  pdu.max = 576;
  pdu.len = 0;

  for (i = 0; i < count; i++) {
    switch (op[i].state) {
      case EXO_REQUEST_NEW:
        // Build and Send Request
        switch (op[i].type) {
          case EXO_READ:
            exo_build_msg_read(&pdu, op[i].alias);
            break;
          case EXO_SUBSCRIBE:
            exo_build_msg_observe(&pdu, op[i].alias);
            break;
          case EXO_WRITE:
            exo_build_msg_write(&pdu, op[i].alias, op[i].value);
            break;
          case EXO_ACTIVATE:
            exo_build_msg_activate(&pdu, vendor, model, serial);
            break;
          default:
            op[i].type = EXO_NULL;
            continue;
        }

        if (exopal_udp_send(pdu.buf, pdu.len) == 0) {
          op[i].state = EXO_REQUEST_PENDING;
          op[i].timeout = exopal_get_time() + 4000000;
          op[i].mid = coap_get_mid(&pdu);
          op[i].token = coap_get_token(&pdu);
        }

        break;
      case EXO_REQUEST_SUBSCRIBED:
      case EXO_REQUEST_PENDING:
        // check if pending requests have reached timeout
        if (op[i].timeout <= now){
          switch (op[i].type) {
            case EXO_READ:
            case EXO_WRITE:
              // TODO: may want to do some sort of retry system
              op[i].state = EXO_REQUEST_ERROR;
              break;
            case EXO_SUBSCRIBE:
              // force a new observe request
              op[i].state = EXO_REQUEST_NEW;
              break;
            default:
              break;
          }
        }
        break;
      case EXO_REQUEST_SUB_ACK_NEW:
      case EXO_REQUEST_SUB_ACK:
        // send ack for observe notification
        exo_build_msg_ack(&pdu, coap_get_mid(&pdu));

        if (exopal_udp_send(pdu.buf, pdu.len) == 0) {
          if (op[i].state == EXO_REQUEST_SUB_ACK)
            op[i].state = EXO_REQUEST_SUBSCRIBED;
          else if (op[i].state == EXO_REQUEST_SUB_ACK_NEW)
            op[i].state = EXO_REQUEST_SUCCESS;

          // TODO: this should get set to Max-Age value, hard coding 120 s
          op[i].timeout = exopal_get_time() + 120000000 + (rand() % 15 * 100000);
        }
        break;
      default:
        break;
    }
  }
}


exo_error exo_build_msg_activate(coap_pdu *pdu, const char *vendor, const char *model, const char *serial_number)
{
    coap_error ret;
    coap_init_pdu(pdu);
    ret = coap_set_version(pdu, COAP_V1);
    ret |= coap_set_type(pdu, CT_CON);
    ret |= coap_set_code(pdu, CC_POST);
    ret |= coap_set_mid(pdu, message_id_counter++);
    ret |= coap_set_token(pdu, rand(), 2);
    ret |= coap_add_option(pdu, CON_URI_PATH, (uint8_t*)"provision", 9);
    ret |= coap_add_option(pdu, CON_URI_PATH, (uint8_t*)"activate", 8);
    ret |= coap_add_option(pdu, CON_URI_PATH, (uint8_t*)vendor, strlen(vendor));
    ret |= coap_add_option(pdu, CON_URI_PATH, (uint8_t*)model, strlen(model));
    ret |= coap_add_option(pdu, CON_URI_PATH, (uint8_t*)serial_number, strlen(serial_number));

    if (ret != CE_NONE)
      return EXO_GENERAL_ERROR;

    return EXO_OK;
}

exo_error exo_build_msg_read(coap_pdu *pdu, const char *alias)
{
    coap_error ret;
    coap_init_pdu(pdu);
    ret = coap_set_version(pdu, COAP_V1);
    ret |= coap_set_type(pdu, CT_CON);
    ret |= coap_set_code(pdu, CC_GET);
    ret |= coap_set_mid(pdu, message_id_counter++);
    ret |= coap_set_token(pdu, rand(), 2);
    ret |= coap_add_option(pdu, CON_URI_PATH, (uint8_t*)"1a", 2);
    ret |= coap_add_option(pdu, CON_URI_PATH, (uint8_t*)alias, strlen(alias));
    ret |= coap_add_option(pdu, CON_URI_QUERY, (uint8_t*)cik, 40);

    if (ret != CE_NONE)
      return EXO_GENERAL_ERROR;

    return EXO_OK;
}

exo_error exo_build_msg_observe(coap_pdu *pdu, const char *alias)
{
    uint8_t obs_opt = 0;
    coap_error ret;
    coap_init_pdu(pdu);
    ret = coap_set_version(pdu, COAP_V1);
    ret |= coap_set_type(pdu, CT_CON);
    ret |= coap_set_code(pdu, CC_GET);
    ret |= coap_set_mid(pdu, message_id_counter++);
    ret |= coap_set_token(pdu, rand(), 2);
    ret |= coap_add_option(pdu, CON_OBSERVE, &obs_opt, 1);
    ret |= coap_add_option(pdu, CON_URI_PATH, (uint8_t*)"1a", 2);
    ret |= coap_add_option(pdu, CON_URI_PATH, (uint8_t*)alias, strlen(alias));
    ret |= coap_add_option(pdu, CON_URI_QUERY, (uint8_t*)cik, 40);

    if (ret != CE_NONE)
      return EXO_GENERAL_ERROR;

    return EXO_OK;
}

exo_error exo_build_msg_write(coap_pdu *pdu, const char *alias, const char *value)
{
    coap_error ret;
    coap_init_pdu(pdu);
    ret = coap_set_version(pdu, COAP_V1);
    ret |= coap_set_type(pdu, CT_CON);
    ret |= coap_set_code(pdu, CC_POST);
    ret |= coap_set_mid(pdu, message_id_counter++);
    ret |= coap_set_token(pdu, rand(), 2);
    ret |= coap_add_option(pdu, CON_URI_PATH, (uint8_t*)"1a", 2);
    ret |= coap_add_option(pdu, CON_URI_PATH, (uint8_t*)alias, strlen(alias));
    ret |= coap_add_option(pdu, CON_URI_QUERY, (uint8_t*)cik, 40);
    ret |= coap_set_payload(pdu, (uint8_t *)value, strlen(value));

    if (ret != CE_NONE)
      return EXO_GENERAL_ERROR;

    return EXO_OK;
}

exo_error exo_build_msg_rst(coap_pdu *pdu, const uint16_t mid, const uint64_t token, const uint8_t tkl)
{
    coap_error ret;
    coap_init_pdu(pdu);
    ret = coap_set_version(pdu, COAP_V1);
    ret |= coap_set_type(pdu, CT_RST);
    ret |= coap_set_code(pdu, CC_EMPTY);
    ret |= coap_set_mid(pdu, mid);
    ret |= coap_set_token(pdu, token, tkl);

    if (ret != CE_NONE)
      return EXO_GENERAL_ERROR;

    return EXO_OK;
}

exo_error exo_build_msg_ack(coap_pdu *pdu, const uint16_t mid)
{
    coap_error ret;
    coap_init_pdu(pdu);
    ret = coap_set_version(pdu, COAP_V1);
    ret |= coap_set_type(pdu, CT_ACK);
    ret |= coap_set_code(pdu, CC_EMPTY);
    ret |= coap_set_mid(pdu, mid);

    if (ret != CE_NONE)
      return EXO_GENERAL_ERROR;

    return EXO_OK;
}

exo_error exo_do_activate()
{
  exo_op op;

  exo_op_init(&op);


  return EXO_OK;
}

uint8_t exo_util_is_ascii_hex(const char *str, size_t len)
{
    size_t i;

    for (i = 0; i < len; i++) {
        if (!((str[i] >= 'a' && str[i] <= 'f') ||
              (str[i] >= '0' && str[i] <= '9'))) {
            return 1;
        }
    }

    return 0;
}

