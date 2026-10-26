#ifndef RENDER_POSTPROCESS_BOXBLUR_H
#define RENDER_POSTPROCESS_BOXBLUR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Interface.h>

namespace PostProcess
{

class C_BoxBlur : public I_PostProcess
{
public:

	C_BoxBlur( C_Manager& manager ) : I_PostProcess( manager ) { }
	~C_BoxBlur() {}

	bool Create();
	void Destroy();
	bool CreateVolatile() { return true; }
	void DestroyVolatile() {}
	void Update() {}

	void Display() const;
	
private:

};

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_GUASSIANBLUR_H
