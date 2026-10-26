#include "stdafx.h"

#include <system/xml/Xml.h>
#include <system/misc/BtlStringUtil.h>

#include <EditorLib/Aftk/AftkRefHelpers.h>
#include <EditorLib/Aftk/AftkRefManager.h>
#include <EditorLib/Aftk/AftkRef.h>

#include <EditorLib/Xt/XtVisualisation.h>
#include <EditorLib/Xt/XtVisualisationHelpers.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneNodeIterator.h>
#include <EditorLib/Dg/SceneAftkNode.h>
#include <EditorLib/Dg/SceneAftkActorNode.h>
#include <EditorLib/Dg/XtNodeHelpers.h>

#include <EditorLib/Core.h>
#include <EditorLib/CoreConfig.h>
#include <EditorLib/GlobalSignals.h>

#include <Tools_Shared/Xml/XmlDom.h>
#include <Tools_Shared/Utility/MacroExpander.h>

namespace EditorLib
{

LarvaField* FindFieldAnnotation(
	struct LtStruct* type, 
	const char* pAttribute,
	const char* parameter)
{
	if (type->base)
	{
		LarvaField* pLarvaField = FindFieldAnnotation(
			type->base->type, pAttribute, parameter);
		if (pLarvaField) return pLarvaField;
	}

	for(XtUt::List<LarvaField>::Iterator it(&type->fields); it; ++it)
	{
		if (it->attrs.FindParam(pAttribute, parameter))
		{
			return &(*it);
		}
	}

	return 0;
}

LarvaParam* FindStructParameter(
	struct LtStruct* type, 
	const char* pAttribute,
	const char* parameter)
{
	if (type->base)
	{
		LarvaParam* pParam = FindStructParameter(
			type->base->type, pAttribute, parameter);
		if (pParam) return pParam;
	}

	return type->attrs.FindParam(pAttribute, parameter);
}


XtVisMeshInfo::XtVisMeshInfo()
: aftkUniqueId(0xffffffff)
{
}

XtVisMeshInfo::XtVisMeshInfo(const XtVisMeshInfo& r)
: stringKey(r.stringKey), aftkUniqueId(r.aftkUniqueId)
{
}

XtVisMeshInfo::XtVisMeshInfo(const Btl::String& stringKey_, U32 aftkUniqueId_)
	: stringKey(stringKey_), aftkUniqueId(aftkUniqueId_)
{
}

XtVisMeshInfo::XtVisMeshInfo(const Btl::String& meshGuid)
{
	Btl::Vector<Btl::String> tokenised;
	meshGuid.Tokenise(tokenised, '|');
	if (tokenised.Size() == 0)
	{
		aftkUniqueId = 0xffffffff;
	}
	else if (tokenised.Size() == 1)
	{
		stringKey = meshGuid;
		aftkUniqueId = 0xffffffff;
	}
	else if (tokenised.Size() == 2)
	{
		stringKey = tokenised[0];
		aftkUniqueId = (U32)atoi(tokenised[1].Chars());
	}
	else
		aftkUniqueId = 0xffffffff;
}

void XtVisMeshInfo::GenerateAftkUniqueMeshGuid(
	Btl::String& guidMesh)
{
	guidMesh.Printf("%s|%u", stringKey.Chars(), aftkUniqueId);
}

const XtVisMeshInfo& XtVisMeshInfo::operator=(const XtVisMeshInfo& r)
{
	stringKey = r.stringKey;
	aftkUniqueId = r.aftkUniqueId;
	return *this;
}

XtVisualisationDef* XtVisualisationDef::Clone()
{
	XtVisualisationDef* pClone = new XtVisualisationDef;
	pClone->visualisationType = visualisationType;
	pClone->filePath = filePath;
	pClone->id = id;
	pClone->scale = scale;

	return pClone;
}



C_XtVisualisationData::C_State::~C_State()
{
	for (S32 i=0; i < definitions.Size(); ++i)
		delete definitions[i];

	definitions.Clear();
	visualisationFolders.Clear();
	mappingById.Clear();
	mappingByFilePath.Clear();
}

C_XtVisualisationData::C_State* C_XtVisualisationData::C_State::Clone() const
{
	//This is all seriously nasty and needs binning :/

	typedef Ut::OpenHashMap<XtVisualisationDef*, XtVisualisationDef*, Ut::HashKeyOpPtr> 
		CloneMapping;

	C_XtVisualisationData::C_State* pClone = new C_XtVisualisationData::C_State;

	pClone->definitions.Resize(definitions.Size());
	CloneMapping cloneMapping;
	for (S32 i = 0; i < definitions.Size(); ++i)
	{
		pClone->definitions[i] = definitions[i]->Clone();
		cloneMapping[definitions[i]] = pClone->definitions[i];
	}

	//Copy with incorrect ptrs then remap.
	pClone->visualisationFolders = visualisationFolders;
	pClone->mappingById			= mappingById;
	pClone->mappingByFilePath	= mappingByFilePath;
	for (VisualisationMap::Iterator it = pClone->mappingById.Begin(); 
		it != pClone->mappingById.End();
		++it)
	{
		//Swap defs.
		it->GetValue() = cloneMapping[it->GetValue()];
	}
	for (VisualisationMap::Iterator it = pClone->mappingByFilePath.Begin(); 
		it != pClone->mappingByFilePath.End();
		++it)
	{
		//Swap defs.
		it->GetValue() = cloneMapping[it->GetValue()];
	}

	return pClone;
}



C_XtVisualisationData::C_XtVisualisationData()
{
	m_pPristineState = 0;
	m_pState = 0;
	m_slots.Connect();
}

C_XtVisualisationData::~C_XtVisualisationData()
{
	UnregisterDefinitions();
	m_slots.Disconnect();
}

void CleanFilePath(
	Btl::String& filePath, const char* pFilePath)
{
	filePath = pFilePath;
	Tools::C_MacroExpander::Instance().Expand(filePath);
	filePath = Misc::FormatUnixFileString(filePath);
	filePath.ConvertToLower();
}

void C_XtVisualisationData::AddMapping(
	const char* pId,
	XtVisualisationType visualisationType, 
	const char* pFileName, 
	const char* pScale)
{
	if (!pId || !pFileName)
		return;

	Btl::String id = pId;
	id.ConvertToLower();
	Btl::String filePath;
	CleanFilePath(filePath, pFileName);

	if (m_pState->mappingByFilePath.Find(filePath) != m_pState->mappingByFilePath.End())
		return;

	XtVisualisationDef *pNewDef = new XtVisualisationDef();
	pNewDef->filePath = filePath;
	pNewDef->id = id;
	pNewDef->visualisationType = visualisationType;

	if (pScale)
		pNewDef->scale = (F32)atof(pScale);
	else
		pNewDef->scale = 1;

	{
		//check for duplicates
		S32 index = -1;
		for (S32 j=0; j<m_pState->definitions.Size() && index==-1; ++j)
			if (*m_pState->definitions[j] == *pNewDef)
				index = j;

		if (index == -1)
			m_pState->definitions.PushBack(pNewDef);
		else
		{
			delete pNewDef;
			pNewDef = m_pState->definitions[index];
		}
	}

	m_pState->mappingByFilePath.Insert(VisualisationMap::DataType(filePath, pNewDef));

	//Adjust id mapping if present.
	if (m_pState->mappingById.Find(id) == m_pState->mappingById.End())
	{
		m_pState->mappingById.Insert(VisualisationMap::DataType(id, pNewDef));
	}
}

void C_XtVisualisationData::ParseVisualisation(
	const char* pId, 
	const Sys::C_XmlDomElement* pVisualisation)
{
	const char* pFilePath = 0;
	const char* pScale = 0;
	XtVisualisationType visualisationType;

	const char* pVisType = pVisualisation->GetName();

	if (stricmp(pVisType, "mesh")==0) {
		visualisationType = XtVisualisationMesh;

	} else if (stricmp(pVisType, "sprite")==0) {
		visualisationType = XtVisualisationSprite;
		pScale = pVisualisation->GetAttributes().GetValueByName("scale");
	} else {
		return;
	}
	
	pFilePath = pVisualisation->GetAttributes().GetValueByName("file");

	if (!pId || !pFilePath)
		return;

	AddMapping(pId, visualisationType, pFilePath, pScale);
}

void C_XtVisualisationData::ParseVisualisationFolder(const Btl::String& root, const Sys::C_DirectoryInfo& directoryInfo, bool recursive)
{
	const Sys::FileInfoArr& fileInfoArray = directoryInfo.FileInfoArray();

	for (int i=0; i < fileInfoArray.Size(); i++)
	{
		if (fileInfoArray[i].m_FileType == Sys::E_FileType_File)
		{
			XtVisualisationType visualisationType = XtVisualisationMesh;
			
			Btl::String filePath = fileInfoArray[i].m_FileName.Chars();
			filePath = Misc::FormatUnixFileString(filePath);

			const char* kVisNamingConvention = "_vis.a3d";
			
			U32 visLocation = filePath.ISearch(kVisNamingConvention);
			if (visLocation == -1 || visLocation < filePath.Length()-strlen(kVisNamingConvention))
				continue;

			//Make id root relative.
			Btl::String id = filePath.IReplace(root, "");
			//Remove _vis extension
			id = id.IReplace(kVisNamingConvention, "");

			AddMapping(id.Chars(), visualisationType, filePath.Chars(), 0);
		}
		else if (fileInfoArray[i].m_FileType == Sys::E_FileType_Directory && recursive)
		{
			Sys::C_DirectoryInfo directoryInfo;
			Sys::C_FileSystem::Instance().DirectoryInfo(fileInfoArray[i].m_FileName, &directoryInfo);
			ParseVisualisationFolder(root, directoryInfo, recursive);
		}
	}
}

void C_XtVisualisationData::ParseVisualisationFolder(const Sys::C_XmlDomElement* pVisualisationFolder)
{
	Btl::String directory = pVisualisationFolder->GetAttributes().GetValueByName("directory");

	if (directory.Length() == 0)
		return;

	bool recursive = false;
	const char* recursiveText = pVisualisationFolder->GetAttributes().GetValueByName("recursive");
	if (recursiveText && stricmp(recursiveText,"true")==0)
		recursive = true;

	bool abSync = false;
	const char* abSyncText = pVisualisationFolder->GetAttributes().GetValueByName("abSync");
	if (abSyncText && stricmp(abSyncText,"true")==0)
		abSync = true;

	Sys::C_DirectoryInfo directoryInfo;
	Sys::C_FileSystem::Instance().DirectoryInfo(directory.Chars(), &directoryInfo);

	m_pState->visualisationFolders.PushBack();
	m_pState->visualisationFolders.Back().path = directory;
	m_pState->visualisationFolders.Back().abSync = abSync;
	ParseVisualisationFolder(directory, directoryInfo, recursive);
}

C_Status C_XtVisualisationData::RegisterDefinitions(const char* pConfigPath)
{
	m_configFilePath = pConfigPath;

	m_pState = new C_State;
	assert(!m_pPristineState);

	if (C_Core::Instance().GetConfig().appType == E_AppType_Interactive)
	{
		//add a default 'none' visualisation
		AddMapping("NONE", XtVisualisationUnknown, "", "");

		Sys::C_XmlDom dom;
		if (!dom.Read(pConfigPath))
			return E_Status_Failure;

		const Sys::C_XmlDomElement *pDefinitionRoot =
			(const Sys::C_XmlDomElement*) Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "visualisation");
		if (!pDefinitionRoot)
			return E_Status_Failure;

		const Sys::C_XmlDomBaseNode::Array& definitions = pDefinitionRoot->GetChildren();
		for (S32 i=0; i<definitions.Size(); ++i)
		{
			const Sys::C_XmlDomElement *pVisDefinition = (const Sys::C_XmlDomElement*) definitions[i];

			const char* pName = pVisDefinition->GetName();
			if (stricmp(pName, "vis") == 0)
			{
				const Sys::C_XmlDomBaseNode::Array& visualisations = pVisDefinition->GetChildren();
				if (visualisations.Size() != 1)
					continue;

				const Sys::C_XmlDomElement *pVisualisation = (const Sys::C_XmlDomElement*)visualisations[0];

				const char* pId = pVisDefinition->GetAttributes().GetValueByName("id");;
				
				ParseVisualisation(pId, pVisualisation);
			}
			else if (stricmp(pName, "visfolder") == 0)
			{
				ParseVisualisationFolder(pVisDefinition);
			}
		}
	}

