#include "stdafx.h"
#include "SkinnedMesh.h"
#include <animation/skeleton/Skeleton.h>
#include <animation/tools/Scene.h>
#include <system/debug/I_DebugRender.h>
#include <system/Perf.h>
#include <system/memory/MemoryTracker.h>
#include "shaders/psConstants.h"
#include <aftkut/AftkUt.h>
#include <system/misc/Progress.h>
#include <display2/Utils/Utils.h>
#include <algorithm>

#define kSkinnedMeshMemoryTag	"mercury.animation.debug.skinnedmesh"

static Debug::Logger& gLog = Debug::GetLogger( "mercury.animation.skinnedmesh" );


C_SkinnedMesh::C_SkinnedMesh():
	m_skinnedBufferIndex( 0 ),
	m_skeleton( 0 ),
	m_numVertices( 0 ),
	m_numTriangleIndices( 0 )
{
}

C_SkinnedMesh::~C_SkinnedMesh()
{
	DestroyIndexBuffer();
	DestroyVertexBuffers();
	DestroyMaterials();
}

struct C_MaterialDef
{

	void Import( const Aftk::C_MaterialResource* m )
	{
		if (m == NULL)
		{
			name = "null";
			diffuseTextureName.Clear();
			normalMapTextureName.Clear();
			return;
		}
		const Aftk::C_MaterialNode& mat = m->GetContent();
		
		name = mat.GetName().c_str();

		diffuseTextureName.Clear();
		
		for ( U32 i = 0; i < mat.GetNumShaderImageNodes(); ++i )
		{
			const Aftk::C_ShaderImageNode* imgNode = mat.GetShaderImageNodePtr( i );
			if ( !imgNode ) continue;
						
			std::string filename;
			imgNode->GetFilePlug().GetValue( filename );

			if (filename.rfind("_DIFF") != std::string::npos)
			{
				// definitely diffuse.
				diffuseTextureName = filename.c_str();
				break;
			}
			else if (diffuseTextureName.Empty())
			{
				// might be diffuse - keep searching though
				diffuseTextureName = filename.c_str();
			}
		}
	
	}
	
	Btl::String	name;
	Btl::String	diffuseTextureName;
	Btl::String	normalMapTextureName;
};

