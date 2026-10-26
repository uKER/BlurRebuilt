#ifndef SAMPLES_SECTIONRESOURCEBUNDLER_H
#define SAMPLES_SECTIONRESOURCEBUNDLER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Samples/Section/Section.h>
#include <Display/Bundler/I_ResourceFactory.h>
#include <Display/Bundler/I_ResourceBundle.h>
#include <Display/Bundler/ResourceBundleSingle.h>

namespace Samples
{

class C_SectionResourceBundler : public C_Section
{
public:

	C_SectionResourceBundler(const Util::C_String& name);
	~C_SectionResourceBundler();

	bool OnCreate();
	void OnDestroy();

	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	S32												m_TextureIndex;
	Util::C_String									m_BundleName;
	Display::ResourceBundler::I_ResourceBundle*		m_pBundle;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONRESOURCEBUNDLER_H
