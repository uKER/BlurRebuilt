#ifndef TEST_SECTION_TEMPLATE_H
#define TEST_SECTION_TEMPLATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>
#include <Misc_Shared/Compression/Deflate.h>
#include <Misc_Shared/Compression/Inflate.h>
#include <System/Scheduler/JobQueue.h>

class C_TestSectionCompression : public Framework::C_TestSection
{
public:

	C_TestSectionCompression(const Util::C_String& name);
	~C_TestSectionCompression();

	void OnUpdate();
	void OnRender2DView();

private:

	void Deflate();
	void Inflate();

private:

	Compression::C_Deflate	m_Deflate;
	Compression::C_Inflate	m_Inflate;
	U32						m_FileSize;

};

#endif // TEST_SECTION_TEMPLATE_H
