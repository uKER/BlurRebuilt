// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once


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

		#pragma warning(disable:4355)	// This used in initialiser list
		#pragma warning(disable:4706)	// Assignment within conditional expression
		#pragma warning(disable:4345)	// An object of POD type constructed with an initializer of the form () will be default-initialized
