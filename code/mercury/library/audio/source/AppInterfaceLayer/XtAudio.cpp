//#include <system\memory.h>

#include "AudioPch.h"

#include "ObjectBankList.h"

#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#endif

#include "system\serialise.h"
#include <core/BizAssert.h>

#include <System/memory/MemoryTracker.h>

#include "XtAudio.h"
#include <Xt/XtGame.h>

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#include <system/misc/stringutil.h>
#include "EntityDefinition.h"

namespace Audio 
{
void EntityDefCallback(XtEventType event, XtRecord* record, Audio::XtEntityDef* data, XtCallbackContext* context);
void AudioObjectCallback(XtEventType event, XtRecord* record, Audio::XtAudioObject* data, XtCallbackContext* context);
void EntityGroupCallback(XtEventType event, XtRecord* record, Audio::XtEntityGroup* data, XtCallbackContext* context);
void AudioEntityProxyCallback(XtEventType event, XtRecord* record, Audio::XtEntityProxy* data, XtCallbackContext* context);

C_XtAudioDb* C_XtAudioDb::s_pInstance = 0;
bool XtEntityProxy::m_sbDefaultBool = false;

Debug::Bool gbAudioLogEntityDefinitionSetup("Audio.System.Logging.LogEntitySetup", false, Debug::Persist());

#ifdef AUDIO_DEFERRED_LOADING
Debug::Bool gbUseDeferredLoading("Audio.DeferredLoading", true);

Audio::C_DeferredLoadManager C_DeferredLoadManager::m_sInstance;
#endif

struct XtObject;

//=============================================================================
// AudioObject record
//=============================================================================
struct XtAudioObject
{
	XtI32 id;			// not xt annotated
	XtSz8 filename;
};

struct XtInstance
{
	XtInstance() : index(InvalidIndex) {}

	XtI32		index;						// not xt annotated. for internal use only
	XtSz8		name;
	XtObject*	object;
};

//=============================================================================
// Object and Instance
//=============================================================================

struct XtObject
{
	XtObject() : index(InvalidIndex) {}

	XtI32					index;			// not xt annotated. for internal use only
	XtHandle<XtAudioObject>	object;
};

//=============================================================================
// Internal Connections
//=============================================================================

struct XtInternalConnector
{
	XtInternalConnector() : index(InvalidIndex) {}

	XtI32		index;						// not xt annotated. for internal use only
	XtInstance*	instance;
	XtSz8		component;
};

struct XtInternalConnectorIn : XtInternalConnector
{
};

struct XtInternalConnectorOut : XtInternalConnector
{
};

struct XtInternalConnection
{
	XtInternalConnectorOut*	from;
	XtInternalConnectorIn*	to;
};

//=============================================================================
// External Connections
//=============================================================================

struct XtExternalConnector
{
	XtSz8	instanceFrom;
	XtSz8	componentFrom;
};

struct XtIncomingConnection
{
	XtExternalConnector		from;
	XtInternalConnectorIn*	to;
};

struct XtOutgoingConnection
{
	XtInternalConnectorOut*	from;
	XtExternalConnector		to;
};

//=============================================================================
// Functions
//=============================================================================

struct XtObjFunction
{
	XtArray<XtObject> objCreates;
};

struct XtInstFunction
{
	Ut::Vector<XtInternalConnector*>	connectors;		// not xt annotated. created by hand

	XtArray<XtInstance>				instances;
	XtArray<XtInternalConnectorOut>	connectorsOut;
	XtArray<XtInternalConnectorIn>	connectorsIn;
	XtArray<XtInternalConnection>	connections;
};

struct XtConnectFunction
{
	XtHandle<XtEntityDef>			otherEntity;
	XtArray<XtIncomingConnection>	incoming;
	XtArray<XtOutgoingConnection>	outgoing;
};

//=============================================================================
// Externals
//=============================================================================

struct XtExternalRef
{
	XtSz8					refString;
	XtInternalConnector*	connector;
};

struct XtExternalInstance
{
	XtInstance*	instance;
};

struct XtExternalParamOut
{
	XtSz8					refString;
	XtInternalConnector*	paramOut;
};

//=============================================================================
// XtEntityDef Record
//=============================================================================
struct XtEntityDef
{
public:
#ifdef CFG_GOLD
	XtEntityDef() : bVerifiedOk(true) {}
#else
	XtEntityDef() : bVerifiedOk(false) {}
#endif

	bool						SetupEntityDefinition(Audio::EntityDefinition* pEntityDef);
	void						SetupArrayIndices();

	// lookup by name
	const XtInstance*			LookupInstance(Util::C_String& strName) const;
	const XtInternalConnector*	LookupConnector(Util::C_String& strInstanceName, Util::C_String& strComponentName) const;

	S32							NumConnectors() const { return instFunction.connectors.Size(); }
	S32							NumInstances() const { return instFunction.instances.count; }
	S32							NumObjects() const { return objCreate.objCreates.count; }

	// lookup by index
	const XtInstance*			Instance(S32 index) const { return &instFunction.instances[index]; }
	const XtObject*				Object(S32 index) const { return &objCreate.objCreates[index]; }
	const XtInternalConnector*	Connector(S32 index) const { return instFunction.connectors[index]; }
	
	const char*					Name() const { return name.Text(); }
	
	bool						Verify();
	bool						VerifiedOk() const { return bVerifiedOk; }

public:
	XtI32						id;						// not xt annotated. for internal use only
	bool						bVerifiedOk;			// not xt annotated

	XtSz8						name;

	XtHandle<XtEntityGroup>		group;

	XtObjFunction				objCreate;
	XtInstFunction				instFunction;	

	XtArray<XtConnectFunction>	connectFuncs;
	XtArray<XtExternalRef>		externalRefs;

	XtArray<XtExternalInstance>	externInstances;
	XtArray<XtExternalParamOut>	externParamOuts;
};

};	// namespace Audio

//-----------------------------------------------------------------------------
// XT type registration
//-----------------------------------------------------------------------------
XT_LIBRARY(AudioXtLib)
	XT_REGISTER_TYPE(Audio::XtAudioObject)
	XT_REGISTER_TYPE(Audio::XtEntityGroup)
	XT_REGISTER_TYPE(Audio::XtEntityDef)
	XT_REGISTER_TYPE(Audio::XtEntityProxy)

	XT_REGISTER_TYPE(Audio::XtObject)
	XT_REGISTER_TYPE(Audio::XtInstance)
	XT_REGISTER_TYPE(Audio::XtInternalConnector)
	XT_REGISTER_TYPE(Audio::XtInternalConnectorIn)
	XT_REGISTER_TYPE(Audio::XtInternalConnectorOut)
	XT_REGISTER_TYPE(Audio::XtInternalConnection)
	XT_REGISTER_TYPE(Audio::XtExternalConnector)
	XT_REGISTER_TYPE(Audio::XtIncomingConnection)
	XT_REGISTER_TYPE(Audio::XtOutgoingConnection)
	XT_REGISTER_TYPE(Audio::XtObjFunction)
	XT_REGISTER_TYPE(Audio::XtInstFunction)
	XT_REGISTER_TYPE(Audio::XtConnectFunction)
	XT_REGISTER_TYPE(Audio::XtExternalRef)
	XT_REGISTER_TYPE(Audio::XtExternalInstance)
	XT_REGISTER_TYPE(Audio::XtExternalParamOut)
XT_END_APPLICATION

//=============================================================================
// AudioObject
//=============================================================================
XT_TYPE(Audio::XtAudioObject, "AudioObject")
	XT_CALLBACK(Audio::AudioObjectCallback)
	XT_FIELD(filename, "filename")
XT_END_TYPE

//=============================================================================
// AudioEntityGroup
//=============================================================================
XT_TYPE(Audio::XtEntityGroup, "AudioEntityGroup")
	XT_CALLBACK(Audio::EntityGroupCallback)
	XT_FIELD(name, "name")
	XT_FIELD(index, "index")
XT_END_TYPE

//=============================================================================
// AudioEntityName
//=============================================================================
XT_TYPE(Audio::XtEntityProxy, "AudioEntityProxy")
	XT_CALLBACK(Audio::AudioEntityProxyCallback)
	XT_FIELD(m_hXtEntity, "EntityDef")
