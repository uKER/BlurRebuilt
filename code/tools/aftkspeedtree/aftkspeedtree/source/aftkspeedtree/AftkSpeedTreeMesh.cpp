#include "stdafx.h"
#include <AftkSpeedTree\AftkSpeedTreeMesh.h>
#include <AftkSpeedTree\AftkSpeedTreeData.h>

using namespace AftkSpeedTree;

//#pragma optimize("", off)

bool MeshGenerator::ReadSeedsXML(const Util::C_String& filename)
{
	AbstractDOMParser::ValSchemes valScheme			= AbstractDOMParser::Val_Auto;
    bool doNamespaces								= false;
    bool doSchema									= false;
    bool schemaFullChecking							= false;

	LOGDEBUG( g_Log, "AftkSpeedTree::MeshGenerator: Looking for XML file %s\n", filename.Chars());

    // Initialize the XML4C system
	XMLPlatformUtils::Initialize();
	
    // Instantiate the DOM parser.
    static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);
    DOMBuilder *pParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);

    pParser->setFeature(XMLUni::fgDOMNamespaces, doNamespaces);
    pParser->setFeature(XMLUni::fgXercesSchema, doSchema);
    pParser->setFeature(XMLUni::fgXercesSchemaFullChecking, schemaFullChecking);
    pParser->setFeature(XMLUni::fgDOMValidateIfSchema, true);
   
	// reset document pool
    pParser->resetDocumentPool();
	XERCES_CPP_NAMESPACE::DOMDocument *pDOMDocument = pParser->parseURI(filename.Chars());
   
	if (!pDOMDocument)
	{
		pParser->release();
		XMLPlatformUtils::Terminate();
		return false;
	}

	// parse xml
	ParseSeedsXML((DOMNode*)pDOMDocument->getDocumentElement());

	pParser->release();
	XMLPlatformUtils::Terminate();
	return true;
}

void MeshGenerator::ParseSeedsXML(DOMNode *pNode)
{
    if (pNode && pNode->getNodeType() == DOMNode::ELEMENT_NODE)
	{
		char *pElementName = XMLString::transcode(pNode->getNodeName());
		
		if (pNode->hasAttributes())
		{
			if (_stricmp(pElementName, "Seed") == 0)
			{
				DOMNamedNodeMap *pAttributes = pNode->getAttributes();
				int nSize = pAttributes->getLength();

				for (int i = 0; i < nSize; ++i) 
				{
					DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(i);
					
					char *pName = XMLString::transcode(pAttributeNode->getName());

					if (_stricmp(pName, "Number") == 0)
					{
						m_Seeds.PushBack();
						Utility::GetValue(m_Seeds.Back(), pAttributeNode);
					}

					XMLString::release(&pName);
				}
			}
		}
		XMLString::release(&pElementName);

		for (DOMNode* child = pNode->getFirstChild(); child != 0; child = child->getNextSibling())
           ParseSeedsXML(child);
	}
}



void MeshGenerator::Process(Aftk::C_Scene& aftk)
{
	Aftk::C_SceneNodeIterator aftkIt(&aftk);
	Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneNode*> > sptPlaceholders;

	for (; aftkIt != NULL; aftkIt++)
	{
		const Aftk::C_GenericAttribute* pAttribute = Utility::FindAttribute(*aftkIt, "SPTData");
		if (pAttribute)
			sptPlaceholders.PushBack((Aftk::C_SceneNode*)(&(*aftkIt)));
	}

	for (S32 i=0; i<sptPlaceholders.Size(); ++i)
		ProcessNode(*sptPlaceholders[i]);
}


