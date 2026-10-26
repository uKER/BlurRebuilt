#include "Stdafx.h"
#include "Main.h"

#include "system\\serialise.h"
#include "system/misc/stringutil.h"
#include "shellapi.h"

#include <System/Debug/Log/LogEvent.h>
#include <System/Debug/Log/DebugLog.h>

#include "Common\XtFlask.h"

//#include "SorceryIntegration.h"

#define CREATE_CMDS

// for the pack with just the entities in
XtDb*				gpXtDb = 0;
XtCallbackContext	gXtContext_Entities('AUD_');
XtPack*				gpXtPack_Entities = 0;

// for the pack with the audioobjects in
XtCallbackContext	gXtContext_Audio('AUD_');
XtPack*				gpXtPack_Audio = 0;

RecordList			gRecordList;

Debug::Logger& g_Log = Debug::GetLogger("AudioTool");


void EntityScriptCallback(XtEventType event, XtRecord* record, AudioEntityDef* data, XtCallbackContext* context);
void AudioObjectCallback(XtEventType event, XtRecord* record, AudioObject* data, XtCallbackContext* context);
void EntityDefCallback(XtEventType event, XtRecord* record, DbEntityDef* data, XtCallbackContext* context);
void AudioEntityGroupCallback(XtEventType event, XtRecord* record, AudioEntityGroup* data, XtCallbackContext* context);

namespace Xt { bool OpenSchema(DomDb* db, const XtString& path, XtResult* xtResult); }


//-----------------------------------------------------------------------------
// XT type registration.
// this schema is for the pack we are converting from. ie native to sorcery.
//-----------------------------------------------------------------------------
XT_APPLICATION
	//XT_REGISTER_TYPE(DbEntityDef)

	XT_REGISTER_TYPE(AudioObject)
	XT_REGISTER_TYPE(Object)
	XT_REGISTER_TYPE(Instance)
	XT_REGISTER_TYPE(InternalConnector)
	XT_REGISTER_TYPE(InternalConnectorIn)
	XT_REGISTER_TYPE(InternalConnectorOut)
	XT_REGISTER_TYPE(InternalConnection)
	XT_REGISTER_TYPE(ExternalConnector)
	XT_REGISTER_TYPE(IncomingConnection)
	XT_REGISTER_TYPE(OutgoingConnection)
	XT_REGISTER_TYPE(ObjFunction)
	XT_REGISTER_TYPE(InstFunction)
	XT_REGISTER_TYPE(ConnectFunction)
	XT_REGISTER_TYPE(External)
	XT_REGISTER_TYPE(AudioEntityDef)
	XT_REGISTER_TYPE(AudioEntityGroup)
XT_END_APPLICATION

//-----------------------------------------------------------------------------
// XT types 
//-----------------------------------------------------------------------------

//=============================================================================
// AudioObject
//=============================================================================
XT_TYPE(AudioObject, "AudioObject")
	XT_CALLBACK(AudioObjectCallback)
	XT_FIELD(filename, "filename")
XT_END_TYPE

//=============================================================================
// Object and Instance
//=============================================================================

XT_TYPE(Object, "AES_Object")
	XT_FIELD(object, "object")
XT_END_TYPE

XT_TYPE(Instance, "AES_Instance")
	XT_FIELD(name, "name")
	XT_FIELD(object, "object")
XT_END_TYPE

//=============================================================================
// Internal Connections
//=============================================================================

XT_TYPE(InternalConnector, "AES_InternalConnector")
	XT_FIELD(instance, "instance")
	XT_FIELD(component, "component")
XT_END_TYPE

XT_TYPE(InternalConnectorIn, "AES_InternalConnectorIn")
	XT_BASE(InternalConnector)
XT_END_TYPE

XT_TYPE(InternalConnectorOut, "AES_InternalConnectorOut")
	XT_BASE(InternalConnector)
XT_END_TYPE

XT_TYPE(InternalConnection, "AES_InternalConnection")
	XT_FIELD(from, "from")
	XT_FIELD(to, "to")
XT_END_TYPE

//=============================================================================
// External Connections
//=============================================================================

XT_TYPE(ExternalConnector, "AES_ExternalConnector")
	XT_FIELD(instanceFrom, "instanceFrom")
	XT_FIELD(componentFrom, "componentFrom")
XT_END_TYPE

XT_TYPE(IncomingConnection, "AES_IncomingConnection")
	XT_FIELD(from, "from")
	XT_FIELD(to, "to")
XT_END_TYPE

XT_TYPE(OutgoingConnection, "AES_OutgoingConnection")
	XT_FIELD(from, "from")
	XT_FIELD(to, "to")
XT_END_TYPE

//=============================================================================
// Functions
//=============================================================================

XT_TYPE(ObjFunction, "AES_ObjFunction")
	XT_FIELD(objCreates, "objCreates")
XT_END_TYPE

XT_TYPE(InstFunction, "AES_InstFunction")
	XT_FIELD(instances, "instances")
	XT_FIELD(connectorsOut, "connectorsOut")
	XT_FIELD(connectorsIn, "connectorsIn")
	XT_FIELD(connections, "connections")
