#include "stdafx.h"
#include <System/Shell/ShellExec.h>

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUtQuery.h>
#include <AftkUt/AftkUtNodeFlagParser.h>

#include <EditorLib/Aftk/AftkRef.h>
#include <EditorLib/Aftk/AftkRefManager.h>
#include <EditorLib/Aftk/AftkRefHelpers.h>
#include <EditorLib/Misc/UndoManager.h>
#include <EditorLib/Xt/XtVisualisation.h>
#include <EditorLib/CoreConfig.h>
#include <EditorLib/Core.h>
#include <AftkConv/Version.h>

#include "System/Xml/Xml.h"
#include "System/Serialise/TextSerialise.h"
#include "Tools_Shared/Xml/XmlDom.h"
#include <Ab_Wrapper.h>

#include "EditorLib/Misc/AsyncHelpers.h"

#include <file_tag_db/FileTagDb.h>
#include <tools_shared/utility/AssetMeta.h>

namespace EditorLib
{
namespace AftkRefHelpers
{

enum E_LoadRefTasks
{
	E_LoadRefTasks_LoadAll,
	E_LoadRefTasks_Reload
};

struct LoadReferencesParams
{
	bool convert;
	bool loadVisualisations;
	U32 convertedScenes;
	E_LoadRefTasks task;
	void(*pYieldCallback)();
};

bool LoadReferencesTask(AsyncOp* op, LoadReferencesParams* params)
{	
	LOGINFO(C_Core::Instance().GetLogger(), "Loading references, this may take a while. Please wait...");
	if (params->pYieldCallback) params->pYieldCallback();

	if (op)
		op->SetTitle("Loading References");
	
	C_CheckpointDisabler cpDisable;

	const C_AftkRefManager& refManager = C_Core::Instance().GetAftkRefManager();
	ConstAftkRefList unfilteredRefList;
	refManager.GetRefs(unfilteredRefList);

	params->convertedScenes = 0;
	int count = 1;
	bool result = true;

	ConstAftkRefList refList;
	for (ConstAftkRefList::ConstIterator it = unfilteredRefList.Begin(); it != unfilteredRefList.End(); ++it)
	{
		if (params->loadVisualisations || !(*it)->IsVisualisation())
		{
			const EditorLib::C_AftkRef* ref = *it;
			refList.PushBack(ref);
		}
	}
	
	int size = refList.Size();
	for (ConstAftkRefList::ConstIterator it = refList.Begin(); it != refList.End(); ++it)
	{
		if (op && op->IsCancelled())
			return false;

		const C_AftkRef* pRef = *it;
		if (params->task == E_LoadRefTasks_Reload)
		{
			//Unload outdated files and vis files.
			if (pRef->IsLoaded() && (pRef->IsCacheOutOfDate() || pRef->IsVisualisation()))
				pRef->Unload();
		}

		Btl::String loading;
		loading.Printf("Loading ref %i/%i %s", count, size, pRef->GetOriginalFilePath().Chars());
		if (op)
		{
			op->AppendMessage(loading.Chars());
			op->SetProgress((float)count/(float)refList.Size());
		}
		else if (C_Core::Instance().GetConfig().appType == E_AppType_Interactive)
		{
			if (params->pYieldCallback) params->pYieldCallback();
			LOGINFO(C_Core::Instance().GetLogger(), loading.Chars());
		}

		if (pRef->IsCacheOutOfDate())
		{
			params->convertedScenes++;
			if (params->convert)
				if (!pRef->Load()) 
				{
					result = false;
					//Quick bail.
					if (C_Core::Instance().GetConfig().appType != E_AppType_Interactive)
						break;
				}
		}
		else
		{
			if (!pRef->Load())
			{
				result = false;
				//Quick bail.
				if (C_Core::Instance().GetConfig().appType != E_AppType_Interactive)
					break;
			}
		}
			
		count++;
	}

	LOGINFO(C_Core::Instance().GetLogger(), "Finished loading references.");
	if (params->pYieldCallback) params->pYieldCallback();
	return result;
}

bool RefConversionRequired()
{	
	const C_AftkRefManager& refManager = C_Core::Instance().GetAftkRefManager();
	ConstAftkRefList refList;
	refManager.GetRefs(refList);

	for (ConstAftkRefList::ConstIterator it = refList.Begin(); it != refList.End(); ++it)
	{
		const C_AftkRef* pRef = *it;
		if (pRef->IsCacheOutOfDate())
			return true;
	}

	return false;
}

bool LoadAllReferences(bool convert, bool loadVisualisations, void(*pYieldCallback)())
{
	LoadReferencesParams params;
	params.convert = convert;
	params.loadVisualisations = loadVisualisations;
	params.task = E_LoadRefTasks_LoadAll;
	params.pYieldCallback = pYieldCallback;
//	AsyncTask((HWND)C_Core::Instance().GetWindowHandle(), LoadReferencesTask, &params);
	return LoadReferencesTask(0, &params);
}

void UnloadAllReferences(bool unloadVisualisations, void(*pYieldCallback)())
{	
	LOGINFO(C_Core::Instance().GetLogger(), "Unloading references, this may take a while. Please wait...");
	if (pYieldCallback) pYieldCallback();
	C_CheckpointDisabler cpDisable;
	const C_AftkRefManager& refManager = C_Core::Instance().GetAftkRefManager();
	ConstAftkRefList refList;
	refManager.GetRefs(refList);

	for (ConstAftkRefList::ConstIterator it = refList.Begin(); it != refList.End(); ++it)
	{
		if ((*it)->IsLoaded())
			(*it)->Unload();
	}
	LOGINFO(C_Core::Instance().GetLogger(), "Finished unloading references.");
	if (pYieldCallback) pYieldCallback();
}

bool ReloadReferences(bool convert, bool reloadVisualisations, void(*pYieldCallback)())
{
	C_CheckpointDisabler cpDisable;

	C_Core::Instance().GetXtVisualisationData().RescanFolders();

	LoadReferencesParams params;
	params.convert = convert;
	params.loadVisualisations = reloadVisualisations;
	params.task = E_LoadRefTasks_Reload;
	params.pYieldCallback = pYieldCallback;
//	AsyncTask((HWND)C_Core::Instance().GetWindowHandle(), LoadReferencesTask, &params);
	return LoadReferencesTask(0, &params);
}

bool AbScriptExecCallback(const char* cmdline, const char* cmdParams, const char* workingPath)
{
	C_AsyncShellExecuteParams params;
	params.command = cmdline;
	params.params = cmdParams;
	params.workingDir = workingPath;
	params.categoriseOutput = true;

	AsyncShellExecute((HWND)C_Core::Instance().GetWindowHandle(), &params);

	if (params.returnCode != 0)
	{
		LOGERROR(C_Core::Instance().GetLogger(), "Failed to get latest references");
		return false;
	}

	return true;
}

bool GetLatestReferences(void(*pYieldCallback)())
{
	const C_CoreConfig& coreConfig = C_Core::Instance().GetConfig();
	Vcs::C_Ab ab(C_Core::Instance().GetLogger(), &AbScriptExecCallback);
	if (!ab.Initialise(coreConfig.alienbrainConfigPath.Chars()))
		return false;

	bool status = true;
	ConstAftkRefList refList;
	C_Core::Instance().GetAftkRefManager().GetRefs(refList);

	Ut::UtVector< Ut::DynamicVector<const C_AftkRef*> > references;

	for (ConstAftkRefList::Iterator it=refList.Begin(); it!=refList.End(); ++it)
	{
		const C_AftkRef *pRef = *it;
		if (!pRef->IsVisualisation())
			references.PushBack(pRef);
	}

	Btl::Vector<C_XtVisualisationRootFolder> rootVisFolders;
	{
		C_XtVisualisationData& visData = C_Core::Instance().GetXtVisualisationData();
		visData.BuildRootFolderList(rootVisFolders);
	}

	//Handle vis files.
	{
		Btl::Vector<Btl::String> visFilePathsToSync;
		Btl::Vector<Btl::String> visFilePathsToUpdate;
		for (S32 i=0; i < rootVisFolders.Size(); ++i)
		{
			//Ignore non sync'd folders.
			if (rootVisFolders[i].abSync)
				visFilePathsToSync.PushBack(rootVisFolders[i].path);
			else
				visFilePathsToUpdate.PushBack(rootVisFolders[i].path);
		}
		//Sync and update as required.
		status &= ab.Sync(visFilePathsToSync);
		status &= ab.Update(visFilePathsToUpdate);
	}

	//Get latest on ref files.
	{
		Btl::Vector<Btl::String> refFilePathsToUpdate;
		for (S32 i=0; i < references.Size(); ++i)
			refFilePathsToUpdate.PushBack(references[i]->GetOriginalFilePath());
		status &= ab.Update(refFilePathsToUpdate);
	}

	return status;
}

bool GetLatestReferences(Btl::Vector<Btl::String>& filePaths, void(*pYieldCallback)())
{
	const C_CoreConfig& coreConfig = C_Core::Instance().GetConfig();
	Vcs::C_Ab ab(C_Core::Instance().GetLogger(), &AbScriptExecCallback);
	if (!ab.Initialise(coreConfig.alienbrainConfigPath.Chars()))
		return false;

	return ab.Update(filePaths);
}

bool IsToBeHiddenPermanently(const Aftk::C_SceneNode* pSceneNode)
{
	AftkUt::Query::C_Flags flags = AftkUt::Query::GetConventionFlags(pSceneNode);
	if (flags.baseInstance || flags.doNotExport || flags.asDynamic || flags.collisionPrimitive)
		return true;

	return false;
}

bool IsActorStudioProperty(const Aftk::C_DynamicPlug* pPlug)
{
	//Find As_ prefix.
	const char* namingConvention = "As_";
	size_t ncLength = strlen(namingConvention);
	if (pPlug->GetName().length() < ncLength)
		return false;

	if (stricmp(pPlug->GetName().substr(0, ncLength).c_str(), namingConvention))
		return false;

	return true;
}

void CopyAftkAttribute(Util::C_KeyValue& editorAttr, const Aftk::C_GenericAttribute& aftkAttr)
{
	editorAttr.SetName(aftkAttr.GetName().c_str());
	switch (aftkAttr.GetDataType())
	{
		case Aftk::E_GenericAttributeType_String:
		{
			std::string v;
			aftkAttr.GetValue(v);
			Btl::String cstrv(v.c_str());
			editorAttr.SetValue(cstrv);
			break;
		}
		case Aftk::E_GenericAttributeType_U32:
		{
			U32 v;
			aftkAttr.GetValue(v);
			editorAttr.SetValue(v);
			break;
		}
		case Aftk::E_GenericAttributeType_U64:
		{
			U64 v;
			aftkAttr.GetValue(v);
			editorAttr.SetValue(v);
			break;
		}
		case Aftk::E_GenericAttributeType_S32:
		{
			S32 v;
			aftkAttr.GetValue(v);
			editorAttr.SetValue(v);
			break;
		}
		case Aftk::E_GenericAttributeType_S64:
		{
			S64 v;
			aftkAttr.GetValue(v);
			editorAttr.SetValue(v);
			break;
		}
		case Aftk::E_GenericAttributeType_F32:
		{
			F32 v;
			aftkAttr.GetValue(v);
			editorAttr.SetValue(v);
			break;
		}
		case Aftk::E_GenericAttributeType_F64:
		{
			F64 v;
			aftkAttr.GetValue(v);
			editorAttr.SetValue(v);
			break;
		}
		case Aftk::E_GenericAttributeType_Boolean:
		{
			bool v;
			aftkAttr.GetValue(v);
			editorAttr.SetValue(v);
			break;
		}
		default:
			break;
	}
}

void GetActorStudioAttrs(
	Btl::Vector<Util::C_KeyValue>& attrs,
	const Aftk::C_SceneNode* pAftkSceneNode) 
{
	//Look for aftk nodes that have ActorStudio properties. ActorStudio properties are custom attributes
	//which have a AS_ prefix in their name.

	//First pass to find number of actor attributes.
	U32 nAttrs(0);
	{
		for (U32 i = 0; i < pAftkSceneNode->GetNumDynamicPlugs(); ++i)
		{
			if (!AftkRefHelpers::IsActorStudioProperty(pAftkSceneNode->GetDynamicPlugPtr(i)))
				continue;
			nAttrs++;
		}

		if (pAftkSceneNode->HasType(Aftk::E_ObjType_SceneInstance))
		{
			const Aftk::C_SceneNode* pBaseInstance = 
				((Aftk::C_SceneInstanceNode*)pAftkSceneNode)->GetTopLevelSourceNodePtr();
			for (U32 i = 0; i < pBaseInstance->GetNumDynamicPlugs(); ++i)
			{
				const Aftk::I_Plug* pPlug = pBaseInstance->GetDynamicPlugPtr(i);
				if (!AftkRefHelpers::IsActorStudioProperty((Aftk::C_DynamicPlug*)pPlug))
					continue;

				//Allow instance plugs to override base.
				if (pAftkSceneNode->GetPlugPtr(pPlug->GetName()))
					continue;
				nAttrs++;
			}
		}
	}

	if (nAttrs == 0)
		return;
	//Examine filters
	AftkUt::C_NodeFlagParser parser(*pAftkSceneNode);
	if (parser.GetFlagGroupName() == "ActorListFilter")
	{
		nAttrs++;
	}

	{
		//Store actor data from this plug.
		attrs.Reserve(nAttrs);

		//Store filter information.
		if (parser.GetFlagGroupName() == "ActorListFilter")
		{
			attrs.PushBack();
			attrs.Back().SetName("ActorString3");
			std::string flagStr;
			parser.GetFlagString(flagStr);
			attrs.Back().SetValue(Btl::String(flagStr.c_str()));
		}

		for (U32 i = 0; i < pAftkSceneNode->GetNumDynamicPlugs(); ++i)
		{
			const Aftk::C_DynamicPlug* pPlug = pAftkSceneNode->GetDynamicPlugPtr(i);
			if (!AftkRefHelpers::IsActorStudioProperty(pPlug))
				continue;

			attrs.PushBack();
			CopyAftkAttribute(attrs.Back(), pPlug->GetAttribute());
		}

		if (pAftkSceneNode->HasType(Aftk::E_ObjType_SceneInstance))
		{
			const Aftk::C_SceneNode* pBaseInstance = 
				((Aftk::C_SceneInstanceNode*)pAftkSceneNode)->GetTopLevelSourceNodePtr();
			for (U32 i = 0; i < pBaseInstance->GetNumDynamicPlugs(); ++i)
			{
				const Aftk::C_DynamicPlug* pPlug = pBaseInstance->GetDynamicPlugPtr(i);
				if (!AftkRefHelpers::IsActorStudioProperty(pPlug))
					continue;
				//Allow instance plugs to override base
				if (pAftkSceneNode->GetPlugPtr(pPlug->GetName()))
					continue;

				attrs.PushBack();
				CopyAftkAttribute(attrs.Back(), pPlug->GetAttribute());
			}
		}
	}
}

bool CheckActorListFilter(const Btl::String& levelName, const Btl::String& nodeFlagStr)
{
	AftkUt::C_NodeFlagParser parser(nodeFlagStr.Chars());
	if (parser.GetFlagGroupName() != "ActorListFilter")
		return true;

	std::string listName;
	if (!parser.GetParameterValue("ListName", listName))
		return true;

	S32 searchResult = levelName.ISearch(listName.c_str());
	if (searchResult > 0)
	{
		if (levelName[searchResult-1] != '.')
			return true;
	}
	else if (searchResult < 0)
		return true;

	Btl::String levelFilterName = levelName(searchResult, levelName.Length());
	for (S32 i = 0; true ; ++i)
	{
		Btl::String numberStr;
		numberStr.Printf("%i", i);
		bool enabled = false;
		//Bail when we run out of filter numbers. Account for artists wanting to index from 1 not 0...
		if (!parser.GetParameterValue(numberStr.Chars(), enabled) && i != 0)
			return false;

		Btl::String currentFilter;
		currentFilter.Printf("%s_%i", listName.c_str(), i);
		if (!stricmp(currentFilter.Chars(), levelFilterName.Chars()))
			return enabled;
	}

	return false;
}

bool GetAftkActorPlaceholderProperties(
	const Util::C_KeyValue** ppAsDynamic,
	const Util::C_KeyValue** ppAsActor,
	const Btl::Vector<Util::C_KeyValue>& attrs,
	const Btl::String& levelName)
{
	*ppAsDynamic = 0;
	*ppAsActor = 0;
	for (S32 i = 0; i < attrs.Size(); ++i)
	{
		const Util::C_KeyValue& attr = attrs[i];
		if (attrs[i].GetName().ISearch("as_actor") != -1)
			*ppAsActor = &attrs[i];
		else if (attrs[i].GetName().ISearch("as_dynamic") != -1)
			*ppAsDynamic = &attrs[i];
		else if (attrs[i].GetName() == "ActorString3")
		{
			Btl::String nodeFlagStr;
			attrs[i].GetValue(nodeFlagStr);
			if (!CheckActorListFilter(levelName, nodeFlagStr))
			{
				return false;
			}
		}
	}

	return true;
}


bool IsSceneOutOfDate(const Btl::String& originalFilePath, Btl::String& cacheFilePath)
{
	Btl::String formattedOriginalPath = Misc::FormatFileString(originalFilePath);
	Btl::String conversionToolPath, conversionToolParams, conversionToolMetaQueryParams;
	C_Core::Instance().GetAftkRefManager().GetConverstionTool(conversionToolPath, conversionToolParams, conversionToolMetaQueryParams);
	Btl::String dir, file, ext;
	Misc::DeconstructFileString(originalFilePath, dir, file, ext);
	ext.ConvertToLower();

	if (ext != "a3d")
	{
		if (!Sys::C_FileSystem::Instance().FileExists(originalFilePath.Chars())) {
			LOGERROR(C_Core::Instance().GetLogger(), "Reference input file is missing: %s", originalFilePath.Chars());
			return true;
		}

		U32 hash = Btl::HashKeyOpBtlString::GetHash(originalFilePath);
		cacheFilePath.Printf("%s%s_%u.a3d", 
			C_Core::Instance().GetAftkRefManager().GetCacheDirectory().Chars(), file.Chars(), hash);

		if (!Sys::C_FileSystem::Instance().FileExists(cacheFilePath.Chars()))
			return true;

		if (!Tools::MetaMismatch(
			cacheFilePath.Chars(), 
			formattedOriginalPath.Chars(),
			AFTK_CONV_CURRENT_VERSION))
		{
			return false;
		}
		
		return true;
	}


	return false;
}

bool ConvertScene(const Btl::String& originalFilePath, Btl::String& cacheFilePath, bool* pConverted)
{
	Btl::String formattedOriginalPath = Misc::FormatFileString(originalFilePath);
	formattedOriginalPath = formattedOriginalPath.Lower();

	Btl::String conversionToolPath, conversionToolParams, conversionToolMetaQueryParams;
	C_Core::Instance().GetAftkRefManager().GetConverstionTool(conversionToolPath, conversionToolParams, conversionToolMetaQueryParams);
	Btl::String dir, file, ext;
	Misc::DeconstructFileString(formattedOriginalPath, dir, file, ext);

	if (ext != "a3d")
	{
		if (!Sys::C_FileSystem::Instance().FileExists(formattedOriginalPath.Chars())) {
			LOGERROR(C_Core::Instance().GetLogger(), "Failed to convert scene, input file is missing: %s", formattedOriginalPath.Chars());
			return false;
		}

		U32 hash = Btl::HashKeyOpBtlString::GetHash(formattedOriginalPath);
		cacheFilePath.Printf("%s%s_%u.a3d", 
			C_Core::Instance().GetAftkRefManager().GetCacheDirectory().Chars(), file.Chars(), hash);
		Misc::FormatWindowsFileString(cacheFilePath);
		cacheFilePath = cacheFilePath.Lower();
		
		if (IsSceneOutOfDate(formattedOriginalPath, cacheFilePath))
		{
			Btl::String expandedParams(conversionToolParams);
			expandedParams = expandedParams.Replace("%scene%", Misc::FormatUnixFileString(formattedOriginalPath));
			expandedParams = expandedParams.Replace("%cache%", Misc::FormatUnixFileString(cacheFilePath));


			C_AsyncShellExecuteParams params;
			params.command = conversionToolPath;
			params.params = expandedParams;
			params.workingDir = Misc::ExtractDirectory(conversionToolPath);
			params.categoriseOutput = false;

			AsyncShellExecute((HWND)C_Core::Instance().GetWindowHandle(), &params);

			FileTag::C_MetaManager::Instance().Clear();

			if (params.returnCode != 0)
			{
				LOGERROR(C_Core::Instance().GetLogger(), "Failed to convert scene, aftk conversion failed: %s", formattedOriginalPath.Chars());
				return false;
			}

			if (pConverted) *pConverted = true;
		}
		else
			if (pConverted) *pConverted = false;
	}
	else
	{
		cacheFilePath = formattedOriginalPath;
		if (pConverted) *pConverted = false;
		return true;
	}

	return true;
}

}
}
