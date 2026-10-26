#ifndef MAIN_H
#define MAIN_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/System/PCApplication.h"

namespace Samples
{

class C_Sample : public C_PCApplication
{
public:

	C_Sample();
	~C_Sample();

	bool MyCreate();
	void MyDestroy();
	void MyUpdate();
};

}; // namespace Samples

#endif // MAIN_H