XT_END_TYPE

XT_TYPE(ConnectFunction, "AES_ConnectFunction")
	XT_FIELD(connectEnv, "connectEnv")
	XT_FIELD(incoming, "incoming")
	XT_FIELD(outgoing, "outgoing")
XT_END_TYPE

XT_TYPE(External, "AES_External")
	XT_FIELD(referenceString, "referenceString")
	XT_FIELD(connector, "connector")
XT_END_TYPE

//=============================================================================
// AudioEntityGroup
//=============================================================================

XT_TYPE(AudioEntityGroup, "AudioEntityGroup")
	XT_CALLBACK(AudioEntityGroupCallback)
	XT_FIELD(name, "name")
	XT_FIELD(index, "index")
XT_END_TYPE

//=============================================================================
// EntityScript Record
//=============================================================================

XT_TYPE(AudioEntityDef, "AudioEntityDef")
	XT_CALLBACK(EntityScriptCallback)
	XT_FIELD(group, "group")
	XT_FIELD(name, "name")
	XT_FIELD(objCreate, "objCreate")
	XT_FIELD(instFunction, "instFunction")	
	XT_FIELD(connectFunctions, "connectFunctions")
	XT_FIELD(externs, "externs")
XT_END_TYPE


//-----------------------------------------------------------------------------
// EntityScriptCallback
//-----------------------------------------------------------------------------
void EntityScriptCallback(XtEventType event, XtRecord* record, AudioEntityDef* pScript, XtCallbackContext* context)
{
	if (pScript)
	{
		LOGINFO(g_Log, "Got EntityDef '%s'\n", pScript->name);
		
		pScript->id = record->id;

		pScript->SetupArrayIndices();

		gRecordList.scripts.PushBack(pScript);
	}
}

//-----------------------------------------------------------------------------
// AudioEntityGroupCallback
//-----------------------------------------------------------------------------
void AudioEntityGroupCallback(XtEventType event, XtRecord* record, AudioEntityGroup* pGroup, XtCallbackContext* context)
{
	if (pGroup)
	{
		LOGINFO(g_Log, "Got Entity Group '%s'. index %d, id 0x%x\n", pGroup->name, pGroup->index, record->id);
		
		pGroup->id = record->id;

		gRecordList.groups.PushBack(pGroup);
	}
}

//-----------------------------------------------------------------------------
// DbEntityDefCallback
//-----------------------------------------------------------------------------
//void EntityDefCallback(XtEventType event, XtRecord* record, DbEntityDef* pDef, XtCallbackContext* context)
//{
//	LOGFATAL(g_Log, "AEDef record found! AEDef records are not allowed. Use EntityScript instead. Aborting...");
//}

//-----------------------------------------------------------------------------
// AudioObjectCallback
//-----------------------------------------------------------------------------
void AudioObjectCallback(XtEventType event, XtRecord* record, AudioObject* pObj, XtCallbackContext* context)
{
	if (pObj)
	{
		pObj->id = record->id;
		LOGINFO(g_Log, "Got AudioObject '%s'. id 0x%x\n", pObj->filename, pObj->id);
		gRecordList.audioObjects.PushBack(pObj);
	}
}

//-----------------------------------------------------------------------------
// SetupArrayIndices
//-----------------------------------------------------------------------------
void AudioEntityDef::SetupArrayIndices()
{
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
}

Util::C_String gStrPackOpenMsg;

//-----------------------------------------------------------------------------
// GameDbOpenCallback
//-----------------------------------------------------------------------------
void GameDbOpenCallback(struct XtPackOpenStatus* status, void* context)
{
	gStrPackOpenMsg = status->message;
	LOGINFO(g_Log, "XtPackOpen msg: '%s'", status->message);
}



//-----------------------------------------------------------------------------
// SorceryPackLoad
//-----------------------------------------------------------------------------
bool SorceryPackLoad(Util::C_String& strBinPath, XtDb*& pXtDb, XtPack*& pXtPack, XtCallbackContext& xtContext)
{
	if (!XtInitialise())
	{
		LOGERROR(g_Log, "XtInitialise failed\n");
		return false;
	}

	if (!pXtDb)
		pXtDb = XtDbCreate(XtGetApplicationSchema(), 1000, &xtContext);

	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(strBinPath);

	if (pFile)
	{
		U32 length	= (U32)pFile->Length();
		U8*	data	= new U8[length];

		if (data)
		{
			if (pFile->Read(data, length))
			{
				if (*((U32*)data) != kXtFlaskTag)
					LOGFATAL(g_Log, "Bin file ''%s is incorrect endian. Needs to be little endian (PC)'\n", strBinPath.Chars());

				XtPackOpenParams params;

				params.callback = GameDbOpenCallback;
				params.context = NULL;
				
				pXtPack = XtPackOpen(pXtDb, data, length, &params);

				if (pXtPack)
					LOGINFO(g_Log, "Opened sorcery pack '%s' ok\n", strBinPath.Chars());
				else
					LOGFATAL(g_Log, "XtPackOpen failed on sorcery pack '%s'. Reason: '%s'\n", strBinPath.Chars(), gStrPackOpenMsg.Chars());
			}
			else
			{
				LOGERROR(g_Log, "Opened sorcery pack file '%s' but could not read from it\n", strBinPath.Chars());
				delete [] data;
				return false;
			}

			delete [] data;
		}
		else
		{
			LOGERROR(g_Log, "Could not allocate %d bytes for buffer for reading in sorcery pack file\n", length);
			delete pFile;
			return false;
		}

		delete pFile;
	}
	else
		LOGFATAL(g_Log, "Failed to open sorcery pack file '%s'\n", strBinPath.Chars());

	return true;
}

