#ifndef CORE_COREPCH_H
#define CORE_COREPCH_H

#ifdef _MSC_VER 
#pragma once
#endif //end _MSC_VER

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0501

// TODO: reference additional headers your program requires here

// Playstation 3 specific includes
#ifdef PLATFORM_PS3 
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memset
#include <ctype.h> 
#include <libsn.h>  // for SN-System's functions
#include <stdint.h>
#endif // end PLATFORM_PS3

#include <core/core.h>
#include <core/maths.h>
#include <core/utility/utility.h>

#include <core/BizAssert.h>



#endif // CORE_COREPCH_H
