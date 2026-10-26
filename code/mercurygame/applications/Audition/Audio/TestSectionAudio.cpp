#include "stdafx.h"
#include "TestSectionAudio.h"
#include "XtAudio.h"
#include "AppInterfaceLayer.h"
#include <Render/font/Font.h>

#ifdef PLATFORM_XENON
#include "Xbdm.h"
#endif

#ifdef PLATFORM_PC
#ifdef TEST_PC_ITUNES_PLAYER
#include "itunescom\iTunesCOMInterface.h"
#include "itunescom\iTunesCOMInterface_i.c"
#endif
#endif

static Render::Font::C_FontRenderer*	g_pAudioFontRenderer = 0;

void EntityReleaseAll(void* pContext);

Util::C_String C_TestSectionAudio::m_strPath;

Debug::Logger& g_Log = Debug::GetLogger("Audition");

Debug::Bool gbDebugDumpWaveBanks("Audio.System.WaveBanks.WavebankDump", false);
Debug::String gAutoCreateEntity("Audio.Entities.EntityToAutoCreateOnStartup", "A_RaceDialogue_0", Debug::Persist());

Debug::Function func("Audio.Entities.DESTROY_ALL_INSTANCES", EntityReleaseAll, 0);

void AudioPrintString(F32 fX, F32 fY, U32 u32RGBA, F32 f32Size, char* strText);
void AudioDrawBox(F32 fXPos, F32 fYPos, F32 fXSize, F32 fYSize, U32 u32RGBA);

extern Debug::Float g_DebugHudSubscriberWindowHeight;
extern Debug::Float g_DebugHudSubscriberFirstMessageTimeout;

XT_APPLICATION
	XT_DEPENDS_ON(AudioXtLib)
XT_END_APPLICATION

#if !defined USER_JCUTTER
// sam temp
#define SETUP_ENTITIES
#define SETUP_OBJECT_DAT
#endif

#define SORT_ENTITY_NAMES

class TestSystem : public Audio::Subsystem
{
public:

	TestSystem() { }
	~TestSystem() { }

	bool		SendMessage(const U32 u32MessageId)
	{
		return true;
	}
	bool		SendStateTransition(const U32 u32OutgoingState, const U32 u32IncomingState)
	{
		return true;
	}
	void		Update()
	{
	}
};

TestSystem testSystem;

//------------------------------------------------------------------------------
//	AuditionEventHandler::Log
//  This simple log subscriber just halts execution if an error or fatal occurs
//==============================================================================
void C_AudioSectionErrorHandler::Log(Debug::LogEvent *pEvent) 
{ 
	if (pEvent->level != DebugLevel::Error && pEvent->level != DebugLevel::Fatal)
		return;

	#ifdef PLATFORM_PC
	if (IsDebuggerPresent())
		DebugBreak();
	#endif
	#ifdef PLATFORM_XENON
	if (DmIsDebuggerPresent())
		DebugBreak();
	#endif	
	#ifdef PLATFORM_PS3
	__asm__ volatile ( "tw 31,1,1" );
	#endif
}

//------------------------------------------------------------------------------
//	GetDatPathFn
//==============================================================================
void C_TestSectionAudio::GetDatPathFn(const char* pchBuf, U32 uBufLen) 
{ 
	strncpy_s((char*)pchBuf, uBufLen, m_strPath.Chars(), uBufLen); 
}

//------------------------------------------------------------------------------
//	SetupPath
//==============================================================================
void C_TestSectionAudio::SetupPath()
{
	#if defined (PLATFORM_PC)
	// get/set current path
	char pchCurrDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pchCurrDir);
	m_strPath = pchCurrDir;
	m_strPath += "\\Audio\\";
	#endif
	#if defined (PLATFORM_XENON)
	m_strPath = "GAME:\\Audio\\";
	#endif
	#if defined (PLATFORM_PS3)
	m_strPath = "/app_home/audio/";
	#endif
}

C_TestSectionAudio::C_TestSectionAudio(const Util::C_String& name) : Framework::C_TestSection(name)
{
	m_pErrorHandler = new C_AudioSectionErrorHandler;
	if (m_pErrorHandler)
		Debug::GetRootLogger().AddSubscriber( m_pErrorHandler );
	m_pSystem = NULL;
	m_pWaveManager = NULL;
}

C_TestSectionAudio::~C_TestSectionAudio()
{
}

