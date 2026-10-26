#include "stdafx.h"

using namespace MayaIo;

//#pragma optimize("", off)

bool C_MaterialImport::CanImportNode(const MFnDependencyNode& shaderDependencyNode)
{
	if (
		(
		((shaderDependencyNode.typeId().id() >= MAYA_IO_CGFX_ID_RANGE_START) && (shaderDependencyNode.typeId().id() < MAYA_IO_CGFX_ID_RANGE_END))
		|| shaderDependencyNode.typeId().id() == MAYA_IO_CGFX_2008_ID
		|| shaderDependencyNode.typeId().id() == MAYA_IO_HLSL_2008_ID
		)
		||
		(
			shaderDependencyNode.object().hasFn(MFn::kLambert) ||
			shaderDependencyNode.object().hasFn(MFn::kBlinn) ||
			shaderDependencyNode.object().hasFn(MFn::kPhong)
		)
		)
	{
		return true;
	}

	return false;
}

const Aftk::C_SharedResource<Aftk::C_MaterialNode>*	C_MaterialImport::Import(
	C_Importer* pImporter,
	const MFnDependencyNode& shaderDependencyNode,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
	Aftk::C_Status& status)
{
	//Skip node's we can't import
	if (!CanImportNode(shaderDependencyNode))
	{
		std::stringstream ss;
		ss << "Unknown material type for node [" << shaderDependencyNode.name().asChar() << "].";
		AFTK_DO_BEGIN << ss.str() AFTK_DO_DEBUG_END;
		status = Aftk::C_Status::E_Status_Success;
		return 0;
	}

	std::string materialName = shaderDependencyNode.name().asChar();

	const Aftk::C_SharedResource<Aftk::C_MaterialNode>* pExistingMaterial = 
		pImporter->GetImportedMaterialByName(materialName);

	if (pExistingMaterial != 0)
	{
		//We've already imported this material before.
		return pExistingMaterial;
	}

	#ifdef DEBUG_PRINT
	AFTK_DO_BEGIN << "Material import in progress for [" << materialName << "]...\n" AFTK_DO_DEBUG_END;
	#endif

	Aftk::C_MaterialNode aftkMaterialNode;
	if (
		((shaderDependencyNode.typeId().id() >= MAYA_IO_CGFX_ID_RANGE_START) && (shaderDependencyNode.typeId().id() < MAYA_IO_CGFX_ID_RANGE_END))
		)
	{
		#ifdef DEBUG_MATERIALS
		AFTK_DO_BEGIN << "CgFX shader found.\n" AFTK_DO_DEBUG_END;
		#endif
		aftkMaterialNode.SetOrigin(Aftk::E_MaterialOrigin_Maya_CgFX);
		status = ImportMeshMaterialFull(
			pImporter,
			shaderDependencyNode,
			aftkMaterialNode,
			pAftkScene);
		if (!status) return 0;
	} 
	else if (shaderDependencyNode.typeId().id() == MAYA_IO_CGFX_2008_ID)
	{
		#ifdef DEBUG_MATERIALS
		AFTK_DO_BEGIN << "CgFX7 shader found.\n" AFTK_DO_DEBUG_END;
		#endif
		aftkMaterialNode.SetOrigin(Aftk::E_MaterialOrigin_Maya_CgFX7);
		status = ImportMeshMaterialFull(
			pImporter,
			shaderDependencyNode,
			aftkMaterialNode,
			pAftkScene);
		if (!status) return 0;
	}
	else if (shaderDependencyNode.typeId().id() == MAYA_IO_HLSL_2008_ID)
	{
		#ifdef DEBUG_MATERIALS
		AFTK_DO_BEGIN << "Hlsl 2008 shader found.\n" AFTK_DO_DEBUG_END;
		#endif
		aftkMaterialNode.SetOrigin(Aftk::E_MaterialOrigin_Maya_Hlsl_2008);
		status = ImportMeshMaterialFull(
			pImporter,
			shaderDependencyNode,
			aftkMaterialNode,
			pAftkScene);
		if (!status) return 0;
	}
	else
	{
		#ifdef DEBUG_MATERIALS
		AFTK_DO_BEGIN << "Maya shader found.\n" AFTK_DO_DEBUG_END;
		#endif
		aftkMaterialNode.SetOrigin(Aftk::E_MaterialOrigin_Maya);
		status = ImportMeshMaterialSimple(
			shaderDependencyNode,
			aftkMaterialNode,
			pAftkScene,
			pAftkSceneMeshNode);
		if (!status) return 0;
	}

	const Aftk::C_SharedResource< Aftk::C_MaterialNode >* pAftkMaterialHandle = 
		pAftkScene->AddMaterialNode(aftkMaterialNode);
	if (pAftkMaterialHandle != 0)
	{
		pImporter->AddToImportedMaterialMap(pAftkMaterialHandle);
	}

	#ifdef DEBUG_PRINT
	AFTK_DO_BEGIN << "Material import in complete.\n" AFTK_DO_DEBUG_END;
	#endif

	status = Aftk::C_Status::E_Status_Success;
	return pAftkMaterialHandle;
}

