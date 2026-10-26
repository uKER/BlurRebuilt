#ifndef TEST_SECTION_AUDIO_H
#define TEST_SECTION_AUDIO_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>
#include "Audio.h"

class C_TestSectionAudio : public Framework::C_TestSection
{
public:

	C_TestSectionAudio(const Util::C_String& name);
	~C_TestSectionAudio();

	bool OnCreate();
	void OnDestroy();

	static void GetDatPathFn(const char* pchBuf, U32 uBufLen);
	void SetupPath();

	void OnRender2DView();

	Audio::ISystem* m_pSystem;
	Audio::IWaveManager* m_pWaveManager;

	static Util::C_String m_strPath;
};

#endif // TEST_SECTION_AUDIO_H
