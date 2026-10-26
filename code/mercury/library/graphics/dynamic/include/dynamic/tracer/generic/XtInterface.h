#ifndef RENDER_DYNAMICEFFECT_TRACER_XTINTERFACE_H
#define RENDER_DYNAMICEFFECT_TRACER_XTINTERFACE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Dynamic/Generic/XtInterface.h"


namespace DynamicEffect
{

class C_Renderer;
	
namespace Tracer
{

class I_EffectBase;

namespace XtInterface
{
	void			Initialise(C_Manager* pManager);
	void			Shutdown();
	
	void			Update();
	void			Destroy();

	I_EffectBase* FindEffect(const Util::C_String& name);
};
}; // namespace Tracer
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_TRACER_XTINTERFACE_H
