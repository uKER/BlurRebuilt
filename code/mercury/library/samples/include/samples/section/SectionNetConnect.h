#ifndef SECTIONNETCONNECT_H
#define SECTIONNETCONNECT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"

namespace Render
{
	namespace Debug
	{
		class C_DebugRenderSys;
	}
}

namespace Samples
{

class C_SectionNetConnect : public C_Section
{
public:

	C_SectionNetConnect(const Util::C_String& name);
	~C_SectionNetConnect();
	
	void OnRender(const C_Input& input, const C_Timer& timer);
	void OnUpdate(const C_Input& input, const C_Timer& timer);
};

}; // namespace Samples



#endif //SECTIONNETCONNECT_H