bool C_SkinnedMesh::Create(
	Aftk::C_Scene& aftkScene,
	const AnimationTools::C_Scene& scene,
	const Animation::C_Skeleton& skeleton,
	Progress::I_ProgressBar* progressBar
)
{
	MEMORY_TRACKER_PUSH( kSkinnedMeshMemoryTag );
	
	Progress::UpdateStepped step( progressBar, 2, "Building Mesh..." );

	m_skeleton = &skeleton;
	
	const U32 numJoints = scene.GetNumNodes();

	Btl::Vector< C_MaterialDef > materials;
	Btl::Vector<C_TriangleList> triangleLists;

	Btl::Vector< U32 > triangleIndices;
	
	{
		Progress::UpdateSimple meshProg( &step, "Processing Mesh..." );
		
		m_jointVertexWeights.Destroy();
		m_jointVertexWeights.Resize( numJoints );
		m_vertices.Clear();
		
		// Clear existing data.
		DestroyIndexBuffer();
		DestroyVertexBuffers();
		DestroyMaterials();
			
		typedef Btl::Map<Btl::String, U32> C_MaterialMap;

		C_MaterialMap materialMap;

		
		S32 currentMaterialIndex = -1;
		
		Btl::Vector<Btl::Vector<U32> > positionIndexToVertexArrayMap;
		
			
		// Import all meshes in aftkScene.
		Aftk::C_MeshWeldMemoryManager mgr;
		
		
		U32 nodeCount = aftkScene.GetNumSceneNodes();
		
		for ( Aftk::C_SceneNodeIterator nodeIter( &aftkScene ); nodeIter; ++nodeIter )
		{
			meshProg.SetProgress( 100.f - ((100.f * nodeCount) / aftkScene.GetNumSceneNodes()) );
			--nodeCount;
			
			const Aftk::C_SceneNode* node = nodeIter;
			const Aftk::C_SceneNode* sourceNode = node;
			
			if (node->GetType() == Aftk::E_ObjType_SceneInstance)
			{
				const Aftk::C_SceneInstanceNode* instanceNode = static_cast<const Aftk::C_SceneInstanceNode*>(node);
				sourceNode = instanceNode->GetSourceNodePtr();
			}
			
			if (sourceNode->GetType() != Aftk::E_ObjType_SceneMesh )
			{
				continue;
			}
			
			const Aftk::C_SceneMeshNode* meshNode = static_cast<const Aftk::C_SceneMeshNode*>(sourceNode);

	#if 0
			// For cutscenes, import everything.		
			if ( !meshNode->IsSkinned() )
			{
				// Ignore non-skinned meshes.
				continue;
			}
	#endif
	
			// Remove z-flip from bind pose and apply.
			Aftk::Matrix44 basePose;
			if (meshNode->IsSkinned())
			{
				basePose = node->GetBasePose();
				AnimationTools::RemoveAftkMatrixZFlip( basePose );
			}
			else
			{
				UtMaths::SetIdentity(&basePose);
			}
			
			#ifdef ENABLE_NORMAL_MAPS		
			if ( meshNode->GetNumVertexTangents(0) == 0 )
			{
				Aftk::C_MeshCalculateTangents::CalculateTangents( meshNode, true, true );
			}
			#endif

			//
			// Import vertex positions.
			//
			positionIndexToVertexArrayMap.Clear();	
			positionIndexToVertexArrayMap.Resize( meshNode->GetNumVertexPositions() );
			
			const U32 meshVertexOffset = m_vertices.Size();
			
			//
			// Import vertices
			// 
			const U32 numVertices = meshNode->GetNumPolygonVertices();
			m_vertices.Resize( meshVertexOffset + numVertices );
			
			for ( U32 i = 0; i < numVertices; ++i )
			{
				const Aftk::C_PolygonVertex* polyVertex = &meshNode->GetPolygonVertex(i);
			
				SkinnedVertex& v = m_vertices[ meshVertexOffset + i ];
				positionIndexToVertexArrayMap[ polyVertex->GetPositionIndex() ].PushBack( meshVertexOffset + i );

				Aftk::Position3 src = meshNode->GetVertexPosition( polyVertex->GetPositionIndex() );
				src = Aftk::ApplyMatrix(basePose, src);
				
				Maths::HVec3 srcInGameSpace = scene.GetSceneToGameTransform().ToGameSpace( Maths::HVec3( &src.x ) );
				
				v.m_position[ 0 ] = srcInGameSpace.X();
				v.m_position[ 1 ] = srcInGameSpace.Y();
				v.m_position[ 2 ] = srcInGameSpace.Z();
				
				if ( polyVertex->GetNumTextureCoordinateIndices() > 0 )
				{
					S32 texCoordIndex = polyVertex->GetTextureCoordinateIndex(0);
					if ( texCoordIndex >=0 )
					{
						v.m_texCoord[0] = meshNode->GetTextureCoordinate( 0, texCoordIndex ).x;
						v.m_texCoord[1] = meshNode->GetTextureCoordinate( 0, texCoordIndex ).y;
					}
					else
					{
						v.m_texCoord[0] = v.m_texCoord[1] = 0.f;
					}
					
					#ifdef ENABLE_NORMAL_MAPS		
					S32 tangentIndex = polyVertex->GetTangentIndex(0);
					if ( tangentIndex >= 0 )
					{
						// Set tangent.
						Aftk::Vector3F32 t = meshNode->GetVertexTangent( 0, tangentIndex );
						t = Aftk::ApplyMatrix(basePose, t);
						Maths::HVec3 normal = scene.GetSceneToGameTransform().NormalToGameSpace( Maths::HVec3( &t.x ) );
						v.m_tangent[0] = normal.X();
						v.m_tangent[1] = normal.Y();
						v.m_tangent[2] = normal.Z();
						
					}
					
					S32 binormalIndex = polyVertex->GetBinormalIndex(0);
					if ( binormalIndex >= 0 )
					{
						// Set binormal.
						Aftk::Vector3F32 b = meshNode->GetVertexBinormal( 0, binormalIndex );
						b = Aftk::ApplyMatrix(basePose, b);
						Maths::HVec3 normal = scene.GetSceneToGameTransform().NormalToGameSpace( Maths::HVec3( &b.x ) );
						v.m_binormal[0] = normal.X();
						v.m_binormal[1] = normal.Y();
						v.m_binormal[2] = normal.Z();
					}
					#endif
				}
				else
				{
					v.m_texCoord[0] = v.m_texCoord[1] = 0.f;
				}
				
				Aftk::Normal3 n = meshNode->GetVertexNormal( polyVertex->GetNormalIndex() );
				n = Aftk::ApplyMatrix(basePose, n);
				Maths::HVec3 normal = scene.GetSceneToGameTransform().NormalToGameSpace( Maths::HVec3( &n.x ) );
				v.m_normal[0] = normal.X();
				v.m_normal[1] = normal.Y();
				v.m_normal[2] = normal.Z();
			}
			
		
			//
			// Import triangles and vertices.
			//
			static const U32 lodLevel = 0;
			U32 numPolys = meshNode->GetNumPolygons( lodLevel );
			
			static const U32 kWinding[ 2 ][ 3 ] =
			{
				{ 0, 2, 1 },
				{ 0, 1, 2 }
			};

			U32 sceneFlip = scene.GetSceneToGameTransform().HasReflection() ? 1 : 0;
			U32 basePoseFlip = UtMaths::GetDeterminant4x4(basePose) < 0.f ? 1 : 0;

			const U32* triWinding = kWinding[ sceneFlip ^ basePoseFlip ];

			U32 numMaterials = meshNode->GetNumMaterialNodes(0);
			U32 numMaterialMappings = meshNode->GetNumPolygonMaterialMappings();
			
			C_MaterialDef materialDef;
					
			for ( U32 i = 0; i < numPolys; ++i )
			{
				const Aftk::C_Polygon& poly = meshNode->GetPolygon( lodLevel, i );
				
				for ( U32 tri = 0; tri < poly.GetNumTriangles(); ++tri )
				{
					// Import triangle material.
					materialDef.Import( meshNode->GetPolygonMaterialMappingPtr( poly.GetMaterialMappingIndex() ) );
						
					C_MaterialMap::Iterator materialIt = materialMap.Find( materialDef.name );
					S32 materialIndex = 0;
					if ( materialIt != materialMap.End() )
					{
						materialIndex = materialIt->GetValue();
					}
					else
					{
						LOGINFO( gLog, "Found material '%s', diffuse map: '%s'\n", materialDef.name.Chars(), materialDef.diffuseTextureName.Chars() );
						materialIndex = materials.Size();
						materials.PushBack( materialDef );
						materialMap.Insert( materialDef.name, materialIndex );
					}

					if ( materialIndex != currentMaterialIndex )
					{
						triangleLists.PushBack( C_TriangleList( triangleIndices.Size(), 0, materialIndex ) );
						currentMaterialIndex = materialIndex;
					}
					
					triangleLists.Back().size++;
				
					U32 polyVertexIndices[3];
					poly.GetTrianglePolygonVertexIndices( tri, polyVertexIndices );
					
					triangleIndices.PushBack( meshVertexOffset + polyVertexIndices[triWinding[0]] );
					triangleIndices.PushBack( meshVertexOffset + polyVertexIndices[triWinding[1]] );
					triangleIndices.PushBack( meshVertexOffset + polyVertexIndices[triWinding[2]] );
				}
			}
			
			LOGINFO( gLog, "Mesh has %d vertices (total %d)", m_vertices.Size() - meshVertexOffset, m_vertices.Size());
			
		
			//
			// Build lists of vertex weights per joint.
			//
			
			if ( meshNode->IsSkinned() )
			{
				U32 numInfluenceClusters = meshNode->GetNumInfluenceClusters();
			
				for ( U32 i = 0; i < numInfluenceClusters; ++i )
				{
					const Aftk::C_InfluenceClusterNode* influenceClusterNode = meshNode->GetInfluenceClusterPtr( i );
									
					U32 numEnvelopes = influenceClusterNode->GetNumEnvelopes();
					
					for ( U32 envIndex = 0; envIndex < numEnvelopes; ++envIndex )
					{
						const Aftk::C_WeightEnvelope& weightEnvelope = influenceClusterNode->GetEnvelope( envIndex );
						
						const Aftk::C_SceneNode* jointPtr = weightEnvelope.GetInfluenceNodePtr();
						if ( !jointPtr )
							continue;
				
						// Attempt to replace influence node with an ancestor that is actually a joint.
						S32 jointIndex = scene.GetNodeIndexByAftkSceneNode( jointPtr );
						
						if (jointIndex < 0)
						{
							LOGWARN(gLog, "Mesh is weighted to joint '%s' which is not present in skeleton", jointPtr->GetName().c_str());
							jointIndex = scene.GetNodeIndexByNearestAftkSceneNode(jointPtr );
						}
												
						if ( jointIndex < 0 )
						{
							// No joints.
							continue;
						}
						
						for ( U32 weightIndex = 0; weightIndex < weightEnvelope.GetNumWeights(); ++weightIndex )
						{
							Aftk::C_Weight< F32 > weight;
							weightEnvelope.GetWeight( weightIndex, weight );
							
							// convert position index to a vertex index.
							const Btl::Vector<U32>& weightVerts = positionIndexToVertexArrayMap[ weight.GetIndex() ];
							
							for ( S32 k = 0; k < weightVerts.Size(); ++k )
							{
								VertexWeight vw;
								vw.m_vertexIndex = weightVerts[k];
								vw.m_weight = weight.GetWeight();
								m_jointVertexWeights[ jointIndex ].PushBack( vw );
							}
						}
					}
				}
			}
			else
			{
				// Mesh isn't skinned... is it a rigid mesh directly connected to one of our joints?
				S32 jointIndex = scene.GetNodeIndexByAftkSceneNode( node );
				
				if ( jointIndex >= 0 )
				{
					// All vertices for this mesh are influenced 100% by this joint.
					for ( U32 meshVertexIndex = 0; meshVertexIndex < numVertices; ++meshVertexIndex )
					{
						VertexWeight vw;
						vw.m_vertexIndex = meshVertexIndex + meshVertexOffset;
						vw.m_weight = 1.f;
						m_jointVertexWeights[ jointIndex ].PushBack( vw );
					}
				}
				
			}
		}
		
		const U32 totalVertices = m_vertices.Size();
		
		// Normalise weights per vertex.
		Btl::Vector< F32 > totalWeightPerVertex;
		totalWeightPerVertex.Resize( totalVertices );
		
		for ( U32 i = 0; i < totalVertices; ++i )
		{
			totalWeightPerVertex[ i ] = 0.f;
		}
		
		for ( U32 jointIndex = 0; jointIndex < scene.GetNumNodes(); ++jointIndex )
		{
			const U32 numWeights = m_jointVertexWeights[ jointIndex ].Size();
			for ( U32 weightIndex = 0; weightIndex < numWeights; ++weightIndex )
			{
				const VertexWeight& vertexWeight = m_jointVertexWeights[ jointIndex ][ weightIndex ];
				
				totalWeightPerVertex[ vertexWeight.m_vertexIndex ] += vertexWeight.m_weight;
			}
		}

		for ( U32 jointIndex = 0; jointIndex < scene.GetNumNodes(); ++jointIndex )
		{
			const U32 numWeights = m_jointVertexWeights[ jointIndex ].Size();
			for ( U32 weightIndex = 0; weightIndex < numWeights; ++weightIndex )
			{
				VertexWeight& vertexWeight = m_jointVertexWeights[ jointIndex ][ weightIndex ];
				
				vertexWeight.m_weight *= Maths::Reciprocal( totalWeightPerVertex[ vertexWeight.m_vertexIndex ] );
			}
		}
		
		CreateVertexBuffers( totalVertices );
		
	}
	
	step.Step();
	
	{
		Progress::UpdateSimple materialProg( &step, "Loading textures..." );
			
		// Create materials, loading textures in the process.
		m_materials.Clear();
		m_materials.Resize( materials.Size() );
		for ( S32 i = 0; i < materials.Size(); ++i )
		{
			materialProg.SetProgress( (100.f * i) / m_materials.Size() );
		
			Btl::String path = materials[i].diffuseTextureName;
			m_materials[i].hasDiffuseTexture = Display2::D3DXCreateTexture2DFromFile( path.Chars(), &m_materials[i].diffuseTexture );
			
			#ifdef ENABLE_NORMAL_MAPS		
			// Create normal map name.
			Btl::String normalMapPath = path.IReplace( "A0_", "N0_" );
			m_materials[i].hasNormalMapTexture = Display::CreateTexture2DFromFile( normalMapPath.Chars(), &m_materials[i].normalMapTexture );
			#else
			m_materials[i].hasNormalMapTexture = false;
			#endif
			
			m_materials[i].hasAlpha = false;
			
			if ( m_materials[i].hasDiffuseTexture )
			{
				Display::E_Format format = static_cast<Display::E_Format>( m_materials[i].diffuseTexture.GetFormat() );
				bool hasAlpha = false;
				switch ( format )
				{
					case Display::E_Format_DXT3:
					case Display::E_Format_DXT5:
					case Display::E_Format_A8:
					case Display::E_Format_A8L8:
					case Display::E_Format_A8R8G8B8:
					case Display::E_Format_A16B16G16R16:
					case Display::E_Format_A16B16G16R16F:
					case Display::E_Format_A2R10G10B10:
					case Display::E_Format_A2B10G10R10F:
						hasAlpha = true;
						break;
					
					default:
						break;
				}
				
				m_materials[i].hasAlpha = hasAlpha;
				
				LOGINFO( gLog, "Texture '%s' %s alpha", path.Chars(), hasAlpha ? "has" : "does not have" );
			}
		}
		
		// Generate opaque and alpha triangle lists.
		m_opaqueTriangleLists.Clear();
		m_alphaTriangleLists.Clear();
		for ( S32 i = 0; i < triangleLists.Size(); ++i )
		{
			const C_TriangleList& triList = triangleLists[i];
		
			if ( triangleLists[i].materialIndex >= 0 )
			{
				if ( m_materials[triList.materialIndex].hasAlpha )
				{
					m_alphaTriangleLists.PushBack( triList );
				}
				else
				{
					m_opaqueTriangleLists.PushBack( triList );
				}
			}
			else
			{
				// No material, assume opaque.
				m_opaqueTriangleLists.PushBack( triList );
			}
		}
		
		// Created sorted alpha triangle lists.
		// Do this by creating triangle lists containing a single triangle, sorting this list and generating
		// a final list with the minimal number of state switches.
		
		
		// Create sortable list of triangles.
		Btl::Vector<C_TriangleList> alphaTriangleList;
		
		for (S32 i = 0; i < m_alphaTriangleLists.Size(); ++i)
		{
			const C_TriangleList& triList = m_alphaTriangleLists[i];
			for (U32 j = 0; j < triList.size; ++j)
			{
				alphaTriangleList.PushBack(C_TriangleList(triList.firstIndex + j * 3, 1, triList.materialIndex));
			}
		}
		
		static const MathsStorage::Float3 kViewDirections[E_ViewDirection_Count] =
		{
			MathsStorage::Float3(-1.f, 0.f, 0.f),
			MathsStorage::Float3(+1.f, 0.f, 0.f),
			MathsStorage::Float3(0.f, -1.f, 0.f),
			MathsStorage::Float3(0.f, +1.f, 0.f),
			MathsStorage::Float3(0.f, 0.f, -1.f),
			MathsStorage::Float3(0.f, 0.f, +1.f)
		};
				
		for (S32 i = 0; i < E_ViewDirection_Count; ++i)
		{
			m_sortedAlphaTriangleLists[i].Clear();
			
			// Sort alpha triangle list by depth along view direction axis.
			std::sort(
				alphaTriangleList.Begin(),
				alphaTriangleList.End(),
				C_SortAlongViewDirection(
					triangleIndices,
					m_vertices,
					Maths::Load<Maths::HVec3, MathsStorage::Float3>(&kViewDirections[i])
				)
			);
			
			// Merge triangle lists with same material together and generate new indices.
			Btl::Vector<C_TriangleList>& sortedTriangleLists = m_sortedAlphaTriangleLists[i];
			
			S32 prevMaterialIndex = -1;
			for (S32 j = 0; j < alphaTriangleList.Size(); ++j)
			{
				if (alphaTriangleList[j].materialIndex != prevMaterialIndex)
				{
					sortedTriangleLists.PushBack();
					sortedTriangleLists.Back().firstIndex = triangleIndices.Size();
					sortedTriangleLists.Back().size = 0;
					sortedTriangleLists.Back().materialIndex = alphaTriangleList[j].materialIndex;
					
					prevMaterialIndex = alphaTriangleList[j].materialIndex;
				}
				
				// Add an extra triangle
				sortedTriangleLists.Back().size++;
				
				// Add indices.
				for (S32 k = 0; k < 3; ++k)
				{
					S32 sourceIndex = alphaTriangleList[j].firstIndex + k;
					triangleIndices.PushBack(triangleIndices[sourceIndex]);
				}
			}
		}
	}
	
	// Create the master index buffer.
	CreateIndexBuffer( triangleIndices.Size(), triangleIndices.Size() > 0 ? &triangleIndices[ 0 ] : NULL );
	
	step.Step();

	MEMORY_TRACKER_POP( kSkinnedMeshMemoryTag );

	return true;
}