//-----------------------------------------------------------------------------
// WriteRecords
// create the db records by hand for the entitydefs
//-----------------------------------------------------------------------------
void WriteRecords(Xt::DomDb& db, Xt::DomPack* pDomPack, Xt::DomPack* pDummyPack)
{
	//-----------------------------------------
	// add AudioObjects to the dummy pack
	//-----------------------------------------
	AudioObject* pObj;
	for (int i=0; i<gRecordList.audioObjects.Size(); i++)
	{
		pObj = gRecordList.audioObjects[i];
		if (!pObj)
			continue;
	
		Xt::DomRecord* pRecord = 0;

		if ((pRecord = Xt::AddRecord(pDummyPack, pObj->id, (const char*)Util::RemoveFileExtension(Util::C_String(pObj->filename.string)), "AudioObject", 0)) == 0)
		{
			LOGFATAL(g_Log, "Failed to create new record for AudioObject '%s'\n", pObj->filename.string ? pObj->filename.string : "NULL");
			return;
		}

		Xt::Set(&pRecord->root, XtString("filename"), pObj->filename);
	}

	//-----------------------------------------
	// add AudioEntityGroups to the dummy pack
	//-----------------------------------------
	AudioEntityGroup* pGroup;
	for (int i=0; i<gRecordList.groups.Size(); i++)
	{
		pGroup = gRecordList.groups[i];
		if (!pGroup)
			continue;
	
		Xt::DomRecord* pRecord = 0;

		if ((pRecord = Xt::AddRecord(pDummyPack, pGroup->id, XtString(pGroup->name.Text()), "AudioEntityGroup", 0)) == 0)
		{
			LOGFATAL(g_Log, "Failed to create new record for AudioEntityGroup '%s'\n", pGroup->name.string ? pGroup->name.string : "NULL");
			return;
		}

		Xt::Set(&pRecord->root, XtString("name"), pGroup->name);
		Xt::Set(&pRecord->root, XtString("index"), Util::C_String().Printf("%d", pGroup->index).Chars());
	}


	//-----------------------------------------
	// add EntityDefs
	//-----------------------------------------
	AudioEntityDef* pScript;
	for (int i=0; i<gRecordList.scripts.Size(); i++)
	{
		pScript = gRecordList.scripts[i];
	
		if (pScript == NULL) goto failed;
	
		Xt::DomRecord* pRecord = 0;

		if ((pRecord = Xt::AddRecord(pDomPack, pScript->id, (const char*)pScript->name, "AudioEntityDef", 0)) == 0)
		{
			LOGFATAL(g_Log, "Failed to add record for AudioDefinition '%s'\n", (const char*)pScript->name);
			return;
		}

		LOGINFO(g_Log, "\nAdded record for entity '%s'\n", (const char*)pScript->name);

		Xt::Set(&pRecord->root, XtString("name"), pScript->name);

		// set the entity group.
		// get the group record id from the EntityGroup in the entity script
		//if (pScript->group == NULL) goto failed;

		// group is currently optional
		if (pScript->group != NULL) 
		{
			XtString str = Format("%08X", pScript->group->id);
			Xt::Set(&pRecord->root, XtString("group"), str);
		}

		// create arrays
		Xt::DomArray* arrCommonFuncs =			AcquireArray(&pRecord->root, "commonFuncs");
		Xt::DomArray* arrLocalFuncs =			AcquireArray(&pRecord->root, "localFuncs");
		Xt::DomArray* arrConnectFuncs =			AcquireArray(&pRecord->root, "connectFuncs");
		Xt::DomArray* arrExternalRefs =			AcquireArray(&pRecord->root, "externalRefs");
		Xt::DomArray* arrInstCom =				AcquireArray(&pRecord->root, "instanceComponents");
		Xt::DomArray* arrInstances =			AcquireArray(&pRecord->root, "instances");
		Xt::DomArray* arrObjectCom =			AcquireArray(&pRecord->root, "objectComponents");
		Xt::DomArray* arrObjects =				AcquireArray(&pRecord->root, "objects");
		
		Xt::DomArray* arrCommonCmds = 0;
		Xt::DomArray* arrLocalCmds = 0;				

		Xt::DomBlock* pSetupObjectBlock = 0;
		Xt::DomBlock* pCreateInstanceBlock = 0;

		if (arrCommonFuncs == NULL) goto failed;
		if (arrLocalFuncs == NULL) goto failed;
		if (arrConnectFuncs == NULL) goto failed;	
		if (arrExternalRefs == NULL) goto failed;	
		if (arrInstCom == NULL) goto failed;		
		if (arrInstances == NULL) goto failed;	
		if (arrObjectCom == NULL) goto failed;	
		if (arrObjects == NULL)	goto failed;	
		
		// create SetupObject func
		pSetupObjectBlock = Xt::Append(arrCommonFuncs, new Xt::DomBlock);

		if (pSetupObjectBlock == NULL) goto failed;

		Xt::Set(pSetupObjectBlock, XtString("name"), XtString("SetupObject"));

		arrCommonCmds = AcquireArray(pSetupObjectBlock, "commands");
		
		if (arrCommonCmds == NULL) goto failed;	

		XtId id = Xt::CreateUniqueId(arrCommonCmds, "__id", 32768);		

		if (id == 0) goto failed;	

		XtString str = Format("%08X", id);
		Xt::Set(pSetupObjectBlock, XtString("__id"), str);
		

		// create CreateInstance func 
		pCreateInstanceBlock = Xt::Append(arrLocalFuncs, new Xt::DomBlock);

		if (pCreateInstanceBlock == NULL) goto failed;	

		Xt::Set(pCreateInstanceBlock, XtString("name"), XtString("CreateInstance"));

		arrLocalCmds = AcquireArray(pCreateInstanceBlock, "commands");

		if (arrLocalCmds == NULL) goto failed;	

		Xt::CreateUniqueId(arrLocalCmds, "__id", 32768);

		if (id == 0) goto failed;	

		str = Format("%08X", id);
		Xt::Set(pCreateInstanceBlock, XtString("__id"), str);


		// for each object create object and loadobject cmds
		for (int j=0; j<pScript->objCreate.objCreates.count; j++)
		{
			Object* pObj = &pScript->objCreate.objCreates[j];
			
			if (pObj == NULL) goto failed;	

			Xt::DomBlock* pObjBlock = Xt::Append(arrObjects, new Xt::DomBlock);

			if (pObjBlock == NULL) goto failed;	
			if (pObj->object == NULL) goto failed;	
			
			if (pObj->object->id == 0) goto failed;

			LOGINFO(g_Log, "   added Object, id ='0x%x'\n", pObj->object->id);

			// get the id from the AudioObject from the entity script
			XtString str = Format("%08X", pObj->object->id);
			Xt::Set(pObjBlock, XtString("objectDef"), str);

			XtId objId = Xt::CreateUniqueId(arrObjects, "__id", 32768);

			if (objId == 0) goto failed;	

			XtString strObjId = Format("%08X", objId);
			Xt::Set(pObjBlock, XtString("__id"), strObjId);
	
			// create loadobject cmd
			Xt::DomBlock* pCmdBlock = Xt::Append(arrCommonCmds, new Xt::DomBlock);

			LOGINFO(g_Log, "   added LoadObject cmd, object id = 0x%x\n", objId);

			if (pCmdBlock == NULL) goto failed;	

			Xt::Set(pCmdBlock, XtString("__type"), XtString("AED_LoadObjectCommand"));
			Xt::Set(pCmdBlock, XtString("object"), strObjId);

			XtId id = Xt::CreateUniqueId(arrCommonCmds, "__id", 32768);	

			if (id == 0) goto failed;	

			str = Format("%08X", id);
			Xt::Set(pCmdBlock, XtString("__id"), str);
		}

		// create instances and spawn instance cmds
		for (int j=0; j<pScript->instFunction.instances.count; j++)
		{
			Instance* pInstance = &pScript->instFunction.instances[j];

			if (pInstance == NULL) goto failed;	

			// create instance
			XtString strInstId;
			Xt::DomBlock* pInstBlock = Xt::Append(arrInstances, new Xt::DomBlock);

			if (pInstBlock == NULL) goto failed;	
			if (pInstance->object == NULL) goto failed;	

			// lookup object in object array
			Xt::DomBlock* pBlock = Xt::AcquireBlock(arrObjects, pInstance->object->index);
			
			if (pBlock == NULL) goto failed;

			// get id of object and store it
			Xt::DomField* pField = Xt::AcquireField(pBlock, XtString("__id"));

			if (pField == NULL) goto failed;	

			LOGINFO(g_Log, "   added instance, object id = %s\n", pField->value.Text());

			Xt::Set(pInstBlock, XtString("object"), pField->value);

			XtId id = Xt::CreateUniqueId(arrCommonCmds, "__id", 32768);	

			if (id == 0) goto failed;	

			strInstId = Format("%08X", id);
			Xt::Set(pInstBlock, XtString("__id"), strInstId);

			#ifdef CREATE_CMDS
			// create spawn instance cmd
			Xt::DomBlock* pCmdBlock = Xt::Append(arrLocalCmds, new Xt::DomBlock);
			
			if (pCmdBlock == NULL) goto failed;	

			Xt::Set(pCmdBlock, XtString("__type"), XtString("AED_SpawnInstanceCommand"));
			Xt::Set(pCmdBlock, XtString("instance"), strInstId);

			LOGINFO(g_Log, "   added spawn instance cmd, instance id = 0x%s\n", strInstId.Text());

			id = Xt::CreateUniqueId(arrLocalCmds, "__id", 32768);	

			if (id == 0) goto failed;	

			XtString str = Format("%08X", id);
			Xt::Set(pCmdBlock, XtString("__id"), str);
			#endif	// #ifdef CREATE_CMDS
		}

		// for each connector create object component, instance component, 
		// get object component cmd and get instance component cmd
		for (int j=0; j<pScript->instFunction.connectors.Size(); j++)
		{
			InternalConnector* pConn = pScript->instFunction.connectors[j];

			if (pConn == NULL) goto failed;	

			XtString strObjComId;

			// create object component
			Xt::DomBlock* pObjComBlock = Xt::Append(arrObjectCom, new Xt::DomBlock);

			if (pObjComBlock == NULL) goto failed;	

			Xt::Set(pObjComBlock, XtString("name"), pConn->component);

			if (pConn->instance == NULL) goto failed;	
			if (pConn->instance->object == NULL) goto failed;	

			// lookup object in object array
			Xt::DomBlock* pObjBlock = Xt::AcquireBlock(arrObjects, pConn->instance->object->index);

			if (pObjBlock == NULL) goto failed;	

			// get id of object and use it
			Xt::DomField* pField = Xt::AcquireField(pObjBlock, XtString("__id"));

			if (pField == NULL) goto failed;	

			Xt::Set(pObjComBlock, XtString("object"), pField->value);

			XtId id = Xt::CreateUniqueId(arrObjectCom, "__id", 32768);	
			
			if (id == 0) goto failed;	

			strObjComId = Format("%08X", id);
			Xt::Set(pObjComBlock, XtString("__id"), strObjComId);


			// create instance component
			XtString strInstComId;

			Xt::DomBlock* pInstComBlock = Xt::Append(arrInstCom, new Xt::DomBlock);

			if (pInstComBlock == NULL) goto failed;	

			Xt::Set(pInstComBlock, XtString("component"), strObjComId);

			if (pConn->instance == NULL) goto failed;

			// lookup instance in instance array
			Xt::DomBlock* pInstBlock = Xt::AcquireBlock(arrInstances, pConn->instance->index);
			
			if (pInstBlock == NULL) goto failed;	

			pField = Xt::AcquireField(pInstBlock, XtString("__id"));
			
			if (pField == NULL) goto failed;	

			Xt::Set(pInstComBlock, XtString("instance"), pField->value);

			id = Xt::CreateUniqueId(arrObjectCom, "__id", 32768);	

			if (id == 0) goto failed;	

			strInstComId = Format("%08X", id);
			Xt::Set(pInstComBlock, XtString("__id"), strInstComId);


			#ifdef CREATE_CMDS
			// create get object component cmd
			Xt::DomBlock* pObjComCmdBlock = Xt::Append(arrCommonCmds, new Xt::DomBlock);

			if (pObjComCmdBlock == NULL) goto failed;

			Xt::Set(pObjComCmdBlock, XtString("__type"), XtString("AED_GetObjectComponentCommand"));
			Xt::Set(pObjComCmdBlock, XtString("objectComponent"), strObjComId);

			id = Xt::CreateUniqueId(arrCommonCmds, "__id", 32768);	

			if (id == 0) goto failed;	

			XtString str = Format("%08X", id);
			Xt::Set(pObjComCmdBlock, XtString("__id"), str);


			// create get instance component cmd
			Xt::DomBlock* pInstComCmdBlock = Xt::Append(arrLocalCmds, new Xt::DomBlock);

			if (pInstComCmdBlock == NULL) goto failed;

			Xt::Set(pInstComCmdBlock, XtString("__type"), XtString("AED_GetInstanceComponentCommand"));
			Xt::Set(pInstComCmdBlock, XtString("instanceComponent"), strInstComId);

			id = Xt::CreateUniqueId(arrCommonCmds, "__id", 32768);	
			
			if (id == 0) goto failed;	

			str = Format("%08X", id);
			Xt::Set(pInstComCmdBlock, XtString("__id"), str);
			#endif
		}

		// add connection cmds
		for (int j=0; j<pScript->instFunction.connections.count; j++)
		{
			InternalConnection* pConn = &pScript->instFunction.connections[j];
			
			if (pConn == NULL) goto failed;

			Xt::DomBlock* pCmdBlock = Xt::Append(arrLocalCmds, new Xt::DomBlock);

			if (pCmdBlock == NULL) goto failed;

			Xt::Set(pCmdBlock, XtString("__type"), XtString("AED_ConnectInternalCommand"));

			// lookup instance component for "from" connector in array
			Xt::DomBlock* pFromBlock = Xt::AcquireBlock(arrInstCom, pConn->from->index);

			if (pFromBlock == NULL) goto failed;

			Xt::DomField* pField = Xt::AcquireField(pFromBlock, XtString("__id"));

			if (pField == NULL) goto failed;	

			Xt::Set(pCmdBlock, XtString("from"), pField->value);


			// lookup instance component for "to" connector in array
			Xt::DomBlock* pToBlock = Xt::AcquireBlock(arrInstCom, pConn->to->index);

			if (pToBlock == NULL) goto failed;

			pField = Xt::AcquireField(pToBlock, XtString("__id"));
			
			if (pField == NULL) goto failed;	

			Xt::Set(pCmdBlock, XtString("to"), pField->value);


			XtId id = Xt::CreateUniqueId(arrCommonCmds, "__id", 32768);	

			if (id == 0) goto failed;	

			XtString str = Format("%08X", id);
			Xt::Set(pCmdBlock, XtString("__id"), str);
		}

		// external refs
		for (int j=0; j<pScript->externs.count; j++)
		{
			External* pExtern = &pScript->externs[j];

			if (pExtern == NULL) goto failed;

			Xt::DomBlock* pRefBlock = Xt::Append(arrExternalRefs, new Xt::DomBlock);
			
			if (pRefBlock == NULL) goto failed;
			if (pExtern->connector == NULL) goto failed;
 
			// look up instance component
			Xt::DomBlock* pInstComBlock = Xt::AcquireBlock(arrInstCom, pExtern->connector->index);

			if (pInstComBlock == NULL) goto failed;

			Xt::DomField* pField = Xt::AcquireField(pInstComBlock, XtString("__id"));
			
			if (pField == NULL) goto failed;

			Xt::Set(pRefBlock, XtString("instanceComponent"), pField->value);
			Xt::Set(pRefBlock, XtString("refString"), pExtern->referenceString);

			XtId id = Xt::CreateUniqueId(arrExternalRefs, "__id", 32768);	

			if (id == 0) goto failed;	

			XtString str = Format("%08X", id);
			Xt::Set(pRefBlock, XtString("__id"), str);
		}
	}	

	return;

failed:
	LOGFATAL(g_Log, "BADNESS HAPPENED\nError occurred creating entity definitions. Bin file was not created.");
}