	m_pPristineState = m_pState->Clone();
	return E_Status_Success;
}

C_Status C_XtVisualisationData::UnregisterDefinitions()
{
	SAFE_DELETE_PTR(m_pState);
	SAFE_DELETE_PTR(m_pPristineState);
	return E_Status_Success;
}

C_Status C_XtVisualisationData::RescanFolders()
{
	ConstAftkRefList refList;
	C_Core::Instance().GetAftkRefManager().GetRefs(refList);
	for (ConstAftkRefList::Iterator it = refList.Begin(); it != refList.End(); ++it)
	{
		if ((*it)->IsVisualisation())
			(*it)->Unload();
	}

	UnregisterDefinitions();
	return RegisterDefinitions(m_configFilePath.Chars());
}

const XtVisualisationDef* C_XtVisualisationData::GetXtVisualisationDef(
	const I_DgNode *pNode) const
{
	if (!m_pState)
		return 0;
	
	if (pNode && pNode->IsA(EditorLib::E_ObjType_SceneXtNodeBase))
	{
		C_SceneXtNodeBase* pSceneXtNode = (C_SceneXtNodeBase*)pNode;

		if (!pSceneXtNode->GetXtType())
			return 0;

		const Xt::DomDb *pDb = C_Core::Instance().GetXtManager().GetDb();
		assert(pDb);

		//See if there's a field within our larva type that's designated as being the visualisation
		//id source.
		LarvaParam* pVisByIdParam = FindStructParameter(
			(LtStruct*)pSceneXtNode->GetXtType(), "SceneView", "visualisationbyfieldval");
		if (pVisByIdParam)
		{
			Xt::FieldDom fieldDom = Xt::GetByDottedPath(
				(LtStruct*)pSceneXtNode->GetXtType(),
				&Xt::AsComponent(pSceneXtNode->GetXtData())->data,
				pVisByIdParam->AsString().Text());
			
			if (fieldDom.dom)
			{
				Xt::DomField* pDomField = Xt::AsField(fieldDom.dom);
				if (pDomField)
				{
					Btl::String id = pDomField->value.Text();
					id.ConvertToLower();
					VisualisationMap::ConstIterator it = m_pState->mappingById.Find(id);
					if (it != m_pState->mappingById.End())
						return it->GetValue();
				}
			}
		}
		//Else perform recursive summary text evaluation. Note that this requires that any
		//referenced records exist, so we can't just start deleting pack files for performance.
		{
			Btl::String id = XtVisualisationHelpers::EvaluateSummaryText(
				"SceneView", 
				"visualisation", 
				pDb->schema, 
				(LtStruct*)pSceneXtNode->GetXtType(), 
				Xt::AsComponent(pSceneXtNode->GetXtData())).Text();
			id.ConvertToLower();
			id = Misc::FormatUnixFileString(id);

			VisualisationMap::ConstIterator it = m_pState->mappingById.Find(id);
			if (it != m_pState->mappingById.End())
				return it->GetValue();
		}
	}

	return 0;
}