void C_SkinnedMesh::Destroy()
{
	DestroyVertexBuffers();
	DestroyIndexBuffer();
	DestroyMaterials();
	
	m_materials.Clear();
	
	m_vertices.Clear();
	
	m_numVertices = 0;
	m_numTriangleIndices = 0;
	m_jointVertexWeights.Destroy();
	m_skeleton = 0;
}

bool C_SkinnedMesh::IsValid() const
{
	return m_skeleton && m_numVertices > 0 && m_numTriangleIndices > 0;
}

void C_SkinnedMesh::CalculateSkinnedVertices( const Maths::Mat43* jointWorldMatrices )
{
	// Transform pose into matrices...
	// Pre-multiply by inverse world base pose joint matrices. Transforms
	// mesh local vertices into joint space.
	
	PERF_EVENT( "SkinnedMesh.CalculateSkinnedVertices" );
	
	if ( !GetNumVertices() )
	{
		return;
	}
	
	U32 numJoints = m_skeleton->GetNumJoints();
	U32 numVertices = GetNumVertices();
	
	m_skinnedBufferIndex ^= 1;
	
	Display::C_VertexBuffer& destVertexBuffer = m_skinnedVertices[ m_skinnedBufferIndex & 1 ];
	
	SkinnedVertex* skinnedVerts = static_cast<SkinnedVertex*>( destVertexBuffer.Lock() );
	
#if 0
	// No skinning - just copy vertices into vertex buffer
	for ( U32 i = 0; i < numVertices; ++i )
	{
		SkinnedVertex& v = skinnedVerts[ i ];
		SkinnedVertex& v2 = m_vertices[ i ];
		v.m_position[ 0 ] = v2.m_position[ 0 ];
		v.m_position[ 1 ] = v2.m_position[ 1 ];
		v.m_position[ 2 ] = v2.m_position[ 2 ];
		v.m_normal[ 0 ] = v2.m_normal[ 0 ];
		v.m_normal[ 1 ] = v2.m_normal[ 1 ];
		v.m_normal[ 2 ] = v2.m_normal[ 2 ];
		
		#ifdef ENABLE_NORMAL_MAPS
		v.m_binormal[ 0 ] = v2.m_binormal[ 0 ];
		v.m_binormal[ 1 ] = v2.m_binormal[ 1 ];
		v.m_binormal[ 2 ] = v2.m_binormal[ 2 ];

		v.m_tangent[ 0 ] = v2.m_tangent[ 0 ];
		v.m_tangent[ 1 ] = v2.m_tangent[ 1 ];
		v.m_tangent[ 2 ] = v2.m_tangent[ 2 ];
		#endif

		v.m_texCoord[ 0 ] = v2.m_texCoord[0];
		v.m_texCoord[ 1 ] = v2.m_texCoord[1];		
	}
#else	
	// Clear skinned vertex positions.
	for ( U32 i = 0; i < numVertices; ++i )
	{
		SkinnedVertex& v = skinnedVerts[ i ];
		v.m_position[ 0 ] = 0.f;
		v.m_position[ 1 ] = 0.f;
		v.m_position[ 2 ] = 0.f;
		v.m_normal[ 0 ] = 0.f;
		v.m_normal[ 1 ] = 0.f;
		v.m_normal[ 2 ] = 0.f;
		#ifdef ENABLE_NORMAL_MAPS
		v.m_binormal[ 0 ] = 0.f;
		v.m_binormal[ 1 ] = 0.f;
		v.m_binormal[ 2 ] = 0.f;
		v.m_tangent[ 0 ] = 0.f;
		v.m_tangent[ 1 ] = 0.f;
		v.m_tangent[ 2 ] = 0.f;
		#endif

		v.m_texCoord[ 0 ] = m_vertices[ i ].m_texCoord[0];
		v.m_texCoord[ 1 ] = m_vertices[ i ].m_texCoord[1];
	}
		
	for ( U32 i = 0; i < numJoints; ++i )
	{
		const Maths::Mat43 jointWorldMatrix = jointWorldMatrices[ i ];
		const Maths::Mat43 invBasePoseMatrix = m_skeleton->GetInverseWorldBasePoseMatrix( i );
		const Maths::Mat43 jointMatrix = invBasePoseMatrix * jointWorldMatrix;
				
		for ( U32 j = 0, count = m_jointVertexWeights[ i ].Size(); j < count; ++j )
		{
			const VertexWeight& vw = ( m_jointVertexWeights[ i ] )[ j ];
			
			// Read model space vertex position.
			const Maths::Point3 modelPos = m_vertices[ vw.m_vertexIndex ].GetPosition();
			const Maths::Vector3 modelNormal = m_vertices[ vw.m_vertexIndex ].GetNormal();
			#ifdef ENABLE_NORMAL_MAPS
			const Maths::Vector3 modelTangent = m_vertices[ vw.m_vertexIndex ].GetTangent();
			const Maths::Vector3 modelBinormal = m_vertices[ vw.m_vertexIndex ].GetBinormal();
			#endif
			
			// *** Reading from vertex buffer memory - could be very slow
			const Maths::Point3 prevWorldPos = skinnedVerts[ vw.m_vertexIndex ].GetPosition();
			const Maths::Vector3 prevWorldNormal = skinnedVerts[ vw.m_vertexIndex ].GetNormal();
			#ifdef ENABLE_NORMAL_MAPS
			const Maths::Vector3 prevWorldTangent = skinnedVerts[ vw.m_vertexIndex ].GetTangent();
			const Maths::Vector3 prevWorldBinormal = skinnedVerts[ vw.m_vertexIndex ].GetBinormal();
			#endif
			
			// Transform.
			Maths::Point3 worldPos = modelPos * jointMatrix;
			Maths::Vector3 worldNormal = modelNormal * jointMatrix;	// TODO: inverse transpose? won't play nice with scaling
			#ifdef ENABLE_NORMAL_MAPS
			Maths::Vector3 worldTangent = modelTangent * jointMatrix;
			Maths::Vector3 worldBinormal = modelNormal * jointMatrix;
			#endif
						
			// Weighted add.
			worldPos = prevWorldPos + worldPos * vw.m_weight;
			worldNormal = Maths::MulAdd( worldNormal, Maths::MakeVF32( vw.m_weight ), prevWorldNormal );
			#ifdef ENABLE_NORMAL_MAPS
			worldTangent = Maths::MulAdd( worldTangent, Maths::MakeVF32( vw.m_weight ), prevWorldTangent );
			worldBinormal = Maths::MulAdd( worldBinormal, Maths::MakeVF32( vw.m_weight ), prevWorldBinormal );
			#endif
			
			skinnedVerts[ vw.m_vertexIndex ].SetPosition( worldPos );
			skinnedVerts[ vw.m_vertexIndex ].SetNormal( worldNormal );
			
			#ifdef ENABLE_NORMAL_MAPS
			skinnedVerts[ vw.m_vertexIndex ].SetTangent( worldTangent );
			skinnedVerts[ vw.m_vertexIndex ].SetBinormal( worldBinormal );
			#endif
		}
	}
#endif
	
	destVertexBuffer.Unlock();
}


