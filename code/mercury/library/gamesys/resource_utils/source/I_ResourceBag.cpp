#include "ResourceUtilsPch.h"
#include <resource_utils/I_ResourceBag.h>

namespace ResourceUtils
{

I_ResourceBag::~I_ResourceBag()
{

}

#ifndef CFG_GOLD
void I_ResourceBag::DebugOutputBagContents(void) const
{

}
#endif



} // namespace ResourceUtils