enum Subsystems
{
	eTestSystem1 = 0,

	TotalNumSubsystems
};

void AudioPrintString(F32 fX, F32 fY, U32 u32RGBA, F32 f32Size, char* strText)
{
	Display::FontRender()->Draw(strText, fX, fY, f32Size, f32Size, Maths::RGBA(u32RGBA));
}

void AudioDrawBox(F32 fXPos, F32 fYPos, F32 fXSize, F32 fYSize, U32 u32RGBA)
{
	Display::BasicRender()->DrawColoured2DRect(fXPos, fYPos, fXSize, fYSize, Maths::RGBA(u32RGBA));
}

Btl::Vector<AudioEntityHandle> gActiveEntities;

//------------------------------------------------------------------------------
//	EntityActivateCallback
//	called when click on entry for entity in debugview/debug menu.
//	creates an instance of the entity and connects to renderer.
//	currently no way of destroying these entities
//==============================================================================
void EntityActivateCallback(void* pContext)
{
	if (pContext)
	{
		Audio::EntityDefinitionHandle* phDef = (Audio::EntityDefinitionHandle*)pContext;
		Audio::EntityDefinition* pDef = Audio::InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(*phDef);

		// calls "SetupObject" function. loads .dat files and banks. perform on entity resource caching
		AudioEntityTypeHandle hEntityType;
		Audio::InterfaceController::Get().CreateEntityType(*phDef, eTestSystem1, hEntityType);

		// create instance of the entity
		AudioEntityHandle hEntityInstance;
		bool bOk = Audio::InterfaceController::Get().CreateEntity(hEntityType, hEntityInstance);

		if (hEntityInstance.IsValid())
		{
			LOGINFO(g_Log, "Created instance of entity '%s' ok.", pDef->Name().Chars());	
			gActiveEntities.PushBack(hEntityInstance);
		}

		if (pDef && pDef->LookupInstanceComponent("OutputToRenderer") != 0xffffffff)
			Audio::InterfaceController::Get().SetMaster(hEntityInstance, "OutputToRenderer");
	}
}

void EntityReleaseAll(void* pContext)
{
	for (int i=0; i<gActiveEntities.Size(); i++)
	{
		bool bOk = Audio::InterfaceController::Get().ReleaseEntity(gActiveEntities[i]);
	}

	LOGINFO(g_Log, "Released all entities.");	

	gActiveEntities.Clear();
}

