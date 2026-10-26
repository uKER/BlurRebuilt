#ifndef MAIN_H
#define MAIN_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/System/XB2Application.h"

namespace Samples
{

class C_Sample : public C_XB2Application
{
public:

	C_Sample();
	~C_Sample();

	bool MyCreate();
};

}; // namespace Samples

#endif // MAIN_H