void MeshGenerator::ProcessNode(Aftk::C_SceneNode &node)
{
	const F32		c_fNearLodFactor = 1.0f;
	const F32		c_fFarLodFactor = 4.0f;
	
	m_pSceneNode = &node;

	// get pathname of SPT file
	if(const Aftk::C_GenericAttribute* attrFilename = Utility::FindAttribute(*m_pSceneNode, "Filename"))
	{
		{
			std::string tmp;
			attrFilename->GetValue(tmp);
			m_sFilename = tmp.c_str();
		}

		U32	nSeed;
		F32 fSizeVariance;

		// get the attributes of this tree node
		const Aftk::C_GenericAttribute* attrSize = Utility::FindAttribute(*m_pSceneNode, "Size");
		attrSize->GetValue(m_fSize);
		
		const Aftk::C_GenericAttribute* attrSizeVariance = Utility::FindAttribute(*m_pSceneNode, "SizeVariance");
		attrSizeVariance->GetValue(fSizeVariance);

		const Aftk::C_GenericAttribute* attrSeed = Utility::FindAttribute(*m_pSceneNode, "Seed");
		attrSeed->GetValue(nSeed);

		// see if this tree has already been created
		S32 nIndex = m_pTreeData->FindTreeMeshInfo(m_sFilename, nSeed, m_fSize);

		// if it has make this node an instance rather than a new mesh
		if(nIndex > -1)
		{
			//NOTE: we may have multiple 'sub-scenes' with the same instance refence in the final merged scene... in this case,
			//it doesn't make sense to remap the base instance from one 'sub-scene' to the base instance to another
			if (AftkUt::Query::IsBaseInstance(&node) && m_pTreeData->m_treeMesh[nIndex].meshNodes.Size())
			{
				//point all instances of this node to the other base instance
				Aftk::C_SceneNode *pSPTNode = m_pTreeData->m_treeMesh[nIndex].meshNodes[0]->GetParentPtr();

				U32 nFail = 0;
				while (nFail < node.GetNumInstancesOfThisNode())
				{
					Aftk::C_SceneNode *pInst = node.GetInstanceOfThisNode(nFail);
					if (pInst->HasType(Aftk::E_ObjType_SceneInstance))
					{
						Aftk::C_SceneInstanceNode *pInstNode = static_cast<Aftk::C_SceneInstanceNode*>(pInst);
						pInstNode->SetSourceNodePtr(pSPTNode);
					}
					else
					{
						//uber cautious...
						//should never get here...how can an instance not be an instance?
						nFail++;
					}
				}
			}
			else
			{
				//what does this do?
				LOGDEBUG( g_Log, "\tMeshGenerator: %s already created, making instance...\n", m_sFilename.Chars());
				SetupMeshFromTreeMeshInfo(&m_pTreeData->m_treeMesh[nIndex]);
			}
		}
		else
		{
			// always make the first mesh be the seed from the maya file
			m_AftkSceneMeshNodes.Clear();
			m_Seeds.Clear();

			// theres always at least one seed
			m_Seeds.PushBack();
			m_Seeds[0] = nSeed;

			// add any extra meshes which are specified in this definition file's XML file
			ReadSeedsXML(m_sFilename + ".xml");

			// generate meshes for each of the seeds assigned to this tree
			for(S32 i = 0; i < m_Seeds.Size(); i++)
			{	
				m_pSpeedTree = new CSpeedTreeRT;
				m_pSpeedTree->SetTextureFlip(true);

				// load tree
				if(m_pSpeedTree->LoadTree(m_sFilename.Chars()))
				{
					LOGDEBUG( g_Log, "\tMeshGenerator: %s Loaded Successfully\n", m_sFilename.Chars());

					// setup tree properties
					{
						// tell speedtree we are doing lighting ourselves
						m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
						m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
						m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);

						// tell speedtree we want per vertex bone and weight for wind method
						m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_GPU);
						m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_GPU);
						m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_GPU);

						// make the leaves rock in the wind
						m_pSpeedTree->SetLeafRockingState(true);
						// specificy the number of unique matrices for screenalignment variations
						m_pSpeedTree->SetNumLeafRockingGroups(1);
						m_pSpeedTree->SetNumWindMatrices(4);

						static F32 afZeroes[6] = { 0.0f };
						m_pSpeedTree->SetWindStrengthAndLeafAngles(0.5f, afZeroes, afZeroes, 6);

						// set the tree size
						m_pSpeedTree->SetTreeSize(m_fSize, fSizeVariance);
					}

					
					// generate the tree based on the seeds
					if(m_pSpeedTree->Compute(NULL, m_Seeds[i]))
					{
						// get the dimensions
						F32 afBoundingBox[6];
						m_pSpeedTree->GetBoundingBox(afBoundingBox);
						memcpy(&m_bounds, afBoundingBox, 6 * sizeof(F32));
				
						// use the height of the tree to set some LOD parameters
						F32 fHeight = afBoundingBox[4] - afBoundingBox[1];
						m_pSpeedTree->SetLodLimits(fHeight * c_fNearLodFactor, fHeight * c_fFarLodFactor);
			
						// extract geometry
						SetupMesh(i);
					}
					else // tree failed to compute
						LOGERROR( g_Log, "\tMeshGenerator: SpeedTreeRT Error: cannot compute tree [%s]\n\n", CSpeedTreeRT::GetCurrentError( ));
					
				}
				else // tree failed to load
				{
					LOGERROR( g_Log, "\tMeshGenerator: SpeedTreeRT Error on %s: %s\n", node.GetPath().c_str(), CSpeedTreeRT::GetCurrentError( ));
				}

				// deallocate speedtree
				delete m_pSpeedTree;
				m_pSpeedTree = NULL;
			}

			// add info about the mesh, and also the instance position
			if(!m_AftkSceneMeshNodes.Empty())
			{
				m_pTreeData->AddTreeMeshInfo(
					m_sFilename, 
					nSeed, 
					m_fSize,
					m_bounds, 
					m_AftkSceneMeshNodes);

				m_pTreeData->AddTreeInstanceInfo(
					node.GetWorldMatrix(), 
					0,
					m_pAftkSceneMeshNode,
					0);
			}
		}
	}
	else // couldn't find SPT file
		LOGERROR( g_Log, "\tMeshGenerator: Couldn't find attribute 'Filename' on node %s\n",node.GetPath().c_str());

	
}

void MeshGenerator::SetupMeshFromTreeMeshInfo(const TreeMeshInfo* pTreeMeshInfo)
{
	// create a new instace and make it point at mesh node which already exists
	Aftk::C_SceneInstanceNode* pAftkSceneInstanceNode = new Aftk::C_SceneInstanceNode(m_pSceneNode->GetScenePtr());

	//inherit layer membership
	for (U32 i=0; i<m_pSceneNode->GetNumLayers(); ++i)
	{
		Aftk::C_LayerNode *pLayerNode = m_pSceneNode->GetLayerPtr(i);
		pLayerNode->AddMember(pAftkSceneInstanceNode);
	}

	m_pSceneNode->AddChildPtr(pAftkSceneInstanceNode);

	pAftkSceneInstanceNode->SetSourceNodePtr(pTreeMeshInfo->meshNodes[rand()%pTreeMeshInfo->meshNodes.Size()]);
	Utility::SetupDefaultSHBlindData(m_pSceneNode, pAftkSceneInstanceNode);

	m_pTreeData->m_nDuplicates += 1;
}