void C_XtVisualisationData::BuildRootFolderList(Btl::Vector<C_XtVisualisationRootFolder>& folderList) const
{
	if (!m_pState)
		return;

	folderList = m_pState->visualisationFolders;
}

void C_XtVisualisationData::BuildResourceFileList(Btl::Vector<Btl::String>& filelist) const
{
	if (!m_pState)
		return;

	Btl::OpenHashSet<Btl::String, Btl::HashKeyOpBtlString> uniqueFiles;

	for (S32 i=0; i<m_pState->definitions.Size(); ++i)
	{
		if (m_pState->definitions[i]->visualisationType == XtVisualisationMesh)
		{
			Btl::String path = m_pState->definitions[i]->filePath;
			if (path.Length() && uniqueFiles.Find(path) == uniqueFiles.End())
			{
				uniqueFiles.Insert(path);
				filelist.PushBack(path);
			}
		}
	}
}

const XtVisualisationDef* C_XtVisualisationData::FindByFilePath(const char *pOriginalFilePath) const
{
	if (!m_pState)
		return 0;

	Btl::String file = Misc::FormatUnixFileString(Btl::String(pOriginalFilePath).Lower());
	VisualisationMap::Iterator it = m_pState->mappingByFilePath.Find(file);
	if (it == m_pState->mappingByFilePath.End())
		return 0;

	return it->GetValue();
}