bool C_SkinnedMesh::CreateVertexBuffers(
	U32 numVertices
)
{
	m_numVertices = numVertices;
	
	if ( m_numVertices == 0 )
	{
		return false;
	}
	
	if ( !Display::DeviceExists() )
	{
		return false;
	}

	const U32 skinnedBufferSize = sizeof( SkinnedVertex ) * numVertices;
	const U32 skinnedBufferFlags = Display::E_Usage_Managed; // | Display::E_Usage_Dynamic;
	
	for ( U32 i = 0; i < sizeof( m_skinnedVertices ) / sizeof( m_skinnedVertices[ 0 ] ); ++i )
	{
		if ( !Display::Device()->CreateVertexBuffer(
			skinnedBufferSize,
			skinnedBufferFlags,
			&m_skinnedVertices[ i ] ))
		{
			return false;
		}
	}
	
	m_skinnedBufferIndex = 1;
	
	return true;
}


void C_SkinnedMesh::DestroyVertexBuffers()
{

	if ( m_skinnedVertices[ 0 ].IsCreated() )
	{
		Display::Device()->DestroyVertexBuffer( &m_skinnedVertices[ 0 ] );
	}
	if ( m_skinnedVertices[ 1 ].IsCreated() )
	{
		Display::Device()->DestroyVertexBuffer( &m_skinnedVertices[ 1 ] );
	}
}

