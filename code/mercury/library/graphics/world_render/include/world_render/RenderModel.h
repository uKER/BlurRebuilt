#ifndef WORLD_RENDER_MODEL_RENDER_H
#define WORLD_RENDER_MODEL_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <world_render/RenderModeldata.h>
#include <renderingdata/EffectVariationCallback.h>

namespace WorldRender
{

class C_RenderModel : public C_RenderModelData
{
public:
	C_RenderModel();
	~C_RenderModel();
	
	bool Read( Sys::I_File* pFile, const Util::C_String& paramType );

	void OnLoad(RenderingData::Target::EffectVariationCallbackFunc func);
	

private:
	

	
};


} // namespace WorldRender


#endif // WORLD_RENDER_MODEL_RENDER_H

