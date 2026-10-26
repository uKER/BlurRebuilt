#include "stdafx.h"

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorLib/Misc/AsyncHelpers.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtLarvaHelper.h>
#include <EditorLib/Xt/Export/XtBuildLevelPack.h>

// TODO : REMOVE HACK
#include <EditorLib/Network/NetworkManager.h>


#include <EditorLib/CoreConfig.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

#include "../../../../../../mercury/library/common/xtwin/include/XtWin/svnxt.h"

//#define HACKY_LEGACY_SPLINE_CONVERSION
#ifdef HACKY_LEGACY_SPLINE_CONVERSION

#include <Misc_Shared/NewCurve/Definitions.h>
#include <Misc_Shared/NewCurve/CurveHelpers.h>
#include <Misc_Shared/NewCurve/Reparameterise.h>
#include <Misc_Shared/NewCurve/CurveData.h>
#include <Misc_Shared/NewCurve/Curve.h>
#include <Misc_Shared/NewCurve/HermitePolicies.h>
#include <Misc_Shared/NewCurve/HermiteSpline.h>
#include <Misc_Shared/NewCurve/HermiteHelpers.h>

#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtFieldHelper.h>

namespace EditorLib
{
void HackyLegacySplineConversion();
}
#endif

//#define HACKY_TRIGGER_CONVERSION
#ifdef HACKY_TRIGGER_CONVERSION

#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtFieldHelper.h>

namespace EditorLib
{
void HackyTriggerConversion();
}
#endif

//#define HACKY_GROUP_CONVERSION
#ifdef HACKY_GROUP_CONVERSION

#include <Aftk/AftkTypedefs.h>
#include <Aftk/AftkEnum.h>
#include <Aftk/AftkBounds.h>
#include <Aftk/AftkMaths.h>
#include <Aftk/AftkEuler.h>

#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtFieldHelper.h>

namespace EditorLib
{
	void HackyGroupConversion();
}
#endif

namespace EditorLib
{

C_XtManager::C_XtManager()
{
	m_pDb = 0;
}

C_XtManager::~C_XtManager()
{
	Close();
	Xt::Monitor(&m_dbMonitor, 0);
}

C_Status C_XtManager::Close()
{
	m_dbPath = "";

	if (!m_pDb || !m_pDb->open)
		return E_Status_Failure;

	Xt::Free(m_pDb);
	SAFE_DELETE_PTR(m_pDb);

	return E_Status_Success;
}

C_Status C_XtManager::Open(const Btl::String& dbPath, bool ensureUnique)
{
	Btl::String formattedPath = Misc::AddTrailingSlash(dbPath, '/');
	C_Status status;
	if (m_pDb && m_pDb->open)
	{
		status = Close();
		if (!status) return status;
	}

	m_pDb = new Xt::DomDb;
	Xt::Monitor(&m_dbMonitor, m_pDb);
	m_dbMonitor.SetCallback(&StaticOnXtDomMonitorEvent, this);

	SvnXtParams svnParams(m_pDb, formattedPath.Chars());
	HWND hwnd = (HWND)C_Core::Instance().GetWindowHandle();
	svnParams.noWindow = !hwnd ? true : false;

	if (!SvnXtOpen(hwnd, &svnParams))
		return E_Status_Failure;

	m_dbPath = formattedPath;

	//Ensure unique serialisable ids across entire db.
	if (ensureUnique)
		EnsureUniqueUids(false);

	return E_Status_Success;
}

C_Status C_XtManager::Save()
{
	//Ensure uniqueness within dirty record.
	EnsureUniqueUids(true);

	//Write data.
	SvnXtParams svnParams(m_pDb, m_dbPath.Chars());
	HWND hwnd = (HWND)C_Core::Instance().GetWindowHandle();
	svnParams.noWindow = !hwnd ? true : false;
	bool result = SvnXtSave(hwnd, &svnParams);
	if (result)
		return E_Status_Success;

	return E_Status_Failure;
}

void C_XtManager::Revert()
{
	//Restore pristine state
	for(Xt::DomDb::Iterator itPack(m_pDb); itPack; ++itPack)
	{
		for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
		{
			Xt::DomRecord* pRecord = itRecord;
			if (pRecord->dirty && !RevertToPristine(pRecord))
				LOGWARN(C_Core::Instance().GetLogger(), "Unable to revert changes to %s.", pRecord->name.Text());
		}
	}
}

Xt::DomDb* C_XtManager::GetDb()
{
	return m_pDb;
}

const Xt::DomDb* C_XtManager::GetDb() const
{
	return m_pDb;
}

XtId C_XtManager::CreateRecord(XtId packId, const Btl::String& typeName)
{
	Xt::DomPack* pPack = FindPack(packId);
	if (!pPack)
		return 0;

	LtStruct* pType = XtLarvaHelpers::FindLarvaType(C_Core::Instance().GetXtManager().GetDb()->schema, typeName, E_XtSchemaType_Record);
	if (!pType)
		return 0;

	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;

	Xt::DomRecord* pNewRecord = Xt::NewRecord(pPack, "NewRecord", pType->name);
	SetDefaults(pSchema, pType, &pNewRecord->root);

	return pNewRecord->id;
}

bool C_XtManager::DeleteRecord(XtId id)
{
	if (Xt::DomRecord* record = Xt::FindRecordById(m_pDb, id))
	{
		if (Xt::DomPack* pack = Xt::PackOf(record))
		{
			return Xt::RemoveRecord(pack, record->id);
		}
	}

	return false;
}

void C_XtManager::RevertRecord(XtId id)
{
	Xt::DomRecord* pRecord = FindRecord(id);
	if (pRecord && pRecord->dirty && !RevertToPristine(pRecord))
		LOGERROR(C_Core::Instance().GetLogger(), "Unable to revert changes to %s", pRecord->name.Text());
}

Xt::DomPack* C_XtManager::FindPack(const Btl::String& name)
{
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(m_pDb->packs)); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;
		if (name == (const char *)pPack->name)
			return pPack;
	}

	return 0;
}