XT_END_TYPE

//=============================================================================
// Object and Instance
//=============================================================================

XT_TYPE(Audio::XtObject, "AES_Object")
	XT_FIELD(object, "object")
XT_END_TYPE

XT_TYPE(Audio::XtInstance, "AES_Instance")
	XT_FIELD(name, "name")
	XT_FIELD(object, "object")
XT_END_TYPE

//=============================================================================
// Internal Connections
//=============================================================================

XT_TYPE(Audio::XtInternalConnector, "AES_InternalConnector")
	XT_FIELD(instance, "instance")
	XT_FIELD(component, "component")
XT_END_TYPE

XT_TYPE(Audio::XtInternalConnectorIn, "AES_InternalConnectorIn")
	XT_BASE(Audio::XtInternalConnector)
XT_END_TYPE

XT_TYPE(Audio::XtInternalConnectorOut, "AES_InternalConnectorOut")
	XT_BASE(Audio::XtInternalConnector)
XT_END_TYPE

XT_TYPE(Audio::XtInternalConnection, "AES_InternalConnection")
	XT_FIELD(from, "from")
	XT_FIELD(to, "to")
XT_END_TYPE

//=============================================================================
// External Connections
//=============================================================================

XT_TYPE(Audio::XtExternalConnector, "AES_ExternalConnector")
	XT_FIELD(instanceFrom, "instanceFrom")
	XT_FIELD(componentFrom, "componentFrom")
XT_END_TYPE

XT_TYPE(Audio::XtIncomingConnection, "AES_IncomingConnection")
	XT_FIELD(from, "from")
	XT_FIELD(to, "to")
XT_END_TYPE

XT_TYPE(Audio::XtOutgoingConnection, "AES_OutgoingConnection")
	XT_FIELD(from, "from")
	XT_FIELD(to, "to")
XT_END_TYPE

//=============================================================================
// Functions
//=============================================================================

XT_TYPE(Audio::XtObjFunction, "AES_ObjFunction")
	XT_FIELD(objCreates, "objCreates")
XT_END_TYPE

XT_TYPE(Audio::XtInstFunction, "AES_InstFunction")
	XT_FIELD(instances, "instances")
	XT_FIELD(connectorsOut, "connectorsOut")
	XT_FIELD(connectorsIn, "connectorsIn")
	XT_FIELD(connections, "connections")
XT_END_TYPE

XT_TYPE(Audio::XtConnectFunction, "AES_ConnectFunction")
	XT_FIELD(otherEntity, "connectEnv")
	XT_FIELD(incoming, "incoming")
	XT_FIELD(outgoing, "outgoing")
XT_END_TYPE

//=============================================================================
// Externals
//=============================================================================

XT_TYPE(Audio::XtExternalRef, "AES_External")
	XT_FIELD(refString, "referenceString")
	XT_FIELD(connector, "connector")
XT_END_TYPE

XT_TYPE(Audio::XtExternalParamOut, "AES_ExternalParamOut")
	XT_FIELD(refString, "referenceString")
	XT_FIELD(paramOut, "paramOut")
XT_END_TYPE

XT_TYPE(Audio::XtExternalInstance, "AES_ExternalInstance")
	XT_FIELD(instance, "instance")
XT_END_TYPE

//=============================================================================
// EntityDef Record
//=============================================================================

XT_TYPE(Audio::XtEntityDef, "AudioEntityDef")
	XT_CALLBACK(Audio::EntityDefCallback)
	XT_FIELD(group, "group")
	XT_FIELD(name, "name")
	XT_FIELD(objCreate, "objCreate")
	XT_FIELD(instFunction, "instFunction")	
	XT_FIELD(connectFuncs, "connectFunctions")
	XT_FIELD(externInstances, "externInstances")
	XT_FIELD(externParamOuts, "externParamOuts")
	XT_FIELD(externalRefs, "externs")
XT_END_TYPE

#define AUDIO_ENTITY_DEBUG( ... ) if (gbAudioLogEntityDefinitionSetup) AUDIO_SYS_DEBUG( __VA_ARGS__ )

//-----------------------------------------------------------------------------
// EntityDefCallback
//-----------------------------------------------------------------------------
void Audio::EntityDefCallback(XtEventType event, XtRecord* record, Audio::XtEntityDef* pDef, XtCallbackContext* context)
{
	if (event == kXtEtConstruct && pDef)
	{
		pDef->SetupArrayIndices();
		pDef->id = record->id;

		XtHandle<XtEntityDef> hnd = XtHandle<XtEntityDef>(record);		
		C_XtAudioDb::Get()->AddXtEntityDef(hnd);

		AUDIO_ENTITY_DEBUG("Got EntityDef '%s', 0x%x", pDef->name.string, record->id);
	}
	else
	if (event == kXtEtDestruct && pDef)
		AUDIO_ENTITY_DEBUG("DESTROYING EntityDef '%s', 0x%x", pDef->name.string, record->id);
	
}

//-----------------------------------------------------------------------------
// AudioObjectCallback
//-----------------------------------------------------------------------------
void Audio::AudioObjectCallback(XtEventType event, XtRecord* record, Audio::XtAudioObject* pObj, XtCallbackContext* context)
{
	if (event == kXtEtConstruct && pObj)
	{
		pObj->id = record->id;

		XtHandle<XtAudioObject> hnd = XtHandle<XtAudioObject>(record);		
		C_XtAudioDb::Get()->AddXtAudioObject(hnd);

		AUDIO_ENTITY_DEBUG("Got AudioObject '%s', 0x%x", pObj->filename.string, record->id);
	}
}

//-----------------------------------------------------------------------------
// EntityGroupCallback
//-----------------------------------------------------------------------------
void Audio::EntityGroupCallback(XtEventType event, XtRecord* record, Audio::XtEntityGroup* pDef, XtCallbackContext* context)
{
	if (event == kXtEtConstruct && pDef)
	{
		AUDIO_ENTITY_DEBUG("Got XtEntityGroup %d '%s', 0x%x", pDef->index, pDef->name.string, record->id);

		XtHandle<XtEntityGroup> hnd = XtHandle<XtEntityGroup>(record);
		C_XtAudioDb::Get()->AddXtEntityGroup(hnd);
	}
}

//-----------------------------------------------------------------------------
// AudioEntityProxyCallback
//-----------------------------------------------------------------------------
void Audio::AudioEntityProxyCallback(XtEventType event, XtRecord* record, Audio::XtEntityProxy* pDef, XtCallbackContext* context)
{
	if (event == kXtEtConstruct && pDef)
	{
		XtHandle<XtEntityProxy> hnd = XtHandle<XtEntityProxy>(record);		
		C_XtAudioDb::Get()->AddXtEntityProxy(hnd);

		pDef->SetName(pDef->m_hXtEntity ? Btl::String(pDef->m_hXtEntity->Name()) : "null");

		AUDIO_ENTITY_DEBUG("Got EntityProxy for '%s'", pDef->Name());

		pDef->id = record->id;
	}
}

//-----------------------------------------------------------------------------
// XtEntityDef::SetupArrayIndices
//-----------------------------------------------------------------------------
void Audio::XtEntityDef::SetupArrayIndices()
{
	#ifdef ENTITYDEF_USE_U16
	BIZ_ASSERT_MSG(objCreate.objCreates.count < 0xffff, "num objects in entity definition exceeds 16 bits");
	BIZ_ASSERT_MSG(instFunction.instances.count < 0xffff, "num instances in entity definition exceeds 16 bits");
	#endif

	for (int i=0; i<objCreate.objCreates.count; i++)
		objCreate.objCreates[i].index = i;

	for (int i=0; i<instFunction.instances.count; i++)
		instFunction.instances[i].index = i;

	int c = 0;
	for (int i=0; i<instFunction.connectorsOut.count; i++)
	{
		instFunction.connectors.PushBack(&instFunction.connectorsOut[i]);
		instFunction.connectorsOut[i].index = c++;
	}

	for (int i=0; i<instFunction.connectorsIn.count; i++)
	{
		instFunction.connectors.PushBack(&instFunction.connectorsIn[i]);
		instFunction.connectorsIn[i].index = c++;
	}

	#ifdef ENTITYDEF_USE_U16
	BIZ_ASSERT_MSG(c < 0xffff, "num connectors in entity definition exceeds 16 bits");
	#endif
}

