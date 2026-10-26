#include "stdafx.h"

#include <maya/MFloatVectorArray.h>
#include <core/allocators/SSSMemoryPool.h>

namespace MayaIo
{
	struct UVSet 
	{
		MFloatArray	uArray;
		MFloatArray	vArray;
		MString		name;
		UVSet*		next;
	};

	UVSet*	fHeadUVSet;
}

using namespace MayaIo;

Aftk::C_Status C_SceneMeshImport::Import(
	C_Importer* pImporter,
	const MDagPath& transformDagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;
	//Import the base AFTK scene node
	status = C_SceneNodeImport::Import(
		pImporter,
		transformDagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		true,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	// store location of new AFTK node
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*)pAftkSceneNode;

	//Acquire dag path for shape..
	MDagPath shapeDagPath = MDagPath(transformDagPath);
	mStatus = shapeDagPath.extendToShape();
	if (!mStatus)
	{
		if (pOptions->strict)
		{
			AFTK_DO_BEGIN << TRANS_EXTEND_TO_SHAPE_FAIL AFTK_DO_ERROR_RETURN_END_0;
		}
		else
		{
			AFTK_DO_BEGIN << TRANS_ACQUIRE_FN_WARN AFTK_DO_WARNING_END;
			pImporter->AddDebris(pAftkSceneNode);
			return Aftk::C_Status::E_Status_Success;
		}
	}

	//Acquire mesh function set.
	MFnMesh fnMesh(transformDagPath, &mStatus);
	if (!mStatus)
	{
		if (pOptions->strict)
		{
			AFTK_DO_BEGIN << TRANS_ACQUIRE_FN_FAIL AFTK_DO_ERROR_RETURN_END_0;
		}
		else
		{
			AFTK_DO_BEGIN << TRANS_ACQUIRE_FN_WARN AFTK_DO_WARNING_END;
			pImporter->AddDebris(pAftkSceneNode);
			return Aftk::C_Status::E_Status_Success;
		}
	}

	//Build an array containing the MObjects for our mesh's sets.
	MObjectArray polySetArray;
	MObjectArray polySetComponentsArray;
	Ut::UtVector<Ut::DynamicVector<S32> > localToScenePolySetIndexMapping;
	status = BuildPolySetInfo(
		pImporter,
		shapeDagPath,
		fnMesh,
		polySetArray,
		polySetComponentsArray,
		localToScenePolySetIndexMapping);
	AFTK_CHECK_ERROR_RETURN;

	// Allocate storage for poly->tri remapping
	U32 numPolys = fnMesh.numPolygons(&mStatus);
	MCHECK_BEGIN << "Failed to acquire mesh poly count." MCHECK_ERROR_RETURN_END_0;
	if (numPolys == 0)
	{
		AFTK_DO_BEGIN << TRANS_ZERO_POLY_MESH_FAIL AFTK_DO_WARNING_END;
		pImporter->AddDebris(pAftkSceneNode);
		return Aftk::C_Status::E_Status_Success;
	}

	Ut::UtDynamicVector<U32>* pPolyMapping = new Ut::UtDynamicVector<U32>[numPolys];
	if (!pPolyMapping)
	{
		AFTK_DO_BEGIN << "Failed to allocate memory for poly mapping." AFTK_DO_ERROR_RETURN_END_1;
	}

//#pragma message("[TODO] Abort the import of meshes with bad numbers in. At the moment they will just be removed in a post process if strict=false")

	bool foundInfinitePosition = false;
	// get the vertices that are part of the current mesh
	MPointArray vertexList;
	mStatus = fnMesh.getPoints(vertexList, MSpace::kObject);
	MCHECK_BEGIN << "Failed to get vertex list." MCHECK_ERROR_RETURN_END_1
	//Extract vertex positins
	{
		U32 numVertices = vertexList.length();
		pAftkSceneMeshNode->SetVertexPositionInitialCapacity(numVertices);
		for (U32 i = 0; i < numVertices; i++ )
		{
			MPoint point = vertexList[i];

			static const double e = 0.0001;
			if (abs(point.x) < e) 
				point.x = 0;
			if (abs(point.y) < e) 
				point.y = 0;
			if (abs(point.z) < e) 
				point.z = 0;
			if (!IsNumberValid((F32)point.x, pOptions->absNumericRange))
			{
				point.x = 0;
				if (!foundInfinitePosition)
				{
					foundInfinitePosition = true;
					if (pOptions->strict)
					{
						AFTK_DO_BEGIN << TRANS_INFINITE_POSITION_FAIL AFTK_DO_ERROR_RETURN_END_0;
					}
					else
					{
						AFTK_DO_BEGIN << TRANS_INFINITE_POSITION_WARN AFTK_DO_WARNING_END;
						pImporter->AddDebris(pAftkSceneNode);
					}
				}
			}
			if (!IsNumberValid((F32)point.y, pOptions->absNumericRange))
			{
				point.y = 0;
				if (!foundInfinitePosition)
				{
					foundInfinitePosition = true;
					if (pOptions->strict)
					{
						AFTK_DO_BEGIN << TRANS_INFINITE_POSITION_FAIL AFTK_DO_ERROR_RETURN_END_0;
					}
					else
					{
						AFTK_DO_BEGIN << TRANS_INFINITE_POSITION_WARN AFTK_DO_WARNING_END;
						pImporter->AddDebris(pAftkSceneNode);
					}
				}
			}
			if (!IsNumberValid((F32)point.z, pOptions->absNumericRange))
			{
				point.z = 0;
				if (!foundInfinitePosition)
				{
					foundInfinitePosition = true;
					if (pOptions->strict)
					{
						AFTK_DO_BEGIN << TRANS_INFINITE_POSITION_FAIL AFTK_DO_ERROR_RETURN_END_0;
					}
					else
					{
						AFTK_DO_BEGIN << TRANS_INFINITE_POSITION_WARN AFTK_DO_WARNING_END;
						pImporter->AddDebris(pAftkSceneNode);
					}
				}
			}

			Aftk::Position3 aftkVertexPosition(
				(F32)point.x, 
				(F32)point.y, 
				(F32)point.z);
			pAftkSceneMeshNode->AddVertexPosition(aftkVertexPosition);
		}
	}
	
	bool foundInfiniteNormal = false;
	//Extract normals
	{
		bool foundCorruptNormals = false;
		MFloatVectorArray normalList;
		fnMesh.getNormals(normalList);
		// Copy vertex normals
		U32 numNormals = normalList.length();
		pAftkSceneMeshNode->SetVertexNormalInitialCapacity(numNormals);
		for (U32 i = 0; i < numNormals; i++ ) 
		{
			bool thisNormalIsCorrupt = false;
			MFloatVector mayaNormal = normalList[i];
			if (!IsNumberValid(mayaNormal.x, pOptions->absNumericRange))
			{
				thisNormalIsCorrupt = true;
				mayaNormal.x = 0;
				if (!foundInfiniteNormal)
				{
					foundInfiniteNormal = true;
					if (pOptions->strict)
					{
						AFTK_DO_BEGIN << TRANS_INFINITE_NORMAL_FAIL AFTK_DO_ERROR_RETURN_END_0;
					}
					else
					{
						AFTK_DO_BEGIN << TRANS_INFINITE_NORMAL_WARN AFTK_DO_WARNING_END;
					}
				}
			}
			if (!IsNumberValid(mayaNormal.y, pOptions->absNumericRange))
			{
				thisNormalIsCorrupt = true;
				mayaNormal.y = 0;
				if (!foundInfiniteNormal)
				{
					foundInfiniteNormal = true;
					if (pOptions->strict)
					{
						AFTK_DO_BEGIN << TRANS_INFINITE_NORMAL_FAIL AFTK_DO_ERROR_RETURN_END_1;
					}
					else
					{
						AFTK_DO_BEGIN << TRANS_INFINITE_NORMAL_WARN AFTK_DO_WARNING_END;
					}
				}
			}
			if (!IsNumberValid(mayaNormal.z, pOptions->absNumericRange))
			{
				thisNormalIsCorrupt = true;
				mayaNormal.z = 0;
				if (!foundInfiniteNormal)
				{
					foundInfiniteNormal = true;
					if (pOptions->strict)
					{
						AFTK_DO_BEGIN << TRANS_INFINITE_NORMAL_FAIL AFTK_DO_ERROR_RETURN_END_0;
					}
					else
					{
						AFTK_DO_BEGIN << TRANS_INFINITE_NORMAL_WARN AFTK_DO_WARNING_END;
					}
				}
			}
			if (thisNormalIsCorrupt)
			{
				Aftk::Normal3 aftkVertexNormal;
				aftkVertexNormal[0] = 0;
				aftkVertexNormal[1] = 0;
				aftkVertexNormal[2] = 0;
				pAftkSceneMeshNode->AddVertexNormal(aftkVertexNormal);
			}
			else
			{
				Aftk::Vector3F32 temp((F32)mayaNormal.x, (F32)mayaNormal.y, (F32)mayaNormal.z);

				F32 mag = temp[0] * temp[0] + temp[1] * temp[1] + temp[2] * temp[2];
				if (mag < 0.01f)
				{
					if (!foundCorruptNormals)
					{
						if (pOptions->strict)
						{
							AFTK_DO_BEGIN << TRANS_CORRUPT_NORMAL_FAIL AFTK_DO_ERROR_RETURN_END_0;
						}
						else
						{
							AFTK_DO_BEGIN << TRANS_CORRUPT_NORMAL_WARN AFTK_DO_WARNING_END;
						}
						foundCorruptNormals = true;
					}
				}
				else if (mag < 0.98f)
				{
					UtMaths::SafeNormalise(&temp);
				}
				Aftk::Normal3 aftkVertexNormal = MakeNormal(temp);
				pAftkSceneMeshNode->AddVertexNormal(aftkVertexNormal);
			}
		}
	}

	// Extract UV coordinates
	bool foundInfiniteUVs = false;
	{
		MStringArray uvSetNames;
		mStatus =  fnMesh.getUVSetNames(uvSetNames); 
		MCHECK_BEGIN << "Failed to get UV set names." MCHECK_ERROR_RETURN_END_1;

		U32 uvSetCount = uvSetNames.length();
		U32 uvSetInUse = 0;
		for (U32 uvSet = 0; uvSet < uvSetCount; uvSet++) 
		{
			MFloatArray uArray;
			MFloatArray vArray;

			U32 numUVs;

			if (fnMesh.getUVs(uArray, vArray, &uvSetNames[uvSet]) != MStatus::kSuccess)
			{
				//Assume this was a null uv set
				numUVs = 0;
			}
			else
			{
				numUVs = (U32)fnMesh.numUVs(uvSetNames[uvSet]);
			}
			
			if (numUVs > 0)
			{
				pAftkSceneMeshNode->SetTextureCoordinateInitialCapacity(uvSetInUse, numUVs);
				//g_tcDebugMan << "UV [" << uvSetNames[uvSet].asChar() << "] " << uvSet << " has " << numUVs  << " uvs\n";
				for (U32 j = 0; j < numUVs; j++) 
				{
					F32 u = uArray[j];
					F32 v = vArray[j];
					if (!IsNumberValid(u, pOptions->absNumericRange))
					{
						u = 0;
						if (!foundInfiniteUVs)
						{
							foundInfiniteUVs = true;
							if (pOptions->strict)
							{
								AFTK_DO_BEGIN << TRANS_INFINITE_UV_FAIL AFTK_DO_ERROR_RETURN_END_0;
							}
							else
							{
								AFTK_DO_BEGIN << TRANS_INFINITE_UV_WARN AFTK_DO_WARNING_END;
							}
						}
					}
					if (!IsNumberValid(v, pOptions->absNumericRange))
					{
						v = 0;
						if (!foundInfiniteUVs)
						{
							foundInfiniteUVs = true;
							if (pOptions->strict)
							{
								AFTK_DO_BEGIN << TRANS_INFINITE_UV_FAIL AFTK_DO_ERROR_RETURN_END_0;
							}
							else
							{
								AFTK_DO_BEGIN << TRANS_INFINITE_UV_WARN AFTK_DO_WARNING_END;
							}
						}
					}
					Aftk::TextureCoordinate4 aftkTextureCoordinate(u, 1-v, 0,0);
					pAftkSceneMeshNode->AddTextureCoordinate(uvSetInUse, aftkTextureCoordinate);
				}
			}

			std::string uvSetName = uvSetNames[uvSet].asChar();
			pAftkSceneMeshNode->SetTextureCoordinateSetName(uvSetInUse, uvSetName);
			uvSetInUse++;
		}
	}

	//Count the number of vertex channels
	U32 numVertexColourChannels = 0;
	MItMeshPolygon polyIter(transformDagPath, MObject::kNullObj );
	for ( ; !polyIter.isDone(); polyIter.next()) 
	{
		if (polyIter.hasColor(&mStatus))
		{
			numVertexColourChannels = 1;
			break;
		}
		MCHECK_BEGIN << "Failed to get colour count." MCHECK_ERROR_RETURN_END_1
	}

	//Import vertex data channel description
	if (numVertexColourChannels > 0)
	{
		for (U32 iChannel = 0; iChannel < numVertexColourChannels; iChannel++)
		{
			Aftk::C_VertexDataChannelInfo info;

			info.SetName(AFTK_DATA_CHANNEL_USAGE_VERTEX_COLOUR);
			info.SetDataType(Aftk::E_GenericAttributeType_U32);
			pAftkSceneMeshNode->SetVertexDataChannelInfo(iChannel, info);
		}
	}

	//Import polygons: Stage 1, allocate storage for polygons
	{
		U32 numPolysToImport = 0;
		if (pOptions->triangulate == false)
		{
			numPolysToImport = fnMesh.numPolygons();
		}
		else
		{
			//Calculate the number of triangles in this mesh
			MItMeshPolygon polyIter(transformDagPath, MObject::kNullObj );
			for ( ; !polyIter.isDone(); polyIter.next()) 
			{
				int numTriangles;
				polyIter.numTriangles(numTriangles);
				numPolysToImport += numTriangles;
			}
		}
		pAftkSceneMeshNode->SetPolygonInitialCapacity(0,numPolysToImport);
	}

	
	//Import polygons:: Stage 2, perform polygon import.
	{
		MItMeshPolygon polyIter(transformDagPath, MObject::kNullObj );
		for ( ; !polyIter.isDone(); polyIter.next()) 
		{
			status = ImportPolygon(
				pImporter,
				pAftkScene,
				pAftkSceneMeshNode,
				pOptions,
				shapeDagPath,
				fnMesh,
				&polyIter,
				pPolyMapping,
				polySetArray,
				polySetComponentsArray,
				localToScenePolySetIndexMapping);
			AFTK_CHECK_ERROR_RETURN;
		}
	}

	// If the shape is instanced then we need to determine which
	// instance this path refers to.
	//
	S32 instanceNum = 0;
	if (shapeDagPath.isInstanced()) 
		instanceNum = shapeDagPath.instanceNumber();

	// Import the materials used by this mesh
	status = ImportMaterials(
		pImporter,
		shapeDagPath,
		fnMesh,
		instanceNum,
		pAftkSceneMeshNode, 
		pAftkScene,
		pOptions,
		pPolyMapping);
	AFTK_CHECK_ERROR_RETURN;

//	if (!pOptions->importPolyBlindData)
	{
		status = ImportSetsAsBlindData(
			pImporter,
			shapeDagPath,
			fnMesh,
			pAftkSceneMeshNode,
			pAftkScene,
			pOptions,
			pPolyMapping,
			polySetArray,
			polySetComponentsArray,
			localToScenePolySetIndexMapping);
		AFTK_CHECK_ERROR_RETURN;
	}

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "AFTK mesh [" << pAftkSceneMeshNode->GetName() << "] has [" << pAftkSceneMeshNode->GetNumVertexPositions() << "] vertices\n" AFTK_DO_DEBUG_END;
	#endif

	delete[] pPolyMapping;

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Mesh Node " << fnMesh.name().asChar() << " id " << fnMesh.typeId().id() << "\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

///
/// Fills the given C_Polygon with data from Maya
Aftk::C_Status C_SceneMeshImport::ImportPolygon(
	C_Importer* pImporter,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
	const C_ImportOptions* pOptions,
	const MDagPath& shapeDagPath,
	const MFnMesh& fnMesh,
	MItMeshPolygon* pMayaPolyIter,
	Ut::UtDynamicVector<U32>* pPolyMapping,
	MObjectArray& polySetArray,
	MObjectArray& polySetComponentsArray,
	const Ut::UtVector<Ut::DynamicVector<S32> >& localToScenePolySetIndexMapping)
{
	Aftk::C_Status status;
	MStatus mStatus;

	MIntArray vertexPositionIndices;
	mStatus = pMayaPolyIter->getVertices(vertexPositionIndices);
	MCHECK_BEGIN << "Failed to get vertex position indices from polygon iterator." MCHECK_ERROR_RETURN_END_1

	U32 noPolyVertexPositions = vertexPositionIndices.length();
	U32 maxVerts;
	if (pOptions->triangulate == true) 
		maxVerts = 3;
	else
		maxVerts = pOptions->maxNGonNumVertices;

	if ((noPolyVertexPositions > maxVerts) || (noPolyVertexPositions == 3)) 
	{
		//Import polygon as triangles
		status = ImportTriangles(
			pAftkScene,
			pAftkSceneMeshNode,
			pOptions,
			shapeDagPath,
			fnMesh,
			pMayaPolyIter,
			pPolyMapping);
		AFTK_CHECK_ERROR_RETURN;
	} 
	else 
	{
		//Import polygon as n-gon
		status = ImportNGon(
			pAftkScene,
			pAftkSceneMeshNode,
			pOptions,
			shapeDagPath,
			fnMesh,
			pMayaPolyIter,
			pPolyMapping);
		AFTK_CHECK_ERROR_RETURN;
	}

	return Aftk::C_Status::E_Status_Success;
}

S32 C_SceneMeshImport::FindAftkUVSetIndex(
	const MFnMesh& fnMesh,
	S32 mayaUVSetIndex, 
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
	Aftk::C_Status& status)
{
	MStatus mStatus;
	MStringArray uvSetNames;
	mStatus = fnMesh.getUVSetNames(uvSetNames);
	AFTK_CHECK_BEGIN << "Failed to import triangle." AFTK_CHECK_ERROR_RETURN_END_1

	const char* uvSetNameDebug = uvSetNames[mayaUVSetIndex].asChar();

	//Find the Aftk UV set into which we are to insert
	S32 aftkUVSet = -1;
	for (U32 uvSetSearch = 0; uvSetSearch < pAftkSceneMeshNode->GetNumTextureCoordinateSets(); uvSetSearch++)
	{
		MString aftkUvSetName = pAftkSceneMeshNode->GetTextureCoordinateSetName(uvSetSearch).c_str();
		const char* aftkUvSetNameDebug = aftkUvSetName.asChar();
		if (uvSetNames[mayaUVSetIndex] == aftkUvSetName)
		{
			//Okay, we found the uv set to insert into
			aftkUVSet = uvSetSearch;
			break;
		}
	}

	if (aftkUVSet == -1)
	{
		AFTK_DO_BEGIN << "Could not find uv set index." AFTK_DO_ERROR_RETURN_END_1;
	}
	else
	{
		status = Aftk::C_Status::E_Status_Success;
	}
	
	return aftkUVSet;
}

///
/// Fills the given C_Polygon with triangle data from Maya
Aftk::C_Status C_SceneMeshImport::ImportTriangles(
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
	const C_ImportOptions* pOptions,
	const MDagPath& shapeDagPath,
	const MFnMesh& fnMesh,
	MItMeshPolygon* pMayaPolyIter,
	Ut::UtDynamicVector<U32>* pPolyMapping)
{
	Aftk::C_Status status;
	MStatus mStatus;

	MStringArray uvSetNames;
	mStatus = fnMesh.getUVSetNames(uvSetNames);
	MCHECK_BEGIN << "Could not get UV set names." MCHECK_ERROR_RETURN_END_1
	U32 uvSetCount = uvSetNames.length();

#ifdef AFTK_MAYA_IO_7_0
	if (uvSetCount > MAYA_IO_MAX_UV_SETS)
	{
		AFTK_DO_BEGIN << TRANS_TOO_MANY_UV_SETS AFTK_DO_ERROR_RETURN_END_1;
	}
	//Attempt to minimalise heap memory usage by using fixed arrays of dynamic arrays for tangents 
	//and binormals.
	U32 polyIndex = pMayaPolyIter->index();
	MFloatVectorArray faceVertexTangents[MAYA_IO_MAX_UV_SETS];
	MFloatVectorArray faceVertexBinormals[MAYA_IO_MAX_UV_SETS];
	if (pOptions->importTangentSpace)
	{
		for (U32 iSet = 0; iSet < uvSetCount; iSet++)
		{
			mStatus = fnMesh.getFaceVertexTangents(
				polyIndex,
				faceVertexTangents[iSet],
				MSpace::kObject, 
				&uvSetNames[iSet]);

			mStatus = fnMesh.getFaceVertexBinormals(
				polyIndex,
				faceVertexBinormals[iSet],
				MSpace::kObject, 
				&uvSetNames[iSet]);
		}
	}
#endif

	S32 numTriangles;
	pMayaPolyIter->numTriangles(numTriangles);
	if (numTriangles < 0) 
	{
		AFTK_DO_BEGIN << "Could not extract any triangles from the given polygon iterator." AFTK_DO_ERROR_RETURN_END_1
	}

	// Get mesh-relative indices for the vertices in this polygon.
    MIntArray vertexPositionIndices;
	mStatus = pMayaPolyIter->getVertices(vertexPositionIndices);
	MCHECK_BEGIN << "Could not get vertex positions from polygon iterator." MCHECK_ERROR_RETURN_END_1

	S32 basePolyIndex = -1;
	for (U32 i = 0; i < (U32)numTriangles; i++) 
	{
		MPointArray nonTweaked;
		// object-relative vertex indices for each triangle
		MIntArray	triangleVertices;
		mStatus = pMayaPolyIter->getTriangle(
			i,
			nonTweaked,
			triangleVertices,
			MSpace::kObject);
		MCHECK_BEGIN << "Could not get triangle from polygon iterator." MCHECK_ERROR_RETURN_END_1

		// MItMeshPolygon::normalIndex() and ::getNormal() need face-relative vertex indices.
		// The following function performs this transformation.
		//
		// - vertexPositionIndices Array of object-relative vertex indices for entire face.
		//
		// - triangleVertices is an array of object-relative vertex indices for
		//   the local triangle in face.
		//
		// - Returns an array of face-relative indicies for the specified vertices.
		//
		// face-relative vertex indices for each triangle
		MIntArray faceRelativeVertexIndices = 
			GetLocalIndex(vertexPositionIndices, triangleVertices, status);
		if (!status)
		{
			AFTK_DO_BEGIN 
				<< "A polygon on mesh " << pAftkSceneMeshNode->GetName() << 
				" was found to be corrupt during triangulation. We'll attempt to continue but the triangulation may be incorrect. Try triangulating the object within Maya." AFTK_DO_WARNING_END
			continue;
		}
		//Create a new polygon instance and use this to add a new polygon instance into the mesh
		U32 triIndex = pAftkSceneMeshNode->AddPolygon(0, 0, Aftk::C_Polygon());
		//Using the index of the new polygon we can get a pointer to it.
		//We can then fill out the polygon data directly
		Aftk::C_Polygon* pAftkPolygon = &pAftkSceneMeshNode->GetPolygon(0, triIndex);
		if (i == 0)
		{
			basePolyIndex = triIndex;
			pAftkPolygon->SetBasePolygonIndex(-1);
		}
		else if (i > 0)
		{
			pAftkPolygon->SetBasePolygonIndex(basePolyIndex);
		}
	
		pPolyMapping[pMayaPolyIter->index()].PushBack(triIndex);

		// If we have successfully fetched a triangle, then let's add it to this polygon
		// j indexes the current vertex for this triangle
		const U32 numPolygonVertices = 3;
		#ifdef FLIP_NORMALS
		for (S32 j = 0; j < numPolygonVertices; j++) 
		#else
		for (S32 j = numPolygonVertices-1; j > -1; j--) 
		#endif
		{
			//Add poly vertex to mesh
			U32 aftkPolyVertIndex = pAftkSceneMeshNode->AddPolygonVertex(Aftk::C_PolygonVertex());
			//Index poly vertex on polygon
			pAftkPolygon->AddPolygonVertexIndex(aftkPolyVertIndex);
			//Get a pointer to the pvertex so we can fill it out.
			Aftk::C_PolygonVertex* pAftkPolygonVertex = &pAftkSceneMeshNode->GetPolygonVertex(aftkPolyVertIndex);

			// *** BEGIN Fill polygon vertex ***

			// Mesh relative VERTEX POSITION index
			int vtxIndex = triangleVertices[j];
			
			S32 vertexPositionIndex = (S32)vtxIndex;
			pAftkPolygonVertex->SetPositionIndex(vertexPositionIndex);

			// Mesh relative VERTEX NORMAL index
			S32 vertexNormalIndex = pMayaPolyIter->normalIndex(faceRelativeVertexIndices[j]);
			// Check that index is valid before adding...
			if (vertexNormalIndex >= 0) 
			{
				pAftkPolygonVertex->SetNormalIndex((S32)vertexNormalIndex);
			}

			// Mesh relative vertex colour assumed to be at same index as position
			if (pMayaPolyIter->hasColor(faceRelativeVertexIndices[j]) == true)
			{
				S32 vertexColourCount;
				mStatus = pMayaPolyIter->numColors(vertexColourCount);
				MCHECK_BEGIN << "Failed to get vertex colour count." MCHECK_ERROR_RETURN_END_1
				if (vertexColourCount > 0)
				{
					MColor colour;
					mStatus = pMayaPolyIter->getColor(colour, faceRelativeVertexIndices[j]);
					if (mStatus)
					{
						U32 colour32 =  Aftk::ColourRGBAToU32(colour.r,colour.g,colour.b,colour.a);
						pAftkPolygonVertex->SetData(0, colour32);
					}
					else
					{
						pAftkPolygonVertex->SetData(0, 0xffffffff);
					}
				}
			}
		
			// Get UV COORDINATES
			// Loop thru the uv sets for this polygon vertex
			for (U32 kSet = 0; kSet < uvSetCount; kSet++)
			{
				S32 aftkUVSet = FindAftkUVSetIndex(
					fnMesh,
					kSet, 
					pAftkSceneMeshNode,
					status);
				AFTK_CHECK_BEGIN << "Could not find AFTK UV set." AFTK_CHECK_ERROR_RETURN_END_1;

				S32 uvIndex;
				if (pMayaPolyIter->getUVIndex(faceRelativeVertexIndices[j], uvIndex, &uvSetNames[kSet]) == MStatus::kSuccess) 
				{
					// Check that index is valid before adding...
					if (uvIndex >= 0) 
						// each texture coordinate added is of course for a different UV layer
						pAftkPolygonVertex->SetTextureCoordinateIndex(kSet, uvIndex);
					else
						pAftkPolygonVertex->SetTextureCoordinateIndex(kSet, -1);

					#ifdef AFTK_MAYA_IO_7_0
					if (pOptions->importTangentSpace)
					{
						const MFloatVector& tan = faceVertexTangents[kSet][faceRelativeVertexIndices[j]];
						const MFloatVector& bin = faceVertexBinormals[kSet][faceRelativeVertexIndices[j]];
						S32 tanIndex = pAftkSceneMeshNode->AddVertexTangent(kSet, Aftk::Vector3F32(-tan[0], -tan[1], -tan[2]));
						S32 binIndex = pAftkSceneMeshNode->AddVertexBinormal(kSet, Aftk::Vector3F32(bin[0], bin[1], bin[2]));
						AFTK_ASSERT(tanIndex != -1);
						AFTK_ASSERT(binIndex != -1);
						pAftkPolygonVertex->SetTangentIndex(kSet, tanIndex);
						pAftkPolygonVertex->SetBinormalIndex(kSet, binIndex);
					}
					#endif
				}
				else
				{
					//Only add this for padding such that the number of UV indices is always
					//equal to the number of texture coordinate sets on the mesh.
					if (kSet < pAftkSceneMeshNode->GetNumTextureCoordinateSets())
						pAftkPolygonVertex->SetTextureCoordinateIndex(kSet, -1);
				}
			}
			if (pAftkPolygonVertex->GetNumTextureCoordinateIndices() != pAftkSceneMeshNode->GetNumTextureCoordinateSets())
			{
				if (pAftkPolygonVertex->GetNumTextureCoordinateIndices() < pAftkSceneMeshNode->GetNumTextureCoordinateSets())
					pAftkPolygonVertex->SetTextureCoordinateIndex(uvSetCount, -1);
				else
					AFTK_DO_BEGIN << "UV set error" AFTK_DO_ERROR_RETURN_END_1;
			}
		} // end for
	} // end for

	return Aftk::C_Status::E_Status_Success;
}

///
/// Fills the given C_Polygon with n-gon data from Maya
Aftk::C_Status C_SceneMeshImport::ImportNGon(	
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
	const C_ImportOptions* pOptions,
	const MDagPath& shapeDagPath,
	const MFnMesh& fnMesh,
	MItMeshPolygon* pMayaPolyIter,
	Ut::UtDynamicVector<U32>* pPolyMapping)
{
	Aftk::C_Status status;
	MStatus mStatus;

	MStringArray uvSetNames;
	mStatus = fnMesh.getUVSetNames(uvSetNames);
	MCHECK_BEGIN << "Could not get UV set names." MCHECK_ERROR_RETURN_END_1
	U32 uvSetCount = uvSetNames.length();

#ifdef AFTK_MAYA_IO_7_0
	if (uvSetCount > MAYA_IO_MAX_UV_SETS)
	{
		AFTK_DO_BEGIN << TRANS_TOO_MANY_UV_SETS AFTK_DO_ERROR_RETURN_END_1;
	}

	//Attempt to minimalise heap memory usage by using fixed arrays of dynamic arrays for tangents 
	//and binormals.
	MFloatVectorArray faceVertexTangents[MAYA_IO_MAX_UV_SETS];
	MFloatVectorArray faceVertexBinormals[MAYA_IO_MAX_UV_SETS];
	if (pOptions->importTangentSpace)
	{
		for (U32 iSet = 0; iSet < uvSetCount; iSet++)
		{
			mStatus = fnMesh.getFaceVertexTangents(
				pMayaPolyIter->index(),
				faceVertexTangents[iSet],
				MSpace::kObject, 
				&uvSetNames[iSet]);

			mStatus = fnMesh.getFaceVertexBinormals(
				pMayaPolyIter->index(),
				faceVertexBinormals[iSet],
				MSpace::kObject, 
				&uvSetNames[iSet]);
		}
	}
#endif

	MIntArray vertexPositionIndices;
	pMayaPolyIter->getVertices(vertexPositionIndices);
	U32 noPolyVertexPositions = vertexPositionIndices.length();

	if (noPolyVertexPositions < 3)
	{
		return Aftk::C_Status::E_Status_Success;
	}

	//Use this to add a new polygon instance into the mesh
	U32 polyIndex = pAftkSceneMeshNode->AddPolygon(0, 0,  Aftk::C_Polygon());
	//Using the index of the new polygon we can get a pointer to it.
	//We can then fill out the polygon data directly
	Aftk::C_Polygon* pAftkPolygon = &pAftkSceneMeshNode->GetPolygon(0,polyIndex);

	pPolyMapping[pMayaPolyIter->index()].PushBack(polyIndex);

	// i indexes the current polygon vertex for this polygon
	#ifdef FLIP_NORMALS
	for (U32 i = 0; i < noPolyVertexPositions; i++) 
	#else
	for (S32 i = noPolyVertexPositions-1; i > -1 ; i--) 
	#endif
	{
		// Add poly vertex to mesh
		U32 aftkPolyVertIndex = pAftkSceneMeshNode->AddPolygonVertex(Aftk::C_PolygonVertex());
		//Index polygon vertex on polygon
		pAftkPolygon->AddPolygonVertexIndex(aftkPolyVertIndex);
		Aftk::C_PolygonVertex* pAftkPolygonVertex = &pAftkSceneMeshNode->GetPolygonVertex(aftkPolyVertIndex);

		// *** BEGIN Fill polygon vertex ***

		// Mesh relative VERTEX POSITION index
		S32 vertexPositionIndex = (S32)vertexPositionIndices[i];
		pAftkPolygonVertex->SetPositionIndex(vertexPositionIndex);

		// Mesh relative VERTEX NORMAL index
		S32 vertexNormalIndex = pMayaPolyIter->normalIndex(i);
		// Check that index is valid before adding...
		if (vertexNormalIndex >= 0) 
		{
			pAftkPolygonVertex->SetNormalIndex((S32)vertexNormalIndex);
		}

		// Mesh relative vertex colour assumed to be at same index as position
		if (pMayaPolyIter->hasColor(i) == true)
		{
			S32 vertexColourCount;
			mStatus = pMayaPolyIter->numColors(vertexColourCount);
			AFTK_CHECK_BEGIN << "Failed to get vertex colour count." AFTK_CHECK_ERROR_RETURN_END_1;
			if (vertexColourCount > 0)
			{
				MColor colour;
				mStatus = pMayaPolyIter->getColor(colour, i);
				if (mStatus)
				{
					U32 colour32 = Aftk::ColourRGBAToU32(colour.r,colour.g,colour.b,colour.a);
					pAftkPolygonVertex->SetData(0, colour32);
				}
				else
				{
					pAftkPolygonVertex->SetData(0, 0xffffffff);
				}
			}
		}

		// Get UV COORDINATES
		MStringArray uvSetNames;
		fnMesh.getUVSetNames(uvSetNames);
		U32 uvSetCount = uvSetNames.length();
		// Loop thru the uv sets for this polygon vertex
		// j indexes the current uv set for this polygon
		for (U32 jSet = 0; jSet < uvSetCount; jSet++) 
		{
			S32 aftkUVSet = FindAftkUVSetIndex(
				fnMesh,
				jSet,
				pAftkSceneMeshNode,
				status);
			CHECK_ERROR_RETURN;

			S32 uvIndex;

			if (pMayaPolyIter->getUVIndex(i, uvIndex, &uvSetNames[jSet]) == MStatus::kSuccess) 
			{
				#ifdef AFTK_MAYA_IO_7_0
				if (pOptions->importTangentSpace)
				{
					const MFloatVector& tan = faceVertexTangents[jSet][i];
					const MFloatVector& bin = faceVertexBinormals[jSet][i];
					S32 tanIndex = pAftkSceneMeshNode->AddVertexTangent(jSet, Aftk::Vector3F32(-tan[0], -tan[1], -tan[2]));
					S32 binIndex = pAftkSceneMeshNode->AddVertexBinormal(jSet, Aftk::Vector3F32(bin[0], bin[1], bin[2]));
					AFTK_ASSERT(tanIndex != -1);
					AFTK_ASSERT(binIndex != -1);
					pAftkPolygonVertex->SetTangentIndex(jSet, tanIndex);
					pAftkPolygonVertex->SetBinormalIndex(jSet, binIndex);
				}
				#endif

				// Check that index is valid before adding...
				if (uvIndex >= 0) 
					pAftkPolygonVertex->SetTextureCoordinateIndex(jSet, uvIndex);
				else
					pAftkPolygonVertex->SetTextureCoordinateIndex(jSet, -1);
			}
			else
			{
				//Only add this for padding such that the number of UV indices is always
				//equal to the number of texture coordinate sets on the mesh.
				if (jSet < pAftkSceneMeshNode->GetNumTextureCoordinateSets())
				{
					pAftkPolygonVertex->SetTextureCoordinateIndex(jSet, -1);
				}
			}
		}
		if (pAftkPolygonVertex->GetNumTextureCoordinateIndices() != pAftkSceneMeshNode->GetNumTextureCoordinateSets())
		{
			if (pAftkPolygonVertex->GetNumTextureCoordinateIndices() < pAftkSceneMeshNode->GetNumTextureCoordinateSets())
			{
				//This vertex has no UV coords for this set";
				pAftkPolygonVertex->SetTextureCoordinateIndex(uvSetCount, -1);
			}
			else
			{
				AFTK_DO_BEGIN << "UV set error." AFTK_DO_ERROR_RETURN_END_1;
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

MIntArray C_SceneMeshImport::GetLocalIndex(
	const MIntArray& getVertices, 
	const MIntArray& getTriangle,
	Aftk::C_Status& status)
{
	MIntArray   localIndex;
	unsigned    gv, gt;

	for (gt = 0; gt < getTriangle.length(); gt++ ) 
	{
		for (gv = 0; gv < getVertices.length(); gv++)
		{
			if (getTriangle[gt] == getVertices[gv] )
			{
				localIndex.append( gv );
				break;
			}
		}
		//If we get here then this means that somehow our triangle contains the object-relative
		//index of a vertex that's not present on the face. I've only seen this code triggered once 14/12/2006.
		//I'm not sure under what circumstances maya can generate invalid data like this.
		if ( localIndex.length() == gt )
		{
			status = Aftk::C_Status::E_Status_Failure;
			return localIndex;
		}
	}

	status = Aftk::C_Status::E_Status_Success;
	return localIndex;
}

Aftk::C_Status C_SceneMeshImport::ImportMaterials(	
	C_Importer* pImporter,
	const MDagPath& shapeDagPath,
	const MFnMesh& fnMesh,
	S32 instanceNumber,
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
	Aftk::C_Scene* pAftkScene,	
	const C_ImportOptions* pOptions,
	Ut::UtDynamicVector<U32>* pPolyMapping)
{
	Aftk::C_Status status;
	MStatus mStatus;

	// Get a list of all sets pertaining to the selected shape and the
	// members of those sets.
	//
	MObjectArray fPolygonSets;
	MObjectArray fPolygonComponents;
	mStatus = fnMesh.getConnectedSetsAndMembers(
		instanceNumber,
		fPolygonSets,
		fPolygonComponents, 
		true);
	MCHECK_BEGIN << "Failed to get material proporties for one of the mesh objects." MCHECK_ERROR_RETURN_END_1

	// Loop through all the sets.  If the set is a polygonal set, find the
	// shader attached to the and print out the texture file name for the
	// set along with the polygons in the set.
	//
	bool oneMaterialAppliedToEntireMesh = false;
	bool materialsCorrupt = false;

	for (U32 i = 0; i < fPolygonSets.length(); i++) 
	{
		MObject set = fPolygonSets[i];
		MObject comp = fPolygonComponents[i];

		MFnSet fnSet(set, &mStatus);
		MCHECK_BEGIN << "Error aquiring MFnSet" MCHECK_ERROR_RETURN_END_1

		MStatus polygonIteratorStatus;
		// Make sure the set is a polygonal set.  If not, continue.
		MItMeshPolygon piter(shapeDagPath, comp, &polygonIteratorStatus);
		if ((polygonIteratorStatus == MS::kFailure) || comp.isNull()) 
		{
			// If no other materials have been added, then this material is applied to the 
			// entire mesh. If this is not the case then this material is invalid.
			if (fPolygonSets.length() == 1) 
				oneMaterialAppliedToEntireMesh = true;
			else 
				//Materials are assigned per face
				continue;
		}
		// At this point we should check the 'sanity' of the polygon iterator
		{
			S32 numItPolys = piter.count();
			S32 numMeshPolys = fnMesh.numPolygons();
			
			if( (numItPolys > numMeshPolys) || (numItPolys < 0) )
			{
				//Insane polygon interator, we cannot use this - skip.
				AFTK_DO_BEGIN << "Insane polygon iterator detected." AFTK_DO_WARNING_END;
				continue;
			}
		}
		// Find the texture that is applied to this set.  First, get the
		// shading node connected to the set.  Then, if there is an input
		// attribute called "color", search upstream from it for a texture
		// file node.
		//
		MObject shaderNode = C_MaterialHelper::FindShader(set, status);
		AFTK_CHECK_ERROR_RETURN;
		if (shaderNode == MObject::kNullObj)
			// Skip this material shader.
			continue;
				
		// Determine what type of shader node we have acquired, and create an AFTK material from it.
	
		// Source material shader
		MFnDependencyNode shaderDependencyNode = MFnDependencyNode(shaderNode);

		// Destination material
		const Aftk::C_SharedResource<Aftk::C_MaterialNode>* pAftkMaterialResource = 
			C_MaterialImport::Import(
			pImporter,
			shaderDependencyNode,
			pAftkScene,
			pAftkSceneMeshNode,
			status);
		AFTK_CHECK_ERROR_RETURN;
		#ifdef _DEBUG
		if (pAftkMaterialResource)
			AFTK_DO_BEGIN << "[" << pAftkMaterialResource->GetContent().GetName() << "]\n" AFTK_DO_DEBUG_END;
		#endif

		//Get the local index of this material in the mesh.
		S16 materialLocalIndex = -1;
		if (pAftkMaterialResource)
			materialLocalIndex =
				pAftkSceneMeshNode->AddPolygonMaterialMapping(pAftkMaterialResource);

		// If we've got here, then we have succesfully created the AFTK material
		// for this mesh, or set of mesh polygons.
		// Now add this set's material to the scene's material list
		U32 maxVerts;
		if (pOptions->triangulate == true) 
			maxVerts = 3;
		else
			maxVerts = pOptions->maxNGonNumVertices;

		if (oneMaterialAppliedToEntireMesh == true) 
		{
			for (U32 j = 0; j < pAftkSceneMeshNode->GetNumPolygons(0); j++) 
			{
				Aftk::C_Polygon* pAftkPolygon = &pAftkSceneMeshNode->GetPolygon(0,j);
				pAftkPolygon->SetMaterialMappingIndex(materialLocalIndex);
			}
			break; // out of outer for loop
		}
		else
		{
			// Loop thru the set of polygons that are contained in the current set.
			S32 itCount = 0;
			S32 numItPolys = piter.count();
			S32 numMeshPolys = fnMesh.numPolygons();
			
			for ( ; !piter.isDone(); piter.next() ) 
			{
				// Sanity check
				S32 currentPolyIndex = piter.index();
				if( (currentPolyIndex >= numMeshPolys) || (currentPolyIndex < 0) )
				{
					AFTK_DO_BEGIN << "Insane polygon iterator detected." AFTK_DO_WARNING_END;
					break;
				}

				//g_tcDebugMan << "PITER [" << itCount << "/" << numItPolys << "] IDX " << piter.index() << "\n";
				if (piter.index() != -1) 
				{
					for (U32 subPolyCount = 0; subPolyCount < (U32)pPolyMapping[piter.index()].Size(); subPolyCount++)
					{
						U32 aftkPolyIdx = pPolyMapping[piter.index()][subPolyCount];
						Aftk::C_Polygon* pAftkPolygon = &pAftkSceneMeshNode->GetPolygon(0,aftkPolyIdx);

						if (pAftkPolygon->GetMaterialMappingIndex() == -1)
						{
							pAftkPolygon->SetMaterialMappingIndex(materialLocalIndex);
						}
						else
						{
							materialsCorrupt = true;

							if (pOptions->strict)
							{
								//We have a corrupt material mapping system! This poly is assigned to more than
								//one material. The fix texture corruption script is needed.
								AFTK_DO_BEGIN << TRANS_MULTIMATERIAL_FAIL AFTK_DO_ERROR_RETURN_END_0;
							}
						}
					}
				}

				itCount++;
			}
		} // end else
	}

	if ((!pOptions->strict) && (materialsCorrupt))
	{
		AFTK_DO_BEGIN << TRANS_MULTIMATERIAL_WARN AFTK_DO_WARNING_END;
	}

	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_SceneMeshImport::BuildPolySetInfo(
	C_Importer* pImporter,
	const MDagPath& shapeDagPath,
	const MFnMesh& fnMesh,
	MObjectArray& localPolySetArray,
	MObjectArray& polySetComponentsArray,
	Ut::UtVector<Ut::DynamicVector<S32> >& localToScenePolySetIndexMapping)
{
	Aftk::C_Status status;
	MStatus mStatus;

	int instanceNum = 0;
	if (shapeDagPath.isInstanced()) 
		instanceNum = shapeDagPath.instanceNumber();

	// Get a list of all sets pertaining to the selected shape and the
	// members of those sets.
	//
	mStatus = fnMesh.getConnectedSetsAndMembers(
		instanceNum, 
		localPolySetArray, 
		polySetComponentsArray,
		false);
	MCHECK_ERROR_RETURN_1;

	const Ut::UtVector<Ut::DynamicVector<MObject> >& scenePolySetArray  =
		pImporter->GetPolySetArray();
	//Presize our mapping table.
	localToScenePolySetIndexMapping.Resize(localPolySetArray.length());

	for (U32 i = 0; i < localPolySetArray.length(); i++)
	{
		bool resolved = false;
		for (U32 j = 0; j < (U32)scenePolySetArray.Size(); j++)
		{
			if (localPolySetArray[i] == scenePolySetArray[j])
			{
				localToScenePolySetIndexMapping[i] = j;
				resolved = true;
				break;
			}
		}
		if (!resolved)
			localToScenePolySetIndexMapping[i] = -1;
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneMeshImport::ImportSetsAsBlindData(
	C_Importer* pImporter,
	const MDagPath& shapeDagPath,
	const MFnMesh& fnMesh,
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
	Aftk::C_Scene* pAftkScene,
	const C_ImportOptions* pOptions,
	Ut::UtDynamicVector<U32>* pPolyMapping,
	const MObjectArray& polySetArray,
	const MObjectArray& polySetComponentsArray,
	const Ut::UtVector<Ut::DynamicVector<S32> >& localToScenePolySetIndexMapping)
{
	U32 nPolys = pAftkSceneMeshNode->GetNumPolygons(0);
	if (nPolys == 0)
		return Aftk::C_Status::E_Status_Success;

	Aftk::C_Status status;
	MStatus mStatus;

	S32 localPolySetArrayLength = polySetArray.length();
	AFTK_ASSERT(localToScenePolySetIndexMapping.Size() == localPolySetArrayLength);
	if (localPolySetArrayLength > MAYA_IO_MAX_SETS_PER_POLY)
	{
		AFTK_DO_BEGIN << "There are > " << MAYA_IO_MAX_SETS_PER_POLY << " poly sets on mesh [" << pAftkSceneMeshNode->GetName() << "] this is not supported. " AFTK_DO_ERROR_RETURN_END_1;
	}

	MItMeshPolygon polyIter(shapeDagPath, MObject::kNullObj, &mStatus);
	MCHECK_BEGIN << "Error creating polygon iterator." MCHECK_ERROR_RETURN_END_1;

	S32 numMeshPolys = fnMesh.numPolygons();
	Ut::UtSet<U32> polySetCombination;
	MIntArray currElements;
	U32 n = 0;

	Ut::UtVector<Ut::DynamicVector< Ut::UtSet<S32> > > polySetComponentsSets;
	polySetComponentsSets.Resize(polySetComponentsArray.length());
	for (U32 iSet = 0; iSet < polySetComponentsArray.length(); iSet++) 
	{
#ifdef _DEBUG
		MFnSet fnSet(polySetArray[iSet], &mStatus);
		MCHECK_BEGIN << "Error aquiring MFnSet" MCHECK_ERROR_RETURN_END_1;
		std::string setName = fnSet.name().asChar();
#endif
		MFnSingleIndexedComponent currComp(polySetComponentsArray[iSet], &mStatus);
		if (!mStatus)
			continue;

		mStatus = currComp.getElements(currElements);
		MCHECK_BEGIN << "Failed to query single indexed component." MCHECK_ERROR_RETURN_END_1;
		
		Ut::UtSet<S32>& set = polySetComponentsSets[iSet];
		for (U32 kElement = 0; kElement < currElements.length(); kElement++)
			set.Insert(currElements[kElement]);
	}

	for ( ; !polyIter.isDone(); polyIter.next(), n++) 
	{
		// Sanity check
		S32 currentPolyIndex = polyIter.index();
		if( (currentPolyIndex >= numMeshPolys) || (currentPolyIndex < 0) )
		{
			AFTK_DO_BEGIN << "Insane polygon iterator detected." AFTK_DO_WARNING_END;
			break;
		}

		if (polyIter.index() == -1)
			continue;

		for (U32 subPolyCount = 0; subPolyCount < (U32)pPolyMapping[polyIter.index()].Size(); subPolyCount++)
		{
			U32 aftkPolyIdx = pPolyMapping[polyIter.index()][subPolyCount];
			C_PolySetMembership polySetMembership;
			memset(&polySetMembership, 0, sizeof(C_PolySetMembership));

			for (U32 iSet = 0; iSet < (U32)localPolySetArrayLength; iSet++) 
			{
				MFnSet fnSet(polySetArray[iSet], &mStatus);
				MCHECK_BEGIN << "Error aquiring MFnSet" MCHECK_ERROR_RETURN_END_1;

				std::string setName = fnSet.name().asChar();

				const Ut::UtSet<S32>& polySetComponentSet = polySetComponentsSets[iSet];

				if (polySetComponentSet.Size() == 0 ||
					polySetComponentSet.Find(currentPolyIndex) == polySetComponentSet.End() ||
					setName.find("polySet") == std::string::npos)
					continue;

//				Either Maya's buggy or I'm missing something here. The MFnSet.isMember function appears
//				to be bolloxed. Hence the horrid nested loop above.
//				bool whatMayaSays = fnSet.isMember(shapeDagPath, polyIter.polygon(), &mStatus);
//				if (!isMember)
//					continue;
//				std::cout << "poly set found: " << setName << std::endl;

				polySetMembership.setIndices[polySetMembership.nSets] = iSet;
				polySetMembership.nSets++;
			}

			//Build poly-set combination.
			polySetCombination.Clear();
			for (U32 jSet = 0; jSet < polySetMembership.nSets; jSet++)
			{
				S32 meshLocalPolySetIndex = polySetMembership.setIndices[jSet];
				S32 scenePolySetIndex = 
					localToScenePolySetIndexMapping[meshLocalPolySetIndex];
				if (scenePolySetIndex != -1)
					polySetCombination.Insert(scenePolySetIndex);
			}
			Aftk::C_Polygon& aftkPoly = pAftkSceneMeshNode->GetPolygon(aftkPolyIdx);

			const Aftk::C_BlindDataResource* pRes = 
				pImporter->GetPolySetCombinationBlindData(polySetCombination);
			if (pRes)
			{
				S16 bdLocalIndex = pAftkSceneMeshNode->AddPolygonBlindDataMapping(pRes);
				//Note that this will replace any legacy (non set based) poly-flags.
				aftkPoly.SetBlindDataMappingIndex(bdLocalIndex);
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

