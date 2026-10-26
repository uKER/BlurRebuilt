//========================================================================================
//  
//  $File: //ai/ai13/devtech/sdk/public/samplecode/common/includes/Suites.hpp $
//
//  $Revision: #2 $
//
//  Copyright 1987-2007 Adobe Systems Incorporated. All rights reserved.
//  
//  NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance 
//  with the terms of the Adobe license agreement accompanying it.  If you have received
//  this file from a source other than Adobe, then your use, modification, or 
//  distribution of it requires the prior written permission of Adobe.
//  
//========================================================================================

#ifndef __Suites_hpp__
#define __Suites_hpp__

#include "ASTypes.h"

/*
	A dumb "acquire all these suites when I instantiate a Suites object and
	release them when it's destroyed" class.
*/

typedef struct
{
	char *name;
	int version;
	void *suite;
} ImportSuite;

// The fAcquiredCount field is not necessarily maintained across unload/reloads;
// in fact it probably isn't.  Whenever we first create a Suites object after
// being loaded or reloaded, it's important to call InitializeRefCount on it.

class Suites
{
public:
	Suites();
	~Suites();
	ASErr Error() { return fError; }

	void InitializeRefCount() { fAcquiredCount = 1; }

private:
	static int fAcquiredCount;
	ASErr fError;

	ASErr AcquireSuites();
	ASErr ReleaseSuites(); 
	ASErr AcquireSuite(ImportSuite *suite);
	ASErr ReleaseSuite(ImportSuite *suite);
};

#endif