void MeshGenerator::SetupMesh(U32 nNum)
{
	 // create a new aftk scene mesh node to store tree and add to hierarchy
	m_AftkSceneMeshNodes.PushBack(new Aftk::C_SceneMeshNode(m_pSceneNode->GetScenePtr()));
	m_pAftkSceneMeshNode = m_AftkSceneMeshNodes.Back();

	//inherit layer membership
	for (U32 i=0; i<m_pSceneNode->GetNumLayers(); ++i)
	{
		Aftk::C_LayerNode *pLayerNode = m_pSceneNode->GetLayerPtr(i);
		pLayerNode->AddMember(m_pAftkSceneMeshNode);
	}

	Util::C_String	sName;
	{
		std::stringstream	sStream;
		sStream << "SpeedTreeMesh: " << Util::ExtractFilename(m_sFilename).Chars() << " (" << m_Seeds[0] << "," << m_Seeds[nNum] << "," << m_fSize << ")";
		sName = sStream.str().c_str();
	}

	m_pAftkSceneMeshNode->SetName(sName.Chars());
	m_pSceneNode->AddChildPtr(m_pAftkSceneMeshNode);	

	{
		//copy flags from original SPT node
		AftkUt::C_NodeFlagParser parser(*m_pSceneNode);

		//apply flag overrides
		const SpeedTreeSetup& setup = m_pTreeData->GetSetup();
		for (S32 i=0; i<setup.m_flagOverrides.Size(); ++i)
			parser.SetParameterValue(setup.m_flagOverrides[i].name.Chars(), setup.m_flagOverrides[i].value.Chars(), AftkUt::E_ParamType_Unknown);

		// [Jan] this is to make the PVS tool work with the screen-facing leaf sprites
		//now driven through xml setup file
		//parser.SetParameterValue("PVSUseOcclusionQuery", 1);
		parser.StoreFlags(*m_pAftkSceneMeshNode);
	}
		
	m_pAftkSceneMeshNode->AddPolygonCluster();
	m_pAftkSceneMeshNode->AddPolygonCluster();	

	// generate geometry for highest lod level
	m_pSpeedTree->SetLodLevel(1.0f);

	// get a pointer to the geometry
	m_pGeometryCache = new CSpeedTreeRT::SGeometry;
	
	// create branch geometry and get vertex counts
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);
	m_nBranchVertexCount = m_pGeometryCache->m_sBranches.m_nNumVertices;
	
	// create frond geometry and get vertex counts
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);
	m_nFrondVertexCount = m_pGeometryCache->m_sFronds.m_nNumVertices;

	// create leaf geometry
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry);
	// since leaves can be either cards or mesh we need to manually count number of vertices
	m_nLeafVertexCount = 0;
	const CSpeedTreeRT::SGeometry::SLeaf* pLeaves = m_pGeometryCache->m_pLeaves;
	for(S32 i = 0; i < pLeaves->m_nNumLeaves; ++i)
	{
		S32 iLeafCardIndex = pLeaves->m_pLeafCardIndices[i];
		const CSpeedTreeRT::SGeometry::SLeaf::SCard* pCard = &pLeaves->m_pCards[iLeafCardIndex];
		const CSpeedTreeRT::SGeometry::SLeaf::SMesh* pMesh = pCard->m_pMesh;

		// leaf mesh can have an arbitrary number of verts, leaf cards always have 4
		if(pMesh)
		{
			if(pMesh->m_nNumVertices > 1000)
				LOGFATAL( g_Log, "\tMeshGenerator: Bad vertex data from SpeedTree. Definition file may be out of date");
			m_nLeafVertexCount += pMesh->m_nNumVertices;
		}
		else
			m_nLeafVertexCount += 4;
	}

	if(SetupMaterials())
	{
		LOGDEBUG( g_Log, "\tMeshGenerator: Total Vertices: %d\n", m_nBranchVertexCount + m_nFrondVertexCount + m_nLeafVertexCount);
	
		// reserve space in aftk mesh for the total vertex counts
		U32 nTotalVertices = m_nBranchVertexCount + m_nFrondVertexCount + m_nLeafVertexCount;
		m_pAftkSceneMeshNode->SetVertexPositionInitialCapacity(nTotalVertices);
		m_pAftkSceneMeshNode->SetVertexNormalInitialCapacity(nTotalVertices);
		m_pAftkSceneMeshNode->SetTextureCoordinateInitialCapacity(0, nTotalVertices);
		m_pAftkSceneMeshNode->SetTextureCoordinateInitialCapacity(1, nTotalVertices);
		m_pAftkSceneMeshNode->SetTextureCoordinateInitialCapacity(2, nTotalVertices);

		// create leaves first
		U32 nIndex = 0;
		if(m_pTreeData->m_Setup.m_bGenerateLeaves)
			GenerateLeaves(nIndex);
		if(m_pTreeData->m_Setup.m_bGenerateFronds)
			GenerateFronds(nIndex);
		if(m_pTreeData->m_Setup.m_bGenerateBranches)
			GenerateBranches(nIndex);

		// get rid of the degenerate polys etc as SpeedTree has given us a stripped mesh.
		Aftk::C_MeshCalculateTangents::CalculateTangents(
			m_pAftkSceneMeshNode,
			0,
			true,
			true);
		Aftk::C_MeshWeld::Weld(
			Aftk::C_MeshWeldMemoryManager(),
			m_pAftkSceneMeshNode);
		Aftk::C_MeshRemoveDegenerates::RemoveDegenerates(m_pAftkSceneMeshNode);
		m_pAftkSceneMeshNode->Repair();
	}

	delete m_pGeometryCache;
}