bool C_SkinnedMesh::CreateIndexBuffer( U32 numIndices, const U32* indices )
{
	m_numTriangleIndices = numIndices;
	
	if ( 0 == numIndices )
	{
		return false;
	}
	
	if ( !Display::DeviceExists() )
	{
		return false;
	}	

	U32 indexBufferSize = numIndices * sizeof( U32 );
	U32 indexBufferFlags = Display::E_Usage_Managed | Display::E_Usage_Static;
	
	Display::Device()->CreateIndexBuffer(
		indexBufferSize,
		indexBufferFlags,
		Display::E_Format_Index32,
		&m_triangleIndices
	);
	
	U16* dest = static_cast< U16* >( m_triangleIndices.Lock() );
	
	memcpy( dest, indices, indexBufferSize );
	
	m_triangleIndices.Unlock();
	
	return true;
	
}

void C_SkinnedMesh::DestroyIndexBuffer()
{
	if ( m_triangleIndices.IsCreated() )
	{
		Display::Device()->DestroyIndexBuffer( &m_triangleIndices );
	}
}

void C_SkinnedMesh::DestroyMaterials()
{
	for (U32 i = 0, count = m_materials.Size(); i < count; ++i)
	{
		m_materials[i].Destroy();
	}
}



#include <Display/Shaders/vsConstants.h>
#include "shaders/vsConstants.h"