//-----------------------------------------------------------------------------
// ValidateData
// make sure the entity script data is ok.
// mainly check for null pointers.
//-----------------------------------------------------------------------------
bool ValidateData()
{
	AudioObject* pObj;
	for (int i=0; i<gRecordList.audioObjects.Size(); i++)
	{
		pObj = gRecordList.audioObjects[i];
		if (!pObj)
			LOGFATAL(g_Log, "Data validation failed on AudioObject: AudioObject is NULL.\n");

		if (!strcmp(pObj->filename.string, ""))
			LOGFATAL(g_Log, "Data validation failed on AudioObject: filename is empty.\n");

		Util::C_String strExt = Util::ExtractFileExtension(Util::C_String(pObj->filename.string));
		if (strExt.Lower() != "dat")
			LOGFATAL(g_Log, "Data validation failed on AudioObject: filename '%s' should have .dat extension.\n", pObj->filename.string ? pObj->filename.string : "NULL");
	}

	AudioEntityDef* pScript;
	for (int i=0; i<gRecordList.scripts.Size(); i++)
	{
		pScript = gRecordList.scripts[i];
		if (!pScript)
			continue;

		if (!strcmp(pScript->name, ""))
		{
			LOGFATAL(g_Log, "Data validation failed on Entity Def with id 0x%x: Entity Def has no name.\n", pScript->id);
			return false;
		}
		
		/*if (pScript->group == NULL)
		{
			LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Entity group is not defined.\n", pScript->name);
			return false;
		}*/

		// check objects
		for (int j=0; j<pScript->objCreate.objCreates.count; j++)
		{
			Object* pObj = &pScript->objCreate.objCreates[j];
			if (!pObj)
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Entity Def has null Object field.\n", pScript->name);
				return false;
			}

			if (pObj->object == NULL)
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': LoadObjects contains NULL AudioObject field.\n", pScript->name);
				return false;
			}

			if (pObj->index == INVALID_INDEX)
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Object has invalid index.\n", pScript->name);
				return false;
			}
		}

		// check instances
		for (j=0; j<pScript->instFunction.instances.count; j++)
		{
			Instance* pInstance = &pScript->instFunction.instances[j];
			if (!pInstance)
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': NULL Instance.\n", pScript->name);
				return false;
			}

			if (pInstance->index == INVALID_INDEX)
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Instance '%s' has invalid index.\n", pScript->name, pInstance->name);
				return false;
			}

			if (pInstance->object == NULL)
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Instance '%s' points to NULL Object.\n", pScript->name, pInstance->name);
				return false;
			}
		}

		// check connectors
		if (pScript->instFunction.connectors.Size() != pScript->instFunction.connectorsIn.count + pScript->instFunction.connectorsOut.count)
		{
			LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Connectors not created properly.\n", pScript->name);
			return false;
		}

		for (int j=0; j<pScript->instFunction.connectors.Size(); j++)
		{
			InternalConnector* pConn = pScript->instFunction.connectors[j];
			if (!pConn)
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Connector is NULL.\n", pScript->name);
				return false;
			}
			
			if (pConn->index == INVALID_INDEX)
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Connector has invalid index.\n", pScript->name);
				return false;
			}

			if (!strcmp(pConn->component, ""))
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Connector has component with no name specified.\n", pScript->name);
				return false;
			}

			if (pConn->instance == NULL)
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Connector points to NULL Instance.\n", pScript->name);
				return false;
			}
		}

		// check connections
		for (int j=0; j<pScript->instFunction.connections.count; j++)
		{
			InternalConnection* pConn = &pScript->instFunction.connections[j];
			if (!pConn)
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Connection is NULL.\n", pScript->name);
				return false;
			}

			if (pConn->from == NULL)
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Connection has NULL 'from' field.\n", pScript->name);
				return false;
			}

			if (pConn->to == NULL)
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Connection has NULL 'to' field.\n", pScript->name);
				return false;
			}
		}

		// check externals
		for (int j=0; j<pScript->externs.count; j++)
		{
			External* pExtern = &pScript->externs[j];

			if (!pExtern)
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Extern is NULL.\n", pScript->name);
				return false;
			}

			if (!strcmp(pExtern->referenceString, ""))
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Extern reference string is empty.\n", pScript->name);
				return false;
			}

			// check duplicate external reference string names
			for (int k=0; k<pScript->externs.count; k++)
			{
				External* pCurrExtern = &pScript->externs[k];

				if (pCurrExtern && k != j)
				{
					if (!strcmp(pExtern->referenceString.string, pCurrExtern->referenceString.string))
						LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Duplicate extern reference string '%s'.\n", pScript->name, pExtern->referenceString);
				}
			}

			if (pExtern->connector == NULL)
			{
				LOGFATAL(g_Log, "Data validation failed on Entity Def '%s': Extern points to NULL connector.\n", pScript->name);
				return false;
			}
		}
		
	}

	return true;
}
      
