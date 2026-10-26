////////////////////////////////////////////////////////////////////////////////
///
/// A header file for detecting the Intel MMX instructions set extension.
///
/// Please see 'mmx_win.cpp', 'mmx_cpp.cpp' and 'mmx_non_x86.cpp' for the 
/// routine implementations for x86 Windows, x86 gnu version and non-x86 
/// platforms, respectively.
///
/// Author        : Copyright (c) Olli Parviainen
/// Author e-mail : oparviai 'at' iki.fi
///
////////////////////////////////////////////////////////////////////////////////
//
// Last changed  : $Date: 2006/02/05 16:44:06 $
// File revision : $Revision: 1.4 $
//
// $Id: cpu_detect.h,v 1.4 2006/02/05 16:44:06 Olli Exp $
//
////////////////////////////////////////////////////////////////////////////////
//
// License :
//
//  SoundTouchZ audio processing library
//  Copyright (c) Olli Parviainen
//
//  This library is licensed for commercial use of Bizarre Creations ltd
//  as defined in the executed license agreement. Please see document
//  "SoundTouchZ-license-Bizarre Creations.pdf" for further details.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _CPU_DETECT_H_
#define _CPU_DETECT_H_

#include "STTypes.h"

#define SUPPORT_MMX         0x0001
#define SUPPORT_3DNOW       0x0002
#define SUPPORT_ALTIVEC     0x0004
#define SUPPORT_SSE         0x0008
#define SUPPORT_SSE2        0x0010

/// Checks which instruction set extensions are supported by the CPU.
///
/// \return A bitmask of supported extensions, see SUPPORT_... defines.
uint detectCPUextensions(void);

/// Disables given set of instruction extensions. See SUPPORT_... defines.
void disableExtensions(uint wDisableMask);

#endif  // _CPU_DETECT_H_
