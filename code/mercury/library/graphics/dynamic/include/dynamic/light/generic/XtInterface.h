#ifndef RENDER_DYNAMICEFFECT_LIGHT_XTINTERFACE_H
#define RENDER_DYNAMICEFFECT_LIGHT_XTINTERFACE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Dynamic/Generic/XtInterface.h"
#include "Dynamic/Light/Enums.h"

namespace DynamicEffect
{

class C_Renderer;
	
namespace Light
{

class C_Manager;
class I_EffectBase;

struct LightDefinition;

namespace XtInterface
{

	struct Effect
	{
		Effect() : bInitialised(false), pEffect(0), id(0) {}

		bool													bInitialised;
		I_EffectBase*											pEffect;
		XtU32													id;

		XtSz8													name;
		E_Shape													shapeType;
		XtSz8													texture;
		DynamicEffect::XtInterface::U32Dim2						atlasSize;
		F32														cullDistance;

		XtArray<DynamicEffect::XtInterface::HermiteKeyFrame>	viewCurve;
		XtArray<DynamicEffect::XtInterface::HermiteKeyFrame>	distanceCurve;
		XtArray<DynamicEffect::XtInterface::HermiteKeyFrame>	positionCurve;
	};

	struct Instance
	{
		XtHandle<Effect>	effect;
		XtVec3				transform0;
		XtVec3				transform1;
		XtVec2				size;
		F32					intensity;
		U32					textureId;
	};

	struct Light
	{
		Light() : bInitialised(false), pDefinition(0), id(0) {}

		bool				bInitialised;
		LightDefinition*	pDefinition;
		XtU32				id;

		XtSz8				name;
		XtVec3				size;
		E_Occlusion			occlusion;
		XtArray<Instance>	instances;
	};

	void				Initialise(C_Manager* pManager);
	void				Shutdown();
	
	void				Update();
	void				Reset();

	LightDefinition*	FindLight(const Util::C_String& name);
};

}; // namespace Light
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_LIGHT_XTINTERFACE_H
