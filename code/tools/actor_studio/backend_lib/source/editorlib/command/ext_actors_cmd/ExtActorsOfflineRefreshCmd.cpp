#include "stdafx.h"

#include <System/Xml/Xml.h>
#include <Btl/Containers/OpenhashMap.h>

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUtQuery.h>
#include <AftkUt/AftkUtNodeFlagParser.h>

#include <EditorLib/Aftk/AftkRefHelpers.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Ext_Actors_Cmd/ExtActorsRefresh.h>
#include <EditorLib/Command/Ext_Actors_Cmd/ExtActorsOfflineRefreshCmd.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneAftkNode.h>
#include <EditorLib/Dg/SceneAftkActorNode.h>
#include <EditorLib/Dg/SceneNodeIterator.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgNodeHelpers.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/XtNodeHelpers.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtFieldHelper.h>

#include <EditorLib/level/ILevel.h>
#include <EditorLib/Core.h>

namespace EditorLib
{

C_ExtActorsOfflineRefreshCmd::C_ExtActorsOfflineRefreshCmd(U32 type) : I_Command(type)
{
	m_autoflagExtActorRefs = false;
}

C_ExtActorsOfflineRefreshCmd::~C_ExtActorsOfflineRefreshCmd()
{
}

C_Status C_ExtActorsOfflineRefreshCmd::Do()
{
	enum E_ExtActorContext
	{
		E_ExtActorContext_Record,
		E_ExtActorContext_Pack,
		E_ExtActorContext_Db
	};

	//Determine operating context.
	E_ExtActorContext context;
	if (m_args.IsSet('r'))
		context = E_ExtActorContext_Record;
	else if (m_args.IsSet('p'))
		context = E_ExtActorContext_Pack;
	else if (m_args.IsSet('d'))
		context = E_ExtActorContext_Db;
	else
	{
		//Fail :(
		return E_Status_Failure;
	}

	//Autoflag?
	if (m_args.IsSet('a'))
	{
		m_autoflagExtActorRefs = true;
	}

	if (m_args.IsSet('c'))
	{
		switch(context)
		{
		case E_ExtActorContext_Record:
			{
				const ArgumentDataArray& data = *m_args.GetArgData('r');
				XtId id;
				data[0].GetValue(id);
				Xt::DomRecord* pRec = C_Core::Instance().GetXtManager().FindRecord(id);
				if (!pRec) return E_Status_Failure;
				ClearRecord(pRec);
			}
			return E_Status_Success;
		case E_ExtActorContext_Pack:
			{
				const ArgumentDataArray& data = *m_args.GetArgData('p');
				XtId id;
				data[0].GetValue(id);
				Xt::DomPack* pPack = C_Core::Instance().GetXtManager().FindPack(id);
				if (!pPack)	return E_Status_Failure;
				if (PackContainsActorLists(pPack))
					ClearPack(pPack);
			}
			return E_Status_Success;
		case E_ExtActorContext_Db:
			{
				ClearDb();
			}
			return E_Status_Success;
		default:
			return E_Status_Failure;
		}
	}

	//Perform refresh...
	C_ExtActorsDbStats dbStats;

	bool status;
	switch(context)
	{
	case E_ExtActorContext_Record:
		{
			const ArgumentDataArray& data = *m_args.GetArgData('r');
			XtId id;
			data[0].GetValue(id);
			Xt::DomRecord* pRec = C_Core::Instance().GetXtManager().FindRecord(id);
			if (!pRec) return E_Status_Failure;

			dbStats.packStatsArray.PushBack();
			dbStats.packStatsArray.Back().recordStatsArray.PushBack();
			status = DoRecord(dbStats.packStatsArray.Back().recordStatsArray.Back(), pRec);
		}
		break;
	case E_ExtActorContext_Pack:
		{
			const ArgumentDataArray& data = *m_args.GetArgData('p');
			XtId id;
			data[0].GetValue(id);
			Xt::DomPack* pPack = C_Core::Instance().GetXtManager().FindPack(id);
			if (!pPack)	return E_Status_Failure;
			if (!PackContainsActorLists(pPack))
				return E_Status_Success;

			dbStats.packStatsArray.PushBack();
			status = DoPack(dbStats.packStatsArray.Back(), pPack);
		}
		break;
	case E_ExtActorContext_Db:
		{
			status = DoDb(dbStats);
		}
		break;
	default:
		return E_Status_Failure;
	}

	//Accumulate unique ref list.
	for (S32 iPack = 0; iPack < dbStats.packStatsArray.Size(); ++iPack)
	{
		for (UniqueRefSet::Iterator it = dbStats.packStatsArray[iPack].uniqueRefFilePaths.Begin(); 
			it != dbStats.packStatsArray[iPack].uniqueRefFilePaths.End(); ++it)
		dbStats.uniqueRefFilePaths.Insert(*it);
	}

	//Ensure Uids across dirty records.
	C_Core::Instance().GetXtManager().EnsureUniqueUids(true);

	DisplayRefreshSummary(dbStats);
	return status ? E_Status_Success : E_Status_Failure;
}

void C_ExtActorsOfflineRefreshCmd::BuildRecordInfo(C_LevelInfo& levelInfo, Xt::DomRecord* pRec)
{
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;

	levelInfo.pRecord = pRec;

	//Acquire actor array.
	Xt::FieldDom fdArray = AcquireBySemantic(
		Xt::TypeOf(pSchema, pRec), 
		&pRec->root.data,
		kXtSemanticAttr,
		kXtSemanticParam,
		kXtSemanticLabel_ActorArray);
	BIZ_ASSERT(fdArray.field->kind == kLfArray);
	LfArray* fieldArr = (LfArray*)fdArray.field;
	Xt::DomArray* pLevelActorArray = Xt::AsArray(fdArray.dom);

	//Process references.
	{
		Xt::DomBlock* pAftkRefs = Xt::AsBlock(Xt::At(&pRec->root.data, "__aftkRefs"));
		if (!pAftkRefs)	return;

		Xt::DomArray* pXtAftkRefArray = Xt::AsArray(Xt::At(pAftkRefs, "__aftkRefArray"));
		if (!pXtAftkRefArray) return;

		for(Xt::DomArray::IndexIterator it(pXtAftkRefArray); it; ++it)
		{
			Xt::DomBlock* pXtItemBlock = Xt::AsBlock(it);
			if (!pXtItemBlock) return;

			Xt::DomBlock* pXtAftkRef = Xt::AsBlock(Xt::At(pXtItemBlock, "__aftkRef"));
			if (!pXtAftkRef) return;

			//Skip scenes which don't contain ext actor ref data. Unless we're autoflagging!
			if (!m_autoflagExtActorRefs)
			{
				XtString isExtActorRef = Xt::AsText(Xt::At(pXtAftkRef, "__isExtActorRef"));
				if (isExtActorRef != "1")
					continue;
			}

			XtString filePath = Xt::AsText(Xt::At(pXtAftkRef, "__filePath"));
			if (filePath.Length() == 0) return;

			levelInfo.refInfo.PushBack();
			levelInfo.refInfo.Back().pRefBlock = pXtAftkRef;
			levelInfo.refInfo.Back().refFilePath =
				Misc::FormatWindowsFileString(filePath.Text());
		}
	}
}

void C_ExtActorsOfflineRefreshCmd::BuildPackInfo(
	C_ExtActorsPackStats& stats, Btl::Vector<C_LevelInfo>& levelInfoArray, Xt::DomPack* pPack)
{
	stats.packName = pPack->name.Text();

	for(Xt::DomPack::Iterator it(pPack); it; ++it)
	{
		Xt::DomRecord* pRec = it;
		if (!pRec || pRec->root.type != kXtLevelRecordType)
			continue;

		levelInfoArray.PushBack();

		BuildRecordInfo(levelInfoArray.Back(), pRec);
	}
}

bool C_ExtActorsOfflineRefreshCmd::ProcessOffline(
	C_ExtActorsPackStats& stats, const Btl::Vector<C_LevelInfo>& levelInfoArray)
{
	//Build unique ref set.
	{
		for (S32 i = 0; i < levelInfoArray.Size(); ++i)
			for (S32 j = 0; j < levelInfoArray[i].refInfo.Size(); ++j)
			{
				//Already in correct path format.
				stats.uniqueRefFilePaths.Insert(levelInfoArray[i].refInfo[j].refFilePath);
			}
	}

	//Grab alienbrain content
	{
		Btl::Vector<Btl::String> filePathsToUpdate;
		for (UniqueRefSet::Iterator it = stats.uniqueRefFilePaths.Begin(); it != stats.uniqueRefFilePaths.End(); ++it)
		{
			filePathsToUpdate.PushBack(it->GetKey());
		}
		if (!AftkRefHelpers::GetLatestReferences(filePathsToUpdate, 0))
		{
			return false;
		}
	}

	//Convert Maya assets
	typedef Btl::OpenHashMap<Btl::String, C_RefInfo, Btl::HashKeyOpBtlString> AftkPlacementDataMap;		
	AftkPlacementDataMap aftkPlacementData;
	U32 sceneCount = 1;
	for (UniqueRefSet::Iterator it = stats.uniqueRefFilePaths.Begin(); it != stats.uniqueRefFilePaths.End(); ++it, ++sceneCount)
	{
		AftkPlacementDataMap::InsertionReturnType r = 
			aftkPlacementData.Insert(it->GetKey());
		BIZ_ASSERT(r.inserted);
		C_RefInfo& refInfo = r.iterator->GetValue();
		refInfo.originalFilePath = it->GetKey();

		bool converted;
		if (!AftkRefHelpers::ConvertScene(
			refInfo.originalFilePath, 
			refInfo.cacheFilePath,
			&converted))
		{
			return false;
		}
		if (converted)
			LOGINFO(C_Core::Instance().GetLogger(), "Finished converting scene %i/%i %s.", 
				sceneCount, stats.uniqueRefFilePaths.Size(), refInfo.originalFilePath.Chars());
	}

	//Store actor placement for all refs.
	for (AftkPlacementDataMap::Iterator it = aftkPlacementData.Begin(); it != aftkPlacementData.End(); ++it)
	{
		C_RefInfo& refInfo = it->GetValue();

		Aftk::C_Scene scene;
		if (!scene.Read(refInfo.cacheFilePath.Chars()))
		{
			return false;
		}

		for (Aftk::C_SceneNodeIterator aftkIt(scene.GetRootSceneNodePtr()); aftkIt; ++aftkIt)
		{
			const Aftk::C_SceneNode* pAftkSceneNode = aftkIt;
			//Skip base instances.
			if (AftkUt::Query::IsBaseInstance(pAftkSceneNode))
				continue;

			C_AftkActorPlaceholder placeholder;
			AftkRefHelpers::GetActorStudioAttrs(
				placeholder.attrs,
				pAftkSceneNode);
			if (placeholder.attrs.Size() > 0)
			{		
				//Store placement info.
				placeholder.name = pAftkSceneNode->GetName().c_str();
				placeholder.path = pAftkSceneNode->GetPath().c_str();
				placeholder.refFilePath = refInfo.originalFilePath;
				placeholder.worldMatrix = pAftkSceneNode->GetWorldMatrix();
				refInfo.actorPlaceholders.PushBack(placeholder);
			}
		}
	}

	//Create actors
	stats.recordStatsArray.Resize(levelInfoArray.Size());
	for (S32 iLevel = 0; iLevel < levelInfoArray.Size(); ++iLevel)
	{
		const C_LevelInfo& levelInfo = levelInfoArray[iLevel];
		if (!levelInfo.pRecord) continue;

		//Only interested in levels.
		BIZ_ASSERT(levelInfo.pRecord->root.type == kXtLevelRecordType);

		C_ExtActorsRecordStats& recordStats = stats.recordStatsArray[iLevel];

		//For each ref, find our pre-created actor placeholder for it.
		S32 nRecordActorsCreated = 0;
		for (S32 jRef = 0; jRef < levelInfo.refInfo.Size(); ++jRef)
		{
			const Btl::String& refFilePath = levelInfo.refInfo[jRef].refFilePath;

			AftkPlacementDataMap::Iterator itRef = aftkPlacementData.Find(refFilePath);
			if (itRef == aftkPlacementData.End())
			{
				BIZ_ASSERT_FAIL("Failed to find ref filepath");
				return false;
			}

			const C_RefInfo& refInfo = itRef->GetValue();

			//Iterate actor placeholders and using stored attrs attempt to create actor.
			S32 nActorsCreated = CreateActors(levelInfo.pRecord, refFilePath, refInfo.actorPlaceholders);
			if (nActorsCreated == -1)
			{
				return false;
			}
			if (nActorsCreated > 0)
			{
				nRecordActorsCreated += nActorsCreated;
				recordStats.refArray.PushBack();
				recordStats.refArray.Back().originalFilePath = refFilePath;
				recordStats.refArray.Back().nActors = nActorsCreated;
			}

			if (m_autoflagExtActorRefs)
			{
				Xt::DomBlock* pXtAftkRef = levelInfo.refInfo[jRef].pRefBlock;
				BIZ_ASSERT(pXtAftkRef);

				Xt::DomField* pField = Xt::AcquireField(pXtAftkRef, "__isExtActorRef");
				Xt::Set(pField, nActorsCreated > 0 ? "1" : "0");
			}
		}

		//Store stat info.
		recordStats.recordName = levelInfo.pRecord->name.Text();
		recordStats.nActorsCreated = nRecordActorsCreated;
	}

	return true;
}

bool C_ExtActorsOfflineRefreshCmd::DoDb(C_ExtActorsDbStats& dbStats)
{
	bool status = true;
	
	Xt::DomDb* pDb = C_Core::Instance().GetXtManager().GetDb();

	Btl::Vector<Xt::DomPack*> packsToProcess;
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(pDb->packs)); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;
		if (!pPack) continue;

