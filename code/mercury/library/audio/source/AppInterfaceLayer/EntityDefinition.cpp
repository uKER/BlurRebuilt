#include "AudioPch.h"

#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#endif

//using namespace Audio;

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

Btl::DynamicStackAllocator Audio::EntityDefinition::m_Allocator;

namespace Audio
{
	#ifdef USE_STRING_HASHES_FOR_ENTITY_DEFINITIONS
	bool EntityDefString::operator == ( const char* pchStr )
	{ 
		return (m_Str == StringHash(pchStr));
	}	
	void EntityDefString::Set(const char* pchStr)
	{
		m_Str = StringHash(pchStr);
	}
	bool EntityDefString::IsValid()
	{
		return m_Str.IsValid();
	}
	#else
	bool EntityDefString::operator == ( const char* pchStr )
	{ 
		return !stricmp(m_Str.Chars(), pchStr);
	}
	void EntityDefString::Set(const char* pchStr)
	{
		m_Str = pchStr;
	}
	bool EntityDefString::IsValid()
	{
		return m_Str != "";
	}
	#endif
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	(constructor)
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityDefinition::EntityDefinition()
{
	m_bSettingUp = true;
	m_bInited = false;
	m_u32NumObjectDataMembers = 0;
	m_u32NumObjectComponentDataMembers = 0;
	m_u32NumInstanceDataMembers = 0;
	m_u32NumInstanceComponentDataMembers = 0;
	m_u32UniqueId = 0;
//	m_u32NumDeclaredParameterOuts = 0;
	m_u32NumDeclaredActionOuts = 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	(destructor)
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityDefinition::~EntityDefinition()
{
	for (S32 i = 0; i < m_CommonFunctions.Size(); i++)
	{
		for (S32 i2 = 0; i2 < m_CommonFunctions[i].m_Instructions.Size(); i2++)
		{
			#ifdef USE_ALLOCATOR_FOR_ENTITY_DEFINITIONS
			Allocator().Free<Audio::EntityInstruction>(m_CommonFunctions[i].m_Instructions[i2]);	// TODO: this has not been tested.
			#else
			delete m_CommonFunctions[i].m_Instructions[i2];
			#endif

		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AddCommonFunction
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::CommonFunctionHandle Audio::EntityDefinition::AddCommonFunction(const char* strName)
{
	//MEMORY_TRACKER_PUSH("Audio.EntityDefinitions.Classes.CommonFuncs");

	EntityFunction func;

	func.m_strName = strName;
	func.m_bAcceptParameter = false;

	m_CommonFunctions.PushBack(func);

	//MEMORY_TRACKER_POP("Audio.EntityDefinitions.Classes.CommonFuncs");

	return (CommonFunctionHandle)(m_CommonFunctions.Size() - 1);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AddLocalFunction
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::LocalFunctionHandle Audio::EntityDefinition::AddLocalFunction(const char* strName)
{
	//MEMORY_TRACKER_CONTEXT("Audio.EntityDefinitions.Classes.LocalFuncs");

	EntityFunction func;

	func.m_strName = strName;
	func.m_bAcceptParameter = false;

	m_LocalFunctions.PushBack(func);

	return (CommonFunctionHandle)(m_LocalFunctions.Size() - 1);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AddParamaterisedLocalFunction
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::LocalFunctionHandle Audio::EntityDefinition::AddParameterisedLocalFunction(const char* strName, U32 u32ParameterId)
{
	//MEMORY_TRACKER_CONTEXT("Audio.EntityDefinitions.Classes.ParamLocalFuncs");

	EntityFunction func;

	func.m_strName = strName;
	func.m_bAcceptParameter = true;
	func.m_u32ParameterId = u32ParameterId;

	m_ParameterisedLocalFunctions.PushBack(func);

	return (CommonFunctionHandle)(m_ParameterisedLocalFunctions.Size() - 1);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AddInstructionToCommonFunction
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::AddInstructionToCommonFunction(CommonFunctionHandle hFunction, EntityInstruction* pInstruction)
{
	//MEMORY_TRACKER_CONTEXT("Audio.EntityDefinitions.Classes.CommonFuncInstructions");

	if (!pInstruction)
	{
		InterfaceController::Get().Error("Could not add instruction to common function. pInstruction is NULL");
		return false;
	}

	if (! (m_bSettingUp && m_bInited))
		return false;

	if ((U32)(m_CommonFunctions.Size()) <= hFunction)
	{
		InterfaceController::Get().Error("function index outside array bounds during definition setup");
		return false;
	}

	switch (pInstruction->Command())
	{
		case eLoadObject:
		{
			if (! ((EntityLoadObjectInstruction*)pInstruction)->IsValid())
			{
				InterfaceController::Get().Error("Couldn't set up Load Object instruction in common function- not all data members have been set");
				return false;
			}
		}
		break;

		case eGetObjectComponent:
		{
			if (! ((EntityGetObjectComponentInstruction*)pInstruction)->IsValid())
			{
				InterfaceController::Get().Error("Couldn't set up Get Object Component instruction in common function- not all data members have been set");
				return false;
			}
		}
		break;

		default:
			InterfaceController::Get().Error("invalid command type");
			return false;
	}

	m_CommonFunctions[hFunction].AddInstruction(pInstruction);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AddInstructionToLocalFunction
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::AddInstructionToLocalFunction(LocalFunctionHandle hFunction, EntityInstruction* pInstruction)
{
	//MEMORY_TRACKER_CONTEXT("Audio.EntityDefinitions.Classes.LocalFuncInstructions");

	if (!pInstruction)
	{
		InterfaceController::Get().Error("Could not add instruction to common function. pInstruction is NULL");
		return false;
	}

	if (! (m_bSettingUp && m_bInited))
		return false;

	if ((U32)(m_LocalFunctions.Size()) <= hFunction)
	{
		InterfaceController::Get().Error("function index outside array bounds during definition setup");
		return false;
	}

	switch (pInstruction->Command())
	{
		case eConnectInternal:
		{
			if (! ((EntityConnectInternalInstruction*)pInstruction)->IsValid())
			{
				InterfaceController::Get().Error("Couldn't set up Connect Internal instruction in local function- not all data members have been set");
				return false;
			}
		}
		break;

		case eConnectToParameterised:
		{
			if (! ((EntityConnectToParameterisedInstruction*)pInstruction)->IsValid())
			{
				InterfaceController::Get().Error("Couldn't set up Connect To Parameterised instruction in local function- not all data members have been set");
				return false;
			}
		}
		break;

		case eConnectFromParameterised:
		{
			if (! ((EntityConnectFromParameterisedInstruction*)pInstruction)->IsValid())
			{
				InterfaceController::Get().Error("Couldn't set up Connect From Parameterised instruction in local function- not all data members have been set");
				return false;
			}
		}
		break;

		case eSpawnInstance:
		{
			if (! ((EntitySpawnInstanceInstruction*)pInstruction)->IsValid())
			{
				InterfaceController::Get().Error("Couldn't set up Spawn Instance instruction in local function- not all data members have been set");
				return false;
			}
		}
		break;

		case eGetInstanceComponent:
		{
			if (! ((EntityGetInstanceComponentInstruction*)pInstruction)->IsValid())
			{
				InterfaceController::Get().Error("Couldn't set up Get Instance Component instruction in local function- not all data members have been set");
				return false;
			}
		}
		break;

		default:
			InterfaceController::Get().Error("invalid command type");
			return false;
	}


	m_LocalFunctions[hFunction].AddInstruction(pInstruction);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AddInstructionToParameterisedLocalFunction
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::AddInstructionToParameterisedLocalFunction(LocalFunctionHandle hFunction, EntityInstruction* pInstruction)
{
	//MEMORY_TRACKER_CONTEXT("Audio.EntityDefinitions.Classes.ParamLocalFuncInstructions");

	if (!pInstruction)
	{
		InterfaceController::Get().Error("Could not add instruction to common function. pInstruction is NULL");
		return false;
	}

	if (! (m_bSettingUp && m_bInited))
		return false;

	if ((U32)(m_ParameterisedLocalFunctions.Size()) <= hFunction)
	{
		InterfaceController::Get().Error("function index outside array bounds during definition setup");
		return false;
	}

	switch (pInstruction->Command())
	{
		case eConnectInternal:
		{
			if (! ((EntityConnectInternalInstruction*)pInstruction)->IsValid())
			{
				InterfaceController::Get().Error("Couldn't set up Connect Internal instruction in parameterised local function- not all data members have been set");
				return false;
			}
		}
		break;

		case eConnectToParameterised:
		{
			if (! ((EntityConnectToParameterisedInstruction*)pInstruction)->IsValid())
			{
				InterfaceController::Get().Error("Couldn't set up Connect To Parameterised instruction in parameterised local function- not all data members have been set");
				return false;
			}
		}
		break;

		case eConnectFromParameterised:
		{
			if (! ((EntityConnectFromParameterisedInstruction*)pInstruction)->IsValid())
			{
				InterfaceController::Get().Error("Couldn't set up Connect From Parameterised instruction in parameterised local function- not all data members have been set");
				return false;
			}
		}
		break;

		case eSpawnInstance:
		{
			if (! ((EntitySpawnInstanceInstruction*)pInstruction)->IsValid())
			{
				InterfaceController::Get().Error("Couldn't set up Spawn Instance instruction in parameterised local function- not all data members have been set");
				return false;
			}
		}
		break;

		case eGetInstanceComponent:
		{
			if (! ((EntityGetInstanceComponentInstruction*)pInstruction)->IsValid())
			{
				InterfaceController::Get().Error("Couldn't set up Get Instance Component instruction in parameterised local function- not all data members have been set");
				return false;
			}
		}
		break;

		default:
			InterfaceController::Get().Error("invalid command type");
			return false;
	}

	m_ParameterisedLocalFunctions[hFunction].AddInstruction(pInstruction);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DeclareObject
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::DeclareObject(const char* strName, EntityTypeDataMemberHandle hObject)
{
	MEMORY_TRACKER_CONTEXT("Audio.EntityDefinitions.Classes.CommonDeclarations");

	if (! (m_bSettingUp && m_bInited))
	{
		InterfaceController::Get().Error("can't set up object declaration");
		return false;
	}

	if (m_u32NumObjectDataMembers <= hObject)
	{
		InterfaceController::Get().Error("data member out of range");
		return false;
	}

#ifdef ENTITYDEF_USE_U16
	BIZ_ASSERT_MSG(hObject <= 0xffff, "hObject is too big to fit in 16 bits");
#endif

	EntityDeclaration* declaration = Allocate<EntityDeclaration>();

	declaration->m_Name.Set(strName);
	declaration->SetVariableType(eObject);
	declaration->m_u32Index = hObject;
	declaration->SetActionOut(false);

	m_CommonDeclarations.PushBack(declaration);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DeclareObjectComponent
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::DeclareObjectComponent(const char* strName, EntityTypeDataMemberHandle hObjectComponent)
{
	MEMORY_TRACKER_CONTEXT("Audio.EntityDefinitions.Classes.DeclareObjectComponent");

	if (! (m_bSettingUp && m_bInited))
	{
		InterfaceController::Get().Error("can't set up object component declaration");
		return false;
	}

	if (m_u32NumObjectComponentDataMembers <= hObjectComponent)
	{
		InterfaceController::Get().Error("data member out of range");
		return false;
	}

	#ifdef ENTITYDEF_USE_U16
	BIZ_ASSERT_MSG(hObjectComponent <= 0xffff, "hObjectComponent is too big to fit in 16 bits");
	#endif

	EntityDeclaration* declaration = Allocate<EntityDeclaration>();

	declaration->m_Name.Set(strName);
	declaration->SetVariableType(eObjectComponent);
	declaration->m_u32Index = hObjectComponent;
	declaration->SetActionOut(false);

	m_CommonDeclarations.PushBack(declaration);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DeclareInstance
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::DeclareInstance(const char* strName, EntityDataMemberHandle hInstance)
{
	MEMORY_TRACKER_CONTEXT("Audio.EntityDefinitions.Classes.LocalDeclarations");

	if (! (m_bSettingUp && m_bInited))
	{
		InterfaceController::Get().Error("can't set up instance declaration");
		return false;
	}

	if (m_u32NumInstanceDataMembers <= hInstance)
	{
		InterfaceController::Get().Error("data member out of range");
		return false;
	}

	#ifdef ENTITYDEF_USE_U16
	BIZ_ASSERT_MSG(hInstance <= 0xffff, "hInstance is too big to fit in 16 bits");
	#endif

	EntityDeclaration* declaration = Allocate<EntityDeclaration>();

	declaration->m_Name.Set(strName);
	declaration->SetVariableType(eInstance);
	declaration->m_u32Index = hInstance;
	declaration->SetActionOut(false);

	m_LocalDeclarations.PushBack(declaration);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DeclareInstanceComponent
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::DeclareInstanceComponent(const char* strName, EntityDataMemberHandle hInstanceComponent)
{
	if (! (m_bSettingUp && m_bInited))
	{
		InterfaceController::Get().Error("can't set up instance component declaration");
		return false;
	}

	if (m_u32NumInstanceComponentDataMembers <= hInstanceComponent)
	{
		InterfaceController::Get().Error("data member out of range");
		return false;
	}

	#ifdef ENTITYDEF_USE_U16
	BIZ_ASSERT_MSG(hInstanceComponent <= 0xffff, "hInstanceComponent is too big to fit in 16 bits");
	#endif

	EntityDeclaration* declaration = Allocate<EntityDeclaration>();

	declaration->m_Name.Set(strName);
	declaration->SetVariableType(eInstanceComponent);
	declaration->m_u32Index = hInstanceComponent;
	declaration->SetActionOut(false);

	//MEMORY_TRACKER_PUSH("Audio.EntityDefinitions.Classes.LocalDeclarations");

	m_LocalDeclarations.PushBack(declaration);

	//MEMORY_TRACKER_POP("Audio.EntityDefinitions.Classes.LocalDeclarations");


	//MEMORY_TRACKER_PUSH("Audio.EntityDefinitions.Classes.InstanceStringHashes");

	// add to string hash map
	Audio::StringHash hash(strName);
	m_LocalDeclHashMap.Insert(hash.GetValue(), hInstanceComponent);

	//MEMORY_TRACKER_POP("Audio.EntityDefinitions.Classes.InstanceStringHashes");

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DeclareParameterInInstanceComponent
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::DeclareParameterInInstanceComponent(const char* strName, EntityDataMemberHandle hInstanceComponent)
{
	return DeclareInstanceComponent(strName, hInstanceComponent);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DeclareActionInInstanceComponent
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::DeclareActionInInstanceComponent(const char* strName, EntityDataMemberHandle hInstanceComponent)
{
	return DeclareInstanceComponent(strName, hInstanceComponent);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DeclareParameterOutInstanceComponent
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::DeclareParameterOutInstanceComponent(const char* strName, EntityDataMemberHandle hInstanceComponent)
{
	bool bSuccess = DeclareInstanceComponent(strName, hInstanceComponent);

	if (! bSuccess)
		return false;

//	m_LocalDeclarations[m_LocalDeclarations.Size() - 1].m_u32OutputIndex = m_u32NumDeclaredParameterOuts;

//	m_u32NumDeclaredParameterOuts++;

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DeclareActionOutInstanceComponent
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::DeclareActionOutInstanceComponent(const char* strName, EntityDataMemberHandle hInstanceComponent)
{
	bool bSuccess = DeclareInstanceComponent(strName, hInstanceComponent);

	if (! bSuccess)
		return false;

	m_LocalDeclarations[m_LocalDeclarations.Size() - 1]->SetActionOut(true);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DeclareVectorInInstanceComponent
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::DeclareVectorInInstanceComponent(const char* strName, EntityDataMemberHandle hInstanceComponent)
{
	return DeclareInstanceComponent(strName, hInstanceComponent);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DeclareAudioOutInstanceComponent
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::DeclareAudioOutInstanceComponent(const char* strName, EntityDataMemberHandle hInstanceComponent)
{
	return DeclareInstanceComponent(strName, hInstanceComponent);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetParameterOutIndex
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
//S32 Audio::EntityDefinition::GetParameterOutIndex(EntityDataMemberHandle hInstanceComponent)
//{
//	S32 s32RetValue = m_LocalDeclarations[hInstanceComponent].m_u32OutputIndex;
//
//	if (s32RetValue >= (S32)m_u32NumDeclaredParameterOuts)
//		return -1;
//
//	return s32RetValue;
//}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AddObjectMember
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityTypeDataMemberHandle Audio::EntityDefinition::AddObjectMember()
{
	m_u32NumObjectDataMembers++;

	#ifdef ENTITYDEF_USE_U16
	BIZ_ASSERT_MSG(m_u32NumObjectDataMembers <= 0xffff, "num objects is too big to fit in 16 bits");
	#endif

	return (EntityTypeDataMemberHandle)(m_u32NumObjectDataMembers - 1);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AddObjectComponentMember
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityTypeDataMemberHandle Audio::EntityDefinition::AddObjectComponentMember()
{
	m_u32NumObjectComponentDataMembers++;

	#ifdef ENTITYDEF_USE_U16
	BIZ_ASSERT_MSG(m_u32NumObjectComponentDataMembers <= 0xffff, "num object components is too big to fit in 16 bits");
	#endif

	return (EntityTypeDataMemberHandle)(m_u32NumObjectComponentDataMembers - 1);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AddInstanceMember
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityDataMemberHandle Audio::EntityDefinition::AddInstanceMember()
{
	m_u32NumInstanceDataMembers++;

	#ifdef ENTITYDEF_USE_U16
	BIZ_ASSERT_MSG(m_u32NumInstanceDataMembers <= 0xffff, "num instances is too big to fit in 16 bits");
	#endif

	return (EntityDataMemberHandle)(m_u32NumInstanceDataMembers - 1);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AddInstanceComponentMember
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityDataMemberHandle Audio::EntityDefinition::AddInstanceComponentMember()
{
	m_u32NumInstanceComponentDataMembers++;

	#ifdef ENTITYDEF_USE_U16
	BIZ_ASSERT_MSG(m_u32NumInstanceComponentDataMembers <= 0xffff, "num instance components is too big to fit in 16 bits");
	#endif

	return (EntityDataMemberHandle)(m_u32NumInstanceComponentDataMembers - 1);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SpawnEntityType
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityType* Audio::EntityDefinition::SpawnEntityType()
{
	if ((! m_bInited) || m_bSettingUp)
	{
		InterfaceController::Get().Error("couldn't spawn common entity");
		return 0;
	}

	EntityType* pNewEntity = new EntityType();

	// new can't legally return 0, would have to be overridden with compiler warnings disabled.. but check if 0 anyway for now
	if (! pNewEntity)
	{
		InterfaceController::Get().Error("couldn't spawn common entity");
		return 0;
	}

	pNewEntity->Init(m_u32NumObjectDataMembers, m_u32NumObjectComponentDataMembers);

	return pNewEntity;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SpawnEntityInstance
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityInstance* Audio::EntityDefinition::SpawnEntityInstance()
{
	if ((! m_bInited) || m_bSettingUp)
	{
		InterfaceController::Get().Error("couldn't spawn local entity");
		return 0;
	}

	EntityInstance* pNewEntity = new EntityInstance();

	// new can't legally return 0, would have to be overridden with compiler warnings disabled.. but check if 0 anyway for now
	if (! pNewEntity)
	{
		InterfaceController::Get().Error("couldn't spawn local entity");
		return 0;
	}

	if (pNewEntity->Init(m_u32NumInstanceDataMembers, m_u32NumInstanceComponentDataMembers/*, m_u32NumDeclaredParameterOuts*/))
	{
		// for each declared ActionOut, call EntityManager::AddCallbackData
		for (S32 i = 0; i < m_LocalDeclarations.Size(); i++)
		{
			if (m_LocalDeclarations[i]->IsActionOut())
				InterfaceController::Get().GetEntityManager().AddCallbackData(pNewEntity, m_LocalDeclarations[i]->m_u32Index);
		}

	}

	return pNewEntity;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetCommonFunction
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::CommonFunctionHandle Audio::EntityDefinition::GetCommonFunction(const char* strName)
{
	if ((! m_bInited) || m_bSettingUp)
	{
		InterfaceController::Get().Error("can't get function from definition");
		return 0xffffffff;
	}

	for (S32 i = 0; i < m_CommonFunctions.Size(); i++)
	{
		if (stricmp(m_CommonFunctions[i].m_strName.Chars(), strName) == 0)
			return (CommonFunctionHandle)i;
	}

	return 0xffffffff;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetLocalFunction
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::LocalFunctionHandle Audio::EntityDefinition::GetLocalFunction(const char* strName)
{
	if ((! m_bInited) || m_bSettingUp)
	{
		InterfaceController::Get().Error("can't get function from definition");
		return 0xffffffff;
	}

	for (S32 i = 0; i < m_LocalFunctions.Size(); i++)
	{
		if (stricmp(m_LocalFunctions[i].m_strName.Chars(), strName) == 0)
			return (LocalFunctionHandle)i;
	}

	return 0xffffffff;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetNumParameterisedFunctions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
S32	Audio::EntityDefinition::GetNumParameterisedFunctions(const char* strName)
{
	if ((! m_bInited) || m_bSettingUp)
	{
		InterfaceController::Get().Error("can't get function from definition");
		return 0;
	}

	S32 sNum = 0;
	for (S32 i = 0; i < m_ParameterisedLocalFunctions.Size(); i++)
	{
		if (stricmp(m_ParameterisedLocalFunctions[i].m_strName.Chars(), strName) == 0)
			sNum++;
	}

	return sNum;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetParameterisedFunctionByIndex
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::LocalFunctionHandle Audio::EntityDefinition::GetParameterisedFunctionByIndex(const char* strName, const S32 sIndex)
{
	if ((! m_bInited) || m_bSettingUp)
	{
		InterfaceController::Get().Error("can't get function from definition");
		return 0xffffffff;
	}

	S32 sCurrFunc = 0;
	for (S32 i = 0; i < m_ParameterisedLocalFunctions.Size(); i++)
	{
		if (stricmp(m_ParameterisedLocalFunctions[i].m_strName.Chars(), strName) == 0)
		{
			if (sCurrFunc == sIndex)
				return (LocalFunctionHandle)i;

			sCurrFunc++;
		}
	}

	return 0xffffffff;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetParameterisedFunctionById
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::LocalFunctionHandle Audio::EntityDefinition::GetParameterisedFunctionById(const char* strName, const U32 uParameterId)
{
	if ((! m_bInited) || m_bSettingUp)
	{
		InterfaceController::Get().Error("can't get function from definition");
		return 0xffffffff;
	}

	for (S32 i = 0; i < m_ParameterisedLocalFunctions.Size(); i++)
	{
		if (stricmp(m_ParameterisedLocalFunctions[i].m_strName, strName) == 0)
		{
			if (m_ParameterisedLocalFunctions[i].m_u32ParameterId == uParameterId)
				return (LocalFunctionHandle)i;
		}
	}

	return 0xffffffff;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LookupObject
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityTypeDataMemberHandle Audio::EntityDefinition::LookupObject(const char* strName)
{
	if ((! m_bInited) || m_bSettingUp)
	{
		InterfaceController::Get().Error("can't look up object in definition");
		return 0xffffffff;
	}

	for (S32 i = 0; i < m_CommonDeclarations.Size(); i++)
	{
		if (m_CommonDeclarations[i]->VariableType() == eObject && m_CommonDeclarations[i]->m_Name == strName)
			return m_CommonDeclarations[i]->m_u32Index;
	}

	return 0xffffffff;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LookupObjectComponent
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityTypeDataMemberHandle Audio::EntityDefinition::LookupObjectComponent(const char* strName)
{
	if ((! m_bInited) || m_bSettingUp)
	{
		InterfaceController::Get().Error("can't look up object component in definition");
		return 0xffffffff;
	}

	for (S32 i = 0; i < m_CommonDeclarations.Size(); i++)
	{
		if (m_CommonDeclarations[i]->VariableType() == eObjectComponent && m_CommonDeclarations[i]->m_Name == strName)
			return m_CommonDeclarations[i]->m_u32Index;
	}

	return 0xffffffff;// why not use a constant?????? What if you forget an f ???
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LookupInstance
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityDataMemberHandle Audio::EntityDefinition::LookupInstance(const char* strName)
{
	if ((! m_bInited) || m_bSettingUp)
	{
		InterfaceController::Get().Error("can't look up instance in definition");
		return 0xffffffff;// why not use a constant?????? What if you forget an f ???
	}

	for (S32 i = 0; i < m_LocalDeclarations.Size(); i++)
	{
		if (m_LocalDeclarations[i]->VariableType() == eInstance && m_LocalDeclarations[i]->m_Name == strName)
			return m_LocalDeclarations[i]->m_u32Index;
	}

	return 0xffffffff;// why not use a constant?????? What if you forget an f ???
}

Audio::EntityDataMemberHandle Audio::EntityDefinition::LookupInstance( StringHash hash )
{
#ifdef USE_STRING_HASHES_FOR_ENTITY_DEFINITIONS
	if ((! m_bInited) || m_bSettingUp)
	{
		InterfaceController::Get().Error("can't look up instance in definition");
		return 0xffffffff;// why not use a constant?????? What if you forget an f ???
	}

	for (S32 i = 0; i < m_LocalDeclarations.Size(); i++)
	{
		if (m_LocalDeclarations[i]->m_Name.m_Str == hash && m_LocalDeclarations[i]->VariableType() == eInstance )
			return m_LocalDeclarations[i]->m_u32Index;
	}
#endif
	return 0xffffffff;// why not use a constant?????? What if you forget an f ???
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LookupInstanceComponent - string version
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityDataMemberHandle Audio::EntityDefinition::LookupInstanceComponent(const char* strName)
{
	if ((! m_bInited) || m_bSettingUp)
	{
		InterfaceController::Get().Error("can't look up instance component in definition");
		
		return 0xffffffff;// why not use a constant?????? What if you forget an f ???
	}

	for (S32 i = 0; i < m_LocalDeclarations.Size(); i++)
	{
		if (m_LocalDeclarations[i]->VariableType() == eInstanceComponent && m_LocalDeclarations[i]->m_Name == strName)
			return m_LocalDeclarations[i]->m_u32Index;
	}

	//char name[128];
	//strcpy(name,strName);
	//strcat(name,": could not find this instance component in definition");
	//InterfaceController::Get().Info(name);
	//DebugBreak();
	
	
	return 0xffffffff;// why not use a constant?????? What if you forget an f ???
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LookupInstanceComponent - string hash version
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityDataMemberHandle Audio::EntityDefinition::LookupInstanceComponent(StringHash hash)
{
	if ((! m_bInited) || m_bSettingUp)
	{
		InterfaceController::Get().Error("can't look up instance component in definition");
		return 0xffffffff;	// why not use a constant?????? What if you forget an f ???
	}

	C_DeclarationHashMap::ConstIterator it = m_LocalDeclHashMap.Find(hash.GetValue());
	return (it != m_LocalDeclHashMap.End()) ? it->GetValue() : 0xffffffff;
}
 
void Audio::EntityFunction::AddInstruction(EntityInstruction* pInst)
{
	m_Instructions.PushBack(pInst);
}


Audio::EntityInstruction* Audio::EntityFunction::GetInstruction(U32 index)
{
	return m_Instructions[index];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RunEntityTypeFunction
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::RunEntityTypeFunction(CommonFunctionHandle hFunction, EntityType* pEntityType, bool bReuseHandles)
{
	if ((! m_bInited) || m_bSettingUp || hFunction >= (U32)(m_CommonFunctions.Size()))
	{
		InterfaceController::Get().Error("can't run entity type function");
		return false;
	}

	for (S32 i = 0; i < m_CommonFunctions[hFunction].m_Instructions.Size(); i++)
	{
		switch (m_CommonFunctions[hFunction].GetInstruction(i)->Command())
		{
			case eLoadObject:
			{
				EntityLoadObjectInstruction* pLoadObjectInstruction = (EntityLoadObjectInstruction*)m_CommonFunctions[hFunction].GetInstruction(i);

				if (bReuseHandles)
				{
					System::Get()->GetObjectManager()->CreateUsingHandle(pLoadObjectInstruction->m_strFilename, InterfaceController::Get().GetWaveManager(), pEntityType->GetObject(pLoadObjectInstruction->m_u32ObjectIndex));
				}
				else
				{
					ObjectHandle hObj = System::Get()->GetObjectManager()->CreateImmediate(pLoadObjectInstruction->m_strFilename, InterfaceController::Get().GetWaveManager());
					pEntityType->SetObjectHandle(pLoadObjectInstruction->m_u32ObjectIndex, hObj);
				}
			}
			break;

			case eGetObjectComponent:
			{
				EntityGetObjectComponentInstruction* pGetComponentInstruction = (EntityGetObjectComponentInstruction*)m_CommonFunctions[hFunction].GetInstruction(i);
				ObjectHandle hObj = pEntityType->GetObject(pGetComponentInstruction->m_u32ObjectIndex);

				if (! bReuseHandles)
				{
					#ifdef USE_STRING_HASHES_FOR_ENTITY_DEFINITIONS
					ObjectComponentHandle hComp = System::Get()->GetObjectManager()->GetObjectComponentFromHash(hObj, pGetComponentInstruction->m_ComponentName.m_Str);
					#else
					ObjectComponentHandle hComp = System::Get()->GetObjectManager()->GetObjectComponent(hObj, pGetComponentInstruction->m_ComponentName.m_Str);
					#endif

					pEntityType->SetComponentHandle(pGetComponentInstruction->m_u32ComponentIndex, hComp);
				}
				else
				{
					System::Get()->GetObjectManager()->ReuseObjectComponent(hObj, pEntityType->GetComponent(pGetComponentInstruction->m_u32ComponentIndex), pGetComponentInstruction->m_ComponentName.m_Str);
				}
			}
			break;

			default:
			{
				InterfaceController::Get().Error("invalid instruction encountered when trying to run entity type function");
				return false;
			}
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RunReverseEntityTypeFunction
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::RunReverseEntityTypeFunction(CommonFunctionHandle hFunction, EntityType* pEntityType, bool bMaintainHandles)
{
	if ((! m_bInited) || m_bSettingUp || hFunction >= (U32)(m_CommonFunctions.Size()))
	{
		InterfaceController::Get().Error("can't run reverse entity type function");
		return false;
	}

	for (S32 i = m_CommonFunctions[hFunction].m_Instructions.Size() - 1; i >= 0; i--)
	{
		switch (m_CommonFunctions[hFunction].GetInstruction(i)->Command())
		{
			case eLoadObject:
			{
				EntityLoadObjectInstruction* pLoadObjectInstruction = (EntityLoadObjectInstruction*)m_CommonFunctions[hFunction].GetInstruction(i);

				ObjectHandle hObj = pEntityType->GetObject(pLoadObjectInstruction->m_u32ObjectIndex);
				System::Get()->GetObjectManager()->Destroy(hObj, bMaintainHandles);

				if (! bMaintainHandles)
				{
					ObjectHandle hInvalid;
					pEntityType->SetObjectHandle(pLoadObjectInstruction->m_u32ObjectIndex, hInvalid);
				}
			}
			break;

			case eGetObjectComponent:
			{
				EntityGetObjectComponentInstruction* pGetComponentInstruction = (EntityGetObjectComponentInstruction*)m_CommonFunctions[hFunction].GetInstruction(i);

				ObjectComponentHandle hObjComp = pEntityType->GetComponent(pGetComponentInstruction->m_u32ComponentIndex);
				System::Get()->GetObjectManager()->ReleaseObjectComponentHandle(hObjComp, bMaintainHandles);
			}
			break;

			default:
			{
				InterfaceController::Get().Error("invalid instruction encountered when trying to run entity type function");
				return false;
			}
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RunEntityInstanceFunction
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::RunEntityInstanceFunction(LocalFunctionHandle hFunction, EntityInstance* pEntityInstance, bool bReuseHandles)
{
	if ((! m_bInited) || m_bSettingUp || hFunction >= (U32)(m_LocalFunctions.Size()))
	{
		InterfaceController::Get().Error("can't run entity instance function");
		return false;
	}

	for (S32 i = 0; i < m_LocalFunctions[hFunction].m_Instructions.Size(); i++)
	{
		switch (m_LocalFunctions[hFunction].GetInstruction(i)->Command())
		{
			case eSpawnInstance:
			{
				EntitySpawnInstanceInstruction* pSpawnInstanceInstruction = (EntitySpawnInstanceInstruction*)m_LocalFunctions[hFunction].GetInstruction(i);
				EntityType* pParentEntityType = pEntityInstance->GetParentEntityType();

				if (! pParentEntityType)
				{
					InterfaceController::Get().Error("couldn't retrieve a parent entity type when running an entity instance function");
					return false;
				}

				ObjectHandle hObj = pParentEntityType->GetObject(pSpawnInstanceInstruction->m_u32ObjectIndex);

				if (! bReuseHandles)
				{
					InstanceHandle hInst = System::Get()->GetObjectManager()->CreateObjectInstance(hObj, GroupName().Chars(), Name().Chars());
					pEntityInstance->SetInstanceHandle(pSpawnInstanceInstruction->m_u32InstanceIndex, hInst);
				}
				else
					System::Get()->GetObjectManager()->CreateObjectInstanceUsingHandle(hObj, pEntityInstance->GetInstance(pSpawnInstanceInstruction->m_u32InstanceIndex), GroupName().Chars(), Name().Chars());
			}
			break;

			case eGetInstanceComponent:
			{
				// TODO
				EntityGetInstanceComponentInstruction* pGetInstanceComponentInstruction = (EntityGetInstanceComponentInstruction*)m_LocalFunctions[hFunction].GetInstruction(i);
				InstanceHandle hInst = pEntityInstance->GetInstance(pGetInstanceComponentInstruction->m_u32InstanceIndex);
				EntityType* pParentEntityType = pEntityInstance->GetParentEntityType();

				if (! pParentEntityType)
				{
					InterfaceController::Get().Error("couldn't retrieve a parent entity type when running an entity instance function");
					return false;
				}

				ObjectComponentHandle hComp = pParentEntityType->GetComponent(pGetInstanceComponentInstruction->m_u32ObjectComponentIndex);

				if (! bReuseHandles)
				{
					InstanceComponentHandle hInstComp = System::Get()->GetObjectManager()->GetInstanceComponentHandle(hComp, hInst);
					pEntityInstance->SetConnectorHandle(pGetInstanceComponentInstruction->m_u32InstanceComponentIndex, hInstComp);
				}
				// otherwise, should be the same as it was last time... we don't need to look anything up again as the handle is just a combination of 2 other handles
			}
			break;

			case eConnectInternal:
			{
				EntityConnectInternalInstruction* pConnectInternalInstruction = (EntityConnectInternalInstruction*)m_LocalFunctions[hFunction].GetInstruction(i);

				InstanceComponentHandle hUpstream = pEntityInstance->GetConnector(pConnectInternalInstruction->m_u32UpstreamConnectionIndex);
				InstanceComponentHandle hDownstream = pEntityInstance->GetConnector(pConnectInternalInstruction->m_u32DownstreamConnectionIndex);

				System::Get()->GetObjectManager()->ConnectGeneric(hUpstream, hDownstream);
			}
			break;

			default:
			{
				InterfaceController::Get().Error("invalid instruction encountered when trying to run entity instance function");
				return false;
			}
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RunReverseEntityInstanceFunction
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::RunReverseEntityInstanceFunction(LocalFunctionHandle hFunction, EntityInstance* pEntityInstance, bool bMaintainHandles)
{
	if ((! m_bInited) || m_bSettingUp || hFunction >= (U32)(m_LocalFunctions.Size()))
	{
		InterfaceController::Get().Error("can't run reverse entity instance function");
		return false;
	}

	for (S32 i = m_LocalFunctions[hFunction].m_Instructions.Size() - 1; i >= 0 ; i--)
	{
		switch (m_LocalFunctions[hFunction].GetInstruction(i)->Command())
		{
			case eSpawnInstance:
			{
				EntitySpawnInstanceInstruction* pSpawnInstanceInstruction = (EntitySpawnInstanceInstruction*)m_LocalFunctions[hFunction].GetInstruction(i);

				InstanceHandle hInst = pEntityInstance->GetInstance(pSpawnInstanceInstruction->m_u32InstanceIndex);

				System::Get()->GetObjectManager()->DestroyObjectInstance(hInst, bMaintainHandles);

				if (! bMaintainHandles)
				{
					InstanceHandle hInvalidInst;
					pEntityInstance->SetInstanceHandle(pSpawnInstanceInstruction->m_u32InstanceIndex, hInvalidInst);
				}
			}
			break;

			case eGetInstanceComponent:
			{
				// get instance component just synchronously combines two pre-existing handles,
				// so we don't need to do owt here
			}
			break;

			case eConnectInternal:
			{
				EntityConnectInternalInstruction* pConnectInternalInstruction = (EntityConnectInternalInstruction*)m_LocalFunctions[hFunction].GetInstruction(i);

				InstanceComponentHandle hUpstream = pEntityInstance->GetConnector(pConnectInternalInstruction->m_u32UpstreamConnectionIndex);
				InstanceComponentHandle hDownstream = pEntityInstance->GetConnector(pConnectInternalInstruction->m_u32DownstreamConnectionIndex);

				System::Get()->GetObjectManager()->DisconnectGeneric(hUpstream, hDownstream);
			}
			break;

			default:
			{
				InterfaceController::Get().Error("invalid instruction encountered when trying to run reverse entity instance function");
				return false;
			}
		}
	}

	return true;
}
 

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RunEntityInstanceParameterisedFunction
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::RunEntityInstanceParameterisedFunction(LocalFunctionHandle hFunction, EntityInstance* pEntityInstance, EntityInstance* pParameterInstance, bool bReuseHandles)
{
	if ((! m_bInited) || m_bSettingUp || hFunction >= (U32)(m_ParameterisedLocalFunctions.Size()))
	{
		InterfaceController::Get().Error("can't run entity instance function");
		return false;
	}

	#ifndef CFG_GOLD
	// check we are passing the correct type of entity
	Assert(m_ParameterisedLocalFunctions[hFunction].m_bAcceptParameter);
	
	U32 id = m_ParameterisedLocalFunctions[hFunction].m_u32ParameterId;

	EntityDefinition* pDef = InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(pParameterInstance->GetDefinitionHandle());
	if (pDef)
	{
		if (pDef->GetId() != id)
		{
			InterfaceController::Get().Error(Util::C_String().Printf("Trying to run parameterized function with invalid entity. Expected id 0x%x and got id 0x%x", id, pDef->GetId()).Chars());
		}
	}
	#endif

	for (S32 i = 0; i < m_ParameterisedLocalFunctions[hFunction].m_Instructions.Size(); i++)
	{
		switch (m_ParameterisedLocalFunctions[hFunction].GetInstruction(i)->Command())
		{
			case eSpawnInstance:
			{
				EntitySpawnInstanceInstruction* pSpawnInstanceInstruction = (EntitySpawnInstanceInstruction*)m_ParameterisedLocalFunctions[hFunction].GetInstruction(i);
				EntityType* pParentEntityType = pEntityInstance->GetParentEntityType();

				if (! pParentEntityType)
				{
					InterfaceController::Get().Error("couldn't retrieve a parent entity type when running an entity instance function");
					return false;
				}

				ObjectHandle hObj = pParentEntityType->GetObject(pSpawnInstanceInstruction->m_u32ObjectIndex);

				if (! bReuseHandles)
				{
					InstanceHandle hInst = System::Get()->GetObjectManager()->CreateObjectInstance(hObj, GroupName().Chars(), Name().Chars());
					pEntityInstance->SetInstanceHandle(pSpawnInstanceInstruction->m_u32InstanceIndex, hInst);
				}
				else
					System::Get()->GetObjectManager()->CreateObjectInstanceUsingHandle(hObj, pEntityInstance->GetInstance(pSpawnInstanceInstruction->m_u32InstanceIndex), GroupName().Chars(), Name().Chars());
			}
			case eGetInstanceComponent:
			{
				// TODO
				EntityGetInstanceComponentInstruction* pGetInstanceComponentInstruction = (EntityGetInstanceComponentInstruction*)m_ParameterisedLocalFunctions[hFunction].GetInstruction(i);
				InstanceHandle hInst = pEntityInstance->GetInstance(pGetInstanceComponentInstruction->m_u32InstanceIndex);
				EntityType* pParentEntityType = pEntityInstance->GetParentEntityType();

				if (! pParentEntityType)
				{
					InterfaceController::Get().Error("couldn't retrieve a parent entity type when running an entity instance function");
					return false;
				}

				ObjectComponentHandle hComp = pParentEntityType->GetComponent(pGetInstanceComponentInstruction->m_u32ObjectComponentIndex);

				if (! bReuseHandles)
				{
					InstanceComponentHandle hInstComp = System::Get()->GetObjectManager()->GetInstanceComponentHandle(hComp, hInst);
					pEntityInstance->SetConnectorHandle(pGetInstanceComponentInstruction->m_u32InstanceComponentIndex, hInstComp);
				}
				// otherwise, should be the same as it was last time... we don't need to look anything up again as the handle is just a combination of 2 other handles
			}
			break;

			case eConnectToParameterised:
			{
				EntityConnectToParameterisedInstruction* pConnectExternalInstruction = (EntityConnectToParameterisedInstruction*)m_ParameterisedLocalFunctions[hFunction].GetInstruction(i);

				InstanceComponentHandle hUpstream = pEntityInstance->GetConnector(pConnectExternalInstruction->m_u32UpstreamConnectionIndex);
				InstanceComponentHandle hDownstream = pParameterInstance->GetConnector(pConnectExternalInstruction->m_u32DownstreamConnectionIndex);

				System::Get()->GetObjectManager()->ConnectGeneric(hUpstream, hDownstream);
			}
			break;

			case eConnectFromParameterised:
			{
				EntityConnectFromParameterisedInstruction* pConnectExternalInstruction = (EntityConnectFromParameterisedInstruction*)m_ParameterisedLocalFunctions[hFunction].GetInstruction(i);

				InstanceComponentHandle hUpstream = pParameterInstance->GetConnector(pConnectExternalInstruction->m_u32UpstreamConnectionIndex);
				InstanceComponentHandle hDownstream = pEntityInstance->GetConnector(pConnectExternalInstruction->m_u32DownstreamConnectionIndex);

				System::Get()->GetObjectManager()->ConnectGeneric(hUpstream, hDownstream);
			}
			break;

			default:
			{
				InterfaceController::Get().Error("invalid instruction encountered when trying to run parameterised entity instance function");
				return false;
			}
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RunReverseEntityInstanceParameterisedFunction
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityDefinition::RunReverseEntityInstanceParameterisedFunction(LocalFunctionHandle hFunction, EntityInstance* pEntityInstance, EntityInstance* pParameterInstance, bool bMaintainHandles)
{
	if ((! m_bInited) || m_bSettingUp || hFunction >= (U32)(m_ParameterisedLocalFunctions.Size()))
	{
		InterfaceController::Get().Error("can't run reverse entity instance function");
		return false;
	}

	for (S32 i = m_ParameterisedLocalFunctions[hFunction].m_Instructions.Size() - 1; i >= 0; i--)
	{
		switch (m_ParameterisedLocalFunctions[hFunction].GetInstruction(i)->Command())
		{
			case eSpawnInstance:
			{
				EntitySpawnInstanceInstruction* pSpawnInstanceInstruction = (EntitySpawnInstanceInstruction*)m_ParameterisedLocalFunctions[hFunction].GetInstruction(i);

				InstanceHandle hInst = pEntityInstance->GetInstance(pSpawnInstanceInstruction->m_u32InstanceIndex);

				System::Get()->GetObjectManager()->DestroyObjectInstance(hInst);

				if (! bMaintainHandles)
				{
					InstanceHandle hInvalidInst;
					pEntityInstance->SetInstanceHandle(pSpawnInstanceInstruction->m_u32InstanceIndex, hInvalidInst);
				}
			}
			break;

			case eGetInstanceComponent:
			{
				// get instance component just synchronously combines two pre-existing handles,
				// so we don't need to do owt here
			}
			break;

			case eConnectToParameterised:
			{
				EntityConnectToParameterisedInstruction* pConnectExternalInstruction = (EntityConnectToParameterisedInstruction*)m_ParameterisedLocalFunctions[hFunction].GetInstruction(i);

				InstanceComponentHandle hUpstream = pEntityInstance->GetConnector(pConnectExternalInstruction->m_u32UpstreamConnectionIndex);
				InstanceComponentHandle hDownstream = pParameterInstance->GetConnector(pConnectExternalInstruction->m_u32DownstreamConnectionIndex);

				System::Get()->GetObjectManager()->DisconnectGeneric(hUpstream, hDownstream);
			}
			break;

			case eConnectFromParameterised:
			{
				EntityConnectFromParameterisedInstruction* pConnectExternalInstruction = (EntityConnectFromParameterisedInstruction*)m_ParameterisedLocalFunctions[hFunction].GetInstruction(i);

				InstanceComponentHandle hUpstream = pParameterInstance->GetConnector(pConnectExternalInstruction->m_u32UpstreamConnectionIndex);
				InstanceComponentHandle hDownstream = pEntityInstance->GetConnector(pConnectExternalInstruction->m_u32DownstreamConnectionIndex);

				System::Get()->GetObjectManager()->DisconnectGeneric(hUpstream, hDownstream);
			}
			break;

			default:
			{
				InterfaceController::Get().Error("invalid instruction encountered when trying to run parameterised entity instance function");
				return false;
			}
		}
	}

	return true;
}

void Audio::EntityDefinition::TrimArrays()
{
#ifdef TRIM_ARRAYS
	//MEMORY_TRACKER_PUSH("Audio.EntityDefinitions.Classes.CommonDeclarations");
	m_CommonDeclarations.TrimCapacity();
	//MEMORY_TRACKER_POP("Audio.EntityDefinitions.Classes.CommonDeclarations");

	//MEMORY_TRACKER_PUSH("Audio.EntityDefinitions.Classes.LocalDeclarations");
	m_LocalDeclarations.TrimCapacity();
	//MEMORY_TRACKER_POP("Audio.EntityDefinitions.Classes.LocalDeclarations");
	

	//MEMORY_TRACKER_PUSH("Audio.EntityDefinitions.Classes.LocalFuncs");
	m_LocalFunctions.TrimCapacity();
	//MEMORY_TRACKER_POP("Audio.EntityDefinitions.Classes.LocalFuncs");

	for (int i=0; i<m_LocalFunctions.Size(); i++)
	{
		//MEMORY_TRACKER_PUSH("Audio.EntityDefinitions.Classes.LocalFuncInstructions");
		m_LocalFunctions[i].m_Instructions.TrimCapacity();
		//MEMORY_TRACKER_POP("Audio.EntityDefinitions.Classes.LocalFuncInstructions");
	}

	//MEMORY_TRACKER_PUSH("Audio.EntityDefinitions.Classes.CommonFuncs");
	m_CommonFunctions.TrimCapacity();
	//MEMORY_TRACKER_POP("Audio.EntityDefinitions.Classes.CommonFuncs");

	for (int i=0; i<m_CommonFunctions.Size(); i++)
	{
		//MEMORY_TRACKER_PUSH("Audio.EntityDefinitions.Classes.CommonFuncInstructions");
		m_CommonFunctions[i].m_Instructions.TrimCapacity();
		//MEMORY_TRACKER_POP("Audio.EntityDefinitions.Classes.CommonFuncInstructions");
	}
	
	m_ParameterisedLocalFunctions.TrimCapacity();
	for (int i=0; i<m_ParameterisedLocalFunctions.Size(); i++)	
	{
		//MEMORY_TRACKER_PUSH("Audio.EntityDefinitions.Classes.ParamLocalFuncInstructions");
		m_ParameterisedLocalFunctions[i].m_Instructions.TrimCapacity();
		//MEMORY_TRACKER_POP("Audio.EntityDefinitions.Classes.ParamLocalFuncInstructions");
	}
#endif // TRIM_ARRAYS
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