void MeshGenerator::GenerateBranches(U32& nVertexIndexOffset)
{
	// get a pointer to the branch vertices and indices
	CSpeedTreeRT::SGeometry::SIndexed* pBranches = &(m_pGeometryCache->m_sBranches);
    
	// find out how many sets of indices (LODs) for this branch data
	S32 nNumLodLevels = m_pSpeedTree->GetNumBranchLodLevels();

	// get a pointer to array of shorts storing various index counts for each unique index buffer
	LOGDEBUG( g_Log, "\tMeshGenerator: Branch Vertices: %d\n", m_nBranchVertexCount);

	// loop through geometry from speed tree and add it to aftk mesh
	for (U32 i = 0; i < m_nBranchVertexCount; ++i)
    {
		Aftk::Position3 aftkVertexPosition;
		Aftk::Normal3 aftkVertexNormal;
		Aftk::Normal3 aftkVertexTangent;
		Aftk::Normal3 aftkVertexBinormal;
		Aftk::TextureCoordinate4 aftkTextureCoordinate0(pBranches->m_pTexCoords[CSpeedTreeRT::TL_DIFFUSE][i * 2], pBranches->m_pTexCoords[CSpeedTreeRT::TL_DIFFUSE][i * 2 + 1], 0.0f, 0.0f);
		Aftk::TextureCoordinate4 aftkTextureCoordinate1(0.0f, 0.0f, 0.0f, 0.0f);
		
		// check to see if tree has shadow coords, add if so
		if(pBranches->m_pTexCoords[CSpeedTreeRT::TL_SHADOW])
		{
			aftkTextureCoordinate1.x = pBranches->m_pTexCoords[CSpeedTreeRT::TL_SHADOW][i * 2];
			aftkTextureCoordinate1.y = pBranches->m_pTexCoords[CSpeedTreeRT::TL_SHADOW][i * 2 + 1];
		}
		
		Aftk::TextureCoordinate4 aftkTextureCoordinate2((F32)pBranches->m_pWindMatrixIndices[1][i], pBranches->m_pWindWeights[1][i], 0.0f, 0.0f);
		Aftk::TextureCoordinate4 aftkTextureCoordinate3((F32)pBranches->m_pWindMatrixIndices[0][i], pBranches->m_pWindWeights[0][i], 0.0f, 0.0f);

        memcpy(&aftkVertexPosition, &(pBranches->m_pCoords[i * 3]), 3 * sizeof(F32));
		memcpy(&aftkVertexNormal, &(pBranches->m_pNormals[i * 3]), 3 * sizeof(F32));
		memcpy(&aftkVertexBinormal, &(pBranches->m_pBinormals[i * 3]), 3 * sizeof(F32));
		memcpy(&aftkVertexTangent, &(pBranches->m_pTangents[i * 3]), 3 * sizeof(F32));

		S32 posIndex = m_pAftkSceneMeshNode->AddVertexPosition(aftkVertexPosition);
		S32 normIndex = m_pAftkSceneMeshNode->AddVertexNormal(aftkVertexNormal);
		//m_pAftkSceneMeshNode->AddVertexTangent(0, aftkVertexTangent);
		//m_pAftkSceneMeshNode->AddVertexBinormal(0, aftkVertexBinormal);

		S32 uv0Index = m_pAftkSceneMeshNode->AddTextureCoordinate(0, aftkTextureCoordinate0);
		S32 uv1Index = m_pAftkSceneMeshNode->AddTextureCoordinate(1, aftkTextureCoordinate1);
		S32 uv2Index = m_pAftkSceneMeshNode->AddTextureCoordinate(2, aftkTextureCoordinate2);

		// add this vertex as an aftk polygon vertex
		S32 pvIndex = m_pAftkSceneMeshNode->AddPolygonVertex();
		Aftk::C_PolygonVertex* pPolygonVertex = &m_pAftkSceneMeshNode->GetPolygonVertex(pvIndex);
		pPolygonVertex->SetPositionIndex(posIndex);
		pPolygonVertex->SetNormalIndex(normIndex);
		//pPolygonVertex->SetTangentIndex(0, index);
		//pPolygonVertex->SetBinormalIndex(0, index);

		pPolygonVertex->SetTextureCoordinateIndex(0, uv0Index);
		pPolygonVertex->SetTextureCoordinateIndex(1, uv1Index);
		pPolygonVertex->SetTextureCoordinateIndex(2, uv2Index);
	}

	for(S32 i = 0; i < pBranches->m_pNumStrips[0]; ++i)
	{
		for(S32 j = 0; j < pBranches->m_pStripLengths[0][i] - 2; j++)
		{
			const S32* pStrip = pBranches->m_pStrips[0][i];
			S32 index = m_pAftkSceneMeshNode->AddPolygon(0, 0, Aftk::C_Polygon());
			Aftk::C_Polygon* pPolygon = &m_pAftkSceneMeshNode->GetPolygon(index);
			
			// depending on whether triangle is odd/even we need to add indices in different order
			if(j%2 == 0)
			{
				pPolygon->AddPolygonVertexIndex(pStrip[j+1] + nVertexIndexOffset);
				pPolygon->AddPolygonVertexIndex(pStrip[j] + nVertexIndexOffset);
				pPolygon->AddPolygonVertexIndex(pStrip[j+2] + nVertexIndexOffset);
			}
			else
			{
				pPolygon->AddPolygonVertexIndex(pStrip[j] + nVertexIndexOffset);
				pPolygon->AddPolygonVertexIndex(pStrip[j+1] + nVertexIndexOffset);
				pPolygon->AddPolygonVertexIndex(pStrip[j+2] + nVertexIndexOffset);
			}
			pPolygon->SetMaterialMappingIndex(m_nBranchMaterialIndex);
			pPolygon->SetBlindDataMappingIndex(m_nBranchPolyflagsIndex);
		}
	}
	
	nVertexIndexOffset += m_nBranchVertexCount;
}

