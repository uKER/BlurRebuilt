#ifndef SAMPLES_SECTIONPCGL_H
#define SAMPLES_SECTIONPCGL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"

namespace Samples
{

class C_SectionPCGL : public C_Section
{
public:

	C_SectionPCGL(const Util::C_String& name);
	~C_SectionPCGL();

	void OnUpdate(const C_Input& input, const C_Timer& timer);
	void OnRender(const C_Input& input, const C_Timer& timer);

protected:

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

private:

	CGparameter		m_cgClipVector;
	CGprogram		m_cgVertexShader;
	CGprogram		m_cgPixelShader;
	GLuint			m_glVertexBuffer;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONPCGL_H