//------------------------------------------------------------------------------
//	C_TestSectionAudio::OnCreate
//==============================================================================
bool C_TestSectionAudio::OnCreate()
{
	// get the app path. we presume everything comes from the same path (ie bafs & dats)
	SetupPath();

	g_DebugHudSubscriberWindowHeight = 256.f;
	g_DebugHudSubscriberFirstMessageTimeout = 30.f;

#ifdef PLATFORM_PC
#ifdef TEST_PC_ITUNES_PLAYER
	HRESULT  hRes;
	IiTunes* iITunes;

	hRes = CoInitialize(NULL);

	// note - CLSID_iTunesApp and IID_IiTunes are defined in iTunesCOMInterface_i.c
	hRes = ::CoCreateInstance(CLSID_iTunesApp, NULL, CLSCTX_LOCAL_SERVER, IID_IiTunes, (PVOID *)&iITunes);

	iITunes->Play();

	//IWMPPlayer player;
	//IWMPCore core;

	//CComPtr<IWMPControls> spWMPControls;

	//hr = player.get_controls(&spWMPControls);

#endif
#endif

	//  create audio system
	if (m_pSystem == NULL)
	{
		//	set up scheduler
#ifdef PLATFORM_XENON
		Scheduler::Policy policy[] = {10, 10, 10, 10, 10, 10, 10, 10 };
#endif
#ifdef PLATFORM_PC
		Scheduler::Policy policy[] = {10, 10, 10, 10, 10, 10, 10, 10 };
#endif
#ifdef PLATFORM_PS3
		Scheduler::Policy policy[] = {0, 0, 10, 0, 0, 0, 0, 0 };
#endif
		Scheduler::PolicyHandle hPolicy = Scheduler::CreatePolicy(policy);

#ifdef PLATFORM_XENON
		Scheduler::jqEnableWorkers(2|4|8);
#endif
		Scheduler::Init();
		Scheduler::Start();

		Scheduler::SetGroupPoolSize(10);
		Scheduler::GroupHandle hGroup = Scheduler::CreateGroup();

		extern MT::Event AudioStreamingEvent;
		AudioStreamingEvent.Signal();

		Audio::ISystem::InitParams params;
		memset(&params, 0, sizeof(Audio::ISystem::InitParams));
		params.uMessageQueueNumBytes = 512 * 1024;
		params.uReturnMessageQueueNumBytes = 128 * 1024;
		params.uNumReverbs = 0;
		params.uTempFrameBufferNumBytes = 256 * 256 * sizeof(F32);
		params.pfnGetDatPath = C_TestSectionAudio::GetDatPathFn;
		params.hPolicy = hPolicy;
		params.hGroup = hGroup;
		params.uInstanceStateDataBufferSize = 8 * 1024;

#if defined (PLATFORM_PC)
		params.hWnd = GetForegroundWindow();
#endif
		m_pSystem = Audio::CreateSystem(&params);

#ifndef CFG_GOLD
		if (g_pAudioFontRenderer == NULL)
		{
			//Create the font renderer
			g_pAudioFontRenderer = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");

			g_pAudioFontRenderer->SetShadowEnable( true );
			g_pAudioFontRenderer->SetColourOutline(Maths::RGBA(0,0,25,255));
			g_pAudioFontRenderer->SetOutlineSize(0.35f);

			//Set Font Size
			g_pAudioFontRenderer->SetScaleX(24);
			g_pAudioFontRenderer->SetScaleY(24);

			m_pSystem->SetPrintAtCallback(AudioPrintString);
			m_pSystem->SetDrawBoxCallback(AudioDrawBox);
		}
#endif
	}

	m_pSystem->Init();

	// load the sorcery entities db
	#if defined SETUP_ENTITIES
	bool bFoundEntitiesBin = false;
	Util::C_String strAudioBin = Util::C_String("gamedata\\xt\\Audio.bin");
	Util::C_String strAudioEnt = Util::C_String("gamedata\\xt\\AudioEntities.bin");

	if (Sys::C_FileSystem::Instance().FileExists(strAudioBin) && Sys::C_FileSystem::Instance().FileExists(strAudioEnt))
	{
		Audio::C_XtAudioDb::Create();
		Audio::C_XtAudioDb::Get()->XtInit();
		Audio::C_XtAudioDb::Get()->EntitiesPack().LoadDefs(strAudioEnt);
		Audio::C_XtAudioDb::Get()->AudioPack().LoadDefs(strAudioBin);
		bFoundEntitiesBin = true;
	}
	else
		bFoundEntitiesBin = false;
	#endif // SETUP_ENTITIES

//	Audio::DebugObject::SetSystem(m_pSystem);

	//  create wavebank manager
	if (m_pWaveManager == NULL)
	{
		m_pWaveManager = Audio::CreateWaveManager();
		m_pWaveManager->SetBankFilePath(m_strPath);
	}

	#if defined SETUP_ENTITIES
	if (!bFoundEntitiesBin)
	#endif
	{
		#if defined SETUP_OBJECT_DAT
		Audio::ObjectHandle hTestObj1 = m_pSystem->GetIObjectManager()->Create("object.dat", m_pWaveManager);
		Audio::InstanceHandle hInst1 = m_pSystem->GetIObjectManager()->CreateObjectInstance(hTestObj1);
		Audio::ObjectComponentHandle hOutputToRenderer = m_pSystem->GetIObjectManager()->GetObjectSurroundLFEAudioOutput(hTestObj1, "Out");
		Audio::InstanceComponentHandle hMasterOut = m_pSystem->GetIObjectManager()->GetInstanceComponentHandle(hOutputToRenderer, hInst1);

		m_pSystem->SetMaster(hMasterOut);
		#endif
	}

	#if defined SETUP_ENTITIES
	Audio::InterfaceController::Get().Init(TotalNumSubsystems, m_pSystem, m_pWaveManager);

	Audio::InterfaceController::Get().AddSubsystem(&testSystem, eTestSystem1);

	Audio::EntityManager& entityManager = Audio::InterfaceController::Get().GetEntityManager();

	Audio::C_XtAudioDb::Get()->CreateEntityDefClasses();

	U32 uNumEntitiesSetup = 0;

	for (int i=0; i<Audio::C_XtAudioDb::Get()->XtEntityProxies().Size(); i++)
	{
		// create entry for entity in debug menu/debug view and setup callback for creating entity instance
		AudioEntityDebug* pEntDb = new AudioEntityDebug;
		if (pEntDb)
		{
			XtHandle<Audio::XtEntityProxy> hProxy = Audio::C_XtAudioDb::Get()->XtEntityProxies()[i];
			if (hProxy != NULL)
			{
				if (hProxy->IsValid())
				{
					pEntDb->hEntityDef = hProxy->GetEntityDefHnd();
					pEntDb->strName = Util::C_String("Audio.Entities.") + Util::C_String(hProxy->Name());
					#ifndef SORT_ENTITY_NAMES
					pEntDb->pDebugFunc = new Debug::Function(pEntDb->strName.Chars(), EntityActivateCallback, (void*)&pEntDb->hEntityDef);
					#endif
					m_vpEntityDefDebug.PushBack(pEntDb);

					uNumEntitiesSetup++;
				}
			}
		}
	}

	LOGINFO(g_Log, "Setup %d entities for auditioning.", uNumEntitiesSetup);

	#ifdef SORT_ENTITY_NAMES
	SortEntities();
	#endif

	// auto create entity if one specified
	if (strcmp(gAutoCreateEntity.Value(), ""))
	{
		XtHandle<Audio::XtEntityProxy> hProxy = Audio::C_XtAudioDb::Get()->LookupXtEntityProxyHandle(gAutoCreateEntity.Value());
		if (hProxy != NULL && hProxy->IsValid())
			EntityActivateCallback((void*)&hProxy->GetEntityDefHnd());
	}

	Audio::C_XtAudioDb::Get()->Release();

	return true;
	#endif // SETUP_ENTITIES

#if defined USER_JCUTTER 
	
	U32 u32Entity1Id = 0;
	U32 u32Entity2Id = 1;

	Audio::InterfaceController::Get().Init(TotalNumSubsystems, m_pSystem, m_pWaveManager);

	Audio::InterfaceController::Get().AddSubsystem(&testSystem, eTestSystem1);

	Audio::EntityManager& entityManager = Audio::InterfaceController::Get().GetEntityManager();

	// set up test object 1 as an entity
	Audio::EntityDefinitionHandle hDef1 = entityManager.CreateDefinition();

	Audio::EntityDefinition* pDef1 = entityManager.GetDefinitionFromHandle(hDef1);

	pDef1->Init(u32Entity1Id);

	Audio::EntityTypeDataMemberHandle hEntity1Object = pDef1->AddObjectMember();

	Audio::EntityTypeDataMemberHandle hEntity1Output1 = pDef1->AddObjectComponentMember();	//	output1
//		pDef1->AddObjectComponentMember();	//	output2
//		pDef1->AddObjectComponentMember();	//	output3
	Audio::EntityTypeDataMemberHandle hEntity1ActionIn1 = pDef1->AddObjectComponentMember();	//	ActionIn1
//		pDef1->AddObjectComponentMember();	//	ActionIn2
//		pDef1->AddObjectComponentMember();	//	ActionIn3

	Audio::EntityDataMemberHandle hEntity1InstanceOutput1 = pDef1->AddInstanceComponentMember();
	Audio::EntityDataMemberHandle hEntity1InstanceActionIn1 = pDef1->AddInstanceComponentMember();

	Audio::EntityDataMemberHandle hEntity1Instance = pDef1->AddInstanceMember();

	Audio::CommonFunctionHandle hEntity1SetupObjectFunction = pDef1->AddCommonFunction("SetupObject");
	Audio::LocalFunctionHandle hEntity1CreateInstanceFunction = pDef1->AddLocalFunction("CreateInstance");
	Audio::LocalFunctionHandle hDoConnectionsFunction = pDef1->AddParameterisedLocalFunction("DoConnections", u32Entity2Id);

	static char strObject1Filename[] = {"JonTest_01.dat"};
	static char strObject1ActionInName[] = {"PlaySound_01ActionIn"};
	static char strObject1AudioOutName[] = {"JonTest_01Output_01"};

	// set up test object 2 as an entity
	Audio::EntityDefinitionHandle hDef2 = entityManager.CreateDefinition();

	Audio::EntityDefinition* pDef2 = entityManager.GetDefinitionFromHandle(hDef2);

	pDef2->Init(u32Entity2Id);

	Audio::EntityTypeDataMemberHandle hEntity2Object = pDef2->AddObjectMember();

	Audio::EntityTypeDataMemberHandle hEntity2Input1 = pDef2->AddObjectComponentMember();	//	input1
	Audio::EntityTypeDataMemberHandle hEntity2Output1 = pDef2->AddObjectComponentMember();	//	entity 2, output1

	Audio::EntityDataMemberHandle hEntity2Instance = pDef2->AddInstanceMember();

	Audio::EntityDataMemberHandle hEntity2InstanceAudioInput1 = pDef2->AddInstanceComponentMember();
	Audio::EntityDataMemberHandle hEntity2InstanceAudioOutput1 = pDef2->AddInstanceComponentMember();

	Audio::CommonFunctionHandle hEntity2SetupObjectFunction = pDef2->AddCommonFunction("SetupObject");
	Audio::LocalFunctionHandle hEntity2CreateInstanceFunction = pDef2->AddLocalFunction("CreateInstance");

	static char strObject2Filename[] = {"JonTest_02.dat"};
	static char strObject2AudioInName[] = {"JonTest_02Input_01"};
	static char strObject2AudioOutName[] = {"JonTest_02Output"};

	// set up the EntityType function, SetupObject, in Entity 1

	Audio::EntityLoadObjectInstruction loadObjectInstruction;
	loadObjectInstruction.m_u32ObjectIndex = hEntity1Object;
	loadObjectInstruction.m_strFilename = strObject1Filename;

	pDef1->AddInstructionToCommonFunction(hEntity1SetupObjectFunction, &loadObjectInstruction);

	Audio::EntityGetObjectComponentInstruction getObjectActionInComponentInstruction;
	getObjectActionInComponentInstruction.m_u32ObjectIndex = hEntity1Object;
	getObjectActionInComponentInstruction.m_strComponentName = strObject1ActionInName;
	getObjectActionInComponentInstruction.m_u32ComponentIndex = hEntity1ActionIn1;

	pDef1->AddInstructionToCommonFunction(hEntity1SetupObjectFunction, &getObjectActionInComponentInstruction);

	Audio::EntityGetObjectComponentInstruction getObjectAudioOutComponentInstruction;
	getObjectAudioOutComponentInstruction.m_u32ObjectIndex = hEntity1Object;
	getObjectAudioOutComponentInstruction.m_strComponentName = strObject1AudioOutName;
	getObjectAudioOutComponentInstruction.m_u32ComponentIndex = hEntity1Output1;

	pDef1->AddInstructionToCommonFunction(hEntity1SetupObjectFunction, &getObjectAudioOutComponentInstruction);

	// now set up the EntityInstance function, CreateInstance

	Audio::EntitySpawnInstanceInstruction spawnTestObject1InstanceInstruction;
	spawnTestObject1InstanceInstruction.m_u32InstanceIndex = hEntity1Instance;
	spawnTestObject1InstanceInstruction.m_u32ObjectIndex = hEntity1Object;

	pDef1->AddInstructionToLocalFunction(hEntity1CreateInstanceFunction, &spawnTestObject1InstanceInstruction);

	Audio::EntityGetInstanceComponentInstruction getObject1InstanceActionInInstruction;
	getObject1InstanceActionInInstruction.m_u32InstanceComponentIndex = hEntity1InstanceActionIn1;
	getObject1InstanceActionInInstruction.m_u32InstanceIndex = hEntity1Instance;
	getObject1InstanceActionInInstruction.m_u32ObjectComponentIndex = hEntity1ActionIn1;

	pDef1->AddInstructionToLocalFunction(hEntity1CreateInstanceFunction, &getObject1InstanceActionInInstruction);

	Audio::EntityGetInstanceComponentInstruction getObject1InstanceAudioOutInstruction;
	getObject1InstanceAudioOutInstruction.m_u32InstanceComponentIndex = hEntity1InstanceOutput1;
	getObject1InstanceAudioOutInstruction.m_u32InstanceIndex = hEntity1Instance;
	getObject1InstanceAudioOutInstruction.m_u32ObjectComponentIndex = hEntity1Output1;

	pDef1->AddInstructionToLocalFunction(hEntity1CreateInstanceFunction, &getObject1InstanceAudioOutInstruction);

	// now set up the EntityInstance function, DoConnections

	Audio::EntityConnectExternalInstruction connectExternalInstruction;
	connectExternalInstruction.m_u32UpstreamConnectionIndex = hEntity1InstanceOutput1;
	connectExternalInstruction.m_u32DownstreamConnectionIndex = hEntity2InstanceAudioInput1;

	pDef1->AddInstructionToParameterisedLocalFunction(hDoConnectionsFunction, &connectExternalInstruction);

	// now create the SetupObject EntityType function in Entity 2

	Audio::EntityLoadObjectInstruction loadObjectInstruction2;
	loadObjectInstruction2.m_u32ObjectIndex = hEntity2Object;
	loadObjectInstruction2.m_strFilename = strObject2Filename;

	pDef2->AddInstructionToCommonFunction(hEntity2SetupObjectFunction, &loadObjectInstruction2);

	Audio::EntityGetObjectComponentInstruction getObjectAudioInComponentInstruction;
	getObjectAudioInComponentInstruction.m_u32ObjectIndex = hEntity2Object;
	getObjectAudioInComponentInstruction.m_strComponentName = strObject2AudioInName;
	getObjectAudioInComponentInstruction.m_u32ComponentIndex = hEntity2Input1;

	pDef2->AddInstructionToCommonFunction(hEntity2SetupObjectFunction, &getObjectAudioInComponentInstruction);

	Audio::EntityGetObjectComponentInstruction getObject2AudioOutComponentInstruction;
	getObject2AudioOutComponentInstruction.m_u32ObjectIndex = hEntity2Object;
	getObject2AudioOutComponentInstruction.m_strComponentName = strObject2AudioOutName;
	getObject2AudioOutComponentInstruction.m_u32ComponentIndex = hEntity2Output1;

	pDef2->AddInstructionToCommonFunction(hEntity2SetupObjectFunction, &getObject2AudioOutComponentInstruction);

	// now set up the EntityInstance function for Entity 2, CreateInstance

	Audio::EntitySpawnInstanceInstruction spawnTestObject2InstanceInstruction;
	spawnTestObject2InstanceInstruction.m_u32InstanceIndex = hEntity2Instance;
	spawnTestObject2InstanceInstruction.m_u32ObjectIndex = hEntity2Object;

	pDef2->AddInstructionToLocalFunction(hEntity2CreateInstanceFunction, &spawnTestObject2InstanceInstruction);

	Audio::EntityGetInstanceComponentInstruction getObject2InstanceAudioInInstruction;
	getObject2InstanceAudioInInstruction.m_u32InstanceComponentIndex = hEntity2InstanceAudioInput1;
	getObject2InstanceAudioInInstruction.m_u32InstanceIndex = hEntity2Instance;
	getObject2InstanceAudioInInstruction.m_u32ObjectComponentIndex = hEntity2Input1;

	pDef2->AddInstructionToLocalFunction(hEntity2CreateInstanceFunction, &getObject2InstanceAudioInInstruction);

	Audio::EntityGetInstanceComponentInstruction getObject2InstanceAudioOutInstruction;
	getObject2InstanceAudioOutInstruction.m_u32InstanceComponentIndex = hEntity2InstanceAudioOutput1;
	getObject2InstanceAudioOutInstruction.m_u32InstanceIndex = hEntity2Instance;
	getObject2InstanceAudioOutInstruction.m_u32ObjectComponentIndex = hEntity2Output1;

	pDef2->AddInstructionToLocalFunction(hEntity2CreateInstanceFunction, &getObject2InstanceAudioOutInstruction);

	// do declarations for entity 1

	static char strEntity1TriggerSoundDeclaration[] = {"TriggerSound"};

	pDef1->DeclareActionInInstanceComponent(strEntity1TriggerSoundDeclaration, hEntity1InstanceActionIn1);

	// do declarations for entity 2

	static char strEntity2AudioOutDeclaration[] = {"OutputToRenderer"};

	pDef2->DeclareAudioOutInstanceComponent(strEntity2AudioOutDeclaration, hEntity2InstanceAudioOutput1);

	// that's it
	pDef1->EndSetup();
	pDef2->EndSetup();

	// now let's set up the test section
	AudioEntityTypeHandle hEntityType1 = Audio::InterfaceController::Get().CreateEntityType(hDef1, eTestSystem1);
	AudioEntityTypeHandle hEntityType2 = Audio::InterfaceController::Get().CreateEntityType(hDef2, eTestSystem1);

	AudioEntityHandle hEntityInstance1 = Audio::InterfaceController::Get().CreateEntity(hEntityType1);
	AudioEntityHandle hEntityInstance2 = Audio::InterfaceController::Get().CreateEntity(hEntityType2);

	Audio::InterfaceController::Get().ConnectEntities(hEntityInstance1, hEntityInstance2);

	Audio::InterfaceController::Get().SetMonoMaster(hEntityInstance2, "OutputToRenderer");

	Audio::EntityDataMemberHandle hTriggerSound = Audio::InterfaceController::Get().LookupInstanceComponent(hDef1, "TriggerSound");

	Audio::InterfaceController::Get().TriggerEntityActionIn(hEntityInstance1, hTriggerSound);
#endif // #if defined USER_JCUTTER 
	return true;
}

