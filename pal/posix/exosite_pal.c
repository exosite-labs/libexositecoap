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
 * be done either through an OS, or direct calls to the modem.
 *
 * \return 0 if successful, else error code
 */
 uint8_t exopal_udp_sock()
 {
	// Socket to Exosite
	int rv;

	struct addrinfo exohints, *servinfo, *q;

	memset(&exohints, 0, sizeof exohints);
	exohints.ai_family = AF_UNSPEC;
	exohints.ai_socktype = SOCK_DGRAM;
	exohints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(exosite_pal_host, exosite_pal_port, &exohints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(q = servinfo; q != NULL; q = q->ai_next) {
		if ((exosock = socket(q->ai_family, q->ai_socktype, q->ai_protocol)) == -1) {
			perror("Socket Call Failed");
			continue;
		}

		fcntl(exosock, F_SETFL, O_NONBLOCK);

		break;
	}

	if (q == NULL) {
		fprintf(stderr, "Failed to Bind Socket\n");
		return 2;
	}

	if (connect(exosock, q->ai_addr, q->ai_addrlen) == -1)
		return 3;

	return 0;
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
	size_t bytes_sent;
	if ((bytes_sent = send(exosock, buf, len, 0)) == -1){
		fprintf(stderr, "Socket SEND Error: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}

/*!
 * \brief Receives a Packet from Exosite
 *
 * \param[in] bufferSize Size of buffer
 * \param[out] buf Buffer received data will be written to
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
	ssize_t bytes_recv;
	bytes_recv = recv(exosock, buf, size, 0);
	if (bytes_recv < 0) {
		if (errno != EAGAIN){
			fprintf(stderr, "Socket RECV Error: %s\n", strerror(errno));
			return 1;
		} else {
			return 2;
		}
	}

	*rlen = bytes_recv;

	return 0;
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
	size_t bytes_written;
	FILE *file;
	file = fopen("cik", "w");

	if (file == NULL)
		return 1;
	
	bytes_written = fwrite(cik, sizeof(char), 40, file);

	if (bytes_written != 40)
		return 2;

	return 0;
}


/*!
 * \brief Retrieves the cik from non-volatile storage.
 *
 * \param[out] pointer to 40 byte buffer into which you put the CIK.
 *
 * \return 0 if successful , 1 if no CIK has been saved, >1 if fatal error
 */
uint8_t exopal_retrieve_cik(char *cik)
{
	size_t bytes_read;
	FILE *file;
	file = fopen("cik", "r");

	if (file == NULL)
		return 1;
	
	bytes_read = fread(cik, sizeof(char), 40, file);

	if (bytes_read != 40)
		return 2;

	return 0;
}


uint64_t exopal_get_time()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
}

void exopal_set_time(uint64_t timestamp_us)
{
	return;
}