const Xt::DomPack* C_XtManager::FindPack(const Btl::String& name) const
{
	return (const_cast<C_XtManager*>(this))->FindPack(name);
}

Xt::DomPack* C_XtManager::FindPack(XtId id)
{
	return m_pDb->packs[id];
}

const Xt::DomPack* C_XtManager::FindPack(XtId id) const
{
	return m_pDb->packs[id];
}

Xt::DomRecord* C_XtManager::FindRecord(XtId id)
{
	return Xt::FindRecordById(m_pDb, id);
}

const Xt::DomRecord* C_XtManager::FindRecord(XtId id) const
{
	return Xt::FindRecordById(m_pDb, id);
}

Xt::DomElement* C_XtManager::FindElementByPath(
	LtStruct* pRecordType, 
	Xt::DomElement* element, 
	const char* path)
{
	return 0;
}

void C_XtManager::EnsureUniqueUids(XtId recordId)
{
	Xt::DomRecord* pRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(recordId);
	if (!pRecord) return;

	EuuParams params;
	params.ensureGloballyUnique = true;

	if (LtStruct* type = TypeOf(m_pDb->schema, pRecord))
	{
		params.missingCount		= 0;
		params.collisionCount	= 0;
		params.totalCount		= 0;

		::EnsureUniqueUids(m_pDb->schema, type, &pRecord->root, &params);

		if (params.totalCount > 0)
		{
			LOGDEBUG(C_Core::Instance().GetLogger(), "Ensure UID results for %s: Missing: %i Collisions: %i Total: %i", 
				pRecord->name.Text(), params.missingCount, params.collisionCount, params.totalCount);
		}
	}
	LOGINFO(C_Core::Instance().GetLogger(), "Finished UID enforcement for record %i %s", recordId, pRecord->name.Text());
}

void C_XtManager::EnsureUniqueUids(bool onlyIfDirty)
{
	EuuParams params;
	params.ensureGloballyUnique = true;

	U32 count = 0;

	for(Xt::DomDb::Iterator itPack(m_pDb); itPack; ++itPack)
	{
		for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
		{
			Xt::DomRecord* pRecord = itRecord;
			if (pRecord->dirty || 
				!onlyIfDirty)
			{  
				if (LtStruct* type = TypeOf(m_pDb->schema, itRecord))
				{
					params.missingCount		= 0;
					params.collisionCount	= 0;
					params.totalCount		= 0;

					::EnsureUniqueUids(m_pDb->schema, type, &itRecord->root, &params);

					if (params.totalCount > 0)
					{
						LOGDEBUG(C_Core::Instance().GetLogger(), "Ensure UID results for %s: Missing: %i Collisions: %i Total: %i", 
							pRecord->name.Text(), params.missingCount, params.collisionCount, params.totalCount);
					}

					++count;
				}
			}
		}
	}
	if (count > 0)
		LOGDEBUG(C_Core::Instance().GetLogger(), "Finished UID enforcement for %i records", count);
}

