#include "stdafx.h"
#include "TestSectionAudio.h"
#include "SorceryIntegration.h"


Util::C_String C_TestSectionAudio::m_strPath;

#if defined USER_JCUTTER || defined USER_SHALL
#include "AppInterfaceLayer.h"

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

#endif

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
	m_strPath = "GAME:\\Data\\Audio\\";
	#endif
	#if defined (PLATFORM_PS3)
	m_strPath = "/app_home/Data/Audio/";
	#endif
}

C_TestSectionAudio::C_TestSectionAudio(const Util::C_String& name) : Framework::C_TestSection(name), m_pSystem(NULL), m_pWaveManager(NULL)
{
}

C_TestSectionAudio::~C_TestSectionAudio()
{
}

bool C_TestSectionAudio::OnCreate()
{
	// get the app path. we presume everything comes from the same path (ie bafs, dats and scripts)
	SetupPath();

	SorceryDbInit();

	//  create audio system
	if (m_pSystem == NULL)
	{
		Audio::ISystem::InitParams params;
		memset(&params, 0, sizeof(Audio::ISystem::InitParams));
		params.uMessageQueueNumBytes = 512 * 1024;
		params.uReturnMessageQueueNumBytes = 128 * 1024;
		params.uNumReverbs = 1;
		params.uTempFrameBufferNumBytes = 256 * 256 * sizeof(F32);
		params.pfnGetDatPath = C_TestSectionAudio::GetDatPathFn;

#if defined (PLATFORM_PC)
		params.hWnd = GetForegroundWindow();
#endif
		m_pSystem = Audio::CreateSystem(&params);
	}

	m_pSystem->Init();

//	Audio::DebugObject::SetSystem(m_pSystem);

	//  create wavebank manager
	if (m_pWaveManager == NULL)
	{
		m_pWaveManager = Audio::CreateWaveManager();
		m_pWaveManager->AddBankFilePath(m_strPath);
	}

#if defined USER_JCUTTER || defined USER_SHALL
	enum Subsystems
	{
		eTestSystem1 = 0,

		TotalNumSubsystems
	};

	U32 u32Entity1Id = 0;
	U32 u32Entity2Id = 1;

	Audio::InterfaceController::Get().Init(TotalNumSubsystems, m_pSystem, m_pWaveManager);

	Audio::InterfaceController::Get().AddSubsystem(&testSystem, eTestSystem1);

	Audio::EntityManager& entityManager = Audio::InterfaceController::Get().GetEntityManager();

	#ifdef USER_SHALL
	for (U32 i=0; i<gEntityDefList.defs.Size(); i++)
	{
		LOGINFO(Samples::g_Log, "%d setting up EntityDefinition '%s'\n", i, gEntityDefList.defs[i]->name);

		Audio::EntityDefinitionHandle hDef = entityManager.CreateDefinition();
		Audio::EntityDefinition* pDef = entityManager.GetDefinitionFromHandle(hDef);
		pDef->Init(i);

		if (gEntityDefList.defs[i]->SetupEntityDefinition(pDef) == false)
			LOGWARN(Samples::g_Log, "Could not create entity definition\n");
			
		AudioEntityTypeHandle hEntityType = Audio::InterfaceController::Get().CreateEntityType(hDef, eTestSystem1);
		AudioEntityHandle hEntityInstance = Audio::InterfaceController::Get().CreateEntity(hEntityType);

		//Audio::InterfaceController::Get().SetMonoMaster(hEntityInstance2, "OutputToRenderer");
		//Audio::EntityDataMemberHandle hTriggerSound = Audio::InterfaceController::Get().LookupInstanceComponent(hDef1, "TriggerSound");
		//Audio::InterfaceController::Get().TriggerEntityActionIn(hEntityInstance1, hTriggerSound);
	}

	return true;

	#else
	
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

	pDef1->AddInstructionToCommonFunction(hEntity1SetupObjectFunction, loadObjectInstruction);

	Audio::EntityGetObjectComponentInstruction getObjectActionInComponentInstruction;
	getObjectActionInComponentInstruction.m_u32ObjectIndex = hEntity1Object;
	getObjectActionInComponentInstruction.m_strComponentName = strObject1ActionInName;
	getObjectActionInComponentInstruction.m_u32ComponentIndex = hEntity1ActionIn1;

	pDef1->AddInstructionToCommonFunction(hEntity1SetupObjectFunction, getObjectActionInComponentInstruction);

	Audio::EntityGetObjectComponentInstruction getObjectAudioOutComponentInstruction;
	getObjectAudioOutComponentInstruction.m_u32ObjectIndex = hEntity1Object;
	getObjectAudioOutComponentInstruction.m_strComponentName = strObject1AudioOutName;
	getObjectAudioOutComponentInstruction.m_u32ComponentIndex = hEntity1Output1;

	pDef1->AddInstructionToCommonFunction(hEntity1SetupObjectFunction, getObjectAudioOutComponentInstruction);

	// now set up the EntityInstance function, CreateInstance

	Audio::EntitySpawnInstanceInstruction spawnTestObject1InstanceInstruction;
	spawnTestObject1InstanceInstruction.m_u32InstanceIndex = hEntity1Instance;
	spawnTestObject1InstanceInstruction.m_u32ObjectIndex = hEntity1Object;

	pDef1->AddInstructionToLocalFunction(hEntity1CreateInstanceFunction, spawnTestObject1InstanceInstruction);

	Audio::EntityGetInstanceComponentInstruction getObject1InstanceActionInInstruction;
	getObject1InstanceActionInInstruction.m_u32InstanceComponentIndex = hEntity1InstanceActionIn1;
	getObject1InstanceActionInInstruction.m_u32InstanceIndex = hEntity1Instance;
	getObject1InstanceActionInInstruction.m_u32ObjectComponentIndex = hEntity1ActionIn1;

	pDef1->AddInstructionToLocalFunction(hEntity1CreateInstanceFunction, getObject1InstanceActionInInstruction);

	Audio::EntityGetInstanceComponentInstruction getObject1InstanceAudioOutInstruction;
	getObject1InstanceAudioOutInstruction.m_u32InstanceComponentIndex = hEntity1InstanceOutput1;
	getObject1InstanceAudioOutInstruction.m_u32InstanceIndex = hEntity1Instance;
	getObject1InstanceAudioOutInstruction.m_u32ObjectComponentIndex = hEntity1Output1;

	pDef1->AddInstructionToLocalFunction(hEntity1CreateInstanceFunction, getObject1InstanceAudioOutInstruction);

	// now set up the EntityInstance function, DoConnections

	Audio::EntityConnectExternalInstruction connectExternalInstruction;
	connectExternalInstruction.m_u32UpstreamConnectionIndex = hEntity1InstanceOutput1;
	connectExternalInstruction.m_u32DownstreamConnectionIndex = hEntity2InstanceAudioInput1;

	pDef1->AddInstructionToParameterisedLocalFunction(hDoConnectionsFunction, connectExternalInstruction);

	// now create the SetupObject EntityType function in Entity 2

	Audio::EntityLoadObjectInstruction loadObjectInstruction2;
	loadObjectInstruction2.m_u32ObjectIndex = hEntity2Object;
	loadObjectInstruction2.m_strFilename = strObject2Filename;

	pDef2->AddInstructionToCommonFunction(hEntity2SetupObjectFunction, loadObjectInstruction2);

	Audio::EntityGetObjectComponentInstruction getObjectAudioInComponentInstruction;
	getObjectAudioInComponentInstruction.m_u32ObjectIndex = hEntity2Object;
	getObjectAudioInComponentInstruction.m_strComponentName = strObject2AudioInName;
	getObjectAudioInComponentInstruction.m_u32ComponentIndex = hEntity2Input1;

	pDef2->AddInstructionToCommonFunction(hEntity2SetupObjectFunction, getObjectAudioInComponentInstruction);

	Audio::EntityGetObjectComponentInstruction getObject2AudioOutComponentInstruction;
	getObject2AudioOutComponentInstruction.m_u32ObjectIndex = hEntity2Object;
	getObject2AudioOutComponentInstruction.m_strComponentName = strObject2AudioOutName;
	getObject2AudioOutComponentInstruction.m_u32ComponentIndex = hEntity2Output1;

	pDef2->AddInstructionToCommonFunction(hEntity2SetupObjectFunction, getObject2AudioOutComponentInstruction);

	// now set up the EntityInstance function for Entity 2, CreateInstance

	Audio::EntitySpawnInstanceInstruction spawnTestObject2InstanceInstruction;
	spawnTestObject2InstanceInstruction.m_u32InstanceIndex = hEntity2Instance;
	spawnTestObject2InstanceInstruction.m_u32ObjectIndex = hEntity2Object;

	pDef2->AddInstructionToLocalFunction(hEntity2CreateInstanceFunction, spawnTestObject2InstanceInstruction);

	Audio::EntityGetInstanceComponentInstruction getObject2InstanceAudioInInstruction;
	getObject2InstanceAudioInInstruction.m_u32InstanceComponentIndex = hEntity2InstanceAudioInput1;
	getObject2InstanceAudioInInstruction.m_u32InstanceIndex = hEntity2Instance;
	getObject2InstanceAudioInInstruction.m_u32ObjectComponentIndex = hEntity2Input1;

	pDef2->AddInstructionToLocalFunction(hEntity2CreateInstanceFunction, getObject2InstanceAudioInInstruction);

	Audio::EntityGetInstanceComponentInstruction getObject2InstanceAudioOutInstruction;
	getObject2InstanceAudioOutInstruction.m_u32InstanceComponentIndex = hEntity2InstanceAudioOutput1;
	getObject2InstanceAudioOutInstruction.m_u32InstanceIndex = hEntity2Instance;
	getObject2InstanceAudioOutInstruction.m_u32ObjectComponentIndex = hEntity2Output1;

	pDef2->AddInstructionToLocalFunction(hEntity2CreateInstanceFunction, getObject2InstanceAudioOutInstruction);

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
	#endif // #ifdef USER_SHALL
#endif // #if defined USER_JCUTTER || defined USER_SHALL
	return true;
}

void C_TestSectionAudio::OnDestroy()
{
}

void C_TestSectionAudio::OnRender2DView()
{
	Display::FontRender()->Draw("Hello!", 1024.f/2.f, 768.f/2.f, 50.f, 50.f, Maths::RGBA(255, 255, 255, 255), Display::E_FontRenderAlign_Ct);
}
