#include "StdAfx.h"
#include "Main.h"
#include <TestSections/TestSectionLayer.h>
#include "TestSections/TestSectionTemplate.h"
#include <TestSections/TestSectionSkyDome.h>

#include <Network/Core.h>
#include <SystemProfile/SystemProfileManager.h>
#include <Network/PlayerSkillCombiner.h>

#if defined (PLATFORM_PS3)
#include <cell/sysmodule.h>
#include <np.h>

// nicked this from the np sample code, 
// we will need to request a correct communication id for the game
SceNpCommunicationId communication_id = {
	{'N','P','X','S','0','0','0','0','5'},
	'\0',
	0,
	0
};

static bool sQuitApp = false;

static void ps3NpManagerCallback(int event, int result, void *arg)
{
	SysProfile::C_Manager::Instance().NpManagerCallback(event, result, arg);
}

static int ps3NpBasicEventHandler(int event, int retCode, uint32_t reqId, void *arg)
{
	SysProfile::C_Manager::Instance().NpBasicEventHandler(event, retCode, reqId, arg);
	return 0;
}

void SysutilCallback(uint64_t status, uint64_t param, void *userdata)
{
	switch (status)
	{
	case CELL_SYSUTIL_REQUEST_EXITGAME: // exit the game asap..
		sQuitApp = true;
		break;

	default:
		break;
	}
}
#endif


C_TestApp::C_TestApp() : m_NetworkStackStartedExternally(false)
{
}

C_TestApp::~C_TestApp()
{
}

void C_TestApp::SetupPlatformCallbacks(void)
{
#if defined (PLATFORM_PS3)
	S32 sysutil = cellSysutilRegisterCallback(0, SysutilCallback, NULL);
	assert(sysutil == 0);
	S32 manager = sceNpManagerRegisterCallback(ps3NpManagerCallback, NULL); // connection changes callback
	assert(manager == 0);
	S32 basic = sceNpBasicRegisterContextSensitiveHandler(&communication_id, ps3NpBasicEventHandler, NULL); // friends management callback.
	assert(basic == 0);
#endif
}

HRESULT C_TestApp::PlatformInitialise(const Framework::C_ApplicationParams& params)
{
	if (Framework::C_ApplicationBase::PlatformInitialise(params) != S_OK)
		return E_FAIL;

	m_NetworkStackStartedExternally = params.m_NetworkStackAlreadyStarted;

#if defined (PLATFORM_PS3)
	if (!m_NetworkStackStartedExternally)
		cellSysmoduleLoadModule(CELL_SYSMODULE_NET);
	cellSysmoduleLoadModule(CELL_SYSMODULE_NETCTL);
	cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL);
	cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP2);
	cellSysmoduleLoadModule(CELL_SYSMODULE_L10N);
#endif
	bool ok = true;

	NetPlay::C_Core::Instance().Initialise(true, m_NetworkStackStartedExternally);
	if (NetPlay::C_Core::Instance().IsInitialised())
	{
		SetupPlatformCallbacks();
		ok = ok && SysProfile::C_Manager::Instance().Initialise(); // no age requirement.
	}
	else
		ok = false;

//	NetPlay::C_PlayerSkillCombiner<20> skillCombiner;
//	NetPlay::PlayerSkill skills[3];
//	NetPlay::PlayerSkill res = skillCombiner.Combine(skills, 3);

	return ok ? S_OK : E_FAIL;
}


void C_TestApp::PlatformShutdown()
{
	SysProfile::C_Manager::Instance().Shutdown();
	NetPlay::C_Core::Instance().Shutdown();

#if defined (PLATFORM_PS3)
	cellSysmoduleUnloadModule(CELL_SYSMODULE_L10N);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP2);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_NETCTL);
	if (!m_NetworkStackStartedExternally)
		cellSysmoduleUnloadModule(CELL_SYSMODULE_NET);
#endif

	Framework::C_ApplicationBase::PlatformShutdown();
}


HRESULT C_TestApp::OnInitialise()
{
	Framework::C_TestSectionLayer* pTestSectionsLayer = new Framework::C_TestSectionLayer();
	InsertLayer(pTestSectionsLayer);

	//pTestSectionsLayer->AddTestSection<C_TestSectionTemplate>("Template");

	pTestSectionsLayer->AddTestSection<Framework::C_TestSectionSkyDome>("SkyDome");

	return S_OK;
}

void C_TestApp::OnShutdown()
{
}

void C_TestApp::OnUpdate()
{
#if defined (PLATFORM_PS3)
	//fire off any syatem callbacks that may have data waiting..
	cellSysutilCheckCallback();
#endif

#if defined (PLATFORM_PS3)
	// quit the application ?
	m_Quit = sQuitApp;
#endif

	NetPlay::C_Core::Instance().Poll(false);

	SysProfile::GameInvite inv;
	SysProfile::Notifications notifications;
	SysProfile::C_Manager::Instance().Poll(inv, notifications);
		
	CommonUpdate();
}

void C_TestApp::OnRender()
{
	if(BeginRender())
	{
		CommonRender();

		EndRender();
	}
}

#include <Framework/Application/ApplicationMain.inl>