void C_XtManager::ClearPacksPendingDeployment()
{
	m_packsPendingDeployment.Clear();
}

void C_XtManager::GetPacksPendingDeployment(Btl::Vector<Xt::DomPack*>& packs)
{
	packs.Clear();
	packs.Reserve(m_packsPendingDeployment.Size());
	for (Btl::OpenHashSet<XtId>::ConstIterator it = m_packsPendingDeployment.Begin(); it != m_packsPendingDeployment.End(); ++it)
	{
		Xt::DomPack* pPack = FindPack(*it);
		if (pPack)
			packs.PushBack(pPack);
	}
}

void C_XtManager::StaticOnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent, 
	C_XtManager* pThis)
{
	// HACK : NETWORK MANAGER : Let the network manager know something has changed
	C_Core::Instance().GetNetworkManager().SetDirty();

	if (pEvent->code == Xt::kDomPackChanged)
	{
		Xt::DomPack* pPack = (Xt::DomPack*)pEvent->from;
		if (pPack)
			pThis->m_packsPendingDeployment.Insert(pPack->id);
	}
	else if (pEvent->code == Xt::kDomDbClosing)
	{
		C_Core::Instance().GetGlobalSignals().databaseEvent(*(Xt::DomDb*)pEvent->from, E_DatabaseEvent_Closing);
		C_Core::Instance().OnCloseXtDb();
	}
	else if (pEvent->code == Xt::kDomDbOpened)
	{
		#ifdef HACKY_LEGACY_SPLINE_CONVERSION
			HackyLegacySplineConversion();
		#endif
		#ifdef HACKY_TRIGGER_CONVERSION
			HackyTriggerConversion();
		#endif
		#ifdef HACKY_GROUP_CONVERSION
			HackyGroupConversion();
		#endif

		C_Core::Instance().OnOpenXtDb();
		C_Core::Instance().GetGlobalSignals().databaseEvent(*(Xt::DomDb*)pEvent->from, E_DatabaseEvent_Opened);
	}
}

#ifdef HACKY_TRIGGER_CONVERSION
void HackyTriggerConversion()
{
	LarvaSchema* pLarvaSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	Xt::DomDb* pDb = C_Core::Instance().GetXtManager().GetDb();
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(pDb->packs)); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;
		for(Xt::DomPack::Iterator it(pPack); it; ++it)
		{
			Xt::DomRecord* pRec = it;

			Xt::DomArray* pLevelActorArray = Xt::AsArray(
				FindBySemantic(
					Xt::TypeOf(pLarvaSchema, pRec), 
					&pRec->root.data, 
					kXtSemanticAttr,
					kXtSemanticParam,
					kXtSemanticLabel_ActorArray).dom);
			if (!pLevelActorArray)
				continue;

			for(Xt::DomArray::IndexIterator it(pLevelActorArray); it; ++it)
			{
				Xt::DomComponent* pComponent = Xt::AsComponent(it);
				if (!pComponent)
					continue;
	
				if (pComponent->type == "TriggerRegion")
					Xt::SetType(pComponent, "BoxTriggerRegion");

				if (pComponent->type == "PhysicsModifier")
					Xt::SetType(pComponent, "BoxPhysicsModifier");
			}
		}
	}

	Xt::Flush(pDb, 0);
}
#endif

#ifdef HACKY_LEGACY_SPLINE_CONVERSION

