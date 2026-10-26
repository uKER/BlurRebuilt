#include "StdAfx.h"

#include "../XsiIoUtility.h"
#include "XsiIoSceneMeshExport.h"
#include "XsiIoSceneExport.h"

using namespace XsiIo;

Aftk::C_Status C_SceneMeshExport::Export(
	XsiIo::C_SceneExport *pExporter,
	CSLModel *pXsiNode,
	const Aftk::C_Scene *pAftkScene,
	const Aftk::C_SceneNode *pAftkSceneNode)
{

	AFTK_ASSERT( pExporter );
	AFTK_ASSERT( pXsiNode );
	AFTK_ASSERT( pAftkScene );
	AFTK_ASSERT( pAftkSceneNode );

	Aftk::C_Status status;
	
	status = C_SceneNodeExport::Export( pExporter, pXsiNode, pAftkScene, pAftkSceneNode );


	CSLVisibility * visibility = pXsiNode->Visibility();
	visibility->SetVisibility( 1 );

	// Xsi mesh
	CSLMesh* pXsiMesh = (CSLMesh*) pXsiNode->Primitive();
	CSLBaseShape* pBaseShape = pXsiMesh->Shape();

	// Aftk mesh
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*) pAftkSceneNode;

	// Copy vertices into Xsi mesh
	{
		U32 nVertices = pAftkSceneMeshNode->GetNumVertexPositions();
		if( nVertices > 0 )
		{
			const Aftk::Position3Array& aftkVertices = pAftkSceneMeshNode->GetVertexPositionArray();
			CSLBaseShape::CSLVector3DArray* xsiVertices = pBaseShape->GetVertexList();

			U32 nAllocated = xsiVertices->Resize( nVertices );
			if( nAllocated < nVertices )
				return Aftk::C_Status::E_Status_OutOfMemory;

			for( U32 i=0; i<nVertices; i++ )
			{
				(*xsiVertices)[i].Set( aftkVertices[i].X(), aftkVertices[i].Y(), aftkVertices[i].Z() );
			}
		}
	}

	//Copy normals into Xsi mesh
	{
		U32 nNormals = pAftkSceneMeshNode->GetNumVertexNormals();
		if( nNormals > 0 )
		{
			const Aftk::Normal3Array aftkNormals = pAftkSceneMeshNode->GetVertexNormalArray();
			CSLBaseShape::CSLVector3DArray* xsiNormals = pBaseShape->CreateNormals();

			U32 nAllocated = xsiNormals->Resize( nNormals );
			if( nAllocated < nNormals )
				return Aftk::C_Status::E_Status_OutOfMemory;

			for( U32 i=0; i<nNormals; ++i )
			{
				(*xsiNormals)[i].Set( aftkNormals[i].X(), aftkNormals[i].Y(), aftkNormals[i].Z() );
			}
		}
	}

	// Copy UV sets into Xsi mesh
	{
		U32 nTexCoordSets = pAftkSceneMeshNode->GetNumTextureCoordinateSets();
		if( nTexCoordSets > 0 )
		{
			// > dotXsi v3.5
			CSLShape_35* pShape = (CSLShape_35 *)pBaseShape;

			for( U32 i=0; i<nTexCoordSets; ++i )
			{
				const Aftk::C_TextureCoordinateSet& aftkUVSet = pAftkSceneMeshNode->GetTextureCoordinateSet( i );
				const Aftk::TextureCoordinate4Array aftkUVArray = aftkUVSet.GetTextureCoordinateArray();
				U32 nTexCoords = aftkUVSet.GetNumTextureCoordinates();

				CSLUVCoordArray* xsiUVArray = pShape->AddUVCoordArray();
				xsiUVArray->SetTextureProjection( (char*) aftkUVSet.GetName().c_str() );

				CSLBaseShape::CSLVector2DArray* xsiUVList  = xsiUVArray->GetUVCoordList();
				U32 nAllocated = xsiUVList->Resize( nTexCoords );
				if( nAllocated < nTexCoords )
					return Aftk::C_Status::E_Status_OutOfMemory;

				for( U32 j=0; j<nTexCoords; ++j )
				{
					(*xsiUVList)[j].Set( aftkUVArray[j].X(), 1.0f-aftkUVArray[j].Y() );
				}
			}
		}
	}

	// Copy polygon clusters into Xsi mesh
	{
		U32 nClusters = pAftkSceneMeshNode->GetNumPolygonClusters();

		for( U32 i=0; i<nClusters; ++i )
		{
			const Aftk::C_PolygonCluster* pAftkPolyCluster = pAftkSceneMeshNode->GetPolygonClusterPtr( i );
			
			// C_SceneMeshNode::OneTimeInit() pushes an empty polygon cluster into collection.
			// Not used by Xsi importer as it adds its own.
			if( pAftkPolyCluster->GetNumLodLevels()==0 )
				continue;

			ExportPolygonCluster( pExporter, pXsiMesh, pAftkPolyCluster, pAftkSceneMeshNode );
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

// Redundant - using ExportPolygonCluster() instead...
Aftk::C_Status C_SceneMeshExport::ExportTriangleCluster(
	C_SceneExport* pExporter,
	CSLMesh* pXsiMesh,
	const Aftk::C_PolygonCluster* pAftkPolygonCluster,
	const Aftk::C_SceneMeshNode* pAftkSceneMeshNode)
{
	#if 0
	CSLTriangleList* pXsiTriangleList = pXsiMesh->AddTriangleList();
	U32 nPolygonIndices = pAftkPolygonCluster->GetNumPolygonIndices( 0 );		// LOD 0 ??
	const U32 numVertices = 3;

	pXsiTriangleList->SetName( (char*) pAftkPolygonCluster->GetName().c_str() );
	
	// Pre-allocate memory for ...
	// Vertex position indices
	CSLTriangleList::CSLTriangleArray* pXsiVertexArray = pXsiTriangleList->GetVertexIndices();
	pXsiVertexArray->Reserve( nPolygonIndices*numVertices );

	// Normal indices
	CSLTriangleList::CSLTriangleArray* pXsiNormalArray;
	if( pAftkSceneMeshNode->GetNumVertexNormals() )
	{
		pXsiNormalArray = pXsiTriangleList->CreateNormalIndices();
		if( !pXsiNormalArray )
			return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi normal indices");

		pXsiNormalArray->Resize( nPolygonIndices*numVertices );
	}

	// Texcoord sets
	if( U32 nUVSets=pAftkSceneMeshNode->GetNumTextureCoordinateSets() > 0 )
	{
		for( U32 i=0; i<nUVSets; ++i )
		{
			CSLTriangleList::CSLTriangleArray* pUVSet = pXsiTriangleList->AddUVArray();
			if( !pUVSet )
				return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi UV indices");

			pUVSet->Resize( nPolygonIndices*numVertices );
		}
	}

	// Copy data into xsi
	for( U32 nPolyIndex=0; nPolyIndex<nPolygonIndices; ++nPolyIndex )
	{
		U32 aftkPolyIndex = pAftkPolygonCluster->GetPolygonIndex( 0, nPolyIndex );
		const Aftk::C_Polygon& aftkPolygon = pAftkSceneMeshNode->GetPolygon( 0, aftkPolyIndex );

		AFTK_ASSERT( aftkPolygon.GetNumPolygonVertices() == numVertices );

		for( U32 nVertexIndex=0; nVertexIndex<numVertices; ++nVertexIndex )
		{
			const Aftk::C_PolygonVertex& aftkVertex = aftkPolygon.GetPolygonVertex( nVertexIndex );
			S32* pXsiIndexData;

			// Copy vertex position index into xsi
			pXsiIndexData = &(*pXsiVertexArray)[nPolyIndex*numVertices].m_iVtx1;
			pXsiIndexData += nVertexIndex;
			*pXsiIndexData = aftkVertex.GetPositionIndex();

			// Normals

			// UV sets
		}
	}
	#endif
	return Aftk::C_Status::E_Status_Success;
}


struct XsiClusterData
{
	CSLPolygonList *pXsiPolyList;
	CSLBaseMaterial *pBaseMaterial;
	U32	nPolygons;
	U32 nVertices;
	const Aftk::C_MaterialResource *pMappedMaterial;

	XsiClusterData()
		: pXsiPolyList(0), pBaseMaterial(0), nPolygons(0), nVertices(0), pMappedMaterial(0)
	{
	}
};


Aftk::C_Status C_SceneMeshExport::ExportPolygonCluster(
	C_SceneExport* pExporter,
	CSLMesh* pXsiMesh,
	const Aftk::C_PolygonCluster* pAftkPolygonCluster,
	const Aftk::C_SceneMeshNode* pAftkSceneMeshNode)
{
	U32 nMaterials = pExporter->GetAftkScene()->GetNumMaterialNodes();
	U32 nPolygonIndices = pAftkPolygonCluster->GetNumPolygonIndices( 0 );

	Ut::Vector< XsiClusterData > xsiPolygonClusters;
	xsiPolygonClusters.Resize( nMaterials + 1);	//Extra poly cluster for those with no assigned materials

	// Do an initial pass over the polygon cluster.
	// Xsi assigns materials per cluster, so we may need to split the cluster up.
	{
		for( U32 i=0; i<nPolygonIndices; i++ )
		{
			S32 aftkPolyIndex = pAftkPolygonCluster->GetPolygonIndex( 0, i );
			const Aftk::C_Polygon& aftkPolygon = pAftkSceneMeshNode->GetPolygon( 0, aftkPolyIndex );

			S16 materialMappingIndex = aftkPolygon.GetMaterialMappingIndex();
			xsiPolygonClusters[ materialMappingIndex+1 ].nPolygons++;
			xsiPolygonClusters[ materialMappingIndex+1 ].nVertices += aftkPolygon.GetNumPolygonVertices();

			if( materialMappingIndex != -1 )
			{
				xsiPolygonClusters[ materialMappingIndex+1 ].pMappedMaterial = 
				pAftkPolygonCluster->GetMeshNodePtr()->GetPolygonMaterialMappingPtr( materialMappingIndex );
			}
		}

		for( U32 i=0; i<nMaterials+1; ++i)
		{
			if( xsiPolygonClusters[ i ].nPolygons > 0 )
			{
				CSLPolygonList* pXsiPolygonList = pXsiMesh->AddPolygonList();
				if( !pXsiPolygonList )
					return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create polygon list" );

				std::string clusterName = pAftkSceneMeshNode->GetName() + "_" + pAftkPolygonCluster->GetName() + "_PolygonList";
				pXsiPolygonList->SetName( (char*) clusterName.c_str() );
				//pXsiPolygonList->SetName( (char*) pAftkPolygonCluster->GetName().c_str() );
				pXsiPolygonList->GetPolygonVertexCountList()->Resize( xsiPolygonClusters[ i ].nPolygons );

				xsiPolygonClusters[ i ].pXsiPolyList = pXsiPolygonList;

				if( i>0)
				{
					const Aftk::C_MaterialResource *pMaterialResource = xsiPolygonClusters[ i ].pMappedMaterial;
					if( pMaterialResource )
					{
						CSLBaseMaterial *pBaseMaterial;
						Aftk::C_Status status = pExporter->GetXsiMaterialFromAftk( pMaterialResource, &pBaseMaterial );
						xsiPolygonClusters[ i-1 ].pBaseMaterial = pBaseMaterial;
						pXsiPolygonList->SetMaterial( pBaseMaterial );

						#if 0
						if( !pXsiMesh->ParentModel()->GlobalMaterial() )
						{
							SI_Error error = pXsiMesh->ParentModel()->AddGlobalMaterial();
							if ( error )
								return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi global material" );

							pXsiMesh->ParentModel()->GlobalMaterial()->SetMaterial( pBaseMaterial );
							pXsiMesh->ParentModel()->GlobalMaterial()->SetPropagationType( CSLGlobalMaterial::SI_BRANCH );
						}
						#endif
					}
				}
			}
		}
	}

	// allocate polygon storage
	for( U32 i=0; i<nMaterials+1; ++i)
	{
		if( xsiPolygonClusters[i].pXsiPolyList )
		{
			CSLPolygonList::CSLIntArray* pPolyVertexCountList = xsiPolygonClusters[i].pXsiPolyList->GetPolygonVertexCountList();
			pPolyVertexCountList->Resize( xsiPolygonClusters[ i ].nPolygons );
		}
	}

	// Number of vertices in each poly
	{
		Ut::Vector< U32 > materialPolyCount;
		materialPolyCount.Resize( nMaterials+1 );

		for( U32 i=0; i<nMaterials+1; ++i)
		{
			materialPolyCount[i] = 0;
		}

		for( U32 i=0; i<nPolygonIndices; ++i )
		{
			S32 aftkPolyIndex = pAftkPolygonCluster->GetPolygonIndex( 0, i );
			const Aftk::C_Polygon& aftkPolygon = pAftkSceneMeshNode->GetPolygon( 0, aftkPolyIndex );

			U32 materialIndex = aftkPolygon.GetMaterialMappingIndex();

			CSLPolygonList::CSLIntArray * pPolyVertexCountList = 
				xsiPolygonClusters[ materialIndex+1 ].pXsiPolyList->GetPolygonVertexCountList();

			// Copy vertex count for this polygon into xsi
			U32 numVerticesThisPoly = aftkPolygon.GetNumPolygonVertices();
			(*pPolyVertexCountList)[ materialPolyCount[ materialIndex+1 ] ] = numVerticesThisPoly;

			materialPolyCount[ materialIndex+1 ]++;
		}
	}


	for( U32 i=0; i<nMaterials+1; ++i )
	{
		CSLPolygonList *pXsiPolygonList = xsiPolygonClusters[ i ].pXsiPolyList;
		if( pXsiPolygonList )
		{
			// Vertex position indices
			CSLPolygonList::CSLIntArray* pXsiVertexArray = pXsiPolygonList->GetVertexIndices();
			pXsiVertexArray->Resize( xsiPolygonClusters[ i ].nVertices );

			// Normal indices
			if( pAftkSceneMeshNode->GetNumVertexNormals() )
			{
				CSLPolygonList::CSLIntArray* pXsiNormalArray = pXsiPolygonList->CreateNormalIndices();
				if( !pXsiNormalArray )
					return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi normal indices");

				pXsiNormalArray->Resize( xsiPolygonClusters[ i ].nVertices );
			}

			// Texcoord sets
			U32 nUVSets=pAftkSceneMeshNode->GetNumTextureCoordinateSets();
			if( nUVSets > 0 )
			{
				for( U32 j=0; j<nUVSets; ++j )
				{
					CSLPolygonList::CSLIntArray* pUVSet = pXsiPolygonList->AddUVArray();
					if( !pUVSet )
						return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi UV indices");

					pUVSet->Resize( xsiPolygonClusters[ i ].nVertices );
				}
			}
		}
	}


	// Copy index data into Xsi polygon list reversing the winding order (mirroring the import code)
	{

		Ut::Vector< U32 > vertexCount;
		vertexCount.Resize( nMaterials+1 );

		for( U32 i=0; i<nMaterials+1; ++i)
		{
			vertexCount[i] = 0;
		}

		for( U32 i=0; i<nPolygonIndices; ++i )
		{
			U32 aftkPolyIndex = pAftkPolygonCluster->GetPolygonIndex( 0, i );
			const Aftk::C_Polygon& aftkPolygon = pAftkSceneMeshNode->GetPolygon( 0, aftkPolyIndex );

			U32 nVertices = aftkPolygon.GetNumPolygonVertices();

			S32 materialIndex = aftkPolygon.GetMaterialMappingIndex();
			CSLPolygonList *pXsiPolygonList = xsiPolygonClusters[ materialIndex+1 ].pXsiPolyList;

			CSLPolygonList::CSLIntArray *pXsiVertexArray = pXsiPolygonList->GetVertexIndices();
			CSLPolygonList::CSLIntArray *pXsiNormalArray = pXsiPolygonList->GetNormalIndices();

			vertexCount[ materialIndex+1 ] += nVertices;

			U32 v = vertexCount[ materialIndex+1 ];

			for( U32 j=0; j<nVertices; ++j)
			{
				const Aftk::C_PolygonVertex& aftkVertex = aftkPolygon.GetPolygonVertex( j );

				// Vertex positions
				(*pXsiVertexArray)[vertexCount[ materialIndex+1 ]-j-1] = aftkVertex.GetPositionIndex();

				// Vertex normals
				(*pXsiNormalArray)[vertexCount[ materialIndex+1 ]-j-1] = aftkVertex.GetNormalIndex();

				// UV Sets
				U32 nUVSets = pAftkSceneMeshNode->GetNumTextureCoordinateSets();
				for( U32 k=0; k<nUVSets; ++k )
				{
					U32 i = pXsiPolygonList->GetUVArrayCount();
					CSLPolygonList::CSLIntArray* pXsiUVArray = pXsiPolygonList->GetUVIndices( k );
					if( !pXsiUVArray )
						return Aftk::C_Status( Aftk::C_Status::E_Status_Failure, "Failed to get Xsi UV index array");

					(*pXsiUVArray)[vertexCount[ materialIndex+1 ]-j-1] = aftkVertex.GetTextureCoordinateIndex( k );
				}
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}