//
// C_SkinnedMeshRenderer
// 

EXTERN_SHADER(vsSkinnedMesh)
EXTERN_SHADER(psSkinnedMesh)
EXTERN_SHADER(psFlat)

C_SkinnedMeshRenderer::C_SkinnedMeshRenderer():
	Display::I_Renderer("SkinnedMeshRenderer")
{
}

C_SkinnedMeshRenderer::~C_SkinnedMeshRenderer()
{
}

Debug::Float gExposure( "SkinnedMesh.Exposure", 1.8f, Debug::Range(0.f, 10.f) );
Debug::Float gDepthBias( "SkinnedMesh.DepthBias", 0.f, Debug::Range(-0.1f, 0.f) );
Debug::Colour gWireframeColour( "SkinnedMesh.WireframeColour", 1.f, 1.f, 1.f, 0.4f );

Debug::Colour gKeyColour( "SkinnedMesh.KeyColour", 1.f, 0.902f, 0.808f, 1.4f );
Debug::Colour gMidColour( "SkinnedMesh.MidColour", 0.431f, 0.745f, 0.949f, 1.f );
Debug::Colour gFillColour( "SkinnedMesh.FillColour", 0.14f, 0.14f, 0.14f, 2.f );
Debug::Colour gFlatColour( "SkinnedMesh.FlatColour", 0.8f, 0.8f, 0.9f, 1.f );
Debug::Float gFlatBlend( "SkinnedMesh.FlatBlend", 0.f, Debug::Range(0.f, 1.f) );
Debug::Float gNormalMapBlend( "SkinnedMesh.NormalMapBlend", 1.f, Debug::Range(0.f, 1.f) );