void HackyLegacySplineConversion()
{
	LarvaSchema* pLarvaSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	Xt::DomDb* pDb = C_Core::Instance().GetXtManager().GetDb();
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(pDb->packs)); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;
		for(Xt::DomPack::Iterator it(pPack); it; ++it)
		{
			Xt::DomRecord* pRec = it;

			Xt::DomArray* pLevelActorArray = Xt::AsArray(
				FindBySemantic(
					Xt::TypeOf(pLarvaSchema, pRec), 
					&pRec->root.data, 
					kXtSemanticAttr,
					kXtSemanticParam,
					kXtSemanticLabel_ActorArray).dom);
			if (!pLevelActorArray)
				continue;

			for(Xt::DomArray::IndexIterator it(pLevelActorArray); it; ++it)
			{
				Xt::DomComponent* pComponent = Xt::AsComponent(it);
				if (!pComponent)
					continue;

				Xt::DomBlock* pBlock = Xt::AsBlock(Xt::At(&pComponent->data, "SplineData"));
				if (!pBlock)
					continue;

				Xt::DomArray* pKnotsArray = Xt::AsArray(Xt::At(pBlock, "knots"));
				if (!pKnotsArray)
					continue;

				Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> > knots;
				knots.Clear();

				knots.Resize(Xt::Count(pKnotsArray));
				S32 i = 0;
				for(Xt::DomArray::IndexIterator it(pKnotsArray); it; ++it, ++i)
				{
					Xt::DomComponent* pXtItemComponent = Xt::AsComponent(it);
					if (!pXtItemComponent)
						continue;

					Xt::DomBlock* pXtItemBlock = &pXtItemComponent->data;
					if (!pXtItemBlock)
						continue;

					{
						Xt::DomBlock* pBlock = Xt::AsBlock(Xt::At(pXtItemBlock, "position"));
						Vector3 fieldVector(0,0,0);
						if (pBlock)	XtFieldHelper::GetVector(fieldVector, pBlock);
				
						Maths::HVec3& position = knots[it.index].value;
						position.X(fieldVector.X());
						position.Y(fieldVector.Y());
						position.Z(fieldVector.Z());
					}

					{
						Xt::DomBlock* pBlock = Xt::AsBlock(Xt::At(pXtItemBlock, "tangentIn"));
						Vector3 fieldVector(0,0,0);
						if (pBlock)	XtFieldHelper::GetVector(fieldVector, pBlock);
				
						Maths::HVec3& position = knots[it.index].tangentIn;
						position.X(fieldVector.X());
						position.Y(fieldVector.Y());
						position.Z(fieldVector.Z());
					}

					{
						Xt::DomBlock* pBlock = Xt::AsBlock(Xt::At(pXtItemBlock, "tangentOut"));
						Vector3 fieldVector(0,0,0);
						if (pBlock)	XtFieldHelper::GetVector(fieldVector, pBlock);
				
						Maths::HVec3& position = knots[it.index].tangentOut;
						position.X(fieldVector.X());
						position.Y(fieldVector.Y());
						position.Z(fieldVector.Z());
					}

					Xt::DomField* pField = Xt::AsField(Xt::At(pXtItemBlock, "tension"));
					F32 tangentScale(1.f);
					if (pField)	XtFieldHelper::GetScalar(tangentScale, pField);
					knots[it.index].tangentScale = tangentScale;
					if (pField)
						Xt::Remove(pXtItemBlock, "tension");
				}

				Curve::Helpers::ConvertLegacyHermiteTangents(knots);

				for (S32 i = 0; i < knots.Size(); i++)
				{
					Xt::DomComponent* pXtItemComponent = Xt::AcquireComponent(pKnotsArray,i);
					if (!pXtItemComponent)
						continue;

					Xt::DomBlock* pXtItemBlock = &pXtItemComponent->data;

					Xt::DomBlock* pBlock = Xt::AcquireBlock(pXtItemBlock, "position");
					assert(pBlock);
					XtFieldHelper::SetVector(pBlock, knots[i].value);

					pBlock = Xt::AcquireBlock(pXtItemBlock, "tangentIn");
					assert(pBlock);
					XtFieldHelper::SetVector(pBlock, knots[i].tangentIn);

					pBlock = Xt::AcquireBlock(pXtItemBlock, "tangentOut");
					assert(pBlock);
					XtFieldHelper::SetVector(pBlock, knots[i].tangentOut);

					Xt::DomField* pField = Xt::AcquireField(pXtItemBlock, "tangentScale");
					assert(pBlock);
					XtFieldHelper::SetScalar(pField, knots[i].tangentScale);
				}
			}
		}
	}

	Xt::Flush(pDb, 0);
}

#endif