// sort entities. very inefficient sort algorithm
void C_TestSectionAudio::SortEntities()
{
	bool bSwapped;
	do
	{
		bSwapped = false;
		for (S32 i=0; i<m_vpEntityDefDebug.Size()-1; i++)
		{
			if (!(m_vpEntityDefDebug[i]->strName < m_vpEntityDefDebug[i+1]->strName))
			{
				// swap
				AudioEntityDebug* pTemp = m_vpEntityDefDebug[i];
				m_vpEntityDefDebug[i] = m_vpEntityDefDebug[i+1];
				m_vpEntityDefDebug[i+1] = pTemp;
				bSwapped = true;
			}
		}
	} while (bSwapped);

	for (S32 s=0; s< m_vpEntityDefDebug.Size(); s++)
	{
		AudioEntityDebug* pDebug = m_vpEntityDefDebug[s];
		//LOGINFO(g_Log, "%s", pDebug->strName.Chars());
		m_vpEntityDefDebug[s]->pDebugFunc = new Debug::Function(pDebug->strName.Chars(), EntityActivateCallback, (void*)&pDebug->hEntityDef);
	}
}

void C_TestSectionAudio::OnUpdate()
{
	// for debug vars
	Debug::NetworkUpdate();

	if (gbDebugDumpWaveBanks)
	{
		if (m_pWaveManager)
			m_pWaveManager->LogLoadedBanks();
		gbDebugDumpWaveBanks = false;
	}
}