void MeshGenerator::GenerateFronds(U32& nVertexIndexOffset)
{
	// get a pointer to the frond vertices and indices
	CSpeedTreeRT::SGeometry::SIndexed* pFronds = &(m_pGeometryCache->m_sFronds);
    
	// find out how many sets of indices (LODs) for this frond data
	U32 nNumLodLevels = m_pSpeedTree->GetNumFrondLodLevels();
	LOGDEBUG( g_Log, "\tMeshGenerator: Frond Vertices: %d\n", m_nFrondVertexCount);

	for (int iPass=0; iPass<2; iPass++)
	{

		// create vertices
		for (U16 i = 0; i < m_nFrondVertexCount; ++i)
		{
			Aftk::Position3 aftkVertexPosition;
			Aftk::Normal3 aftkVertexNormal;
			Aftk::TextureCoordinate4 aftkTextureCoordinate0(pFronds->m_pTexCoords[0][i * 2], pFronds->m_pTexCoords[0][i * 2 + 1], 0.0f, 0.0f);
			Aftk::TextureCoordinate4 aftkTextureCoordinate1(pFronds->m_pTexCoords[0][i * 2], pFronds->m_pTexCoords[0][i * 2 + 1], 0.0f, 0.0f);
			Aftk::TextureCoordinate4 aftkTextureCoordinate2((F32)pFronds->m_pWindMatrixIndices[1][i], pFronds->m_pWindWeights[1][i], 0.0f, 0.0f);
			Aftk::TextureCoordinate4 aftkTextureCoordinate3((F32)pFronds->m_pWindMatrixIndices[0][i], pFronds->m_pWindWeights[0][i], 0.0f, 0.0f);

			memcpy(&aftkVertexPosition, &(pFronds->m_pCoords[i * 3]), 3 * sizeof(F32));
			memcpy(&aftkVertexNormal, &(pFronds->m_pNormals[i * 3]), 3 * sizeof(F32));

			// on the 2nd pass, flip the normals
			if (iPass == 1)
				aftkVertexNormal = -aftkVertexNormal;

			S32 posIndex = m_pAftkSceneMeshNode->AddVertexPosition(aftkVertexPosition);
			S32 normIndex = m_pAftkSceneMeshNode->AddVertexNormal(aftkVertexNormal);

			S32 uv0Index = m_pAftkSceneMeshNode->AddTextureCoordinate(0, aftkTextureCoordinate0);
			S32 uv1Index = m_pAftkSceneMeshNode->AddTextureCoordinate(1, aftkTextureCoordinate1);
			S32 uv2Index = m_pAftkSceneMeshNode->AddTextureCoordinate(2, aftkTextureCoordinate2);

			// add this vertex as an aftk polygon vertex
			S32 pvIndex = m_pAftkSceneMeshNode->AddPolygonVertex();
			Aftk::C_PolygonVertex* pPolygonVertex = &m_pAftkSceneMeshNode->GetPolygonVertex(pvIndex);
			pPolygonVertex->SetPositionIndex(posIndex);
			pPolygonVertex->SetNormalIndex(normIndex);
			
			pPolygonVertex->SetTextureCoordinateIndex(0, uv0Index);
			pPolygonVertex->SetTextureCoordinateIndex(1, uv1Index);
			pPolygonVertex->SetTextureCoordinateIndex(2, uv2Index);
		}

		// create polygons
		for(S32 i = 0; i < pFronds->m_pNumStrips[0]; ++i)
		{
			// number of polys in a tri strip = total index count - 2
			for(S32 j = 0; j < pFronds->m_pStripLengths[0][i] - 2; j++)
			{
				const S32* pStrip = pFronds->m_pStrips[0][i];
				S32 index = m_pAftkSceneMeshNode->AddPolygon(0, 1, Aftk::C_Polygon());
				Aftk::C_Polygon* pPolygon = &m_pAftkSceneMeshNode->GetPolygon(index);
					
				// depending on whether triangle is odd/even we need to add indices in different order
				// add branch vertex count to total as we are indexing vertices stored after the branch vertices
				if((j%2 == 0) != (iPass == 0))
				{
					pPolygon->AddPolygonVertexIndex(pStrip[j] + nVertexIndexOffset);
					pPolygon->AddPolygonVertexIndex(pStrip[j+1] + nVertexIndexOffset);
					pPolygon->AddPolygonVertexIndex(pStrip[j+2] + nVertexIndexOffset);
				}
				else
				{
					pPolygon->AddPolygonVertexIndex(pStrip[j+1] + nVertexIndexOffset);
					pPolygon->AddPolygonVertexIndex(pStrip[j] + nVertexIndexOffset);
					pPolygon->AddPolygonVertexIndex(pStrip[j+2] + nVertexIndexOffset);
				}
				pPolygon->SetMaterialMappingIndex(m_nFrondMaterialIndex);
				pPolygon->SetBlindDataMappingIndex(m_nFrondPolyflagsIndex);
			}
		}

		nVertexIndexOffset += m_nFrondVertexCount;

	}

}