//-----------------------------------------------------------------------------
// ConvertDb
// convert db from EntityScripts (sound designer friendly) to 
// EntityDefs (what the game needs to consume)
//-----------------------------------------------------------------------------
bool ConvertDb(Util::C_String& strDbPath, Util::C_String& strBinName, Util::C_String& strOutputFname)
{
	// create a DomDb
	Xt::DomDb domDb;
	XtResult result;
	
	// steve said i'm better off calling XT::Open with whole database 
	// (ie complete sorcery directory with config.xtp, packs, schema etc). that way we will get stuff like 
	// exporting to the right place in the game image. BUT it means we would have to load and export the whole db
	// so better off doing it the way i am doing it for the moment.

	// check schema dir exists as OpenSchema doesnt check
	strDbPath = Util::RemoveTrailingSlash(strDbPath);
	Util::C_String strSchemaPath = strDbPath + "\\schema\\";

	{
	Sys::C_DirectoryInfo info;
	if (!Sys::C_FileSystem::Instance().DirectoryInfo(strSchemaPath, &info))
		LOGFATAL(g_Log, "schema directory '%s' does not appear to exist\n", strSchemaPath.Chars());

	if (info.NumFiles() == 0)
		LOGFATAL(g_Log, "schema directory does contain any files\n");
	}

	// this is a bit naughty to call this directly but steve is working on a way to do it officially
	if (!Xt::OpenSchema(&domDb, strSchemaPath.Chars(), &result))
	{
		LOGFATAL(g_Log, "failed to open the schema: Reason: '%s'\r\n", result.message.Text());
		return false;
	}

	LOGINFO(g_Log, "opened schema OK");

	// create a DomPack for the entities
	Xt::DomPack* domPack = 0;

	if (!domPack)
	{
		if ((domPack = NewPack(&domDb, "AudioEntities_Conv")) == 0)
		{
			LOGFATAL(g_Log, "XT: Failed to create new pack");
			return false;
		}

		LOGINFO(g_Log, "XT: Created entities pack\n");
	}

	// to get round the problem where handles dont work across packs because i am not loading the whole
	// database i need to create a DomPack for the audioobjects. this is like a dummy pack which 
	// makes the export work.
	Xt::DomPack* dummyPack = 0;

	if (!dummyPack)
	{
		if ((dummyPack = NewPack(&domDb, "dummy")) == 0)
		{
			LOGFATAL(g_Log, "XT: Failed to create new pack");
			return false;
		}

		LOGINFO(g_Log, "XT: Created dummy audioobjects pack\n");
	}
	
	WriteRecords(domDb, domPack, dummyPack);


	// TODO: export correct endian (or both)

	//XtSimpleFile f;
	XtUt::Block endianSwappedDataBlock;
	XtUt::Block nonEndianSwappedDataBlock;

	// build pack
	XtTargetProfile profile;
	//profile.endian = kXtLittleEndian;
	profile.endian = kXtBigEndian;

	if (!XtPackBuild(&profile, &domDb, domPack, &nonEndianSwappedDataBlock, &result))
	{
		LOGFATAL(g_Log, "XT: failed to build pack. reason '%s'\r\n", result.message.Text());
		return false;
	}

	// write BIN file
	XtSimpleFile f;

	//Sys::C_DirectoryInfo info;
	Util::C_String strOutPath = Util::ExtractDirectory(strOutputFname);

	Sys::C_DirectoryInfo info;
	if (!Sys::C_FileSystem::Instance().DirectoryInfo(strOutPath, &info))
	{
		if (!Sys::C_FileSystem::Instance().CreateSingleDirectory(strOutPath))
			LOGFATAL(g_Log, "Could not create temp directory '%s'\n", strOutPath.Chars());
	}

	// write XT data for debugging
	//#ifdef _DEBUG
	Util::C_String strXtd = Util::C_String(strOutPath + Util::RemoveFileExtension(Util::ExtractFilename(strOutputFname)) + ".xtd");
	if (!Xt::Export(/*&domDb, */domPack, strXtd.Chars()))
		LOGINFO(g_Log, "Failed to write debug xtd '%s'", strXtd.Chars());
	else
		LOGINFO(g_Log, "wrote xtd file '%s' ok\n", strXtd.Chars());

	//#endif

	if (f.Open(strOutputFname.Chars(), true))
	{
		//if (iPlatform == Export::E_Platform_PC)
		{
			f.Write(nonEndianSwappedDataBlock.start, (int)Capacity(&nonEndianSwappedDataBlock));
		}
		/*else
		{
			f.Write(endianSwappedData.start, (int)Capacity(&endianSwappedData));
		}*/
		f.Close();

		LOGINFO(g_Log, "wrote BIN file '%s' ok\n", strOutputFname.Chars());
	}
	else
	{
		LOGFATAL(g_Log, "failed to open '%s' for writing.\r\n", strOutputFname.Chars());
		return false;
	}

	return true;
}


