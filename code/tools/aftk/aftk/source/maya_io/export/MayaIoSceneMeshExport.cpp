#include "stdafx.h"



using namespace MayaIo;

MObject C_SceneMeshExport::m_initialShadingGroupObject;

Aftk::C_Status C_SceneMeshExport::Export(
	bool create,
	C_Exporter* pExporter,
	MObject& parentTransformObject,
	MObject& mayaNewTransformObject,
	MObject& mayaNewShapeObject,
	const Aftk::C_Scene* pAftkScene,
	const Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;
	MStatus mStatus;

	status = C_SceneNodeExport::Export(
		true,
		pExporter,
		parentTransformObject,
		mayaNewTransformObject,
		mayaNewShapeObject,
		pAftkScene,
		pAftkSceneNode);
	AFTK_CHECK_ERROR_RETURN;

	//We must create this node
	AFTK_ASSERT(create);

	if (m_initialShadingGroupObject.isNull())
	{
		MSelectionList dummy;
		dummy.add("initialShadingGroup");
		dummy.getDependNode(0, m_initialShadingGroupObject);
		AFTK_CHECK_BEGIN << "Could not get initial shading group object via selection list." AFTK_CHECK_ERROR_RETURN_END;
	}

	const Aftk::C_SceneMeshNode* pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*)pAftkSceneNode;

	//Copy vertex and poly information
	U32 numVertices = pAftkSceneMeshNode->GetNumVertexPositions();
	U32 numPolygons = pAftkSceneMeshNode->GetNumPolygons(0);
	U32 numPolygonVertices = pAftkSceneMeshNode->GetNumPolygonVertices();

	if ((numPolygons == 0)
		||
		(numVertices == 0))
	{
		return Aftk::C_Status::E_Status_Success;
	}

	//Copy vertex positional information
	MPointArray pointArray;
	pointArray.setLength(numVertices);

	for (U32 i = 0; i < numVertices; i++)
	{
		const Aftk::Position3* pAftkPos = &pAftkSceneMeshNode->GetVertexPosition(i);
		MPoint p((*pAftkPos)[0], (*pAftkPos)[1], (*pAftkPos)[2]);
		mStatus = pointArray.set(p, i);
		MCHECK_BEGIN << "Failed to copy vertex position" MCHECK_ERROR_RETURN_END_1;
	}

	//Copy polygon count information
	MIntArray polyCountInfo;
	polyCountInfo.setLength(numPolygons);

	for (U32 i = 0; i < numPolygons; i++)
	{
		const Aftk::C_Polygon* pAftkPoly = &pAftkSceneMeshNode->GetPolygon(0,i);
		U32 polyVertCount = pAftkPoly->GetNumPolygonVertices();
		mStatus = polyCountInfo.set(polyVertCount, i);
		MCHECK_BEGIN << "Failed to copy polygon count info" MCHECK_ERROR_RETURN_END_1;
	}

	//Copy polygon connectivity information - a concatinated array of polygon vertex indices
	MIntArray polyConnectInfo;
	polyConnectInfo.setLength(numPolygonVertices);
	U32 polyConnectInfoIterator = 0;

	for (U32 i = 0; i < numPolygons; i++)
	{
		const Aftk::C_Polygon* pAftkPoly = &pAftkSceneMeshNode->GetPolygon(0,i);
		#ifdef FLIP_NORMALS
		for (U32 j = 0; j < pAftkPoly->GetNumPolygonVertices(); j++)
		#else
		for (S32 j = pAftkPoly->GetNumPolygonVertices()-1; j > -1; j--)
		#endif
		{
			const Aftk::C_PolygonVertex* pAftkPolyVert = &pAftkPoly->GetPolygonVertex(j);
			S32 positionIndex = pAftkPolyVert->GetPositionIndex();
			if (positionIndex < 0)
			{
#ifdef _DEBUG
				AFTK_ASSERT(positionIndex > -1);
#endif
				AFTK_DO_BEGIN << "Invalid position index in mesh [" + pAftkSceneMeshNode->GetName() + "]." AFTK_DO_ERROR_RETURN_END;
			}
			if (polyConnectInfoIterator >= polyConnectInfo.length())
			{
#ifdef _DEBUG
				AFTK_ASSERT(polyConnectInfoIterator < polyConnectInfo.length());
#endif
				AFTK_DO_BEGIN << "polyConnectInfo iterator invalid in mesh [" + pAftkSceneMeshNode->GetName() + "]. This was probably caused by shared polygon vertices. Expand polygon vertices prior to Maya conversion." AFTK_DO_ERROR_RETURN_END;
			}

			mStatus = polyConnectInfo.set(positionIndex, polyConnectInfoIterator);
			MCHECK_BEGIN << "Failed to copy polygon connection info" MCHECK_ERROR_RETURN_END_1;
			polyConnectInfoIterator++;
		}
	}

	//These must be the same!
	AFTK_ASSERT(numPolygonVertices == polyConnectInfoIterator);

	MFnMesh meshFn;

	mayaNewShapeObject = meshFn.create( 
		numVertices, 
		numPolygons, 
		pointArray, 
		polyCountInfo,
		polyConnectInfo,
		mayaNewTransformObject,
		&mStatus);
	MCHECK_BEGIN << "Failed to create mesh node" MCHECK_ERROR_RETURN_END_1;
	{
		U32 numMayaPolygons = meshFn.numPolygons();
		if (numPolygons != numMayaPolygons)
		{
#ifdef _DEBUG
			AFTK_ASSERT(numPolygons == numMayaPolygons);
#endif
			AFTK_DO_BEGIN << "Failed to create mesh for [" << pAftkSceneNode->GetName() << "]. Unexpected poly count. Try running repair function prior to conversion." AFTK_DO_ERROR_RETURN_END;
		}
	}
	
	
	//Now execute code common to the cases of having an existing mesh to fill (todo), or having 
	//just created a new mesh node.

	MIntArray mayaFaceIndexList;
	mayaFaceIndexList.setLength(numPolygonVertices);

	MIntArray mayaVertexIndexList;
	mayaVertexIndexList.setLength(numPolygonVertices);

	MVectorArray mayaVertexNormals;
	mayaVertexNormals.setLength(numPolygonVertices);

	MColorArray mayaVertexColours;
	mayaVertexColours.setLength(numPolygonVertices);

	//Get the vertex data channel for the first set of vertex colours.
	S32 vertexColourChannel = -1;
	Aftk::E_GenericAttributeType vertexColourType = Aftk::E_GenericAttributeType_Unknown;

	for (U32 i = 0; i < pAftkSceneMeshNode->GetNumVertexDataChannels(); i++)
	{
		if (pAftkSceneMeshNode->GetVertexDataChannelInfo(i).GetName() == 
			AFTK_DATA_CHANNEL_USAGE_VERTEX_COLOUR)
		{
			vertexColourChannel = i;
			vertexColourType = pAftkSceneMeshNode->GetVertexDataChannelInfo(i).GetDataType();

			if (
				(vertexColourType != Aftk::E_GenericAttributeType_U32) &&
				(vertexColourType != Aftk::E_GenericAttributeType_Vector4F32)
				)
			{
				AFTK_DO_BEGIN << "Unknown vertex data type for mesh [" << pAftkSceneMeshNode->GetName() << "]." AFTK_DO_WARNING_END;
			}
			break;
		}
	}
	U32 vertexIndexIterator = 0;
	for (U32 i = 0; i < numPolygons; i++)
	{
		const Aftk::C_Polygon* pAftkPoly = &pAftkSceneMeshNode->GetPolygon(0,i);

		#ifdef FLIP_NORMALS
		for (U32 j = 0; j < pAftkPoly->GetNumPolygonVertices(); j++)
		#else
		for (S32 j = pAftkPoly->GetNumPolygonVertices()-1; j > -1; j--)
		#endif
		{
			const Aftk::C_PolygonVertex* pAftkPolyVert = &pAftkPoly->GetPolygonVertex(j);

			S32 vertexIndex = pAftkPolyVert->GetPositionIndex();
			AFTK_ASSERT(vertexIndex > -1);

			//Copy normals
			S32 normalIndex = pAftkPolyVert->GetNormalIndex();
			if (normalIndex > -1)
			{
				const Aftk::Normal3& n = pAftkSceneMeshNode->GetVertexNormal(normalIndex);
				if (IsNumberValid(n[0]) && IsNumberValid(n[1]) && IsNumberValid(n[2]))
				{
					MVector mayaNormal(n[0], n[1], n[2]);
					mStatus = mayaVertexNormals.set(mayaNormal, vertexIndexIterator);
				}
				else
				{
					MVector mayaNormal(0,0,0);
					mStatus = mayaVertexNormals.set(mayaNormal, vertexIndexIterator);
				}
			}
			else
			{
 				MVector mayaNormal(0,0,0);
				mStatus = mayaVertexNormals.set(mayaNormal, vertexIndexIterator);
			}
			MCHECK_BEGIN << "Could not set vertex normal" MCHECK_ERROR_RETURN_END_1;

			//Copy colours
			if (vertexColourChannel > -1 && vertexColourType == Aftk::E_GenericAttributeType_U32)
			{
				U32 d;
				pAftkPolyVert->GetData(d, vertexColourChannel);
				Aftk::Vector4F32 colour4F32 = Aftk::ColourRGBAFromU32(d);
				MColor mayaColour(colour4F32[0], colour4F32[1], colour4F32[2], colour4F32[3]);
				mStatus = mayaVertexColours.set(mayaColour, vertexIndexIterator);
				MCHECK_BEGIN << "Could not set vertex colour" MCHECK_ERROR_RETURN_END_1;
			}
			else if (vertexColourChannel > -1 && vertexColourType == Aftk::E_GenericAttributeType_Vector4F32)
			{
				Aftk::Vector4F32 d;
				pAftkPolyVert->GetData(d, vertexColourChannel);
				MColor mayaColour(d[0], d[1], d[2], d[3]);
				mStatus = mayaVertexColours.set(mayaColour, vertexIndexIterator);
				MCHECK_BEGIN << "Could not set vertex colour" MCHECK_ERROR_RETURN_END_1;
			}

			mayaFaceIndexList.set(i, vertexIndexIterator);
			mayaVertexIndexList.set((U32)vertexIndex, vertexIndexIterator);
			vertexIndexIterator++;
		}
	}

	//Apply vertex normals
	mStatus = meshFn.setFaceVertexNormals(
		mayaVertexNormals, 
		mayaFaceIndexList, 
		mayaVertexIndexList, 
		MSpace::kObject);
	MCHECK_BEGIN << "Could not assign vertex normals" MCHECK_ERROR_RETURN_END_1;

	//Apply vertex colours
	mStatus = meshFn.setFaceVertexColors(
		mayaVertexColours, 
		mayaFaceIndexList, 
		mayaVertexIndexList);
	MCHECK_BEGIN << "Could not assign vertex colours" MCHECK_ERROR_RETURN_END_1;

	//Copy UV sets over
	for (U32 set = 0; set < pAftkSceneMeshNode->GetNumTextureCoordinateSets(); set++)
	{
		MString uvSetName;
		if (set > 0)
		{
			uvSetName = pAftkSceneMeshNode->GetTextureCoordinateSetName(set).c_str();
		}
		else
		{
			mStatus = meshFn.getCurrentUVSetName(uvSetName);
			MCHECK_BEGIN << "Could not acquire current uv set (set 0)." MCHECK_ERROR_RETURN_END_1;
		}

		if (set > 0)
		{
			mStatus = 
				meshFn.createUVSet(uvSetName, 0);
			MCHECK_BEGIN << "Could not create UV set" MCHECK_ERROR_RETURN_END_1;
		}
		else
		{
			mStatus = meshFn.clearUVs(&uvSetName);
			MCHECK_BEGIN << "Clear UV set failed" MCHECK_ERROR_RETURN_END_1;
		}

		//Copy UV data into this UV set
		U32 numUVs = pAftkSceneMeshNode->GetNumTextureCoordinates(set);
		if (numUVs == 0)
		{
			continue;
		}
		MFloatArray uArray;
		MFloatArray vArray;
		mStatus = uArray.setLength(numUVs);
		MCHECK_BEGIN << "Could not set u array length" MCHECK_ERROR_RETURN_END_1;
		mStatus = vArray.setLength(numUVs);
		MCHECK_BEGIN << "Could not set v array length" MCHECK_ERROR_RETURN_END_1;


		for (U32 j = 0; j < numUVs; j++)
		{
			const Aftk::TextureCoordinate4* pUV = &pAftkSceneMeshNode->GetTextureCoordinate(set, j);
			mStatus = uArray.set((*pUV)[0], j);
			MCHECK_BEGIN << "Could not assign u coordinate" MCHECK_ERROR_RETURN_END_1;
			mStatus = vArray.set(1-(*pUV)[1], j);
			MCHECK_BEGIN << "Could not assign v coordinate" MCHECK_ERROR_RETURN_END_1;
		}

		mStatus = meshFn.setUVs(uArray, vArray, &uvSetName);
		MCHECK_BEGIN << "Could not assign uv data" MCHECK_ERROR_RETURN_END_1;

		//Assign UV coordinates to polys

		MIntArray mayaUVCountList;
		mStatus = mayaUVCountList.setLength(numPolygons);
		MCHECK_BEGIN << "Could not set UV count" MCHECK_ERROR_RETURN_END_1;

		MIntArray mayaUVIndexList;

		U32 totalCount = 0;
		for (U32 i = 0; i < numPolygons; i++)
		{
			const Aftk::C_Polygon* pAftkPoly = &pAftkSceneMeshNode->GetPolygon(0,i);

			U32 uvCount = 0;

			#ifdef FLIP_NORMALS
			for (U32 j = 0; j < pAftkPoly->GetNumPolygonVertices(); j++)
			#else
			for (S32 j = pAftkPoly->GetNumPolygonVertices()-1; j > -1; j--)
			#endif
			{
				const Aftk::C_PolygonVertex* pAftkPolyVert = &pAftkPoly->GetPolygonVertex(j);

				if (pAftkPolyVert->GetNumTextureCoordinateIndices() > set)
				{
					//Yes this poly vert has a UV coordinate for the current set.
					uvCount++;
				
					U32 uvIndex = pAftkPolyVert->GetTextureCoordinateIndex(set);
					mStatus = mayaUVIndexList.append(uvIndex);
					MCHECK_BEGIN << "Could not append UV index" MCHECK_ERROR_RETURN_END_1
				}
			}
			totalCount += uvCount;
			U32 test = mayaVertexIndexList.length();

			mStatus = mayaUVCountList.set(uvCount, i);
			MCHECK_BEGIN << "Could not set UV count" MCHECK_ERROR_RETURN_END_1;
		}

		AFTK_ASSERT(totalCount == mayaUVIndexList.length());

		mStatus = meshFn.assignUVs(
			mayaUVCountList,
			mayaUVIndexList,
			&uvSetName);
		MCHECK_BEGIN << "Could not perform UVs->poly vert assignment" MCHECK_ERROR_RETURN_END_1;

	}

	MDagPath transformPath;
	mStatus = meshFn.getPath(transformPath);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_PATH_FAIL MCHECK_ERROR_RETURN_END_1;
	status = AssignMaterials(
		pExporter,
		transformPath,
		pAftkScene, 
		pAftkSceneMeshNode);
	AFTK_CHECK_ERROR_RETURN;

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneMeshExport::FindShadingGroup(MObject& material, MObject* pShadingGroup)
{
	Aftk::C_Status status;
	MStatus mStatus;
	MFnDependencyNode mayaMaterialNode(material);

	MPlug outColorPlug = mayaMaterialNode.findPlug("outColor", &mStatus);
	MCHECK_ERROR_RETURN_1;

	MPlugArray connectedPlugArray;
	outColorPlug.connectedTo(connectedPlugArray, false, true, &mStatus);
	MCHECK_ERROR_RETURN_1;
	
	for (U32 i = 0; i < connectedPlugArray.length(); i++)
	{
		MString partialName = connectedPlugArray[i].partialName(
			false, false, false, false, false, true, &mStatus);
		MCHECK_ERROR_RETURN_1;
		if (partialName == MString("surfaceShader"))
		{
			MFnDependencyNode parentNode(connectedPlugArray[i].node());
			MString parentName = parentNode.name().asChar();
			*pShadingGroup = parentNode.object();
			return Aftk::C_Status::E_Status_Success;
		}
	}
    
	AFTK_DO_BEGIN << "Could not find shading group." AFTK_DO_ERROR_RETURN_END_1;
}

