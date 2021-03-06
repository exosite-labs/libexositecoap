/*****************************************************************************
*
*  exosite_hal.c - Exosite hardware & environmenat adapation layer.
*  Copyright (C) 2014 Exosite LLC
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

#define PAL_CIK_LENGTH 40

static int exosock;

static char exosite_pal_host[] = "coap.exosite.com";
static char exosite_pal_port[] = "5683";

int errno;

/*!
 * \brief Creates a udp socket
 *
 * Ensures that the library has access to a client style UDP socket. This can
 * be done either through an OS, or direct calls to a modem.
 *
 * \return 0 if successful, else error code
 */
uint8_t exopal_udp_sock()
{
	// unimplemented, return error
	return 1;
}


/*!
 * \brief
 *
 * Any HW or SW initialization should be performed in here
 *
 * This function is meant to perform any one time initialization and/or setup.
 * This will be called every time Exosite_init is called.
 *
 */
uint8_t exopal_init()
{
	return 0;
}

/*!
 * \brief Sends a UDP Packet to Exosite
 *
 * Write data out to the currently open socket
 *
 * \param[in] buffer Data to write to socket
 * \param[in] len Length of data to write to socket
 *
 * \sa exopal_socketRead
 *
 * \return 0 if successful, else error code
 */
uint8_t exopal_udp_send(const uint8_t *buf, size_t len)
{
	// unimplemented, return error
	return 1;
}

/*!
 * \brief Receives a Packet from Exosite
 *
 * \param[out] buf Buffer received data will be written to
 * \param[in]  size Size of buffer
 * \param[out] rlen amount of data received from modem
 *
 *
 * \sa exopal_socketWrite
 *
 * \note This function should not block, it should return immediately if there
 *       are no waiting UDP packets.
 *
 * \return 0 if successful, else error code
 */
uint8_t exopal_udp_recv(uint8_t *buf, size_t size, size_t *rlen)
{
	// unimplemented, return error
	return 1;
}

/*!
 * \brief Stores the cik in non-volatile storage.
 *
 * \param[in] cik pointer to a buffer containing 40 char CIK.
 *
 * \return 0 if successful, else error code
 */
uint8_t exopal_store_cik(const char *cik)
{
	// unimplemented, return error
	return 1;
}

/*!
 * \brief Retrieves the cik from non-volatile storage.
 *
 * \param[out] cik pointer to 40 byte buffer into which you copy the CIK.
 *
 * \return 0 if successful , 1 if no CIK has been saved, >1 if fatal error
 */
uint8_t exopal_retrieve_cik(char *cik)
{
	// unimplemented, return error
	return 1;
}

/*!
 * \brief Returns the current time in microseconds.
 *
 * \return time in microseconds
 *
 * \note This should just be a timer from 0 at boot.
 */
uint64_t exopal_get_time()
{
	// unimplemented, return error
	return 1;
}

/*!
 * \brief Sets the current UTC time in microseconds.
 * 
 * \note Currently Unused.
 *
 * \param[in] Current time in microseconds.
 */
void exopal_set_time(uint64_t timestamp_us)
{
	return;
}
