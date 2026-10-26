#ifndef SVN_STDAFX_H
#define SVN_STDAFX_H

#define NO_MERCURY_ASSERT

#define WINVER			0x0501
#define _WIN32_WINNT	0x0501

#include <winsock2.h>
#include <windows.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <svn_client.h>
#include <svn_cmdline.h>
#include <svn_pools.h>
#include <svn_path.h>

#include <cstdlib>
#include <string>

#include <Core/Core.h>

#pragma warning(disable:4355)	// This used in initialiser list
#pragma warning(disable:4706)	// Assignment within conditional expression
#pragma warning(disable:4200)	// Zero sized array in struct
#pragma warning(disable:4345)	// An object of POD type constructed with an initializer of the form () will be default-initialized
#pragma warning(disable:4512)	// Assignment operator could not be generated

#endif // SVN_STDAFX_H