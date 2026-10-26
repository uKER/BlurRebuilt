#ifndef MAIN_H
#define MAIN_H

#pragma once

#include "Samples/System/PS3Application.h"

namespace Samples
{

class C_Sample : public C_PS3Application
{
public:

	C_Sample();
	~C_Sample();

	bool MyCreate();
};

}; // namespace Samples

#endif // MAIN_H