bool C_MaterialImport::DoWeImportThisPlug(
	C_Importer* pImporter,
	const MPlug& mayaPlug)
{
	std::string plugName;
	if (!StripNodeNameFromPlugName(mayaPlug.name().asChar(), plugName))
	{
		return false;
	}
	const char* cPlugName = plugName.c_str();

	bool found = pImporter->GetMaterialFilter().Find(cPlugName);
	if (found)
	{
		//Found in our exclusion list
		return false;
	}

	return true;
}

bool C_MaterialImport::DoWeImportThisPlugAsDynamic(const MPlug& mayaPlug)
{
	if (!strcmp(mayaPlug.partialName().asChar(), MAYA_IO_TEXTURE_FILE_NAME_PLUG_PARTIAL_NAME))
		return false;

	return true;
}

Aftk::C_Status	C_MaterialImport::CreateShaderTreeNodes(
	C_Importer* pImporter,
	Ut::UtVector< Ut::DynamicVector<ConnectionMapping> >&	connectionMappings,
	Ut::UtMap<std::string, Aftk::C_Node*>&			shaderMapping,
	const MFnDependencyNode& shaderDependencyNode,
	Aftk::C_MaterialNode& aftkMaterialNode,
	Aftk::C_Scene* pAftkScene)
{
	Aftk::C_Status status;
	MStatus mStatus;

	MItDependencyGraph dgIt(shaderDependencyNode.object(),
		MFn::kInvalid,
		MItDependencyGraph::kUpstream, 
		MItDependencyGraph::kBreadthFirst,
		MItDependencyGraph::kNodeLevel, 
		&mStatus);

	if (mStatus != MStatus::kSuccess)
	{
		AFTK_DO_BEGIN << "Could not create dependency graph iterator." AFTK_DO_ERROR_RETURN_END_1;
	}
	// Disable dependency graph node filtering.
	dgIt.disablePruningOnFilter();

	//Iterate thru shader nodes
	while (!dgIt.isDone())
	{
		MObject dgNodeObject = dgIt.thisNode();
		MFnDependencyNode mFnDependencyNode(dgNodeObject);
		Aftk::C_Node* pNewNode = 0;

		#ifdef DEBUG_MATERIALS
		AFTK_DO_BEGIN << "C_Node Name: " << mFnDependencyNode.name().asChar() << "\n" AFTK_DO_DEBUG_END;
		AFTK_DO_BEGIN << "C_Node has " << mFnDependencyNode.attributeCount() << " attributes\n" AFTK_DO_DEBUG_END;
		AFTK_DO_BEGIN << "C_Node type name " << mFnDependencyNode.typeName().asChar() << "\n" AFTK_DO_DEBUG_END;
		#endif

		//Add to our per shader connection mapping info
		connectionMappings.PushBack();
		ConnectionMapping* pConnectionMapping = 
			&connectionMappings[connectionMappings.Size()-1];
		//For this connection mapping, create a sparse array for maya->aftk index mapping
		pConnectionMapping->m_mayaToAftkIndexMapping.Resize(mFnDependencyNode.attributeCount());

		//Create external nodes not part of material.
		pNewNode = C_ConnectedDgImport::CreateNode(		
			pImporter,
			mFnDependencyNode);

		if (!pNewNode)
		{
			if ( dgNodeObject.hasFn( MFn::kFileTexture ) )
			{
				Aftk::C_ShaderImageNode* pAftkShaderImageNode = new Aftk::C_ShaderImageNode(&status);
				AFTK_CHECK_ERROR_RETURN;

				if (!pAftkShaderImageNode)
					return Aftk::C_Status::E_Status_OutOfMemory;

				pNewNode = (Aftk::C_ShaderNode*)pAftkShaderImageNode;

				// get the attribute for the full texture path
				MPlug ftn = mFnDependencyNode.findPlug(MAYA_IO_TEXTURE_FILE_NAME_PLUG_NAME);

				// get the filename from the attribute
				MString fileName;
				ftn.getValue(fileName);

				// write the file name
				#ifdef DEBUG_MATERIALS
				AFTK_DO_BEGIN << fileName.asChar() << "\n" AFTK_DO_DEBUG_END;
				#endif

				std::string fileNameStd;
				C_MaterialHelper::CleanUpFileName(fileName.asChar(), fileNameStd);
				pAftkShaderImageNode->GetFilePlug().SetValue(fileNameStd);
			}
			else
			{
				pNewNode = new Aftk::C_ShaderNode;
				if (!pNewNode)
				{
					AFTK_DO_BEGIN << "Failed to create new shader node." AFTK_DO_ERROR_RETURN_END_1;
				}
			}
		}

		if (pNewNode->HasType(Aftk::E_ObjType_Shader))
		{
			Aftk::C_ShaderNode* pAftkShaderNode = (Aftk::C_ShaderNode*)pNewNode;

			for (U32 attrPhysicalIdx = 0; attrPhysicalIdx < mFnDependencyNode.attributeCount(); attrPhysicalIdx++)
			{
				MPlug attrPlug(dgNodeObject, mFnDependencyNode.attribute(attrPhysicalIdx));
				//If we are a member of a compound 
				if (attrPlug.isChild())
				{
					continue;
				}

				bool doWeImportThisPlug = DoWeImportThisPlug(
					pImporter, 
					attrPlug);
				if (!doWeImportThisPlug)
				{
					continue;
				}

				//Skip the filename plug if this is a file texture node
				bool doWeImportThisPlugAsDynamic = DoWeImportThisPlugAsDynamic(attrPlug);
				if (!doWeImportThisPlugAsDynamic)
				{
					continue;
				}

				Aftk::C_DynamicPlug* pAftkPlug = new Aftk::C_DynamicPlug();
				if (!pAftkPlug)
					return Aftk::C_Status::E_Status_OutOfMemory;
				S32 plugIndex = pAftkShaderNode->AddPlugPtr(pAftkPlug);

				if (attrPlug.isConnected())
				{
					#ifdef DEBUG_MATERIALS
					g_tcDebugMan << "plug is connected\n";
					#endif

					//Maps a maya physical index to an aftk plug index. This mapping uses a
					//sparse array.
					pConnectionMapping->m_mayaToAftkIndexMapping[attrPhysicalIdx] = plugIndex;
					//Add a new index into the above sparse array
					pConnectionMapping->m_connectedMappingIndices.PushBack(attrPhysicalIdx);
				}
		
				if (!attrPlug.isCompound())
				{
					#ifdef DEBUG_MATERIALS
					g_tcDebugMan << "Attribute plug name " << attrPlug.name().asChar() <<
						" [NON-COMP-PLUG] ";
					#endif
					
					//Mark that we've no connections for this plug
					pConnectionMapping->m_mayaToAftkIndexMapping[attrPhysicalIdx] = -1;
				}

				status = MayaPlugToAftkDynamicPlug(
					pImporter,
					attrPlug, 
					*pAftkPlug);
				CHECK_ERROR_RETURN;

				#ifdef DEBUG_MATERIALS
				g_tcDebugMan << "\n";
				#endif
			}

			//Store the name and type of this shader node
			pAftkShaderNode->SetName(mFnDependencyNode.name().asChar());
			pAftkShaderNode->SetOriginalNodeTypeName(mFnDependencyNode.typeName().asChar());
			aftkMaterialNode.AddShaderNodePtr(pAftkShaderNode);
		}

		//Add to our shader mapping
		if (pNewNode)
			shaderMapping[mFnDependencyNode.name().asChar()] = pNewNode;

		dgIt.next();
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status	C_MaterialImport::CreateShaderTreeNodesSimple(
	const MFnDependencyNode& shaderDependencyNode,
	Aftk::C_MaterialNode& aftkMaterialNode,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode)
{
	Aftk::C_Status status;
	MStatus mStatus;

	Aftk::C_ShaderNode* pAftkShaderNode = new Aftk::C_ShaderNode;
	if (!pAftkShaderNode)
		return Aftk::C_Status::E_Status_OutOfMemory;
	//Add shader node to material.
	aftkMaterialNode.AddShaderNodePtr(pAftkShaderNode);	

	pAftkShaderNode->SetName(shaderDependencyNode.name().asChar());

	MPlug outColour = shaderDependencyNode.findPlug("color", &mStatus);
	MCHECK_ERROR_RETURN_1;

	MPlugArray connectedPlugArray;
	outColour.connectedTo(connectedPlugArray, true, false);
	if (connectedPlugArray.length() > 0)
	{
		std::string destinationPlugName;
		if (connectedPlugArray.length() > 0)
		{
			destinationPlugName = 
				connectedPlugArray[0].name().asChar();
		}
		//If we're connected to a file-texture node...
		if (connectedPlugArray[0].node().hasFn(MFn::kFileTexture))
		{
			Aftk::C_ShaderImageNode* pAftkShaderImageNode = new Aftk::C_ShaderImageNode(&status);
			if (!pAftkShaderImageNode)
				return Aftk::C_Status::E_Status_OutOfMemory;
			//Add shader node to material.
			aftkMaterialNode.AddShaderNodePtr(pAftkShaderImageNode);

			Aftk::C_DynamicPlug* pColourPlug = new Aftk::C_DynamicPlug();
			if (!pColourPlug)
				return Aftk::C_Status::E_Status_OutOfMemory;

			pAftkShaderImageNode->AddPlugPtr(pColourPlug);

			std::string shaderPlugName = "outColor";
			pColourPlug->SetName(shaderPlugName);

			//Get the file name.
			{
				MFnDependencyNode imageNode(connectedPlugArray[0].node());
				// get the attribute for the full texture path
				MPlug ftn = imageNode.findPlug(MAYA_IO_TEXTURE_FILE_NAME_PLUG_NAME, &mStatus);
				MCHECK_ERROR_RETURN_1;

				// get the filename from the attribute
				MString fileName;
				ftn.getValue(fileName);

				// write the file name
				#ifdef DEBUG_MATERIALS
				g_tcDebugMan <<	fileName.asChar() << "\n";
				#endif

				std::string fileNameStd;
				C_MaterialHelper::CleanUpFileName(fileName.asChar(), fileNameStd);
				pAftkShaderImageNode->GetFilePlug().SetValue(fileNameStd);
			}

			//Check for a UVSet Name
			{
				MFnDependencyNode imageNode(connectedPlugArray[0].node());
				// Find the connection to the place2DTexture node
				MPlug outUV = imageNode.findPlug("uvCoord", &mStatus);
				MCHECK_ERROR_RETURN_1;

				MPlugArray connectedPlugArray;
				outUV.connectedTo(connectedPlugArray, true, false);
				if (connectedPlugArray.length() > 0)
				{
					MFnDependencyNode place2DTexNode(connectedPlugArray[0].node());
					// Find the connection to the uvChooser node
					MPlug outUV2 = place2DTexNode.findPlug("uvCoord", &mStatus);
					MCHECK_ERROR_RETURN_1;

					connectedPlugArray.clear();
					outUV2.connectedTo(connectedPlugArray, true, false);
					if (pAftkSceneMeshNode)
					{
						//If we're importing a material connected to a mesh, and there is a chooser present...
						if (connectedPlugArray.length() > 0)
						{
							MFnDependencyNode uvChooserNode(connectedPlugArray[0].node());
							// Find the connection back to the mesh node
							mStatus = uvChooserNode.getConnections(connectedPlugArray);
							MCHECK_ERROR_RETURN_1;

							MString chooserName = uvChooserNode.name();
							MPlug uvSetNamePlug;
							// For some reason the findPlug function is unable to find the
							// "uvSets" array plug and so we need to grab all connections
							// and assume we are searching for the [0] element
							for (unsigned i = 0; i < connectedPlugArray.length(); i ++) {

								MPlug currentPlug = connectedPlugArray[i];
								if (currentPlug.name() == (chooserName + ".uvSets[0]")) {
									uvSetNamePlug = currentPlug;
									break;
								}

							}
						} 
					}

					//Store the name and type of this shader node
					pAftkShaderImageNode->SetName(imageNode.name().asChar());
					pAftkShaderImageNode->SetOriginalNodeTypeName(imageNode.typeName().asChar());
				}

			}
			
			Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
			pAftkShaderNode->AddPlugPtr(pPlug);

			if (!pPlug)
				return Aftk::C_Status::E_Status_OutOfMemory;
			pPlug->SetName("color");
			pPlug->SetSourceConnectionPtr(pColourPlug); 
		}
	}

	return Aftk::C_Status::E_Status_Success;
}


Aftk::I_Plug* C_MaterialImport::GetPlugPtrRecursive(
	Aftk::C_Node& node,
	const std::string& name,
	bool& found,
	Aftk::C_Status& status)
{
	for (U32 i = 0; i < node.GetNumPlugs(); i++)
	{
		//Look thru top level plug
		Aftk::I_Plug* pTopLevelPlug = node.GetPlugPtr(i);
		if (pTopLevelPlug->GetName() == name)
		{
			found = true;
			status = Aftk::C_Status::E_Status_Success;
			return pTopLevelPlug;
		} 
		//Look thru the children of this plug
		if (pTopLevelPlug->IsCompound())
		{
			for (U32 j = 0; j < pTopLevelPlug->GetNumChildren(); j++)
			{
				Aftk::I_Plug* pChildPlug = pTopLevelPlug->GetChildPtr(j);
				if (pChildPlug->GetName() == name)
				{
					found = true;
					status = Aftk::C_Status::E_Status_Success;
					return pChildPlug;
				} 
			}
		}
	}

	status = Aftk::C_Status::E_Status_Success;
	found = false;
	return 0;
}

Aftk::C_Status C_MaterialImport::ResolvePlugConnections(
	C_Importer* pImporter,
	const MFnDependencyNode& shaderDependencyNode,
	Aftk::C_MaterialNode& aftkMaterialNode,
	Aftk::C_Scene* pAftkScene,
	Ut::UtMap<std::string, Aftk::C_Node*>& shaderMapping)
{
	Aftk::C_Status status;
	MStatus mStatus;

	MItDependencyGraph dgIt(
		shaderDependencyNode.object(),
		MFn::kInvalid,
		MItDependencyGraph::kUpstream, 
		MItDependencyGraph::kBreadthFirst,
		MItDependencyGraph::kNodeLevel, 
		&mStatus);

	if (mStatus != MStatus::kSuccess)
	{
		AFTK_DO_BEGIN << "Could not create dependency graph iterator." AFTK_DO_ERROR_RETURN_END_1;
	}
	// Disable dependency graph node filtering.
	dgIt.disablePruningOnFilter();

	//Iterate thru shader nodes
	while (!dgIt.isDone())
	{
		MObject dgNodeObject = dgIt.thisNode();
		MFnDependencyNode mFnDependencyNode(dgNodeObject);

		//Find the shader node that we're currently operating upon
		Aftk::C_Node* pAftkShaderNode = 
			shaderMapping[mFnDependencyNode.name().asChar()];

		for (U32 attrPhysicalIdx = 0; attrPhysicalIdx < mFnDependencyNode.attributeCount(); attrPhysicalIdx++)
		{
			MPlug attrPlug(dgNodeObject, mFnDependencyNode.attribute(attrPhysicalIdx));
			if (!attrPlug.isConnected())
			{
				continue;
			}

			bool doWeImportThisPlug = DoWeImportThisPlug(pImporter, attrPlug);
			if (!doWeImportThisPlug)
			{
				continue;
			}

			std::string currentPlugName = attrPlug.name().asChar();

			MPlugArray connectedPlugArray;
			attrPlug.connectedTo(connectedPlugArray, true, false);
			if (connectedPlugArray.length() > 0)
			{
				#ifdef DEBUG_MATERIALS
				AFTK_DO_BEGIN << currentPlugName << " is the destination of " AFTK_DO_DEBUG_END;
				#endif
			}

			if (connectedPlugArray.length() > 0)
			{
				for (U32 connectedPlugs = 0; connectedPlugs < connectedPlugArray.length(); connectedPlugs++)
				{
					bool doWeImportThisPlug = DoWeImportThisPlug(pImporter, connectedPlugArray[connectedPlugs]);
					if (!doWeImportThisPlug)
					{
						//We are not importing the specified plug
						continue;
					}

					std::string connectionTargetPlugName;
					if (!StripNodeNameFromPlugName(
						connectedPlugArray[connectedPlugs].name().asChar(), 
						connectionTargetPlugName))
					{
						AFTK_DO_BEGIN << "Could not strip plug name for " << connectedPlugArray[connectedPlugs].name().asChar() AFTK_DO_ERROR_RETURN_END_1;
					}

					#ifdef DEBUG_MATERIALS
					AFTK_DO_BEGIN << connectionTargetPlugName << "\n" AFTK_DO_DEBUG_END;
					#endif

					MObject dgSourceNodeObject = 
						connectedPlugArray[connectedPlugs].node();
					MFnDependencyNode mFnSourceDependencyNode(dgSourceNodeObject);
					std::string sourceShaderNodeName = 
						mFnSourceDependencyNode.name().asChar();
                    
					Ut::UtMap<std::string, Aftk::C_Node*>::Iterator it;
					it = shaderMapping.Find(sourceShaderNodeName);

					Aftk::I_Plug* pSourcePlug = 0;
					Aftk::C_Node* pAftkSourceNode = 0;
					if (it == shaderMapping.End())
						continue;

					pAftkSourceNode = it->GetValue();
					assert(pAftkSourceNode);
					if (pAftkSourceNode->HasType(Aftk::E_ObjType_Shader))
					{
						bool found;
						pSourcePlug = 
							GetPlugPtrRecursive(
							*pAftkSourceNode, 
							connectionTargetPlugName, 
							found, 
							status);
						AFTK_CHECK_BEGIN << "Could not get plug." AFTK_CHECK_ERROR_RETURN_END_1;
						if (!found)
						{
							//AFTK_DO_BEGIN << "Could not find source plug for " << connectedPlugArray[connectedPlugs].name().asChar() AFTK_DO_WARNING_END_1;
							continue;
						}
					}

					//Find out where this node is according to the aftk. would be
					//more efficient to use a material global plug mapping system
					std::string thisPlugName;
					if (!StripNodeNameFromPlugName(
						attrPlug.name().asChar(), 
						thisPlugName))
					{
						AFTK_DO_BEGIN << "Could not strip plug name for " << connectedPlugArray[connectedPlugs].name().asChar() AFTK_DO_ERROR_RETURN_END_1;
					}

					bool found;
					Aftk::I_Plug* pThisPlug = 
						GetPlugPtrRecursive(*pAftkShaderNode, 
						thisPlugName,
						found,
						status);
					AFTK_CHECK_BEGIN << "Could not get plug." AFTK_CHECK_ERROR_RETURN_END_1;
					if (!found)
					{
						AFTK_DO_BEGIN << "Could not find plug for " << connectedPlugArray[connectedPlugs].name().asChar() AFTK_DO_WARNING_END;
						continue;
					}
					else
					{
						if (pAftkSourceNode->HasType(Aftk::E_ObjType_Shader))
							//Set plug source connection.
							pThisPlug->SetSourceConnectionPtr(pSourcePlug);
						else
							//HACK: Direct connection to node. God this is ugly.
							pThisPlug->SetSourceConnectionPtr(pAftkSourceNode);
					}
				}
			}
		}
		dgIt.next();
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status	C_MaterialImport::ImportMeshMaterialFull(
	C_Importer* pImporter,
	const MFnDependencyNode& shaderDependencyNode,
	Aftk::C_MaterialNode& aftkMaterialNode,
	Aftk::C_Scene* pAftkScene
)
{
	Aftk::C_Status status;

	Ut::UtVector< Ut::DynamicVector<ConnectionMapping> >	connectionMappings;
	Ut::UtMap<std::string, Aftk::C_Node*>				shaderMapping;

	aftkMaterialNode.SetName(shaderDependencyNode.name().asChar());
	aftkMaterialNode.SetMaterialTypeId(shaderDependencyNode.typeId().id());

	status = CreateShaderTreeNodes(
		pImporter,
		connectionMappings,
		shaderMapping,
		shaderDependencyNode,
		aftkMaterialNode,
		pAftkScene);
	AFTK_CHECK_ERROR_RETURN;

	status = ResolvePlugConnections(
		pImporter,
		shaderDependencyNode,
		aftkMaterialNode,
		pAftkScene,
		shaderMapping);
	AFTK_CHECK_ERROR_RETURN;

	if (aftkMaterialNode.GetNumShaderNodes() == 0)
		//Couldn't process material.
		return Aftk::C_Status::E_Status_Success;

	//Lets create a plug to connect to our first (the main) shader node
	Aftk::C_DynamicPlug* pAftkDynamicPlug = new Aftk::C_DynamicPlug();
	if (!pAftkDynamicPlug)
		return Aftk::C_Status::E_Status_OutOfMemory;
	aftkMaterialNode.AddPlugPtr(pAftkDynamicPlug);

	pAftkDynamicPlug->SetName("mayaSurfaceShader");

	pAftkDynamicPlug->SetSourceConnectionPtr(
		aftkMaterialNode.GetShaderNodePtr(0));

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_MaterialImport::ImportMeshMaterialSimple(
	const MFnDependencyNode& shaderDependencyNode,
	Aftk::C_MaterialNode& aftkMaterialNode,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode)
{
	Aftk::C_Status status;

	Ut::UtMap<std::string, Aftk::C_ShaderNode*>	shaderMapping;

	aftkMaterialNode.SetName(shaderDependencyNode.name().asChar());
	aftkMaterialNode.SetMaterialTypeId(shaderDependencyNode.typeId().id());

	status = CreateShaderTreeNodesSimple(
		shaderDependencyNode,
		aftkMaterialNode,
		pAftkScene,
		pAftkSceneMeshNode);
	AFTK_CHECK_ERROR_RETURN;

	if (aftkMaterialNode.GetNumShaderNodes() == 0)
		//Couldn't process material.
		return Aftk::C_Status::E_Status_Success;

	//Lets create a plug to connect to our first (the main) shader node
	Aftk::C_DynamicPlug* pAftkDynamicPlug = new Aftk::C_DynamicPlug();
	if (!pAftkDynamicPlug)
		return Aftk::C_Status::E_Status_OutOfMemory;
	aftkMaterialNode.AddPlugPtr(pAftkDynamicPlug);
	pAftkDynamicPlug->SetName("mayaSurfaceShader");
	pAftkDynamicPlug->SetSourceConnectionPtr(
		aftkMaterialNode.GetShaderNodePtr(0));

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_MaterialImport::MayaPlugToAftkPlug(
	C_Importer* pImporter,
	const MPlug& mayaPlug,
	Aftk::C_DynamicPlug& aftkPlug)
{
	Aftk::C_Status status;
	status = MayaPlugToAftkDynamicPlug(pImporter, mayaPlug, aftkPlug);
	return Aftk::C_Status::E_Status_Success;
}

