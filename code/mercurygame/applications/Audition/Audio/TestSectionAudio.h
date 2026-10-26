#ifndef TEST_SECTION_AUDIO_H
#define TEST_SECTION_AUDIO_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>
#include "Audio.h"
#include <System/Debug/Log/DebugLog.h>

#include "AppInterfaceLayer.h"

namespace Audio { struct C_XtAudioDb; }

struct AudioEntityDebug
{
	Util::C_String					strName;
	Debug::Function*				pDebugFunc;
	Audio::EntityDefinitionHandle	hEntityDef;
};

class C_AudioSectionErrorHandler : public Debug::SubscriberCommon
{
public:
				C_AudioSectionErrorHandler() {}
 
				~C_AudioSectionErrorHandler() {}
 
	void		Log(Debug::LogEvent *pEvent); 

	void        LogString(const Util::C_String &string) {}
};

class C_TestSectionAudio : public Framework::C_TestSection
{
public:

	C_TestSectionAudio(const Util::C_String& name);
	~C_TestSectionAudio();

	bool OnCreate();
	void OnDestroy();
	void OnUpdate();

	static void GetDatPathFn(const char* pchBuf, U32 uBufLen);
	void SetupPath();
	
	void SortEntities();

	void OnRender2DView();

	C_AudioSectionErrorHandler* m_pErrorHandler;

	Audio::ISystem* m_pSystem;
	Audio::IWaveManager* m_pWaveManager;

	static Util::C_String m_strPath;

	Ut::Vector<AudioEntityDebug*> m_vpEntityDefDebug;
};

#endif // TEST_SECTION_AUDIO_H