void C_XtVisualisationData::RemapVisualisationByExtActors(
	const Btl::String& originalFilePath)
{
	C_AftkRefManager& manager = C_Core::Instance().GetAftkRefManager();
	const C_AftkRef* pRef = manager.GetRef(originalFilePath);
	if (!pRef || !pRef->IsVisualisation() || !pRef->GetSceneRootNode())
		return;

	Btl::String filePath;
	CleanFilePath(filePath, originalFilePath.Chars());

	VisualisationMap::Iterator it = m_pState->mappingByFilePath.Find(filePath);
	if (it == m_pState->mappingByFilePath.End())
		return;

	XtVisualisationDef* pDef = it->GetValue();

	//This limits us to one mesh per vis def and one vis def per file. Not great, but it's a start.
	for (C_SceneNodeConstIterator it(pRef->GetSceneRootNode()->GetChild()); it; ++it)
	{
		C_SceneNode* pSceneNode = it;
		if (!pSceneNode->IsA(E_ObjType_SceneAftkActorNode))
			continue;

		C_SceneAftkActorNode* pActorNode = (C_SceneAftkActorNode*)pSceneNode;
		
		Util::ConstKeyValueArray& attributes = pActorNode->GetAftkProperties();
		for (S32 i = 0; i < attributes.Size(); ++i)
		{
			const Util::C_KeyValue& attr = attributes[i];
			if (attr.GetName().ISearch("AS_Actor") != -1)
			{
				XtVisMeshInfo guidBuilder(filePath, pActorNode->GetAftkUniqueId());
				Btl::String meshGuid;
				guidBuilder.GenerateAftkUniqueMeshGuid(meshGuid);

				Btl::String xml;
				attr.GetValue(xml);
				RemapVisualisationByExtActor(pDef, filePath, xml.Chars());
				return;
			}
		}
	}

	//No remapping by as_actor, use mesh name as visId
	m_pState->mappingById.Insert(VisualisationMap::DataType(pDef->id, pDef));
}

