// ***************************************************************
//  NetDebug   version:  1.0   ·  date: 07/02/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Used to turn off optimizations for specific network 
//			source files when compiled by specific users.
// ***************************************************************

#ifndef __NET_DEBUG_H__
#define __NET_DEBUG_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Core.h>


#if defined ( USER_DWORSWICK ) 
#if defined (CFG_ALLOW_OPTIMIZE_OFF) && !defined ( CFG_GOLD )
	#define RACER_NET_OPTIMIZE_OFF
#endif 
#endif 


#endif // __NET_DEBUG_H__