void C_SkinnedMeshRenderer::RenderSkinnedMesh(
	C_SkinnedMesh& skinnedMesh,
	const Maths::HMat43& modelWorldMatrix,
	const Maths::HMat43& cameraWorldMatrix,
	bool renderSolid,
	bool renderWireframe
)
{
	if ( !skinnedMesh.IsValid() )
	{
		return;
	}
	
	if  ( !Display::DeviceExists() )
	{
		return;
	}

	Display::C_Device* dev = Display::Device();
	
	dev->SetVertexDeclaration( m_vertexDecls[ E_VertexFormat_Skinned ] );
	dev->SetVertexShader( &m_vertexShaders[ E_VertexShader_Default ] );
	dev->SetPixelShaderParameters( &m_pixelShaders[ E_PixelShader_Default ] );
	dev->SetPixelShader( &m_pixelShaders[ E_PixelShader_Default ] );
	
	Maths::HMat44 modelWorldMatrix44(modelWorldMatrix);
	modelWorldMatrix44.Transpose();

	// Set world matrix.
	Display::C_Matrix displayWorldMatrix;
	Display::MatrixStore(&displayWorldMatrix, modelWorldMatrix);
	
	dev->SetVertexShaderConstantF( CV_DISPLAY_WORLD_MATRIX, (F32*)&displayWorldMatrix, 4 );
	
	F32 keyColour[4] = { gKeyColour->r, gKeyColour->g, gKeyColour->b, gKeyColour->a };
	F32 midColour[4] = { gMidColour->r, gMidColour->g, gMidColour->b, gMidColour->a };
	F32 fillColour[4] = { gFillColour->r, gFillColour->g, gFillColour->b, gFillColour->a };
	F32 flatColour[4] = { gFlatColour->r, gFlatColour->g, gFlatColour->b, gFlatColour->a };

	dev->SetPixelShaderConstantF( CP_SKINNEDMESH_KEYCOLOUR, keyColour, 1 );
	dev->SetPixelShaderConstantF( CP_SKINNEDMESH_MIDCOLOUR, midColour, 1 );
	dev->SetPixelShaderConstantF( CP_SKINNEDMESH_FILLCOLOUR, fillColour, 1 );
	dev->SetPixelShaderConstantF( CP_SKINNEDMESH_FLATCOLOUR, flatColour, 1 );
	
	dev->SetFillMode(Display::E_FillMode_Solid);
	dev->SetDepthEnable(Display::E_Bool_True);
	dev->SetDepthWriteEnable( Display::E_Bool_True );
	dev->SetDepthFunc( Display::E_CmpFunc_Default );
	//dev->SetCullModeFlip();
	
	dev->SetIndexBuffer( &skinnedMesh.GetTriangleIndices() );
	dev->SetVertexBuffer( 0, &skinnedMesh.GetSkinnedVertices(), 0, sizeof( C_SkinnedMesh::SkinnedVertex ) ); // sizeof(SkinnedVertex)
		
	dev->SetTextureFilter( 0, Display::E_TextureFilter_Linear );
	dev->SetTextureFilter( 1, Display::E_TextureFilter_Linear );
	
	if ( renderSolid )
	{
		//
		// Opaque.
		//
		dev->SetBlendMode( Display::E_BlendMode_Solid );
		dev->SetDepthWriteEnable( Display::E_Bool_True );
		dev->SetCullMode( Display::E_CullMode_Default );
		
		RenderTriangleLists( skinnedMesh.m_opaqueTriangleLists, skinnedMesh );
		
		//
		// Alpha.
		//
		dev->SetBlendMode( Display::E_BlendMode_Alpha );
		dev->SetDepthWriteEnable( Display::E_Bool_False );
		dev->SetCullMode( Display::E_CullMode_Default );
		
		Maths::HMat43 cameraToModelMatrix = cameraWorldMatrix * modelWorldMatrix.InverseRotTrans();
		
		// Assuming camera +Z is forward, find predominant view direction.
		MathsStorage::Float3 cameraDir;
		Maths::Store(cameraToModelMatrix.AZ(), &cameraDir);
		
		S32 dominantAxis = 0;
		F32 dominantAxisMag = Maths::Abs(cameraDir.x);
		if (Maths::Abs(cameraDir.y) > dominantAxisMag)
		{
			dominantAxisMag = Maths::Abs(cameraDir.y);
			dominantAxis = 1;
		}
		if (Maths::Abs(cameraDir.z) > dominantAxisMag)
		{
			dominantAxisMag = Maths::Abs(cameraDir.z);
			dominantAxis = 2;
		}
		
		C_SkinnedMesh::E_ViewDirection viewDir = C_SkinnedMesh::E_ViewDirection_PosZ;
		switch (dominantAxis)
		{
			case 0:
				viewDir = cameraDir.x < 0.f ? C_SkinnedMesh::E_ViewDirection_NegX : C_SkinnedMesh::E_ViewDirection_PosX;
				break;				
			case 1:
				viewDir = cameraDir.y < 0.f ? C_SkinnedMesh::E_ViewDirection_NegY : C_SkinnedMesh::E_ViewDirection_PosY;
				break;				
			case 2:
				viewDir = cameraDir.z < 0.f ? C_SkinnedMesh::E_ViewDirection_NegZ : C_SkinnedMesh::E_ViewDirection_PosZ;
				break;				
			default:
				break;
		}
		
		RenderTriangleLists( skinnedMesh.m_sortedAlphaTriangleLists[viewDir], skinnedMesh );
	}
		
	if ( renderWireframe )
	{
		dev->SetBlendMode(Display::E_BlendMode_Alpha);
		dev->SetFillMode(Display::E_FillMode_Wireframe);
		dev->SetDepthWriteEnable( Display::E_Bool_False );
		dev->SetDepthBias( gDepthBias );
		
		F32 wireColour[4] = { gWireframeColour->r, gWireframeColour->g, gWireframeColour->b, gWireframeColour->a };
		dev->SetPixelShaderConstantF( CP_DISPLAY_COLOUR, wireColour, 1 );
		
		dev->SetPixelShaderParameters( &m_pixelShaders[ E_PixelShader_Flat ] );
		dev->SetPixelShader( &m_pixelShaders[ E_PixelShader_Flat] );				
	
		dev->DrawIndexedPrimitive(
			Display::E_PrimitiveType_TriangleList,
			0, // baseVertex
			0, // minIndex
			skinnedMesh.GetNumVertices(), // vertex count
			0, // startIndex
			skinnedMesh.GetNumTriangleIndices() // index count
			);
	}

	dev->SetFillMode( Display::E_FillMode_Default );
	dev->SetDepthWriteEnable( Display::E_Bool_Default );
	dev->SetDepthBias( 0.f );
	dev->SetBlendMode(Display::E_BlendMode_Solid);
	dev->SetCullMode( Display::E_CullMode_Default );
}