LarvaType* ParseLarvaType(Sys::C_XmlDomBaseNode* pRootXmlBlock)
{
	LarvaSchema* pLarvaSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	assert(pLarvaSchema);

	for (S32 i = 0; i < pRootXmlBlock->GetChildren().Size(); ++i)
	{
		if (pRootXmlBlock->GetChildren()[i]->GetType() != Sys::E_XmlDomNodeType_Element)
			continue;

		Sys::C_XmlDomElement* pElement = (Sys::C_XmlDomElement*)pRootXmlBlock->GetChildren()[i];
		const char* pElementType = pElement->GetName().Chars();

		if (!strcmp(pElementType, "field"))
		{
			if (pElement->GetAttributes().Elements() != 1)
			{
				LOGERROR(
					C_Core::Instance().GetLogger(), 
					"Invalid number of attributes for field");
				continue;
			}

			const Sys::C_XmlAttribute* pAttr = pElement->GetAttributes()[0];
			const char* pName = pAttr->name.Chars();
			const char* pValue = pAttr->value.Chars();

			if (!stricmp(pName, "__type"))
			{
				return pLarvaSchema->types[pValue];
			}
		}
	}

	return 0;
}

bool ParseVisId(
	Btl::String& visId, 
	Sys::C_XmlDomBaseNode* pRootXmlBlock, 
	const char* pFieldName)
{
	for (S32 i = 0; i < pRootXmlBlock->GetChildren().Size(); ++i)
	{
		if (pRootXmlBlock->GetChildren()[i]->GetType() != Sys::E_XmlDomNodeType_Element)
			continue;

		Sys::C_XmlDomElement* pElement = (Sys::C_XmlDomElement*)pRootXmlBlock->GetChildren()[i];
		const char* pElementType = pElement->GetName().Chars();

		if (!strcmp(pElementType, "field"))
		{
			if (pElement->GetAttributes().Elements() != 1)
				continue;

			const Sys::C_XmlAttribute* pAttr = pElement->GetAttributes()[0];
			const char* pName = pAttr->name.Chars();
			const char* pValue = pAttr->value.Chars();
			if (strlen(pValue) == 0)
				return false;

			if (!stricmp(pName, pFieldName))
			{
				//Found our visualisation id field value!
				visId = pValue;
				return true;
			}
		}
	}
	return false;
}