		bool containsActorLists = false;
		for(Xt::DomPack::Iterator it(pPack); it; ++it)
		{
			Xt::DomRecord* pRec = it;
			if (!pRec) continue;
			//Only interested in levels.
			if (pRec->root.type != kXtLevelRecordType) continue;

			containsActorLists = true;
			break;
		}
		if (containsActorLists)
			packsToProcess.PushBack(pPack);
	}

	bool packStatus = true;
	dbStats.packStatsArray.Resize(packsToProcess.Size());
	for (S32 iPack = 0; iPack < packsToProcess.Size(); ++iPack)
	{
		Btl::Vector<C_LevelInfo> levelInfoArray;
		Xt::DomPack* pPack = packsToProcess[iPack];
		BIZ_ASSERT(pPack);

		LOGINFO(C_Core::Instance().GetLogger(), "Performing ExtActor refresh for pack %i/%i %s.", 
			iPack+1, packsToProcess.Size(), pPack->name.Text());
	
		BuildPackInfo(dbStats.packStatsArray[iPack], levelInfoArray, pPack);

		packStatus &= DoPack(dbStats.packStatsArray[iPack], pPack);
	}

	return status ? E_Status_Success : E_Status_Failure;
}

bool C_ExtActorsOfflineRefreshCmd::DoPack(C_ExtActorsPackStats& stats, Xt::DomPack* pPack)
{
	Btl::Vector<C_LevelInfo> levelInfoArray;
	BuildPackInfo(stats, levelInfoArray, pPack);
	
	return ProcessOffline(stats, levelInfoArray);
}