Aftk::C_Status C_SceneMeshExport::AssignMaterials(
	C_Exporter* pExporter,
	MDagPath& mayaShapeDagPath,
	const Aftk::C_Scene* pAftkScene,
	const Aftk::C_SceneMeshNode* pAftkSceneMeshNode)
{
	Aftk::C_Status status;
	MStatus mStatus;
	MFnSet setFn;

	U32 numMaterials = pAftkScene->GetNumMaterialNodes();

	//Okay, what we need to do now is to build lists of the faces on this mesh that use a particular material.
	//A list is produced for every material in the scene. TODO: we could perhaps do with an AFTK function to
	//return the indices of the materials used by a given mesh. This would make the following operation a bit
	//more efficient as our batch array would never be sparse.

	//array of poly indices per material
	Ut::UtVector< Ut::DynamicVector<U32> >* pMaterialBatches;
	//dimension array of poly indices to the number of materials in the scene.
	pMaterialBatches = new Ut::UtVector< Ut::DynamicVector<U32> >[numMaterials];
	if (!pMaterialBatches)
	{
		#ifdef _DEBUG
		AFTK_ASSERT(pMaterialBatches);
		#endif
		return Aftk::C_Status::E_Status_OutOfMemory;
	}

	U32 numPolys = pAftkSceneMeshNode->GetNumPolygons(0);
   	U32* pMaterialUsageCount = new U32[numMaterials];
	{
		memset(pMaterialUsageCount, 0, sizeof(U32)*numMaterials);

		for (U32 i = 0; i < numPolys; i++)
		{
			const Aftk::C_MaterialResource* pMaterialResource = 
				pAftkSceneMeshNode->GetPolygonMaterialMappingPtr(
					pAftkSceneMeshNode->GetPolygon(0,i).GetMaterialMappingIndex());
			if (pMaterialResource)
			{
				//Increase the poly count for this material
				pMaterialUsageCount[pMaterialResource->GetIndex()]++;
			}
		}
		//Reserve memory...
		for (U32 i = 0; i < numMaterials; i++)
		{
			pMaterialBatches[i].Reserve(pMaterialUsageCount[i]);
		}
	}

	//Build our batches.
	MIntArray faceIndicesWithNoMaterial;
	for (U32 i = 0; i < numPolys; i++)
	{
		const Aftk::C_Polygon* pAftkPolygon = &pAftkSceneMeshNode->GetPolygon(0,i);
		const Aftk::C_MaterialResource* pMaterialResource = 
				pAftkSceneMeshNode->GetPolygonMaterialMappingPtr(
					pAftkPolygon->GetMaterialMappingIndex());
		
		if (!pMaterialResource)
		{
			//Skip null materials
			faceIndicesWithNoMaterial.append(i);
			continue;
		}
		//Add the index of this polygon to our array of poly indices for the current material index.
		pMaterialBatches[pMaterialResource->GetIndex()].PushBack(i);
	}

	for (U32 i = 0; i < numMaterials; i++)
	{
		if (pMaterialBatches[i].Size() > 0)
		{
			const Aftk::C_MaterialNode* pAftkMaterialNode = &pAftkScene->GetMaterialResourcePtr(i)->GetContent();
			MObject sgObject = pExporter->GetMayaMaterialShadingGroup(i);
			MFnDependencyNode materialSgFn(sgObject);

			MFnSingleIndexedComponent faceFn;
			MObject faces = faceFn.create(MFn::kMeshPolygonComponent, &mStatus);
			MCHECK_BEGIN << "Could not create face component object." MCHECK_ERROR_RETURN_END_1;

			MIntArray faceIndices;
			mStatus = faceIndices.setLength(pMaterialUsageCount[i]);
			MCHECK_BEGIN << "Could not set face index array size." MCHECK_ERROR_RETURN_END_1
			for (U32 j = 0; j < (U32)pMaterialBatches[i].Size(); j++)
			{
				//Add face index
				mStatus = faceIndices.set(pMaterialBatches[i][j], j);
				MCHECK_BEGIN << "Could not set face indices." MCHECK_ERROR_RETURN_END_1;
			}
			//Add our array of face elements to our face set.
			mStatus = faceFn.addElements(faceIndices);
			MCHECK_BEGIN << "Could not add face indices for set." MCHECK_ERROR_RETURN_END_1;
			{
				setFn.setObject(sgObject);
				mStatus = setFn.addMember(mayaShapeDagPath, faces);
				MCHECK_BEGIN << "Could not affix SG to polys." MCHECK_ERROR_RETURN_END_1
			}
		}
	}
	
	//Delete our material batches, now that we're finished with them.
	delete[] pMaterialBatches;
	delete[] pMaterialUsageCount;

	//Assign polys with no material
	if (faceIndicesWithNoMaterial.length() > 0)
	{
		MFnSingleIndexedComponent faceFn;
		MObject faces = faceFn.create(MFn::kMeshPolygonComponent, &mStatus);
		MCHECK_BEGIN << "Could not create face component object." MCHECK_ERROR_RETURN_END_1;

		mStatus = faceFn.addElements(faceIndicesWithNoMaterial);
		MCHECK_BEGIN << "Could not add face indices for set." MCHECK_ERROR_RETURN_END_1;
		{
			setFn.setObject(m_initialShadingGroupObject);
			mStatus = setFn.addMember(mayaShapeDagPath, faces);
			MCHECK_BEGIN << "Could not affix SG to polys." MCHECK_ERROR_RETURN_END_1;
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