void MeshGenerator::GenerateLeaves(U32& nVertexIndexOffset)
{
	const short anVertexIndices[6] = { 0, 1, 2, 0, 2, 3 };
	U32 nNumLeafLods = m_pSpeedTree->GetNumLeafLodLevels();

	// only generate first lod for now
	if(nNumLeafLods > 1)
		nNumLeafLods = 1;
	
	srand(12345);

	for (U32 nLod = 0; nLod < nNumLeafLods; ++nLod)
	{
		const CSpeedTreeRT::SGeometry::SLeaf* pLeaves = &m_pGeometryCache->m_pLeaves[nLod];
		S32 nLeafCount = pLeaves->m_nNumLeaves;

		LOGDEBUG( g_Log, "\tMeshGenerator: Leaf Vertices: %d\n", m_nLeafVertexCount);
		
		// run through all leaves in this LOD
		for (S32 nLeaf = 0; nLeaf < pLeaves->m_nNumLeaves; ++nLeaf)
		{
			const CSpeedTreeRT::SGeometry::SLeaf::SCard* pCard = &pLeaves->m_pCards[pLeaves->m_pLeafCardIndices[nLeaf]];
			const CSpeedTreeRT::SGeometry::SLeaf::SMesh* pMesh = pCard->m_pMesh;

			F32 fOffset[4][2];
			{
				F32 fWidth = pCard->m_fWidth / 2.0f;
				F32 fHeight = pCard->m_fHeight / 2.0f;
				fOffset[0][0] = -fWidth;
				fOffset[0][1] = fHeight;

				fOffset[1][0] = fWidth;
				fOffset[1][1] = fHeight;

				fOffset[2][0] = fWidth;
				fOffset[2][1] = -fHeight;

				fOffset[3][0] = -fWidth;
				fOffset[3][1] = -fHeight;
			}
			
			U8 nIntensity = 255;
			U32 nLeafColour = (255<<24) + (nIntensity<<16) + (nIntensity<<8) + nIntensity;

			F32 fLeafScaling = 1.0f;
		
			Aftk::Position3 aftkCentrePosition;
			memcpy(&aftkCentrePosition, &pLeaves->m_pCenterCoords[nLeaf * 3], 3 * sizeof(F32));
			
			const F32* pNormal = &pLeaves->m_pNormals[nLeaf * 12];
			const F32* pBinormal = &pLeaves->m_pBinormals[nLeaf * 12];
			const F32* pTangent = &pLeaves->m_pTangents[nLeaf * 12];

			Aftk::TextureCoordinate4 aftkTextureCoordinate2((F32)pLeaves->m_pWindMatrixIndices[1][nLeaf], (F32)pLeaves->m_pWindWeights[1][nLeaf], 0.0f, 0.0f);
			Aftk::TextureCoordinate4 aftkTextureCoordinate3((F32)pLeaves->m_pWindMatrixIndices[0][nLeaf], (F32)pLeaves->m_pWindWeights[0][nLeaf], 0.0f, 0.0f);

			if(pMesh)
			{
				// this leaf is non aligned mesh
				// setup the vertex attributes

				Aftk::Matrix44 leafOrient;
				UtMaths::SetIdentity(&leafOrient);
				memcpy(&leafOrient[0], &pLeaves->m_pTangents[nLeaf*3], 3*sizeof(F32));
				memcpy(&leafOrient[1], &pLeaves->m_pBinormals[nLeaf*3], 3*sizeof(F32));
				memcpy(&leafOrient[2], &pLeaves->m_pNormals[nLeaf*3], 3*sizeof(F32));

				for (S32 nVertex = 0; nVertex < pMesh->m_nNumVertices; ++nVertex)
                {
					Aftk::Position3 aftkVertexPosition;
					Aftk::Position3 aftkVertexPositionOffset;
					Aftk::Normal3 aftkVertexNormal;
					Aftk::Normal3 aftkVertexBinormal;
					Aftk::Normal3 aftkVertexTangent;
					Aftk::TextureCoordinate4 aftkTextureCoordinate0(pMesh->m_pTexCoords[nVertex * 2], pMesh->m_pTexCoords[nVertex * 2 + 1], 0, 0);//(pLeaf->m_pLeafMapTexCoords[nLeaf][anVertexIndices[unVert] * 2], pLeaf->m_pLeafMapTexCoords[nLeaf][anVertexIndices[unVert] * 2 + 1], 0.0f, 0.0f);
					Aftk::TextureCoordinate4 aftkTextureCoordinate1(0.0f, 0.0f, 0.0f, 0.0f);
					
					memcpy(&aftkVertexNormal, &pMesh->m_pNormals[nVertex * 3], 3 * sizeof(F32));
					memcpy(&aftkVertexPosition, &pMesh->m_pCoords[nVertex * 3], 3 * sizeof(F32));
					
					aftkVertexPosition = Aftk::ApplyMatrix(leafOrient, aftkVertexPosition);

					aftkVertexPosition.x += aftkCentrePosition.x;
					aftkVertexPosition.y += aftkCentrePosition.y;
					aftkVertexPosition.z += aftkCentrePosition.z;
				
					// add the positions and coordinates to the mesh
					S32 posIndex = m_pAftkSceneMeshNode->AddVertexPosition(aftkVertexPosition);
					S32 normIndex = m_pAftkSceneMeshNode->AddVertexNormal(aftkVertexNormal);
					
					S32 uv0Index = m_pAftkSceneMeshNode->AddTextureCoordinate(0, aftkTextureCoordinate0);
					S32 uv1Index = m_pAftkSceneMeshNode->AddTextureCoordinate(1, aftkTextureCoordinate1);
					S32 uv2Index = m_pAftkSceneMeshNode->AddTextureCoordinate(2, aftkTextureCoordinate2);

					// add this vertex as an aftk polygon vertex
					S32 pvIndex = m_pAftkSceneMeshNode->AddPolygonVertex();
					Aftk::C_PolygonVertex* pPolygonVertex = &m_pAftkSceneMeshNode->GetPolygonVertex(pvIndex);
					pPolygonVertex->SetPositionIndex(posIndex);
					pPolygonVertex->SetNormalIndex(normIndex);	
					pPolygonVertex->SetTextureCoordinateIndex(0, uv0Index);
					pPolygonVertex->SetTextureCoordinateIndex(1, uv1Index);
					pPolygonVertex->SetTextureCoordinateIndex(2, uv2Index);									
                }

				// indices are for an indexed triangle list so add accordingly
				const S32* pStrip = pMesh->m_pIndices;
				for(S32 i = 0; i < pMesh->m_nNumIndices; i+=3)
				{	
					S32 index = m_pAftkSceneMeshNode->AddPolygon(0, 1, Aftk::C_Polygon());
					Aftk::C_Polygon* pPolygon = &m_pAftkSceneMeshNode->GetPolygon(index);
					
					pPolygon->AddPolygonVertexIndex(pStrip[i] + nVertexIndexOffset);
					pPolygon->AddPolygonVertexIndex(pStrip[i+1] + nVertexIndexOffset);
					pPolygon->AddPolygonVertexIndex(pStrip[i+2] + nVertexIndexOffset);

					//The 2nd uv (pivot offset) is ignored in the scene converter, and is instead auto-generated
					//as these leaves are not meant to be screen aligned, ensure the 'shader' plug in the material is not "SpeedTree.fx"
					pPolygon->SetMaterialMappingIndex(m_nLeafMeshMaterialIndex);
					pPolygon->SetBlindDataMappingIndex(m_nLeafPolyflagsIndex);


					{
						//add back facing polygon?
						S32 index = m_pAftkSceneMeshNode->AddPolygon(0, 1, Aftk::C_Polygon());
						Aftk::C_Polygon* pBackPolygon = &m_pAftkSceneMeshNode->GetPolygon(index);
						
						pBackPolygon ->AddPolygonVertexIndex(pStrip[i] + nVertexIndexOffset);
						pBackPolygon ->AddPolygonVertexIndex(pStrip[i+2] + nVertexIndexOffset);
						pBackPolygon ->AddPolygonVertexIndex(pStrip[i+1] + nVertexIndexOffset);

						//The 2nd uv (pivot offset) is ignored in the scene converter, and is instead auto-generated
						//as these leaves are not meant to be screen aligned, ensure the 'shader' plug in the material is not "SpeedTree.fx"
						pBackPolygon ->SetMaterialMappingIndex(m_nLeafMeshMaterialIndex);
						pBackPolygon ->SetBlindDataMappingIndex(m_nLeafPolyflagsIndex);
					}

				}
				
				nVertexIndexOffset += pMesh->m_nNumVertices;
			}
			else
			{
				// this leaf is an aligned leaf card

				// seeing as we are doing unique screen align, may aswell make each leaf uniquely rotated
				F32 fAngle = -Maths::Pi * 0.1f * (((F32)(rand() - RAND_MAX / 2))/ (RAND_MAX / 2));
				
				// add vertices
				for(S32 nCorner = 0; nCorner < 4; ++nCorner)
				{
					Aftk::Position3 aftkVertexPosition = aftkCentrePosition;
					Aftk::Normal3 aftkVertexNormal;
					Aftk::Normal3 aftkVertexBinormal;
					Aftk::Normal3 aftkVertexTangent;
					Aftk::TextureCoordinate4 aftkTextureCoordinate0(0, 0, 0, 0);//(pLeaf->m_pLeafMapTexCoords[nLeaf][anVertexIndices[unVert] * 2], pLeaf->m_pLeafMapTexCoords[nLeaf][anVertexIndices[unVert] * 2 + 1], 0.0f, 0.0f);
					Aftk::TextureCoordinate4 aftkTextureCoordinate1(0, 0, 0, 0);//(pLeaf->m_pLeafMapTexCoords[nLeaf][anVertexIndices[unVert] * 2], pLeaf->m_pLeafMapTexCoords[nLeaf][anVertexIndices[unVert] * 2 + 1], 0.0f, 0.0f);
						
					// copy texture coords from card structure
					memcpy(&aftkTextureCoordinate0, &(pCard->m_pTexCoords[nCorner * 2]), 2 * sizeof(F32));
					memcpy(&aftkVertexNormal, &pNormal[nCorner * 3], 3 * sizeof(F32));
					memcpy(&aftkVertexBinormal, &pBinormal[nCorner * 3], 3 * sizeof(F32));
					memcpy(&aftkVertexTangent, &pTangent[nCorner * 3], 3 * sizeof(F32));

					// add the appropriate displacements to the vertex position and screen aligning texture coords
					{
						F32 fCos = cosf(fAngle);
						F32 fSin = sinf(fAngle);

						F32 x = fCos * fOffset[nCorner][0] - fSin * fOffset[nCorner][1];
						F32 y = fSin * fOffset[nCorner][0] + fCos * fOffset[nCorner][1];

						// we need to add manual offsets to the vertex positions otherwise they get ditched as degenerate
						aftkVertexPosition.x += x * fLeafScaling;
						aftkVertexPosition.y += y * fLeafScaling;

						aftkTextureCoordinate1.x = x * fLeafScaling;
						aftkTextureCoordinate1.y = y * fLeafScaling;
					}

					// add the positions and coordinates to the mesh
					S32 posIndex = m_pAftkSceneMeshNode->AddVertexPosition(aftkVertexPosition);
					S32 normIndex = m_pAftkSceneMeshNode->AddVertexNormal(aftkVertexNormal);
					
					S32 uv0Index = m_pAftkSceneMeshNode->AddTextureCoordinate(0, aftkTextureCoordinate0);
					S32 uv1Index = m_pAftkSceneMeshNode->AddTextureCoordinate(1, aftkTextureCoordinate1);
					S32 uv2Index = m_pAftkSceneMeshNode->AddTextureCoordinate(2, aftkTextureCoordinate2);

					// add this vertex as an aftk polygon vertex
					S32 pvIndex = m_pAftkSceneMeshNode->AddPolygonVertex();
					Aftk::C_PolygonVertex* pPolygonVertex = &m_pAftkSceneMeshNode->GetPolygonVertex(pvIndex);
					pPolygonVertex->SetPositionIndex(posIndex);
					pPolygonVertex->SetNormalIndex(normIndex);	
					pPolygonVertex->SetTextureCoordinateIndex(0, uv0Index);
					pPolygonVertex->SetTextureCoordinateIndex(1, uv1Index);
					pPolygonVertex->SetTextureCoordinateIndex(2, uv2Index);									
				}

				// add indices
				Aftk::C_Polygon* pPolygon;
				S32 index, index1;
				
				// add first leaf poly
				index1 = index = m_pAftkSceneMeshNode->AddPolygon(0, 2, Aftk::C_Polygon());
				pPolygon = &m_pAftkSceneMeshNode->GetPolygon(index);
				pPolygon->AddPolygonVertexIndex(nVertexIndexOffset);
				pPolygon->AddPolygonVertexIndex(nVertexIndexOffset + 2);
				pPolygon->AddPolygonVertexIndex(nVertexIndexOffset + 1);
				pPolygon->SetMaterialMappingIndex(m_nLeafMaterialIndex);
				pPolygon->SetBlindDataMappingIndex(m_nLeafPolyflagsIndex);
				
				// add second leaf poly
				index = m_pAftkSceneMeshNode->AddPolygon(0, 2, Aftk::C_Polygon());
				pPolygon = &m_pAftkSceneMeshNode->GetPolygon(index);
				pPolygon->AddPolygonVertexIndex(nVertexIndexOffset);
				pPolygon->AddPolygonVertexIndex(nVertexIndexOffset + 3);
				pPolygon->AddPolygonVertexIndex(nVertexIndexOffset + 2);
				pPolygon->SetMaterialMappingIndex(m_nLeafMaterialIndex);
				pPolygon->SetBlindDataMappingIndex(m_nLeafPolyflagsIndex);
				pPolygon->SetBasePolygonIndex(index1);

				// leaf cards are 4 vertices, add to vertex index offset
				nVertexIndexOffset += 4;
			}
            
		}
	}
}

