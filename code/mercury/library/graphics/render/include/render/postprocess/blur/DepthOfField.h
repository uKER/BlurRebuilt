#ifndef RENDER_POSTPROCESS_DEPTHOFFIELD_H
#define RENDER_POSTPROCESS_DEPTHOFFIELD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Interface.h>

namespace PostProcess
{

class C_DepthOfField : public I_PostProcess
{
public:
	C_DepthOfField( C_Manager& manager ) : I_PostProcess( manager ) {}
	~C_DepthOfField() {}

	bool Create();
	void Destroy();
	bool CreateVolatile() { return true; }
	void DestroyVolatile() {}
	void Update() {}

	void Display() const;
	
protected:
	mutable F32 m_blurScale;
	mutable F32 m_focusInvDepth; // (1/depth, e.g. 0.25 to focus on a point 4 metres away)
	
private:

	Display::C_PixelShader m_Shader;
};

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_DEPTHOFFIELD_H