#ifdef HACKY_GROUP_CONVERSION
void HackyGroupConversion()
{
	Btl::Vector<Xt::DomComponent*> clones;
	Btl::OpenHashMap<XtId, XtId> remapTable;
	LarvaSchema* pLarvaSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	Xt::DomDb* pDb = C_Core::Instance().GetXtManager().GetDb();
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(pDb->packs)); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;
		for(Xt::DomPack::Iterator it(pPack); it; ++it)
		{
			Xt::DomRecord* pLevelRecord = it;

			Xt::DomArray* pLevelActorArray = Xt::AsArray(
				AcquireBySemantic(
					Xt::TypeOf(pLarvaSchema, pLevelRecord), 
					&pLevelRecord->root.data, 
					kXtSemanticAttr,
					kXtSemanticParam,
					kXtSemanticLabel_ActorArray).dom);
			if (pLevelActorArray)
			{
				for(Xt::DomArray::IndexIterator it(pLevelActorArray); it; ++it)
				{
					Xt::DomComponent* pComponent = Xt::AsComponent(it);
					if (!pComponent)
						continue;

					Xt::DomBlock* pBlock = Xt::AsBlock(Xt::At(&pComponent->data, "__transform"));
					//If we've no __transform field then initialise ourselves using the
					//game's world matrix.
					if (!pBlock)
					{
						Xt::DomBlock* pMatrixBlock = Xt::AsBlock(Xt::At(&pComponent->data, "WorldMatrix"));
						Matrix44 m;
						XtFieldHelper::GetMatrix(m, pMatrixBlock);
						F32 det = GetDeterminant4x4(m);
						if (det == 0)
							UtMaths::SetIdentity(&m);

						Maths::HVec3 scale;
						Maths::HVec3 shear;
						Maths::HVec3 rotate;
						Maths::HVec3 translate;

						Maths::HMat43 temp;
						Aftk::TemplateToNightmareMatrix43(temp, m);

						//GetSRT grabs rotation values in roll, pitch, heading order. This is no good to us as legacy
						//animation curves are in XYZ order, and set matrix will be used on the nodes driven by these curves.
						//Therefore we must grab the rotation values in XYZ order from the given matrix.
						Aftk::GetSShRT(
							scale,
							shear,
							rotate,
							translate, 
							temp, 
							(Aftk::E_RotationOrder)E_RotationOrder_XYZ);

						Xt::DomBlock *pTransform = 
							Xt::AcquireBlock(&pComponent->data, "__transform");
						{
							Xt::DomBlock *pComponent = 0;
							pComponent = Xt::AcquireBlock(pTransform, "scale");
							XtFieldHelper::SetVector(pComponent, scale);

							pComponent = Xt::AcquireBlock(pTransform, "shear");
							XtFieldHelper::SetVector(pComponent, shear);

							pComponent = Xt::AcquireBlock(pTransform, "rotate");
							XtFieldHelper::SetVector(pComponent, rotate);

							pComponent = Xt::AcquireBlock(pTransform, "translate");
							XtFieldHelper::SetVector(pComponent, translate);
						}
					}
				}
			}

			Xt::DomArray *pLevelGroupArray = Xt::AsArray(Xt::At(&pLevelRecord->root.data, "__groupArray"));
			if (pLevelGroupArray)
			{
				Xt::DomArray* pLevelNodeArray = Xt::AsArray(
					AcquireBySemantic(
						Xt::TypeOf(pLarvaSchema, pLevelRecord), 
						&pLevelRecord->root.data, 
						kXtSemanticAttr,
						kXtSemanticParam,
						kXtSemanticLabel_NodeArray).dom);
				BIZ_ASSERT(pLevelNodeArray);
				for(Xt::DomArray::IndexIterator it(pLevelGroupArray); it; ++it)
				{
					Xt::DomComponent* pComponent = Xt::AsComponent(it);
					if (!pComponent)
						continue;

					if (pComponent->type != "ActorStudioGroup")
						continue;

					Xt::DomComponent* pNewComponent = Xt::AsComponent(Xt::Clone(pComponent));
					clones.PushBack(pNewComponent);
					remapTable[pComponent->id] = pNewComponent->id;

					Xt::SetType(pNewComponent, "AsSceneXtGroupNode");

					Xt::DomField* pField = Xt::AsField(Xt::At(&pNewComponent->data, "__name"));
					if (pField)
					{
						LarvaType* pNewType = FindTypeByName(pLarvaSchema, pNewComponent->type);
						BIZ_ASSERT(pNewType);
						Xt::DomField* pNewField  = Xt::AsField(AcquireBySemantic(
							AsStruct(pNewType),
							&pNewComponent->data,
							kXtSemanticAttr,
							kXtSemanticParam,
							kXtSemanticLabel_Name).dom);
						Xt::Set(pNewField, pField->value);
						Xt::Remove(&pNewComponent->data, "__name");
					}
					Xt::Append(pLevelNodeArray, pNewComponent);
				}
			}
		}
	}

	for (S32 i = 0; i < clones.Size(); ++i)
	{
		Xt::DomComponent* pComponent = clones[i];
		Xt::DomRef* pParentField = Xt::AsRef(Xt::At(&pComponent->data, "__parent"));
		if (pParentField)
		{
			Btl::OpenHashMap<XtId, XtId>::Iterator it = remapTable.Find(pParentField->value);
			if (it == remapTable.End())
				continue;

			XtId newParent = it->GetValue();
			Xt::Set(pParentField, newParent);
		}
	}

	Xt::Flush(pDb, 0, 0);
}
#endif

}