void C_TestSectionAudio::OnDestroy()
{
}

//extern U32 u32NumMP3sActive;
//extern U32 u32PacketSubmitMultiple;
void C_TestSectionAudio::OnRender2DView()
{
#ifndef CFG_GOLD
	m_pSystem->DebugRender();
#if 0
	Display::FontRender()->Draw("Hello!", 1024.f/2.f, 768.f/2.f, 50.f, 50.f, Maths::RGBA(255, 255, 255, 255), Display::E_FontRenderAlign_Ct);
#else
	static U32 u32FrameCount = 0;
	static U32 u32Instances = 10;
	static U32 u32Step = 5;
	u32FrameCount++;

	for (U32 i = 0; i < u32Instances; i++)
	{
		U32 u32FrameCountMod = u32FrameCount + (i * u32Step);
		Display::FontRender()->Draw("Hello!", 1024.f/2.f + (sin(F32(u32FrameCountMod) / 20.f) * 100.f), 768.f/2.f - (cos(F32(u32FrameCountMod) / 14.1f) * 100.f), 50.f, 50.f, Maths::RGBA(S32(sin(F32(u32FrameCountMod) / 12.f) * 127.f) + 127, S32(sin(F32(u32FrameCountMod) / 11.f) * 127.f) + 127, S32(sin(F32(u32FrameCountMod) / 10.f) * 127.f) + 127, 255), Display::E_FontRenderAlign_Ct);
	}

	//char strBuffer[256];
	//sprintf(strBuffer, "MP3s Active = %d", u32NumMP3sActive);
	//Display::FontRender()->Draw(strBuffer, 100, 100, 25, 25, Maths::RGBA(0xffffffff));
	//sprintf(strBuffer, "Num packets per decode = %d", u32PacketSubmitMultiple);
	//Display::FontRender()->Draw(strBuffer, 100, 135, 25, 25, Maths::RGBA(0xffffffff));
//	Audio::System::PrintAt(100, 100, 0xffffffff, 10, strBuffer);
#endif
#endif
}