bool MeshGenerator::SetupMaterials()
{
	// load textures
	CSpeedTreeRT::SMapBank mapBank;
	m_pSpeedTree->GetMapBank(mapBank);

	LOGDEBUG( g_Log, "\tMeshGenerator: Branch Filename: %s\n", mapBank.m_pBranchMaps[CSpeedTreeRT::TL_DIFFUSE]);
	for(U32 a = 0; a < mapBank.m_uiNumFrondMaps; a++)
	{
		LOGDEBUG( g_Log, "\tMeshGenerator: Frond Filename: %s\n", mapBank.m_pFrondMaps[CSpeedTreeRT::TL_DIFFUSE][a]);
	}
	for(U32 a = 0; a < mapBank.m_uiNumLeafMaps; a++)
	{
		LOGDEBUG( g_Log, "\tMeshGenerator: Leaf Filename: %s\n", mapBank.m_pLeafMaps[CSpeedTreeRT::TL_DIFFUSE][a]);
	}
	
	if(*mapBank.m_pCompositeMaps[CSpeedTreeRT::TL_DIFFUSE] == '\0')
	{
		LOGWARN( g_Log, "\tMeshGenerator: WARNING: No composite texture defined for %s\n", Util::ExtractFilename(m_sFilename).Chars());
		//return false;
	}

	LOGDEBUG( g_Log, "\tMeshGenerator: Composite Filename: %s\n", mapBank.m_pCompositeMaps[CSpeedTreeRT::TL_DIFFUSE]);

	// get diffuse textures
	Util::C_String folder = Util::ExtractDirectory(m_sFilename);
	Util::C_String strBranchFilename = Util::C_String(mapBank.m_pBranchMaps[CSpeedTreeRT::TL_DIFFUSE]);
	if (strBranchFilename.Length() > 0 && Util::IsRelativePath(strBranchFilename))
		strBranchFilename = folder + strBranchFilename;
	Util::C_String strCompositeFilename = Util::C_String(mapBank.m_pCompositeMaps[CSpeedTreeRT::TL_DIFFUSE]);
	if (strCompositeFilename.Length() > 0 && Util::IsRelativePath(strCompositeFilename))
		strCompositeFilename = folder + strCompositeFilename;

	// replace any double slashes in the filename with single slashes
	strBranchFilename = strBranchFilename.Replace("\\\\","\\");
	strCompositeFilename = strCompositeFilename.Replace("\\\\","\\");

	// get normal map textures
	Util::C_String strBranchNormFilename = Util::C_String(mapBank.m_pBranchMaps[CSpeedTreeRT::TL_NORMAL]);
	if (strBranchNormFilename.Length() > 0 && Util::IsRelativePath(strBranchNormFilename))
		strBranchNormFilename = folder + strBranchNormFilename;
	Util::C_String strCompositeNormFilename = Util::C_String(mapBank.m_pCompositeMaps[CSpeedTreeRT::TL_NORMAL]);
	if (strCompositeNormFilename.Length() > 0 && Util::IsRelativePath(strCompositeNormFilename))
		strCompositeNormFilename = folder + strCompositeNormFilename;
	// replace any double slashes in the filename with single slashes
	strBranchNormFilename = strBranchNormFilename.Replace("\\\\","\\");
	strCompositeNormFilename = strCompositeNormFilename.Replace("\\\\","\\");

	m_nBranchMaterialIndex = m_nFrondMaterialIndex = m_nLeafMaterialIndex = m_nLeafMeshMaterialIndex = -1;
	if(m_nBranchVertexCount)
	{
		const Aftk::C_MaterialResource*		pBranchMaterialResource =Utility::CreateMaterial(m_pSceneNode->GetScenePtr(), strBranchFilename, strBranchNormFilename, "SpeedTree.fx");
		assert(pBranchMaterialResource != NULL);
		m_nBranchMaterialIndex = m_pAftkSceneMeshNode->AddPolygonMaterialMapping(pBranchMaterialResource);
		assert(m_nBranchMaterialIndex != -1);
	}

	if(m_nFrondVertexCount)
	{
		const Aftk::C_MaterialResource*		pFrondMaterialResource = Utility::CreateMaterial(m_pSceneNode->GetScenePtr(), strCompositeFilename, strCompositeNormFilename, "SpeedTree.fx");
		assert(pFrondMaterialResource != NULL);
		m_nFrondMaterialIndex = m_pAftkSceneMeshNode->AddPolygonMaterialMapping(pFrondMaterialResource);
		assert(m_nFrondMaterialIndex != -1);
	}

	if(m_nLeafVertexCount)
	{
		const Aftk::C_MaterialResource*		pLeafMaterialResource = Utility::CreateMaterial(m_pSceneNode->GetScenePtr(), strCompositeFilename, strCompositeNormFilename, "SpeedTreeLeaf.fx");
		assert(pLeafMaterialResource != NULL);
		m_nLeafMaterialIndex = m_pAftkSceneMeshNode->AddPolygonMaterialMapping(pLeafMaterialResource);
		assert(m_nLeafMaterialIndex != -1);

		const Aftk::C_MaterialResource*		pLeafMeshMaterialResource = Utility::CreateMaterial(m_pSceneNode->GetScenePtr(), strCompositeFilename, strCompositeNormFilename, "SpeedTree.fx");
		assert(pLeafMeshMaterialResource != NULL);
		m_nLeafMeshMaterialIndex = m_pAftkSceneMeshNode->AddPolygonMaterialMapping(pLeafMeshMaterialResource);
		assert(m_nLeafMeshMaterialIndex != -1);
	}

	// flags for do not cast shadows
	Aftk::C_GenericAttribute attr;

	//Branch blind data is the basis for the blind data for our leaves and fronds.
	Aftk::C_BlindDataNode branchBlindData;

	{
		attr.SetName("polySet_doNotLight");
		branchBlindData.AddAttribute(attr);
	}

	m_nBranchPolyflagsIndex = m_pAftkSceneMeshNode->AddPolygonBlindDataMapping(
		m_pSceneNode->GetScenePtr()->AddBlindDataNode(branchBlindData));
	assert(m_nBranchPolyflagsIndex != -1);

	//Leaf blind data.
	{
		Aftk::C_BlindDataNode leafBlindData(branchBlindData);
		attr.SetName("polySet_doubleSidedForPVS");
		branchBlindData.AddAttribute(attr);

		m_nLeafPolyflagsIndex = m_pAftkSceneMeshNode->AddPolygonBlindDataMapping(
			m_pSceneNode->GetScenePtr()->AddBlindDataNode(leafBlindData));
		assert(m_nLeafPolyflagsIndex != -1);
	}

	//Frond blind data.
	{
		Aftk::C_BlindDataNode frondBlindData(branchBlindData);
		// fronds shouldnt generally cast shadows
		{
			attr.SetName("polySet_doNotCastShadow");
			frondBlindData.AddAttribute(attr);
		}

		if(m_pTreeData->m_Setup.m_bDoubleSidedFronds)
		{
			attr.SetName("polySet_doubleSided");
			frondBlindData.AddAttribute(attr);
		}
		else
		{
			attr.SetName("polySet_doubleSidedForPVS");
			frondBlindData.AddAttribute(attr);
		}
		
		m_nFrondPolyflagsIndex = m_pAftkSceneMeshNode->AddPolygonBlindDataMapping(
			m_pSceneNode->GetScenePtr()->AddBlindDataNode(frondBlindData));
		assert(m_nFrondPolyflagsIndex != -1);
	}

	return true;
}