void C_SkinnedMeshRenderer::RenderTriangleLists(
	const Btl::Vector<C_TriangleList>& triangleLists,
	const C_SkinnedMesh& skinnedMesh
) const
{
	Display::C_Device* dev = Display::Device();
	
	F32 settings[4] = { gExposure, gFlatBlend, 0.f, 0.f };
	dev->SetPixelShaderConstantF( CP_SKINNEDMESH_SETTINGS, settings, 1 );
	
	dev->SetTextureAddressU( 0, Display::E_TextureAddress_Clamp );
	dev->SetTextureAddressV( 0, Display::E_TextureAddress_Clamp );
	
	for ( S32 i = 0; i < triangleLists.Size(); ++i )
	{
		const S32 materialIndex = triangleLists[i].materialIndex;
		bool noTexture = true;
		
		if ( materialIndex >= 0 )
		{
			if ( skinnedMesh.m_materials[materialIndex].hasDiffuseTexture )
			{
				noTexture = false;
					
				settings[1] = gFlatBlend;
				dev->SetTexture( 0, &skinnedMesh.m_materials[materialIndex].diffuseTexture );
			}
			
			if ( skinnedMesh.m_materials[materialIndex].hasNormalMapTexture )
			{
				settings[2] = gNormalMapBlend;
				dev->SetTexture( 1, &skinnedMesh.m_materials[materialIndex].normalMapTexture );
			}
			else
			{
				settings[2] = 0.f;
				dev->SetTexture( 1, NULL );
			}
		}
		
		if ( noTexture )
		{
			settings[1] = 1.f;
			dev->SetTexture( 0, NULL );
		}
		
		dev->SetPixelShaderConstantF( CP_SKINNEDMESH_SETTINGS, settings, 1 );
		
		dev->DrawIndexedPrimitive(
			Display::E_PrimitiveType_TriangleList,
			0, // baseVertex
			0, // minIndex
			skinnedMesh.GetNumVertices(), // vertex count
			triangleLists[i].firstIndex, // startIndex
			triangleLists[i].size * 3 // index count
			);
	}	
}

bool C_SkinnedMeshRenderer::OnCreate()
{
	RESOURCE_MEMORY_ID("SkinnedMeshRenderer");

	if (!CreateShaders())
	{
		return false;
	}
	
	if ( !CreateVertexDeclarations() )
	{
		return false;
	}
	

	return true;
}

bool C_SkinnedMeshRenderer::OnCreateVolatile()
{
	return true;
}

void C_SkinnedMeshRenderer::OnDestroyVolatile()
{
}

void C_SkinnedMeshRenderer::OnDestroy()
{
	RESOURCE_MEMORY_ID("SkinnedMeshRenderer");
	
	DestroyShaders();
}

bool C_SkinnedMeshRenderer::OnInitialise()
{
	return true;
}

void C_SkinnedMeshRenderer::OnShutdown()
{
	
}

void C_SkinnedMeshRenderer::OnReset()
{
}


bool C_SkinnedMeshRenderer::CreateShaders()
{
	if ( !Display::CreateVertexShaderFromFunc(
		"Shaders/vsSkinnedMesh",
		0,
		&m_vertexShaders[ E_VertexShader_Default ],
		REF_SHADER( vsSkinnedMesh )
	) ) return false;

	if ( !Display::CreatePixelShaderFromFunc(
		"Shaders/psSkinnedMesh",
		0,
		&m_pixelShaders[ E_PixelShader_Default ],
		REF_SHADER( psSkinnedMesh )
	) ) return false;
	
	
	if ( !Display::CreatePixelShaderFromFunc(
		"Shaders/psFlat",
		0,
		&m_pixelShaders[ E_PixelShader_Flat ],
		REF_SHADER( psFlat )
	) ) return false;
	
	return true;
}

void C_SkinnedMeshRenderer::DestroyShaders()
{
	for ( U32 i = 0; i < E_VertexShader_Count; ++i )
	{
		if ( m_vertexShaders[ i ].IsCreated() )
		{
			Display::Device()->DestroyVertexShader( &m_vertexShaders[ i ] );
		}
	}
	
	for ( U32 i = 0; i < E_PixelShader_Count; ++i )
	{
		if ( m_pixelShaders[ i ].IsCreated() )
		{
			Display::Device()->DestroyPixelShader( &m_pixelShaders[ i ] );
		}
	}
}

bool C_SkinnedMeshRenderer::CreateVertexDeclarations()
{

	static const Display::C_VertexElement defaultElements[] =
	{
		Display::C_VertexElement( 0, 0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0 ),
		Display::C_VertexElement(  0, 12, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Normal, 0 ),
		Display::C_VertexElement(  0, 24, Display::E_VertexElementType_U8_4, Display::E_VertexElementUsage_BlendIndices, 0 ),
		Display::C_VertexElement( 0, 28, Display::E_VertexElementType_U8_4N, Display::E_VertexElementUsage_BlendWeight, 0 ),
		Display::C_VertexElement()
	};
	
	#ifdef ENABLE_NORMAL_MAPS
	static const Display::C_VertexElement skinnedElements[ 5 ] = 
	{
		{ 0, 0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0 },
		{ 0, 12, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Normal, 0 },
		{ 0, 24, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Tangent, 0 },
		{ 0, 36, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Binormal, 0 },
		{ 0, 48, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 0 }
	};
	#else
	static const Display::C_VertexElement skinnedElements[] = 
	{
		Display::C_VertexElement( 0, 0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0 ),
		Display::C_VertexElement( 0, 12, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Normal, 0 ),
		Display::C_VertexElement( 0, 24, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 0 ),
		Display::C_VertexElement()
	};
	#endif
	
	m_vertexDecls[ E_VertexFormat_Default ] = Display::Device()->CreateVertexDeclaration( 4, defaultElements );
	m_vertexDecls[ E_VertexFormat_Skinned ] = Display::Device()->CreateVertexDeclaration( sizeof(skinnedElements)/sizeof(skinnedElements[0]), skinnedElements );
	
	if ( !m_vertexDecls[ E_VertexFormat_Default ] ) return false;
	if ( !m_vertexDecls[ E_VertexFormat_Skinned ] ) return false;
	
	return true;
}