//-----------------------------------------------------------------------------
// XtEntityDef::LookupInstance
//-----------------------------------------------------------------------------
const Audio::XtInstance* Audio::XtEntityDef::LookupInstance(Util::C_String& strName) const
{
	for (int i=0; i<NumInstances(); i++)
	{
		const XtInstance* pInstance = Instance(i);
		if (pInstance)
		{
			if (strName == pInstance->name.string)
				return pInstance;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
// XtEntityDef::LookupConnector
//-----------------------------------------------------------------------------
const Audio::XtInternalConnector* Audio::XtEntityDef::LookupConnector(Util::C_String& strInstanceName, Util::C_String& strComponentName) const
{
	for (int i=0; i<NumConnectors(); i++)
	{
		const XtInternalConnector* pConn = Connector(i);
		if (pConn && pConn->instance && pConn->component)
		{
			if (!stricmp(strInstanceName, pConn->instance->name.string) && !stricmp(strComponentName, pConn->component.string))
				return pConn;
		}
	}

	return 0;
}


//-----------------------------------------------------------------------------
// SetupEntityDefinition
// Given a pointer to an EntityDefinition class will set it up, creating
// functions, data members etc.
//-----------------------------------------------------------------------------
bool Audio::XtEntityDef::SetupEntityDefinition(Audio::EntityDefinition* pDef)
{
	if (!pDef)
		return false;

	#if !defined CFG_GOLD && !defined CFG_DEBUG
	if (!bVerifiedOk)
		return false;
	#endif

	pDef->Init(id);
	pDef->SetName(name.string);

	if (group)
		pDef->SetGroupName(group->name);

	AUDIO_ENTITY_DEBUG("ENTITY DEF: '%s'. id 0x%x", name.string, id);

	// currently we only allow 1 common function and 1 local function, SetupObject and CreateInstance
	Audio::CommonFunctionHandle hObjFunc = pDef->AddCommonFunction("SetupObject");
	Audio::LocalFunctionHandle hFunc = pDef->AddLocalFunction("CreateInstance");
	
	AUDIO_ENTITY_DEBUG("FUNC: '%s'", "SetupObject");

	//AUDIO_SYS_INFO("===================================", objCreate.objCreates.count + instFunction.connectors.Size());
	//AUDIO_SYS_INFO("Common func instructions needed %d", objCreate.objCreates.count + instFunction.connectors.Size());
	//AUDIO_SYS_INFO("Local func instructions needed %d", instFunction.instances.count + instFunction.connectors.Size() + instFunction.connections.count);
	//AUDIO_SYS_INFO("Local declarations needed %d", externInstances.count + externalRefs.count + externParamOuts.count);

//#define RESERVE_LISTS
#ifdef RESERVE_LISTS
	pDef->m_CommonFunctions[hObjFunc].m_Instructions.Reserve(objCreate.objCreates.count + instFunction.connectors.Size());
	pDef->m_LocalFunctions[hFunc].m_Instructions.Reserve(instFunction.instances.count + instFunction.connectors.Size() + instFunction.connections.count);
	pDef->m_LocalDeclarations.Reserve(externInstances.count + externalRefs.count + externParamOuts.count);
	// TODO : parameterised & connect func instructions
#endif

	// for each object in the objCreate function, create a LoadObject and GetObject command and 
	for (int c=0; c<objCreate.objCreates.count; c++)
	{	
		XtObject* pObject = &objCreate.objCreates[c];

		Audio::EntityLoadObjectInstruction* pInst = pDef->Allocate<Audio::EntityLoadObjectInstruction>();

		if (pInst)
		{
			Audio::EntityDataMemberHandle hObj = pDef->AddObjectMember();

			pInst->m_u32ObjectIndex = hObj;

			pInst->m_strFilename = pObject->object->filename.string;    
			pDef->AddInstructionToCommonFunction(hObjFunc, pInst);

			AUDIO_ENTITY_DEBUG("CMD :\t\tLoadObject '%s'. [hobj %d]", pInst->m_strFilename.Chars(), hObj);
		}
	}

	for (int j=0; j<instFunction.instances.count; j++)
	{
		XtInstance* pXtInst = &instFunction.instances[j];

		Audio::EntitySpawnInstanceInstruction* pInst = pDef->Allocate<Audio::EntitySpawnInstanceInstruction>();

		if (pInst)
		{
			Audio::EntityDataMemberHandle hInst = pDef->AddInstanceMember();

			pInst->m_u32ObjectIndex =  pXtInst->object->index;
			pInst->m_u32InstanceIndex = hInst;  
			pDef->AddInstructionToLocalFunction(hFunc, pInst);

			AUDIO_ENTITY_DEBUG("CMD :\t\tSpawnInstance '%s. [hobj %d hinst %d]", 
				pXtInst->name.string, 
				pXtInst->object->index, 
				pXtInst->index);
		}
	}

	for (int j=0; j<instFunction.connectors.Size(); j++)
	{
		XtInternalConnector* pConn = instFunction.connectors[j];
		
		Audio::EntityDataMemberHandle hObjCom;

		// create GetObjectComponent instruction
		{
			Audio::EntityGetObjectComponentInstruction* pInst = pDef->Allocate<Audio::EntityGetObjectComponentInstruction>();

			hObjCom = pDef->AddObjectComponentMember();

			pInst->m_u32ObjectIndex = pConn->instance->object->index;
			pInst->m_u32ComponentIndex = hObjCom;	
			pInst->m_ComponentName.Set(pConn->component.string);  
			pDef->AddInstructionToCommonFunction(hObjFunc, pInst);
		}	

		// create GetInstanceComponent instruction
		{
			Audio::EntityGetInstanceComponentInstruction* pInst2 = pDef->Allocate<Audio::EntityGetInstanceComponentInstruction>();

			if (pInst2)
			{
				Audio::EntityDataMemberHandle hInstCom = pDef->AddInstanceComponentMember();

				pInst2->m_u32InstanceComponentIndex = hInstCom;
				pInst2->m_u32ObjectComponentIndex = hObjCom; // needs to correspond with handle returned from AddObjectComponentMember?
				pInst2->m_u32InstanceIndex = pConn->instance->index;
				pDef->AddInstructionToLocalFunction(hFunc, pInst2);

				AUDIO_ENTITY_DEBUG("CMD :\t\tGetInstanceComponent '%s', '%s. [hinstcom %d hobjcom %d hinst %d]", 
					pConn->instance->name.string, 
					pConn->component.string, 
					hInstCom, 
					hObjCom, 
					pConn->instance->index);
			}	
		}
	}

	for (int j=0; j<instFunction.connections.count; j++)
	{
		// create ConnectInternal instruction
		XtInternalConnection* pConn = &instFunction.connections[j];

		Audio::EntityConnectInternalInstruction* pInst = pDef->Allocate<Audio::EntityConnectInternalInstruction>();

		if (pInst)
		{
			//pInst->m_u32ConnectorIndex = 0;									// ? might not be needed
			pInst->m_u32UpstreamConnectionIndex = pConn->from->index;		// instance component member
			pInst->m_u32DownstreamConnectionIndex = pConn->to->index;		// instance component member
			pDef->AddInstructionToLocalFunction(hFunc, pInst);

			AUDIO_ENTITY_DEBUG("CMD :\t\tConnectInternal from '%s' '%s' to '%s' '%s'. [from hinstcom %d to hinstcom %d]", 
				pConn->from->instance->name.string, 
				pConn->from->component.string, 
				pConn->to->instance->name.string, 
				pConn->to->component.string, 
				pConn->from->index, 
				pConn->to->index);
		}
	}

	// add external refs declarations
	for (int i=0; i<externalRefs.count; i++)
	{
		XtExternalRef* pRef = &externalRefs[i];
		
		pDef->DeclareAudioOutInstanceComponent(pRef->refString, pRef->connector->index);

		AUDIO_ENTITY_DEBUG("REF : Declare generic '%s' '%s' as '%s'. [instcom %d]", 
			pRef->connector->instance->name.string, 
			pRef->connector->component.string, 
			pRef->refString.string, 
			pRef->connector->index);
	}

	// add external instance declarations
	for (int i=0; i<externInstances.count; i++)
	{
		XtExternalInstance* pRef = &externInstances[i];
		
		pDef->DeclareInstance(pRef->instance->name, pRef->instance->index);

		AUDIO_ENTITY_DEBUG("REF : Declare instance '%s'. [hinst %d]", 
			pRef->instance->name.string, 
			pRef->instance->index);
	}

	// add parameter out external declarations
	for (int i=0; i<externParamOuts.count; i++)
	{
		XtExternalParamOut* pRef = &externParamOuts[i];
		
		pDef->DeclareParameterOutInstanceComponent(pRef->refString, pRef->paramOut->index);

		AUDIO_ENTITY_DEBUG("REF : Declare ParamOut '%s' '%s' as '%s'. [instcom %d]", 
			pRef->paramOut->instance->name.string, 
			pRef->paramOut->component.string, 
			pRef->refString.string, 
			pRef->paramOut->index);
	}

	// connect entity
	for (S32 i=0; i<connectFuncs.count; i++)
	{
		AUDIO_ENTITY_DEBUG("FUNC: 'DoConnections'. Other entity is '%s', [id 0x%x]", connectFuncs[i].otherEntity->name.string, connectFuncs[i].otherEntity->id);

		if (connectFuncs[i].otherEntity == NULL || !connectFuncs[i].otherEntity->VerifiedOk())
			continue;

		// create function
		Audio::LocalFunctionHandle hFunc = pDef->AddParameterisedLocalFunction("DoConnections", connectFuncs[i].otherEntity->id);

		// incoming connections
		for (int j=0; j<connectFuncs[i].incoming.count; j++)
		{
			XtIncomingConnection* pIncoming = &connectFuncs[i].incoming[j];
			XtHandle<Audio::XtEntityDef> hOtherEntity = connectFuncs[i].otherEntity;

			Util::C_String strInstanceName(pIncoming->from.instanceFrom.string);
			const XtInstance* pExternalInst = hOtherEntity->LookupInstance(strInstanceName);

			Util::C_String strComponentName(pIncoming->from.componentFrom.string);
			const XtInternalConnector* pExternalConn = hOtherEntity->LookupConnector(strInstanceName, strComponentName);

			Audio::EntityConnectFromParameterisedInstruction* pInst = pDef->Allocate<Audio::EntityConnectFromParameterisedInstruction>();

			if (pInst)
			{
				pInst->m_u32UpstreamConnectionIndex = pExternalConn->index;					// output. instance component member of EXTERNAL ENTITY
				pInst->m_u32DownstreamConnectionIndex = pIncoming->to->index;				// input. instance component member of this entity

				pDef->AddInstructionToParameterisedLocalFunction(hFunc, pInst);

				AUDIO_ENTITY_DEBUG("IN  :\t\tConnect from '%s' '%s' '%s' to '%s' '%s' '%s' [hinstcom %d to hinstcom %d]",
					hOtherEntity->Name(),
					pExternalInst->name.string, 
					pExternalConn->component.string,
					Name(),
					pIncoming->to->instance->name.string, 
					pIncoming->to->component.string,
					pInst->m_u32UpstreamConnectionIndex,
					pInst->m_u32DownstreamConnectionIndex);
			}
		}

		// outgoing connections
		for (int j=0; j<connectFuncs[i].outgoing.count; j++)
		{
			XtOutgoingConnection* pOutgoing = &connectFuncs[i].outgoing[j];
			XtHandle<Audio::XtEntityDef> hOtherEntity = connectFuncs[i].otherEntity;

			Util::C_String strInstanceName(pOutgoing->to.instanceFrom.string);
			const XtInstance* pExternalInst = hOtherEntity->LookupInstance(strInstanceName);

			Util::C_String strComponentName(pOutgoing->to.componentFrom.string);
			const XtInternalConnector* pExternalConn = hOtherEntity->LookupConnector(strInstanceName, strComponentName);

			Audio::EntityConnectToParameterisedInstruction* pInst = pDef->Allocate<Audio::EntityConnectToParameterisedInstruction>();

			if (pInst)
			{
				pInst->m_u32UpstreamConnectionIndex = pOutgoing->from->index;				// output. instance component member of this entity
				pInst->m_u32DownstreamConnectionIndex = pExternalConn->index;				// input. instance component member of EXTERNAL ENTITY

				pDef->AddInstructionToParameterisedLocalFunction(hFunc, pInst);

				AUDIO_ENTITY_DEBUG("OUT :\t\tConnect from '%s' '%s' '%s' to '%s' '%s' '%s' [hinstcom %d to hinstcom %d]", 
					Name(),
					pOutgoing->from->instance->name.string, 
					pOutgoing->from->component.string,
					hOtherEntity->Name(),
					pExternalInst->name.string, 
					pExternalConn->component.string,
					pInst->m_u32UpstreamConnectionIndex,
					pInst->m_u32DownstreamConnectionIndex);
			}
		}
	}

	pDef->EndSetup();

	AUDIO_ENTITY_DEBUG("ENTITY DEF: '%s' DONE", name.string);

	return true;
}

bool ContainsInvalidSpaceChar(const char* pchString)
{
	Btl::String str(pchString);
	if (str.Length() > 0)
	{
		S32 pos = str.Search(' ');
		if (pos == 0 || pos == str.Length()-1)
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// XtEntityDef::Verify
//-----------------------------------------------------------------------------
bool Audio::XtEntityDef::Verify()
{
	bVerifiedOk = true;

	#ifndef CFG_GOLD
	if (!strcmp(name, ""))
	{
		AUDIO_SYS_WARN("Entity Def with id 0x%x: Entity Def has no name.", id);
		bVerifiedOk = false;
	}
	
	// check objects
	for (S32 j=0; j<objCreate.objCreates.count; j++)
	{
		XtObject& obj = objCreate.objCreates[j];
		
		if (obj.object == NULL)
		{
			AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: LoadObjects contains NULL AudioObject field.", Name(), id);
			bVerifiedOk = false;
		}

		if (obj.index == InvalidIndex)
		{
			AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Object has invalid index.", Name(), id);
			bVerifiedOk = false;
		}
	}

	// check instances
	for (S32 j=0; j<instFunction.instances.count; j++)
	{
		XtInstance& instance = instFunction.instances[j];
		if (instance.index == InvalidIndex)
		{
			AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Instance '%s' has invalid index.", Name(), id, instance.name.string);
			bVerifiedOk = false;
		}

		if (instance.object == NULL)
		{
			AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Instance '%s' points to NULL Object.", Name(), id, instance.name.string);
			bVerifiedOk = false;
		}

		if (!strcmp(instance.name.string, ""))
		{
			AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Instance name string is empty.", Name(), id);
			bVerifiedOk = false;
		}
		else
		{
			if (ContainsInvalidSpaceChar(instance.name.string))
			{
				AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Instance name '%s' string starts or ends with a space.", Name(), id, instance.name.string);
				bVerifiedOk = false;
			}
		}
	}

	// check connectors
	if (instFunction.connectors.Size() != instFunction.connectorsIn.count + instFunction.connectorsOut.count)
	{
		AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Connectors not created properly.", Name(), id);
		bVerifiedOk = false;
	}

	for (int j=0; j<instFunction.connectors.Size(); j++)
	{
		XtInternalConnector* pConn = instFunction.connectors[j];
		if (!pConn)
		{
			AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Connector is NULL.", Name(), id);
			bVerifiedOk = false;
		}
		else
		{
			#ifdef ENTITYDEF_USE_U16
			if (pConn->index >= 0Xffff)
			{
				AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Connector is greater than 16 bits.", Name(), id);
				bVerifiedOk = false;
			}
			#endif

			if (pConn->index == InvalidIndex)
			{
				AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Connector has invalid index.", Name(), id);
				bVerifiedOk = false;
			}

			if (!strcmp(pConn->component.string, ""))
			{
				AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Connector has component with no name specified.", Name(), id);
				bVerifiedOk = false;
			}
			else
			{
				if (ContainsInvalidSpaceChar(pConn->component.string))
				{
					AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Connector name '%s' string starts or ends with a space.", Name(), id, pConn->component.string);
					bVerifiedOk = false;
				}
			}

			if (pConn->instance == NULL)
			{
				AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Connector points to NULL Instance.", Name(), id);
				bVerifiedOk = false;
			}
		}
	}

	// check for duplicate connectorsIn
	for (int j=0; j<instFunction.connectorsIn.count; j++)
	{
		XtInternalConnectorIn& conn = instFunction.connectorsIn[j];
		
		for (int k=0; k<instFunction.connectorsIn.count; k++)
		{
			if (j != k)
			{
				XtInternalConnectorIn& connToCmp = instFunction.connectorsIn[k];

				if (conn.instance && conn.instance == connToCmp.instance && !strcmp(conn.component, connToCmp.component))
				{
					AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Connections In has duplicate connectors. '%s' '%s'", 
						Name(), 
						id, 
						(!conn.instance->object || !conn.instance->object->object) ? "null" : conn.instance->object->object->filename.string, 
						conn.component.string);
					bVerifiedOk = false;
				}
			}
		}
	}

	// check for duplicate connectorsOut
	for (int j=0; j<instFunction.connectorsOut.count; j++)
	{
		XtInternalConnectorOut& conn = instFunction.connectorsOut[j];

		for (int k=0; k<instFunction.connectorsOut.count; k++)
		{
			if (j != k)
			{
				XtInternalConnectorOut& connToCmp = instFunction.connectorsOut[k];

				if (conn.instance == connToCmp.instance && !strcmp(conn.component, connToCmp.component))
				{
					AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Connectors Out has duplicate connectors. '%s'", Name(), id, conn.component.string);
					bVerifiedOk = false;
				}
			}
		}
	}

	// check connections
	for (S32 j=0; j<instFunction.connections.count; j++)
	{
		XtInternalConnection& conn = instFunction.connections[j];
		
		if (conn.from == NULL)
		{
			AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Connection has NULL 'from' field.", Name(), id);
			bVerifiedOk = false;
		}

		if (conn.to == NULL)
		{
			AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Connection has NULL 'to' field.", Name(), id);
			bVerifiedOk = false;
		}

		if (conn.from && conn.to)
		{
			// check for duplicate connections
			for (S32 k=0; k<instFunction.connections.count; k++)
			{
				if (j != k)
				{
					XtInternalConnection& connToCmp = instFunction.connections[k];

					if (connToCmp.from && connToCmp.to)
					{
						if (connToCmp.from == conn.from && connToCmp.to == conn.to)
						{
							AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Duplicate connection found. from '%s' '%s' to '%s' '%s'", 
								Name(), 
								id, 
								conn.from->instance ? conn.from->instance->name.string : "null", 
								conn.from->component.string, 
								conn.to->instance ? conn.to->instance->name.string : "null", 
								conn.to->component.string);
							bVerifiedOk = false;
						}
					}
				}
			}
		}
	}

	// check externals
	for (S32 j=0; j<externalRefs.count; j++)
	{
		XtExternalRef& external = externalRefs[j];

		if (!strcmp(external.refString, ""))
		{
			AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Extern reference string is empty.", Name(), id);
			bVerifiedOk = false;
		}
		else
		{
			if (ContainsInvalidSpaceChar(external.refString.Text()))
			{
				AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Extern reference '%s' string starts or ends with a space.", Name(), id, external.refString.string);
				bVerifiedOk = false;
			}
		}

		// check duplicate external reference string names
		for (int k=0; k<externalRefs.count; k++)
		{
			if (k != j)
			{
				XtExternalRef& currExtern = externalRefs[k];

				if (!strcmp(external.refString.string, currExtern.refString.string))
				{
					AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Duplicate external reference string '%s'.", Name(), id, external.refString.string);
					bVerifiedOk = false;
				}
			}
		}

		if (external.connector == NULL)
		{
			AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Extern points to NULL connector.", Name(), id);
			bVerifiedOk = false;
		}
	}	

	// check external instances
	for (S32 i=0; i<externInstances.count; i++)
	{
		if (externInstances[i].instance == NULL)
		{	
			AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: External instance %d is NULL.", Name(), id, i);
			bVerifiedOk = false;
		}
	}

	// check param out externals
	for (S32 j=0; j<externParamOuts.count; j++)
	{
		XtExternalParamOut& external = externParamOuts[j];

		if (!strcmp(external.refString, ""))
		{
			AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: External paramout reference string is empty.", Name(), id);
			bVerifiedOk = false;
		}
		else
		{
			if (ContainsInvalidSpaceChar(external.refString.Text()))
			{
				AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Extern paramout reference '%s' string contains spaces.", Name(), id, external.refString.string);
				bVerifiedOk = false;
			}
		}

		// check duplicate external reference string names
		for (int k=0; k<externParamOuts.count; k++)
		{
			if (k != j)
			{
				XtExternalParamOut& currExtern = externParamOuts[k];

				if (!strcmp(external.refString.string, currExtern.refString.string))
				{
					AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Duplicate external paramout reference string '%s'.", Name(), id, external.refString.string);
					bVerifiedOk = false;
				}
			}
		}

		if (external.paramOut == NULL)
		{
			AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: External paramout points to NULL connector.", Name(), id);
			bVerifiedOk = false;
		}
	}	

	// check external connection functions
	for (S32 i=0; i<connectFuncs.count; i++)
	{
		if (!connectFuncs[i].otherEntity)
		{
			AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Connect function Other Entity is NULL", Name(), id);
			bVerifiedOk = false;
		}

		for (int j=0; j<connectFuncs[i].incoming.count; j++)
		{
			XtIncomingConnection& incoming = connectFuncs[i].incoming[j];
			XtHandle<Audio::XtEntityDef> hOtherEntity = connectFuncs[i].otherEntity;

			if (!incoming.to)
			{
				AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Connect function incoming %d : 'To' connector is NULL", Name(), id, j);
				bVerifiedOk = false;
			}

			if (hOtherEntity != NULL)
			{
				Util::C_String strInstanceName(incoming.from.instanceFrom.string);
				const XtInstance* pExternalInst = hOtherEntity->LookupInstance(strInstanceName);
				if (!pExternalInst)
				{
					AUDIO_SYS_WARN("Entity Def %s', id 0x%x : Connect function incoming %d : could not lookup external instance '%s'.", Name(), id, j, strInstanceName.Chars());
					bVerifiedOk = false;
				}

				Util::C_String strComponentName = incoming.from.componentFrom.string;
				const XtInternalConnector* pExternalConn = hOtherEntity->LookupConnector(strInstanceName, strComponentName);
				if (!pExternalConn)
				{
					AUDIO_SYS_WARN("Entity Def '%s', id 0x%x : Connect function incoming %d : could not lookup external connector '%s' in other entity '%s'", Name(), id, j, strComponentName.Chars(), hOtherEntity->Name());
					bVerifiedOk = false;
				}
			}
		}

		for (int j=0; j<connectFuncs[i].outgoing.count; j++)
		{
			XtOutgoingConnection& outgoing = connectFuncs[i].outgoing[j];
			XtHandle<Audio::XtEntityDef> hOtherEntity = connectFuncs[i].otherEntity;

			if (!outgoing.from)
			{
				AUDIO_SYS_WARN("Entity Def '%s', id 0x%x: Connect function outgoing %d : 'From' connector is NULL", Name(), id, j);
				bVerifiedOk = false;
			}

			if (hOtherEntity != NULL)
			{
				Util::C_String strInstanceName(outgoing.to.instanceFrom.string);
				const XtInstance* pExternalInst = hOtherEntity->LookupInstance(strInstanceName);
				if (!pExternalInst)
				{
					AUDIO_SYS_WARN("Entity Def %s', id 0x%x : Connect function outgoing %d : could not lookup external instance '%s'.", Name(), id, j, strInstanceName.Chars());
					bVerifiedOk = false;
				}

				Util::C_String strComponentName = outgoing.to.componentFrom.string;
				const XtInternalConnector* pExternalConn = hOtherEntity->LookupConnector(strInstanceName, strComponentName);
				if (!pExternalConn)
				{
					AUDIO_SYS_WARN("Entity Def '%s', id 0x%x : Connect function outgoing %d : could not lookup external connector '%s'. in other entity '%s'", Name(), id, j, outgoing.to.componentFrom.string, hOtherEntity->Name());
					bVerifiedOk = false;
				}
			}
		}
	}

	// check there is a corresponding proxy record
	Util::C_String str(Name());
	Audio::XtEntityProxy* pProxy = C_XtAudioDb::Get()->LookupXtEntityProxy(str);
	if (!pProxy)
	{
		AUDIO_SYS_WARN("Found AudioEntityDef '%s' with no matching AudioEntityProxy.", Name());
	}

	// make sure group is defined
	if (!group)
	{
		AUDIO_SYS_WARN("Found AudioEntityDef '%s' with no group defined.", Name());
	}
	#endif	// #ifdef CFG_GOLD

	return bVerifiedOk;
}

//-----------------------------------------------------------------------------
// XtEntityDef::SetupEntityType
// Desc: Load the audioobjects and banks for an entity.
// Note: If all audioobjects and banks have been loaded before calling
// this function then only the objects will be created. The objects will
// be created blocking, so in theory if all resources have been loaded
// this function will be relatively inexpensive.
//-----------------------------------------------------------------------------
bool Audio::XtEntityProxy::SetupEntityType(bool& bLoadedResources)
{
	bLoadedResources = false;

	if (!IsValid())
	{
		AUDIO_SYS_ERROR(DBSTR " failed as entity was not valid. '%s', id 0x%x. Note: look in the debug log and make sure entity was validated ok.", Name(), id);
		return false;
	}

	BIZ_ASSERT(m_sResourceRefCount >= 0);

	if (!m_sResourceRefCount)
	{
		//BIZ_ASSERT(m_hEntityDef.IsValid());
		//BIZ_ASSERT(!m_hEntityType.IsValid());

		#ifdef AUDIO_DEFERRED_LOADING
		if (C_DeferredLoadManager::Get().LoadingIsDeferred())
		{
			bool bAdded = false;
			bAdded = C_DeferredLoadManager::Get().AddBafsToQueue(m_WaveBankFnameList);
			bAdded = C_DeferredLoadManager::Get().AddDatsToQueue(m_DatFnameList) || bAdded;
			if (bAdded)
				AUDIO_SYS_DEBUG("Added resources to deferred load manager for audio entity '%s'", Name());

			return true;
		}
		else
		#endif // #ifdef AUDIO_DEFERRED_LOADING
		{
			AUDIO_SYS_DEBUG("Loading resources for audio entity '%s'", Name());

			Audio::InterfaceController::Get().CreateEntityType(m_hEntityDef, m_group != NULL ? m_group->index : 0, m_hEntityType);

			if (!m_hEntityType.IsValid())
			{
				AUDIO_SYS_ERROR(DBSTR "Could not create entity type for '%s'", Name());
				return false;
			}

			bLoadedResources = true;
		}
	}

	// prevent wrapping around. clamp at maximum value for signed 32 bit value
	if (m_sResourceRefCount+1 > 0)
		m_sResourceRefCount++;

	//AUDIO_SYS_INFO(DBSTR" '%s' %d", Name(), m_sResourceRefCount);

	return true;
}

//-----------------------------------------------------------------------------
// XtEntityDef::ReleaseEntityType
// Desc: Free the memory for the audioobjects and wavebanks for an entity.
//-----------------------------------------------------------------------------
void Audio::XtEntityProxy::ReleaseEntityType(bool& bReleasedResources)
{
	// took this out because resources where getting freed more times than loaded from the actor stuff for characters in nightwing.
	// was getting freed when destroying the actor then freed again when C_XtHandleDatabaseVisitor_FreeResources was called.
	// TODO: put this back in when the new resource loading and freeing stuff is put in?
	//BIZ_ASSERT(m_sResourceRefCount > 0);
	
	bReleasedResources = false;

	if (m_sResourceRefCount)
	{
		m_sResourceRefCount--;

		if (!m_sResourceRefCount)
		{
			if (!m_hEntityType.IsValid())
			{
				AUDIO_SYS_WARN(DBSTR " could not release entity type entity type was not valid. '%s', id 0x%x.", Name(), id);
				return;
			}

			AUDIO_SYS_DEBUG("Freeing resources for audio entity '%s'", Name());			
			
			Audio::InterfaceController::Get().ReleaseEntityType(m_hEntityType);			
			m_hEntityType = AudioEntityTypeHandle();	// set to invalid handle

			bReleasedResources = true;
		}
	}

	//AUDIO_SYS_INFO(DBSTR" '%s' %d", Name(), m_sResourceRefCount);
}

//-----------------------------------------------------------------------------
// XtEntityProxy::SetEntity
//-----------------------------------------------------------------------------
void Audio::XtEntityProxy::SetEntity(EntityDefinitionHandle hDef)
{ 
	m_hEntityDef = hDef;
	if (m_hXtEntity)
	{
		m_group = m_hXtEntity->group;
		m_strName = m_hXtEntity->Name();
	}
}

//-----------------------------------------------------------------------------
// GameDbOpenCallback
//-----------------------------------------------------------------------------
void DbOpenCallback(struct XtPackOpenStatus* status, void* context)
{
	AUDIO_SYS_WARN("XtOpen: %s", status->message);
}

//-----------------------------------------------------------------------------
// C_XtAudioDb::XtInit
// Desc: call this if using in an app which doesnt setup Xt itself
//-----------------------------------------------------------------------------
bool Audio::C_XtAudioDb::XtInit()
{
	if (!XtInitialise())
		return false;

	Assert(!m_pXtDb);

	m_pXtDb = XtDbCreate(XtGetApplicationSchema(), 4096, &m_Context);
	if (!m_pXtDb)
		AUDIO_SYS_FATAL("XtDbCreate failed for Audio Xt db");

	XtNetworkStart(m_pXtDb);

	return false;
}

//-----------------------------------------------------------------------------
// C_XtAudioDb::LookupXtEntity
// Desc: lookup an xt entity by name
//-----------------------------------------------------------------------------
Audio::XtEntityDef* Audio::C_XtAudioDb::LookupXtEntity(const Util::C_String& strName)
{
	for (int i=0; i<m_XtDefs.Size(); i++)
	{
		if (!stricmp(strName, m_XtDefs[i]->name.string))
			return m_XtDefs[i];
	}

	return 0;
}

//-----------------------------------------------------------------------------
// C_XtAudioDb::LookupXtEntity
// Desc: lookup an xt entity by name
//-----------------------------------------------------------------------------
Audio::XtEntityProxy* Audio::C_XtAudioDb::LookupXtEntityProxy(const Util::C_String& strName)
{
	return LookupXtEntityProxyHandle(strName);
}

//-----------------------------------------------------------------------------
// C_XtAudioDb::LookupXtEntityProxyHandle
// Desc: lookup an xt entity XtHandle by name
//-----------------------------------------------------------------------------
XtHandle<Audio::XtEntityProxy> Audio::C_XtAudioDb::LookupXtEntityProxyHandle(const Util::C_String& strName)
{
	for (int i=0; i<m_XtProxyRecs.Size(); i++)
	{
		if (!stricmp(strName, m_XtProxyRecs[i]->Name()))
		{
			Audio::XtEntityProxy*  pProxy = m_XtProxyRecs[i].Object();
			return m_XtProxyRecs[i];
		}
	}

	return XtHandle<Audio::XtEntityProxy>();
}

//-----------------------------------------------------------------------------
// C_XtAudioDb::Create
//-----------------------------------------------------------------------------
bool Audio::C_XtAudioDb::Create()
{
	if (!s_pInstance)
		s_pInstance = new C_XtAudioDb;
	return s_pInstance != 0;
}

//-----------------------------------------------------------------------------
// C_XtAudioDb::Release
//-----------------------------------------------------------------------------
void Audio::C_XtAudioDb::Release(XtDb* pDb)
{
	if (s_pInstance)
	{
		s_pInstance->Flush(pDb);
		delete s_pInstance;
		s_pInstance = 0;
	}
}

//-----------------------------------------------------------------------------
// C_XtAudioDb::VerifyRecords
// Check all proxy records have an associated Entity record.
//-----------------------------------------------------------------------------
bool Audio::C_XtAudioDb::VerifyRecords()
{
	bool bOk = true;

	// disabled on a debug build and gold build.
	// this function can take _minutes_ to execute on a debug build if there are lots of records.
	#if !defined CFG_GOLD && !defined CFG_DEBUG
	StopWatch sw;

	AUDIO_SYS_DEBUG("C_XtAudioDb::VerifyRecords() : Verifying...");

	// verify audioobjects
	{
		for (S32 i=0; i<XtAudioObjects().Size(); i++)
		{
			Audio::XtAudioObject* pObj = XtAudioObjects()[i];
			if (!pObj)
			{
				LOGWARN(g_Log, "AudioObject, id 0x%x: AudioObject is NULL.", pObj->id);
				bOk = false;
			}
			else
			{
				if (!strcmp(pObj->filename.string, ""))
				{
					LOGWARN(g_Log, "AudioObject, id 0x%x: filename is empty.", pObj->id);
					bOk = false;
				}
				else
				{
					if (ContainsInvalidSpaceChar(pObj->filename.string))
					{
						LOGWARN(g_Log, "AudioObject, id 0x%x: filename '%s' starts or ends with a space.", pObj->id, pObj->filename.string);
						bOk = false;
					}

				}

				Util::C_String strExt = Sys::ExtractFileExtension(pObj->filename.string).Chars();
				if (stricmp(strExt, "dat") != 0)
				{
					LOGWARN(g_Log, "AudioObject, id 0x%x: filename '%s' should have .dat extension.", pObj->id, pObj->filename.string ? pObj->filename.string : "NULL");
					bOk = false;
				}
			}
		}
	}

	// verify EntityDef records
	for (S32 i=0; i<XtEntities().Size(); i++)
	{
		XtHandle<Audio::XtEntityDef> hEntity = XtEntities()[i];

		if (hEntity)
		{
			// check for duplicate named records
			Util::C_String str(hEntity->Name());
			Audio::XtEntityDef* pEntity = LookupXtEntity(str);
			if (pEntity)
			{
				if ((void*)pEntity != hEntity.Object())
				{
					AUDIO_SYS_ERROR("Found duplicate AudioEntityDefs with name '%s'. ids are 0x%x & 0x%x", hEntity->Name(), hEntity.Id(), pEntity->id);
					bOk = false;
				}
			}

			if (!hEntity->Verify())
				bOk = false;
		}
		else
		{
			AUDIO_SYS_WARN("Could not get entity def record index %d", i);
		}
	}

	// verify EntityProxy records
	for (int i=0; i<XtEntityProxies().Size(); i++)
	{
		XtHandle<Audio::XtEntityProxy> hProxy = XtEntityProxies()[i];

		if (hProxy)
		{
			if (!hProxy->m_hXtEntity)
			{
				AUDIO_SYS_WARN("Found AudioEntityProxy with no AudioEntityDef set. id = 0x%x", hProxy.Id());
				bOk = false;
			}

			// check for duplicate named records
			Util::C_String strName(hProxy->Name());
			Audio::XtEntityProxy* pProxy = LookupXtEntityProxy(strName);
			if (pProxy)
			{
				if ((void*)pProxy != hProxy.Object())
				{
					AUDIO_SYS_ERROR("Found duplicate AudioEntityProxies with name '%s'. ids are 0x%x & 0x%x", hProxy->Name(), hProxy.Id(), pProxy->id);
					bOk = false;
				}
			}
		}
		else
		{
			AUDIO_SYS_WARN("Could not get proxy record index %d", i);
		}
	}

	if (bOk)
		AUDIO_SYS_DEBUG("C_XtAudioDb::VerifyRecords() : Verified OK. Took %.2f secs", sw.Time());
	else
		AUDIO_SYS_ERROR("C_XtAudioDb::VerifyRecords() : Found issues. (make sure warnings are on in order to view issues in detail)");
	#endif

	return bOk;
}

//-----------------------------------------------------------------------------
// C_XtAudioDb::CreateEntityDefClasses
// Iterate through all XtEntities and create an EntityDefinition class for 
// each one, setting up the XtEntityProxys at the same time. (storing the
// EntityDefinitionHandle in the XtEntityProxy so it can be used later to
// create EntityTypes and EntityInstances.)
//-----------------------------------------------------------------------------
void Audio::C_XtAudioDb::CreateEntityDefClasses()
{
	VerifyRecords();

	StopWatch sw;

	MEMORY_TRACKER_PUSH("Audio.EntityDefinitions.Classes");

	AUDIO_SYS_DEBUG("==== setting up %d EntityDefinitions ====", XtEntities().Size());

	Audio::EntityManager& entityManager = Audio::InterfaceController::Get().GetEntityManager();

	// TODO: allow each game to set this
	MEMORY_TRACKER_PUSH("Audio.EntityDefinitions.Classes.Allocator");
	EntityDefinition::Allocator().Reserve(512 * 1024);
	MEMORY_TRACKER_POP("Audio.EntityDefinitions.Classes.Allocator");
	
	for (int i=0; i<XtEntities().Size(); i++)
	{
		XtHandle<Audio::XtEntityDef> hXtDef = Get()->XtEntities()[i];
		
		if (hXtDef.Object())
		{
			AUDIO_ENTITY_DEBUG("%d setting up EntityDefinition '%s', id = 0x%x..., addr 0x%x", i, hXtDef->name.string, hXtDef.Id(), (U32)(INTPTR)hXtDef.Object());	// [Blur2PC][x64] debug print
			
			Audio::XtEntityProxy* pProxy=0;

			Util::C_String str(hXtDef->name.string);
			pProxy = LookupXtEntityProxy(str);
			if (!pProxy)
			{
				AUDIO_SYS_WARN("Could not setup entity '%s'. Entity Proxy not found", hXtDef->name.string);
				continue;
			}	

			Audio::EntityDefinitionHandle hDef = entityManager.CreateDefinition();

			Audio::EntityDefinition* pDef = entityManager.GetDefinitionFromHandle(hDef);

			if (!pDef || hXtDef->SetupEntityDefinition(pDef) == false)
			{
				AUDIO_SYS_WARN("Could not create entity definition for '%s'", hXtDef->name.string);
				entityManager.ReleaseDefinition(hDef);
				hDef = hDef.GetInvalidHandle();
			}	

			// set the name of the proxy anyway - even if the EntityDefinition create fails - otherwise
			// it makes it a bit hard to debug stuff if there is no name set
			pProxy->SetEntity(hDef);
			AUDIO_SYS_DEBUG("setting up Proxy '%s'", pProxy->Name());
		}
	}

	AUDIO_SYS_DEBUG("==== set up EntityDefinitions OK. Took %.2f secs ====", sw.Time());

	MEMORY_TRACKER_POP("Audio.EntityDefinitions.Classes");	
}
  
//-----------------------------------------------------------------------------
// C_XtAudioDb::SetupEntityResourceFileLists
//-----------------------------------------------------------------------------
bool Audio::C_XtAudioDb::SetupEntityResourceLists(const Sys::C_FilePath& strPath)
{
	StopWatch sw;
	C_ObjectBankList bankLists;

	bool bOk = bankLists.Load(strPath);

	AUDIO_SYS_INFO("SetupEntityResourceLists() Loading wbl file took %.3f seconds", sw.Time());

	if (!bOk)
	{
		AUDIO_SYS_ERROR("Cannot load wave bank list in directory '%s'", strPath.Chars());
		return false;
	}

	for (int i=0; i<XtEntityProxies().Size(); i++)
	{
		XtHandle<Audio::XtEntityProxy> hProxy = XtEntityProxies()[i];

		if (hProxy && hProxy->XtEntityDef() && hProxy->XtEntityDef()->VerifiedOk())
		{
			for (int j=0; j<hProxy->XtEntityDef()->NumObjects(); j++)
			{
				// add audio object filename to resource filename list for this entity
				const XtObject* pObject = hProxy->XtEntityDef()->Object(j);
				if (pObject)
				{
					hProxy->m_DatFnameList.PushBack(pObject->object->filename.Text());

					// add wavebank filename(s) to resource filename list for this entity
					const BankList* pList = bankLists.Lookup(pObject->object->filename.Text());
					if (pList)
					{
						for (int k=0; k<pList->Size(); k++)
						{
							// only add if doesnt already exist
							int l = 0;
							for (; l<hProxy->m_WaveBankFnameList.Size(); l++)
							{
								if (hProxy->m_WaveBankFnameList[l] == (*pList)[k])
									break;
							}
							if (l == hProxy->m_WaveBankFnameList.Size())
								hProxy->m_WaveBankFnameList.PushBack((*pList)[k]);
						}
					}
					else
					{
						AUDIO_SYS_DEBUG("Cannot look up bank list for object '%s' in record '%s' id 0x%x", pObject->object->filename.Text(), hProxy->Name(), hProxy->id);
					}
				}
			}
		}
	}

	AUDIO_SYS_INFO("SetupEntityResourceLists took total %.3f seconds", sw.Time());

#ifndef CFG_GOLD
	AUDIO_SYS_DEBUG("--------Entity resource file lists:---------");

	for (int i=0; i<XtEntityProxies().Size(); i++)
	{
		XtHandle<Audio::XtEntityProxy> hProxy = XtEntityProxies()[i];
		if (hProxy)
		{
			AUDIO_SYS_DEBUG("Entity '%s':", hProxy->Name());
			for (int i=0; i<hProxy->m_DatFnameList.Size(); i++)
				AUDIO_SYS_DEBUG("   '%s'", hProxy->m_DatFnameList[i].Chars());
			for (int i=0; i<hProxy->m_WaveBankFnameList.Size(); i++)
				AUDIO_SYS_DEBUG("   '%s'", hProxy->m_WaveBankFnameList[i].Chars());
		}
	}
	AUDIO_SYS_DEBUG("------------------------------------------");
#endif

	return true;
}

//-----------------------------------------------------------------------------
// C_XtAudioPack::LoadDefs
//-----------------------------------------------------------------------------
void Audio::C_XtAudioPack::LoadDefs(const Sys::C_FilePath& strDbFName, XtDb* pDbIn)
{
	MEMORY_TRACKER_PUSH("Audio.XtDefs");

	XtDb* pDb = NULL;

	if (!pDbIn)
		pDb = m_pAudioDb->m_pXtDb;
	else
		pDb = pDbIn;

	if (!pDb)
	{
		AUDIO_SYS_ERROR("Cannot create pack '%s': No Db present", strDbFName.Chars());
		MEMORY_TRACKER_POP("Audio.XtDefs");
		return;
	}

	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(strDbFName);

	if (pFile)
	{
		U32 length = static_cast<U32>(pFile->Length());
		U8*	data	= new U8[length];

		if (data)
		{
			if (pFile->Read(data, length))
			{
				XtPackOpenParams params;
				params.callback = DbOpenCallback;
				params.context = NULL;

				m_pXtPack = XtPackOpen(pDb, data, length, &params);

				if (!m_pXtPack)
					AUDIO_SYS_ERROR("XtPackOpen failed on sorcery pack '%s'", strDbFName.Chars());
				else
					AUDIO_SYS_DEBUG("Opened sorcery pack '%s' ok", strDbFName.Chars());
			}
			else
				AUDIO_SYS_ERROR("Opened sorcery pack file '%s' but could not read from it", strDbFName.Chars());
				
			delete [] data;
		}
		else
		{
			AUDIO_SYS_ERROR("Could not allocate %d bytes for buffer for reading in sorcery pack '%s'", length, strDbFName.Chars());
		}

		delete pFile;
	}
	else
	{
		AUDIO_SYS_ERROR("Failed to open sorcery pack file '%s'", strDbFName.Chars());
	}

	MEMORY_TRACKER_POP("Audio.XtDefs");
}

//-----------------------------------------------------------------------------
// C_XtAudioPack::Flush
//-----------------------------------------------------------------------------
void Audio::C_XtAudioPack::Flush(XtDb* pDbIn)
{
	MEMORY_TRACKER_PUSH("Audio.XtDefs");

	XtDb* pDb = pDbIn ? pDbIn : m_pAudioDb->m_pXtDb;
	if (pDb && m_pXtPack)
	{
		XtPackClose(pDb, m_pXtPack);
		m_pXtPack = NULL;
	}

	MEMORY_TRACKER_POP("Audio.XtDefs");
}



//-----------------------------------------------------------------------------
// C_DeferredLoadManager
//-----------------------------------------------------------------------------

#ifdef AUDIO_DEFERRED_LOADING

void Audio::C_DeferredLoadManager::LoadBafs()
{
	StopWatch sw;

	Btl::String strPath("\\audio\\");

	ResourceList::Iterator it;
	for (it = m_BafList.Begin(); it != m_BafList.End(); ++it)
	{
		if (m_pWaveManager)
		{
			Btl::String str(strPath + it->GetKey());
			WM_WAVE_HANDLE hBank = m_pWaveManager->LoadBank(str.Lower().Chars());
			
			// store on the bank handles list so we can release later to return the ref counts to how they should be
			if (hBank != WM_INVALID_HANDLE)
				m_BankHandleList.PushBack(hBank);
		}
	}

	if (!m_BafList.Empty())
		AUDIO_SYS_INFO("Loaded %d audio baf files in %.3f seconds", m_BafList.Size(), sw.Time());
	
	m_BafList.Clear();
}

void Audio::C_DeferredLoadManager::LoadDats()
{
	StopWatch sw;

	Btl::String strPath("audio\\dat\\");

	ResourceList::Iterator it;
	for (it = m_DatList.Begin(); it != m_DatList.End(); ++it)
	{
		Definition* pDef = System::Get()->GetDefinitionManager()->Create(Btl::String(strPath + it->GetKey()).Chars());

		// store on the defs list so we can release later to return the ref counts to how they should be
		if (pDef)
			m_DefsList.PushBack(pDef);
	}

	if (!m_DatList.Empty())
		AUDIO_SYS_INFO("Loaded %d audio dat files in %.3f seconds", m_DatList.Size(), sw.Time());
	
	m_DatList.Clear();
}

void Audio::C_DeferredLoadManager::Load()
{
	LoadBafs();
	LoadDats();
}

void Audio::C_DeferredLoadManager::DecrementRefCounts()
{
	AUDIO_SYS_DEBUG("Decrementing ref counts for %d defs", m_DefsList.Size());

	{
		DefinitionList::Iterator it;
		for (it = m_DefsList.Begin(); it != m_DefsList.End(); ++it)
		{
			 System::Get()->GetDefinitionManager()->Release(*it);
		}
		
		m_DefsList.Clear();
	}

	AUDIO_SYS_DEBUG("Decrementing ref counts for %d banks", m_BankHandleList.Size());

	{
		BankHandleList::Iterator it;
		for (it = m_BankHandleList.Begin(); it != m_BankHandleList.End(); ++it)
		{
			 m_pWaveManager->ReleaseBank(*it);
		}

		m_BankHandleList.Clear();
	}
}

void Audio::C_DeferredLoadManager::Clear()
{
	m_DefsList.Clear();
	m_BankHandleList.Clear();
}

bool Audio::C_DeferredLoadManager::AddBafsToQueue(Btl::Vector<Btl::String>& listBafNames)
{
	bool bAdded = false;

	for (S32 i=0; i<listBafNames.Size(); i++)
	{
		Btl::InsertionResult<ResourceList::Iterator > res = m_BafList.Insert(listBafNames[i]);
		if (res.inserted)
		{
			AUDIO_SYS_DEBUG(DBSTR" Added '%s' to baf queue", listBafNames[i].Chars());
			bAdded = true;
		}
	}
	return bAdded;
}

bool Audio::C_DeferredLoadManager::AddDatsToQueue(Btl::Vector<Btl::String>& listDatNames)
{
	bool bAdded = false;

	for (S32 i=0; i<listDatNames.Size(); i++)
	{
		Btl::InsertionResult<ResourceList::Iterator > res = m_DatList.Insert(listDatNames[i]);
		if (res.inserted)
		{
			AUDIO_SYS_DEBUG(DBSTR" Added '%s' to dat queue", listDatNames[i].Chars());
			bAdded = true;
		}
	}
	return bAdded;
}

#endif	// AUDIO_DEFERRED_LOADING

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