//------------------------------------------------------------------------------
//	AuditionEventHandler::Log
//  print an error or fatal to MessageBox
//==============================================================================
void C_ErrorHandler::Log(Debug::LogEvent *pEvent) 
{ 
	if (pEvent == NULL)
		return;

	OutputDebugString(pEvent->msg.Chars());
	OutputDebugString("\n");

	if (pEvent->level != DebugLevel::Error && pEvent->level != DebugLevel::Fatal)
		return;

	MessageBox(NULL, pEvent->msg.Chars(), "Ouch", MB_ICONERROR);
	
	if (pEvent->level == DebugLevel::Fatal)
		exit(1);
}
 
enum eArgs
{
	eArgSorceryDbpath=0,
	eArgEntityBinFname,
	eArgAudioBinFname,
	eArgBinOutputFname
};

//-----------------------------------------------------------------------------
// SorceryDbInit
//-----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	g_pErrorHandler = new C_ErrorHandler;
	if (g_pErrorHandler)
		Debug::GetRootLogger().AddSubscriber( g_pErrorHandler );

	Util::C_String strCmdLine;
	strCmdLine = lpCmdLine;

	Ut::Vector<Util::C_String> strArgs;
	strCmdLine.Tokenise(strArgs, ' ');

	if (strArgs.Size() != 4)
	{
		MessageBox(NULL, "Not enough arguments\n\nUsage: EntityScriptConverter [sorcery db path] [entity bin filename] [audio bin filename] [entity out filename]\n\nConverts sorcery .bin file specified by [entity bin filename] from EntityScripts to Entity Definitions.", "EntityScriptConverter", MB_OK);
		return 1;
	}

	/*Audio::C_XtAudioDb* pAudioSorceryDb = new Audio::C_XtAudioDb;
	pAudioSorceryDb->AudioPack().LoadDefs(Util::C_String(strArgs[eArgAudioBinFname]));
	pAudioSorceryDb->EntitiesPack().LoadDefs(Util::C_String(strArgs[eArgEntityBinFname]));*/

	// load entities pack
	if (SorceryPackLoad(strArgs[eArgAudioBinFname], gpXtDb, gpXtPack_Audio, gXtContext_Audio) == false)
		return 1;

	// load audioobjects pack
	if (SorceryPackLoad(strArgs[eArgEntityBinFname], gpXtDb, gpXtPack_Entities, gXtContext_Entities) == false)
		return 1;

	if (ValidateData() == false)
		return 1;

	if (ConvertDb(strArgs[eArgSorceryDbpath], strArgs[eArgEntityBinFname], strArgs[eArgBinOutputFname]))
		LOGINFO(g_Log, "Converted bin file OK", "EntityScriptConverter", MB_OK);
		
	return 0;
}
