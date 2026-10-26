// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define AFTKXML_CHECK_ERRORS

#include <iostream>
#include <tchar.h>

#include <aftk/aftkStdafx.h>
#include <aftk/aftkHeaders.h>

#ifdef  AFTKXML_CHECK_ERRORS
#define VALIDATE_ALLOC_RET_INT(PTR)\
	if (PTR == 0)\
	{\
		std::cout << "Alloc failed" << __FILE__ << __LINE__ << std::endl;\
		return 0;\
	}
#else 
	#define VALIDATE_ALLOC_RET_INT(PTR)
#endif

#ifdef  AFTKXML_CHECK_ERRORS
#define VALIDATE_ALLOC(PTR)\
	if (PTR == 0)\
	{\
		std::cout << "Alloc failed" << __FILE__ << __LINE__ << std::endl;\
		stat = AftkStatus::OUT_OF_MEMORY;\
		return;\
	}
#else 
	#define VALIDATE_ALLOC(PTR)
#endif