bool C_ExtActorsOfflineRefreshCmd::DoRecord(C_ExtActorsRecordStats& stats, Xt::DomRecord* pRec)
{
	Btl::Vector<C_LevelInfo> levelInfoArray;
	levelInfoArray.PushBack();
	BuildRecordInfo(levelInfoArray.Back(), pRec);
	
	C_ExtActorsPackStats packStats;
	bool status = ProcessOffline(packStats, levelInfoArray);
	if (status && packStats.recordStatsArray.Size() > 0)
	{
		stats = packStats.recordStatsArray.Back();
	}

	return status;
}

bool C_ExtActorsOfflineRefreshCmd::IsUndoable() const
{
	return false;
}

I_Command* C_ExtActorsOfflineRefreshCmd::Create(U32 type)
{
	return new C_ExtActorsOfflineRefreshCmd(type);
}

C_Syntax* C_ExtActorsOfflineRefreshCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();
	{
		C_Argument arg(
			'c', 
			"clear",
			"clear external actors",
			C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}
	{
		C_Argument arg(
			'r', 
			"record",
			"operate on record",
			C_Argument::E_Required_False);
		VarTypeArray& dataTypes = arg.GetDataTypes();
		dataTypes.PushBack(Util::E_VarType_U32);
		pSyntax->AddArg(arg);
	}
	{
		C_Argument arg(
			'p', 
			"pack",
			"operate on pack",
			C_Argument::E_Required_False);
		VarTypeArray& dataTypes = arg.GetDataTypes();
		dataTypes.PushBack(Util::E_VarType_U32);
		pSyntax->AddArg(arg);
	}
	{
		C_Argument arg(
			'd', 
			"db",
			"operate on database",
			C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}
	{
		C_Argument arg(
			'a', 
			"autoflag",
			"autoflag as ExtActorRef and process all refs.",
			C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}
	return pSyntax;
}

const char* C_ExtActorsOfflineRefreshCmd::GetName()
{
	static char* p = "extActorsOfflineRefresh";
	return p;
}

}