void C_XtVisualisationData::RemapVisualisationByExtActor(
	XtVisualisationDef* pDef,
	const Btl::String& filePath,
	const Btl::String& xml)
{
	//Create a single actor from incoming xml description of the actor's xt dom.
	Sys::C_XmlDom xmlDom;
	//Create a root node
	xmlDom.SetRootNodePtr(new Sys::C_XmlDomBaseNode());

	if (Sys::XmlParse(xmlDom, xml.Chars(), xml.Length()) != Sys::E_XmlResult_Success 
		||
		!xmlDom.GetRootNodePtr())
	{
		LOGERROR(
			C_Core::Instance().GetLogger(), 
			"Failed to parse embedded xml in scene %s", filePath.Chars());
		return;
	}
	//Find xml element representing the root block for our actor.
	Sys::C_XmlDomBaseNode* pRootXmlBlock = xmlDom.GetRootNodePtr();

	LarvaType* pLarvaType = ParseLarvaType(pRootXmlBlock);
	if (!pLarvaType || pLarvaType->kind != kLtStruct)
		return;

	//See if there's a field within our larva type that's designated as being the visualisation
	//id source.
	LarvaParam* pVisByIdParam = FindStructParameter(
		(LtStruct*)pLarvaType, "SceneView", "visualisationbyfieldval");
	if (!pVisByIdParam)
	{
		return;
	}

	//Parse xml to find value of visualisation id, as it would be set when this actor is created.
	Btl::String id;

	//strip hierarchical fields from id
	XtString field = pVisByIdParam->AsString();
	const char* current = field.Text();
	const char* start = field.Text();

	while ((*current != '\0'))
	{
		if (*current == '.')
			start = ++current;
		else
			++current;
	}

	if (!ParseVisId(id, pRootXmlBlock, XtString(start, current-start).Text()))
	{
///		LOGWARN(
//			C_Core::Instance().GetLogger(), 
//			"Missing record definition in actor XML for IRM prepared object %s. The person who prepared this needs to fix it!", 
//				filePath.Chars());
		return;
	}
	id = Misc::FormatUnixFileString(id);
	id.ConvertToLower();

	//Remove mappingbyid entries for the vis scene we're processing.
	{
		Btl::Vector<Btl::String> toErase;
		for (VisualisationMap::Iterator it = m_pState->mappingById.Begin(); 
			it != m_pState->mappingById.End();
			++it)
		{
			if (it->GetValue() == pDef)
				toErase.PushBack(it->GetKey()); 
		}
		for (S32 i = 0; i < toErase.Size(); ++i)
		{
			VisualisationMap::Iterator it = m_pState->mappingById.Find(toErase[i]);
			assert(it != m_pState->mappingById.End());
			m_pState->mappingById.Erase(it);
		}
	}

	m_pState->mappingById.Insert(VisualisationMap::DataType(id, pDef));
	pDef->id = id;
}

void C_XtVisualisationData::RevertRemap()
{
	delete m_pState;
	if (m_pPristineState)
		//Cakeyism naming convention.
		m_pState = m_pPristineState->Clone();
	else
		m_pState = 0;
}

////// SLOTS //////

void C_XtVisualisationData::C_Slots::Connect()
{
	C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.connect(this, &C_XtVisualisationData::C_Slots::OnDatabaseEventSlot);

	s.aftkRefLoaded.connect(this, &C_XtVisualisationData::C_Slots::OnAftkRefLoadedSlot);
	s.aftkRefUnloaded.connect(this, &C_XtVisualisationData::C_Slots::OnAftkRefUnloadedSlot);
}

void C_XtVisualisationData::C_Slots::Disconnect()
{
	C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.levelClosing.disconnect(this);

	s.aftkRefLoaded.disconnect(this);
	s.aftkRefUnloaded.disconnect(this);
}

void C_XtVisualisationData::C_Slots::OnDatabaseEventSlot(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt)
{
	/*
	if (evt == E_DatabaseEvent_Opened)
	{
		C_Core::Instance().GetXtVisualisationData().RescanFolders();
	}
	else if (evt == E_DatabaseEvent_Closing)
	{
		EditorLib::C_Core::Instance().GetXtVisualisationData().RevertRemap();
	}
	*/
}

void C_XtVisualisationData::C_Slots::OnAftkRefLoadedSlot(
	const Btl::String& originalFilePath, 
	const Aftk::C_Scene& aftkScene, 
	const EditorLib::C_SceneNode& sceneNode, 
	const EditorLib::AftkDgMap& aftkDgMap)
{
	EditorLib::C_Core::Instance().GetXtVisualisationData().RemapVisualisationByExtActors(
		originalFilePath);
}

void C_XtVisualisationData::C_Slots::OnAftkRefUnloadedSlot(
	const Btl::String& originalFilePath)
{
}

}
