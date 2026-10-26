// ***************************************************************
//  NetDatagramPacketTypes   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Lists all the possible datagram packet types we will be 
//			sending.
// ***************************************************************
 

#ifndef __NET_DATAGRAM_PACKET_TYPES_H__
#define __NET_DATAGRAM_PACKET_TYPES_H__

#ifdef _MSC_VER
#pragma once
#endif

namespace RacerNet
{
	/// base enum for packet types that will be sent internally or all games will require.
	enum
	{
		eNetMessageTypeReliableAcknowlegement = 1,		/**< acknowledgement to receiving a reliable message. */	
		eNetMessageTypeKeepAlive,						/**< keepalive packet received. */
				
		eNetMessageTypeJoinRequest,						/**< received request to join the session, only host should get this. */
		eNetMessageTypeJoinReply,						/**< host sends this back in response to receiving eNetMessageTypeJoinRequest. */

		eNetMessageTypeMax,								
	};

} // end namespace RacerNet

#endif // __NET_DATAGRAM_PACKET_TYPES_H__
