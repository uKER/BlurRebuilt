#ifndef RENDER_POSTPROCESS_GREYSCALE_H
#define RENDER_POSTPROCESS_GREYSCALE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Interface.h>

namespace PostProcess
{

class C_Greyscale : public I_PostProcess
{
public:

	C_Greyscale( C_Manager& manager ) : I_PostProcess( manager ) {}
	~C_Greyscale() {}

	bool Create();
	void Destroy();
	bool CreateVolatile() { return true; }
	void DestroyVolatile() {}
	void Update() {}

	void Display() const;
	
private:
	Display::C_PixelShader		m_Shader;

};

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_GREYSCALE_H
