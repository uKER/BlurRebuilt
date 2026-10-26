// ***************************************************************
//  Nonce   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Random data buffer used in LAN game enumeration.
// ***************************************************************

#ifndef NETWORK_NONCE_H
#define NETWORK_NONCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <bdCore/bdUtilities/bdTrulyRandom.h> 

// larger values means less chance of conflicting with other consoles..
#define NETPLAY_NONCE_BYTES	8	


namespace NetPlay
{
	/// Used to define a unique data buffer.
	struct Nonce
	{
		/// get the size of the nonce.
		/** @return the size opf the nonce in bytes.
		 */
		const U16 Size(void) const {return NETPLAY_NONCE_BYTES;}

		/// Create a new nonce.
		/** @return true if nonce was created ok.
		 */
		bool Create(void)
		{
			bdTrulyRandom::getInstance()->getRandomUByte8(m_Nonce, Size());
			return true;
		}

		// overloaded operators..
		bool operator == (const Nonce& other) const {return (memcmp(m_Nonce, other.m_Nonce, NETPLAY_NONCE_BYTES) == 0)? true : false;} 
		bool operator != (const Nonce& other) const {return !((*this) == other); } 

		Nonce& operator = ( const Nonce& other) { memcpy( m_Nonce, other.m_Nonce, sizeof(U8)*NETPLAY_NONCE_BYTES ); return *this; }

		// nonce data
		U8	m_Nonce[NETPLAY_NONCE_BYTES];
	};
}



#endif


