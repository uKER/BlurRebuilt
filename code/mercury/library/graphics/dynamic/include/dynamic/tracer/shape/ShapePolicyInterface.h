#ifndef RENDER_DYNAMICEFFECT_TRACER_SHAPEPOLICYINTERFACE_H
#define RENDER_DYNAMICEFFECT_TRACER_SHAPEPOLICYINTERFACE_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace DynamicEffect
{
namespace Tracer
{

struct RenderSettings;

class I_ShapePolicy
{
public:
	I_ShapePolicy() {};
	virtual ~I_ShapePolicy() {}

	void SetRenderSettings(const RenderSettings* pRenderSettings) { m_pRenderSettings = pRenderSettings; }

protected:
	const RenderSettings* m_pRenderSettings;

};

}; // namespace Tracer
}; // namespace DynamicEffect



#endif // RENDER_DYNAMICEFFECT_TRACER_SHAPEPOLICYINTERFACE_H
