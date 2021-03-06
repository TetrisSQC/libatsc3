/** \file alc_rx.h \brief ALC level receiving
 *
 *  $Author: peltotal $ $Date: 2007/02/26 13:48:19 $ $Revision: 1.33 $
 *
 *  MAD-ALCLIB: Implementation of ALC/LCT protocols, Compact No-Code FEC,
 *  Simple XOR FEC, Reed-Solomon FEC, and RLC Congestion Control protocol.
 *  Copyright (c) 2003-2007 TUT - Tampere University of Technology
 *  main authors/contacts: jani.peltotalo@tut.fi and sami.peltotalo@tut.fi
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  In addition, as a special exception, TUT - Tampere University of Technology
 *  gives permission to link the code of this program with the OpenSSL library (or
 *  with modified versions of OpenSSL that use the same license as OpenSSL), and
 *  distribute linked combinations including the two. You must obey the GNU
 *  General Public License in all respects for all of the code used other than
 *  OpenSSL. If you modify this file, you may extend this exception to your version
 *  of the file, but you are not obligated to do so. If you do not wish to do so,
 *  delete this exception statement from your version.
 */

#include <stdbool.h>

#include "atsc3_utils.h"
#include "atsc3_lct_hdr.h"


#ifndef _ALC_RX_H_
#define _ALC_RX_H_

// #define __ALC_RX_ENABLE_DEBUG 1
// #define __ALC_RX_ENABLE_TRACE 1

#define println(...) printf(__VA_ARGS__);printf("\r\n")

#define __ALC_RX_PRINTLN(...) printf(__VA_ARGS__);printf("\r\n")
#define __ALC_RX_PRINTF(...)  printf(__VA_ARGS__);

#define ALC_RX_ERROR(...)   printf("%s:%d:ERROR:",__FILE__,__LINE__);__ALC_RX_PRINTLN(__VA_ARGS__);
#define ALC_RX_WARN(...)    printf("%s:%d:WARN:",__FILE__,__LINE__);__ALC_RX_PRINTLN(__VA_ARGS__);
#define ALC_RX_INFO(...)    printf("%s:%d:INFO:",__FILE__,__LINE__);__ALC_RX_PRINTLN(__VA_ARGS__);

#ifdef __ALC_RX_ENABLE_DEBUG

#define ALC_RX_DEBUG(...)   printf("%s:%d:DEBUG:",__FILE__,__LINE__);__ALC_RX_PRINTLN(__VA_ARGS__);
#define ALC_RX_DEBUGF(...)  printf("%s:%d:DEBUG:",__FILE__,__LINE__);__ALC_RX_PRINTF(__VA_ARGS__);
#define ALC_RX_DEBUGA(...) 	__PRINTF(__VA_ARGS__);
#define ALC_RX_DEBUGN(...)  __PRINTLN(__VA_ARGS__);
#else
#define ALC_RX_DEBUG(...)
#define ALC_RX_DEBUGF(...)
#define ALC_RX_DEBUGA(...)
#define ALC_RX_DEBUGN(...)
#endif

#ifdef __ALC_RX_ENABLE_TRACE
#define ALC_RX_TRACE(...)   printf("%s:%d:TRACE:",__FILE__,__LINE__);__ALC_RX_PRINTLN(__VA_ARGS__);
#else
#define ALC_RX_TRACE(...)
#endif


#include "defines.h"
#include "transport.h"
#include "alc_session.h"

#include "alc_channel.h"
#include "atsc3_lct_hdr.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct alc_packet {
	atsc3_def_lct_hdr_t* def_lct_hdr;
	uint8_t fec_encoding_id;
    
    //for fec_encoding_id == 128, raptor fec
    bool use_sbn_esi;
    uint8_t sbn;	//sbn: source block number for fec recovery
	uint32_t esi; 	//esi: encoding symbol id, our 24bit offset
    
    //for all other fec values
    bool use_start_offset;
    uint32_t start_offset;
    
    uint8_t close_object_flag;
    uint8_t close_session_flag;
    
    unsigned int alc_len;
    unsigned long long transfer_len;

	uint8_t* alc_payload;

} alc_packet_t;

void alc_packet_free(alc_packet_t** alc_packet_ptr);

int alc_rx_analyze_packet_a331_compliant(char *data, int len, alc_channel_t *ch, alc_packet_t** alc_packet_ptr);

/** 
 * This function checks if a received unit belongs to an object which already exists in the session or not. 
 *
 * @param toi transport object identifier
 * @param s pointer to the session
 * @param type of object to be checked (0 = FDT Instance, 1 = normal object)
 *
 * @return pointer to the object in success, NULL otherwise 
 *
 */

trans_obj_t* object_exist(unsigned long long toi, alc_session_t *s, int type);

/**
 * This function checks if the object has been received completely.
 *
 * @param to pointer to the transport object
 *
 * @return TRUE when object is received completely, FALSE otherwise
 *
 */

BOOL object_completed(trans_obj_t *to);

/**
 * This function checks if the block is ready for decoding.
 *
 * @param tb pointer to the block
 *
 * @return TRUE when block is ready for decoding, FALSE otherwise
 *
 */

BOOL block_ready_to_decode(trans_block_t *tb);

/**
 * This function receives packets from the socket and stores them on the list.
 * 
 * @param ch pointer to the channel
 *
 */

void* rx_socket_thread(void *ch);

/**
 * This function waits until channel's rx_socket_thread is completed.
 * 
 * @param ch pointer to the channel
 *
 */

//void  join_rx_socket_thread(alc_channel_t *ch);

/**
 * This function receives packets from the all channels in the session.
 * 
 * @param s pointer to the session	
 *
 */

void* rx_thread(void *s);

/**
 * This function gives an object to application, when it is completely received.
 *
 * @param s_id session identifier
 * @param toi transport object identifier (identifies an object which is wanted to receive)
 * @param data_len stores the object length
 * @param retval stores return value in error cases/stopping situations
 *
 * @return pointer to buffer which contains object's data, NULL in error cases/stopping situations
 *
 */

char* alc_recv(int s_id, unsigned long long toi, unsigned long long *data_len, int *retval);

/**
 * This function gives any object to application, when it is completely received.
 *
 * @param s_id session identifier
 * @param toi stores transport object identifier
 * @param data_len stores the object length
 * @param retval stores return value in error cases/stopping situations
 *
 * @return pointer to buffer which contains object's data, NULL in error cases/stopping situations
 *
 */

char* alc_recv2(int s_id, unsigned long long *toi, unsigned long long *data_len, int *retval);

/**
 * This function gives temporary filename of an object to the application, when object is completely received.
 *
 * @param s_id session identifier
 * @param toi stores transport object identifier
 * @param retval stores return value in error cases/stopping situations
 *
 * @return pointer to buffer which contains temporary filename, NULL in error cases/stopping situations
 *
 */

char* alc_recv3(int s_id, unsigned long long *toi, int *retval);

/**
 * This function gives an FDT Instance to application, when it is completely received.
 *
 * @param s_id session identifier
 * @param data_len stores FDT instance length
 * @param retval stores return value in error cases/stopping situations
 * @param content_enc_algo stores used content encoding
 * @param fdt_instance_id stores FDT instance id
 *
 * @return pointer to buffer which contains FDT Instance's data, NULL in error cases/stopping situations
 *
 */

char* fdt_recv(int s_id,
	       unsigned long long *data_len,
	       int *retval,
	       unsigned char *content_enc_algo,
	       int *fdt_instance_id);





#ifdef __cplusplus
}; //extern "C"
#endif

#endif

