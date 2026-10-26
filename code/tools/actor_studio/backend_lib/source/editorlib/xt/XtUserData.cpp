#include "stdafx.h"
#include "sigslot/SigSlot.h"

#include <EditorLib/Xt/XtUserData.h>
#include <EditorLib/Xt/XtNullStorage.h>
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtLarvaHelper.h>
#include <EditorLib/Xt/XtObject.h>

#include <EditorLib/Core.h>
#include <Editorlib/GlobalSignals.h>
#include <EditorLib/Level/XtLevel.h>

namespace EditorLib
{
	C_XtUserData::C_XtUserData() :
		m_IsInitialized(false)
	{
		m_pStorage = new EditorLib::C_XtNullStorage();
		m_pDb = new Xt::DomDb();
	}

	C_XtUserData::~C_XtUserData()
	{
		if (m_pDb->open)
		{
			Btl::String packPath = Util::RemoveTrailingSlash((const char*)m_pDb->path) + "/packs";
			if (!Sys::C_FileSystem::Instance().FileExists(packPath.Chars()) && 
				!Sys::C_FileSystem::Instance().CreateDirectory(packPath.Chars()))
				LOGERROR(C_Core::Instance().GetLogger(), "Failed to create pack folder!");
			else if (!Flush(m_pDb, m_pStorage, 0))
				LOGERROR(C_Core::Instance().GetLogger(), "Failed to flush user database!");

			Close(m_pDb, m_pStorage, 0);
		}
		delete m_pStorage;
		delete m_pDb;
	}

	LarvaSchema* C_XtUserData::GetSchema()
	{
		return m_pDb->schema;
	}

	bool C_XtUserData::Open(const Btl::String& path)
	{
		m_IsInitialized = false;

		if (Xt::Open(m_pDb, path.Chars(), m_pStorage, 0) != Xt::kOpenSuccess)
			return false;

		Xt::DomPack* userPack = 0;

		for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(m_pDb->packs)); packIt; ++packIt)
		{
			Xt::DomPack* pPack = packIt;
			if (pPack->name == "UserData") {
				userPack = pPack;
				break;
			}
		}

		if (userPack == 0) {
			userPack = NewPack(m_pDb, "UserData");
		}

		LarvaSchema* pSchema = m_pDb->schema;

		LtStruct* pType = XtLarvaHelpers::FindLarvaType(m_pDb->schema, "ActorStudioUserData", ::EditorLib::E_XtSchemaType_Record);
		if (!pType)
			return false;

		Xt::DomRecord* userData = 0;

		for(Xt::DomPack::Iterator it(userPack); it; ++it)
		{
			userData = it;
		}

		if (userData == 0) {
			userData = Xt::NewRecord(userPack, "NewRecord", pType->name);
			SetDefaults(pSchema, pType, &userData->root);
		}

		Xt::DomArray* levelDataArray = Xt::AcquireArray(&userData->root.data, "LevelData");
		
		if (!levelDataArray)
			return false;

		m_LevelDataArray = levelDataArray;

		m_IsInitialized = true;

		return true;
	}

	Xt::DomComponent* C_XtUserData::ObtainPackUserData(U32 pack)
	{
		return ObtainRecordUserData(pack, 0);
	}

	Xt::DomComponent* C_XtUserData::ObtainRecordUserData(U32 pack, U32 record)
	{
		if (!m_IsInitialized)
			return 0;

		Xt::DomComponent* levelData = 0;

		// Try to find existing record
		for(Xt::DomArray::IndexIterator it(m_LevelDataArray); it; ++it)
		{
			Xt::DomComponent* pComponent = Xt::AsComponent(it);

			if (!pComponent)
				continue;

			Xt::DomBlock* pBlock = &pComponent->data;

			U32 packId = 0;
			U32 recordId = 0;

			Xt::DomField *pField;
			pField = Xt::AsField(Xt::At(pBlock, "PackId"));
			if (pField)
			{
				XtFieldHelper::GetScalar(packId, pField);
			}

			pField = Xt::AsField(Xt::At(pBlock, "RecordId"));
			if (pField)
			{
				XtFieldHelper::GetScalar(recordId, pField);
			}

			if (pack == packId && record == recordId)
			{
				levelData = pComponent;
				break;
			}
		}

		// Create a record if it doesn't exist
		if (levelData == 0) {
			levelData = new Xt::DomComponent();
			Xt::DomBlock* pLevelDataBlock = &levelData->data;

			if (!levelData)
				return 0;

			LarvaSchema* pSchema = m_pDb->schema;
			LtStruct* pType = XtLarvaHelpers::FindLarvaType(m_pDb->schema, "ActorStudioLevelData", ::EditorLib::E_XtSchemaType_Other);
			SetDefaults(pSchema, pType, levelData);

			Xt::DomField *pField;

			pField = Xt::AcquireField(pLevelDataBlock, "PackId");
			if (pField)
			{
				XtFieldHelper::SetScalar(pField, pack);
			}

			pField = Xt::AcquireField(pLevelDataBlock, "RecordId");
			if (pField)
			{
				XtFieldHelper::SetScalar(pField, record);
			}

			Append(m_LevelDataArray, levelData);
		}

		return levelData;
	}
}
