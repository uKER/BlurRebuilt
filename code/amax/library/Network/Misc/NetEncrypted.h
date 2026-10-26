// ***************************************************************
//  NetEncrypted   version:  1.0   ·  date: 08/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	functions for reading and writing the encrypted bytes
//			variable in the net packet headers.
// ***************************************************************

#ifndef __NET_ENCRYPTED_H__
#define __NET_ENCRYPTED_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <bdCore/bdContainers/bdBitBuffer.h>

namespace RacerNet
{
	/// Generate the correct value of encrypted bytes
	inline void EncryptedSize(U16& out, const U16 size)
	{
		bdBitBuffer::endianSwap(size, out);
	}

} // end namespace RacerNet

#endif // __NET_ENCRYPTED_H__
