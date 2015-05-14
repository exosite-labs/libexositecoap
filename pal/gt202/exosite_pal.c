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
#include "atheros_stack_offload.h"
#include "mqx.h"

#if 1
#include "coap.h"
#endif

#define PAL_CIK_LENGTH 40

static int exosock;

static const char exosite_pal_host[] = "coap.exosite.com";
static const int exosite_pal_port = 5683;

SOCKADDR_T remote_addr;
extern _enet_handle handle;

int errno;


#define GET_BYTE(i, x) (( x >> (i*8)) & 0xFF)

    #if !BSPCFG_ENABLE_FLASHX
    #error This application requires BSPCFG_ENABLE_FLASHX defined non-zero in user_config.h. Please recompile BSP with this option.
    #endif
    #define         FLASH_NAME "flashx:bank0"

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
  uint32_t exoip;
  DNC_CFG_CMD dns_cfg;
  DNC_RESP_INFO  dns_resp;
  memset(&dns_resp,0,sizeof(dns_resp));
  
  //if(is_driver_initialized() != A_OK){
  //  return 250;
  //}
  
  if((exosock = t_socket((void*)handle, ATH_AF_INET, SOCK_DGRAM_TYPE, 0)) == A_ERROR) {
    return 3;
  }
  
  strcpy((char *)dns_cfg.ahostname,exosite_pal_host);
  dns_cfg.domain = 2;
  dns_cfg.mode =  GETHOSTBYNAME;
  
  if (A_OK != custom_ip_resolve_hostname((void *)handle, &dns_cfg, &dns_resp)) {
    printf("Unable to resolve host name\r\n");
    return 250;
  }
  
  
  exoip = A_CPU2BE32(dns_resp.ipaddrs_list[0]);
  
  if(dns_resp.dns_names[0] != 0) {
    printf("host: name=%s\r\n",dns_resp.dns_names);
  }
  
  printf("addrtype %d \r\n", A_CPU2LE32(dns_resp.h_addrtype));
  printf("length %d \r\n", A_CPU2LE32(dns_resp.h_length));
  printf("addr:%d.%d.%d.%d \r\n",GET_BYTE(3, exoip), GET_BYTE(2, exoip), GET_BYTE(1, exoip), GET_BYTE(0, exoip));
  
  if(0){
    printf("WARNING: OVERIDING IP WITH LOCAL IP FOR DEBUGGING, TURN ON PROXY");
    exoip = (192 << 24) | (168 << 16) | (10 << 8) | (129 << 0);
    printf("addr:%d.%d.%d.%d \r\n",GET_BYTE(3, exoip), GET_BYTE(2, exoip), GET_BYTE(1, exoip), GET_BYTE(0, exoip));
  }
  
  printf("Connecting.\n");
  memset(&remote_addr, 0, sizeof(remote_addr));
  remote_addr.sin_addr = exoip;
  remote_addr.sin_port = exosite_pal_port;
  remote_addr.sin_family = ATH_AF_INET;

  /*Allow small delay to allow other thread to run*/
  //app_time_delay(TX_DELAY);
  
  if(t_connect((void*)handle, exosock, (&remote_addr), sizeof(remote_addr)) == A_ERROR) {
   printf("Conection failed.\n");
   return 4;
  }


/*
  if(send_result == A_ERROR) {
    printf("Sending Failed.\n");
    break;
  } else {
    printf("Sent.\n");
  }

  result = wait_for_response(p_tCxt, foreign_addr, foreign_addr6);

  if(result != A_OK){
    printf("UDP Transmit test failed, did not receive Ack from Peer\n");
  }
  else
  {
    p_tCxt->test_type = UDP_TX;
    print_test_results(p_tCxt);
  }
  */
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
  unsigned char *net_buf;
  int32_t send_result;
  
#if 1
  coap_pdu pdu;
  pdu.buf = (uint8_t*)buf; //eww gross
  pdu.len = len;
  pdu.max = len;
  
  printf(" <-- ");
  coap_pretty_print(&pdu);
#endif

  while((net_buf = CUSTOM_ALLOC(len)) == NULL) {_time_delay_ticks(1);}
  memcpy(net_buf, buf, len);
  
  send_result = t_sendto((void*)handle, exosock, net_buf, len, 0, &remote_addr, sizeof(remote_addr));
  
  CUSTOM_FREE(net_buf);
  
  if(send_result != len) {
    return 250;
  }
  
  return 0;
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
  unsigned char *net_buf;
  int32_t recv_bytes, addr_len;
  SOCKADDR_T addr;

  recv_bytes = t_recvfrom((void*)handle, exosock, (void**)&net_buf, size, 0, &addr, &addr_len);
  
  //todo: check addr is exosite
  
  if(recv_bytes > 0){
    *rlen = recv_bytes;
    memcpy(buf, net_buf, *rlen); //todo: check length
    zero_copy_free(net_buf);
  } else {
    *rlen = 0;
    return 250;
  }
  
  
#if 1
  coap_pdu pdu;
  pdu.buf = buf;
  pdu.len = *rlen;
  pdu.max = *rlen;
  
  printf(" --> ");
  coap_pretty_print(&pdu);
#endif
  
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
	uint32_t ioctl_param;
	size_t bytes_written;
	MQX_FILE_PTR file;
	file = fopen(FLASH_NAME, NULL);

	if (file == NULL)
		return 2;
        
	ioctl_param = 0;
	ioctl(file, FLASH_IOCTL_WRITE_PROTECT, &ioctl_param);
	
        // todo: make this actually work
	bytes_written = write(file, cik, 40);

	fclose(file);
        
        if (bytes_written != 40) {
            printf("\nError writing CIK to the file. (%d)", _io_ferror(file));
            printf("\nYou will need to re-enable your device on Exosite on the next boot.");
            return 1;
        }

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
	MQX_FILE_PTR file;
	file = fopen(FLASH_NAME, NULL);

	if (file == NULL)
		return 1;
	
	bytes_read = read(file, cik, 40);

	fclose(file);
        
	if (bytes_read != 40)
		return 1;

	return 0;
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
  TIME_STRUCT ts;
  _time_get_elapsed(&ts);
  //printf("s: %d ms: %d\n", ts.SECONDS, ts.MILLISECONDS);
  return (ts.SECONDS * 1000000) + (ts.MILLISECONDS * 1000);
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
