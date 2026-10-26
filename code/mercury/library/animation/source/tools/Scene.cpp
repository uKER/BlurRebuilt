#include "stdafx.h"
#include <animation/tools/Scene.h>
#include <animation/tools/ExportConstraints.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/skeleton/SkeletonDeserialise.inl>
//#include <core/maths/hvec/new/IncludeGeneral.h>
//#include <core/maths/hvec/new/IncludeGeneral.inl>
#include <core/maths/hvec/new/IncludeAnimation.h>
#include <core/maths/hvec/new/IncludeAnimation.inl>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>
#include <animation/common/ObjectTransform.h>
#include <animation/common/ObjectTransform.inl>
#include <animation/common/StringUtil.h>
#include <btl/containers/set.h>
#include <btl/containers/openhashset.h>

namespace AnimationTools
{

Debug::Logger& gLog = Debug::GetLogger( "mercury.animation.tools" );

void ConvertAftkMatrix(Maths::Mat43& lhs, const Aftk::Matrix44& rhs)
{
	lhs = Maths::MakeMat43(
		rhs[0][0], rhs[0][1], rhs[0][2],
		rhs[1][0], rhs[1][1], rhs[1][2],
		rhs[2][0], rhs[2][1], rhs[2][2],
		rhs[3][0], rhs[3][1], rhs[3][2]
	);
}

void RemoveAftkMatrixZFlip( Aftk::Matrix44& matrix )
{
	UtMaths::MMatrix33 rotate;
	for ( S32 j = 0; j < 3; ++j )
	{
		for ( S32 i = 0; i < 3; ++i )
		{
			rotate[j][i] = matrix[j][i];
		}
	}
	
	F32 det = UtMaths::GetDeterminant3x3( rotate );
	
	if ( det < 0.f )
	{
		Aftk::Matrix44 zflip;
		UtMaths::SetIdentity(&zflip);
		zflip[2][2] = -1.f;
		matrix =  matrix * zflip;
	}
}

Util::C_String GetCleanAftkSceneNodeName( const Aftk::C_SceneNode* node, const C_SceneConfig& sceneConfig )
{
	if ( node == NULL ) return Util::C_String();
	Util::C_String name( node->GetName().c_str() );
	
	if (sceneConfig.removeNamespaces)
	{
		// Remove everything up to the first namespace delimiter.
		S32 index = name.Search(sceneConfig.namespaceDelimiter);
		if (index >= 0)
		{
			name = name(index + 1, name.Length());
		}
	}
	
	return name;
}

inline bool IsIntegerPlug( const Aftk::I_Plug& plug )
{
	switch (plug.GetDataType())
	{
		case Aftk::E_GenericAttributeType_Boolean:
		case Aftk::E_GenericAttributeType_U8:
		case Aftk::E_GenericAttributeType_U16:
		case Aftk::E_GenericAttributeType_U32:
		case Aftk::E_GenericAttributeType_U64:
		case Aftk::E_GenericAttributeType_S8: 
		case Aftk::E_GenericAttributeType_S16:
		case Aftk::E_GenericAttributeType_S32:
		case Aftk::E_GenericAttributeType_S64:
			return true;
			
		default:
			break;
	}
	return false;
}


inline bool AnyPlugHasDiscontinuity( const Aftk::I_Plug* const * plugs, U32 numPlugs, F32 sceneTime )
{
	const F32 kIntegerValueEps = 1.f / 128.f;
	const F32 kKeyTimeEps = 2.f * (10.f / 250.f);

	for (U32 i = 0; i < numPlugs; ++i)
	{
		const Aftk::I_Plug* p = plugs[i];
		
		if (p == NULL) continue;
	
		const Aftk::I_Object* src = p->GetSourceConnectionPtr();
		
		if (src == NULL) continue;
		if (!src->HasType(Aftk::E_ObjType_AnimationCurve)) continue;
		
		const Aftk::C_AnimationCurveNode* animCurveNode = static_cast<const Aftk::C_AnimationCurveNode*>(src);
		
		if (animCurveNode->IsTimeInput() && animCurveNode->GetNumKeys() > 0)
		{
			// Search for times when a step tangent has finished its influence and there has been
			// an instant change to the new key value.
			
			S32 keyIndex1 = -1;
			S32 keyIndex2 = -1;
			
			bool isIntegerPlug = false;
			if (IsIntegerPlug(*p))
			{
				isIntegerPlug = true;
			}
			
			// Obtain key indices such that key_time_1 <= t < key_time_2
			if (animCurveNode->GetAdjacentKeyIndices(keyIndex1, keyIndex2, sceneTime))
			{
				// two keys, (1.5, x), (3, y)
				// 1) (0,1)
				// 2) (0,1)
				// 3) (0,1) discontinuity here
				// 4) (0,1)
				
				if (keyIndex2 < 0) keyIndex2 = keyIndex1;
					
				if (animCurveNode->GetCurveType() == Aftk::E_AnimCurveType_Hermite)
				{
					Aftk::C_HermiteKey currentKey;
					Aftk::C_HermiteKey nextKey;
					Aftk::C_HermiteKey prevKey;
					animCurveNode->GetKey(currentKey, keyIndex1);
					animCurveNode->GetKey(nextKey, keyIndex2);
					
					if (nextKey.GetX() <= sceneTime)
					{
						prevKey = currentKey;
						currentKey = nextKey;
						BIZ_ASSERT(currentKey.GetX() <= sceneTime);
					}
					else if (keyIndex1 > 0)
					{
						animCurveNode->GetKey(prevKey, keyIndex1 - 1);
						BIZ_ASSERT(currentKey.GetX() <= sceneTime);
					}
					else
					{
						// pre-infinity... assuming constant
						prevKey = currentKey;
					}
					
					if (currentKey.GetX() > sceneTime - 1.f)
					{
						if (prevKey.GetOutTangentType() == Aftk::E_CurveTangentType_Stepped)
						{
							// Stepped tangents introduce discontinuities.
							return true;
						}
						if (isIntegerPlug && Maths::Abs(prevKey.GetValue() - currentKey.GetValue()) > kIntegerValueEps)
						{
							// Integer values are never interpolated.
							return true;
						}
					}
				}
				else if (animCurveNode->GetCurveType() == Aftk::E_AnimCurveType_Linear && isIntegerPlug)
				{
					// Linear curves can only have discontinuities if they are integer curves.
					Aftk::C_LinearKey currentKey;
					Aftk::C_LinearKey nextKey;
					Aftk::C_LinearKey prevKey;
					animCurveNode->GetKey(currentKey, keyIndex1);
					animCurveNode->GetKey(nextKey, keyIndex2);
					
					if (nextKey.GetX() <= sceneTime)
					{
						prevKey = currentKey;
						currentKey = nextKey;
						BIZ_ASSERT(currentKey.GetX() <= sceneTime);
					}
					else if (keyIndex1 > 0)
					{
						animCurveNode->GetKey(prevKey, keyIndex1 - 1);
						BIZ_ASSERT(currentKey.GetX() <= sceneTime);
					}
					else
					{
						// pre-infinity... assuming constant.
						prevKey = currentKey;
					}
					
					if (currentKey.GetX() > sceneTime - 1.f)
					{
						if (Maths::Abs(currentKey.GetValue() - prevKey.GetValue()) > kIntegerValueEps)
						{
							// Integer values are never interpolated.
							return true;						
						}
					}
					
				}
			}
		}
	}
	return false;
}

bool IsModelElement(const Util::C_String& nodeName, const C_SceneConfig& sceneConfig)
{
#if 0
	// Breaks down when namespace doesn't use the default namespace delimiter
	// e.g. bond_E_phone won't get exported
	Util::C_String nameWithoutNamespace = nodeName;
	S32 index = nodeName.Search(sceneConfig.namespaceDelimiter);
	if (index >= 0)
	{
		nameWithoutNamespace = nodeName(index + 1, nodeName.Length());
	}
	
	return (nameWithoutNamespace.ISearch("E_") == 0 || nameWithoutNamespace.ISearch("M_") == 0);
#else
	// Will probably return false positives, but in combination with other criteria (e.g. has keys) it should be fine.
	return nodeName.ISearch("E_") >= 0 || nodeName.ISearch("M_") >= 0;
#endif
}


//
// C_SceneConfig
//

bool C_SceneConfig::operator == ( const C_SceneConfig& rhs ) const
{
	//if ( rootNodeNames != rhs.rootNodeNames ) return false;
	//if ( dynamicAimJointNames != rhs.dynamicAimJointNames ) return false;
	//if ( dynamicSliderJointNames != rhs.dynamicSliderJointNames ) return false;
	//if ( excludedNodeNames != rhs.excludedNodeNames ) return false;
	//if ( referenceNodeNames != rhs.referenceNodeNames ) return false;
	if ( scaleFactor != rhs.scaleFactor ) return false;
//	if ( oversampling != rhs.oversampling ) return false;
	return true;
}

//
// C_NameFiter
//

// Filter scene nodes according to C_SceneConfig naming conventions.
class C_NameFilter: public I_Filter
{
public:
	C_NameFilter( const C_SceneConfig& sceneConfig ):
		m_sceneConfig(sceneConfig)
	{
	}

	virtual bool IsValidNode( const Aftk::C_SceneNode* node ) const
	{
		if ( node == NULL ) return false;
		
		// Ignore constraints.
		if ( node->HasType( Aftk::E_ObjType_ISceneConstraint ) )
		{
			return false;
		}
		
		// Ignore excluded nodes
		Util::C_String nodeName = GetCleanAftkSceneNodeName( node, m_sceneConfig );
		if ( m_sceneConfig.excludedNodeNames.Match( nodeName ) )
		{
			//LOGINFO( gLog, "Node '%s' is excluded from skeleton\n", node->GetName().c_str() );
			return false;
		}

		// Ignore world origin nodes
		if (m_sceneConfig.worldOriginNodeNames.Match(nodeName))
		{
			return false;
		}
		
		// Ignore reference marker nodes as these are extracted in a separate pass.
		if ( m_sceneConfig.referenceNodeNames.Match( nodeName ) )
		{
			return false;
		}

		if ( true )
		{
			if ( node->HasType(Aftk::E_ObjType_SceneMesh) )
			{
				const Aftk::C_SceneMeshNode* meshNode = static_cast<const Aftk::C_SceneMeshNode*>(node);
				
				// Ignore skinned mesh nodes - their transforms will be determined by joints in the scene.
				if (meshNode->IsSkinned())
				{
					return false;
				}
			}
		}
		
		return true;
	}

private:
	const C_SceneConfig& m_sceneConfig;
};

//
// C_ListFilter
//

// Filter scene nodes if they are present in a previous list.
class C_ListFilter: public I_Filter
{
public:
	C_ListFilter( const Btl::Vector<const Aftk::C_SceneNode*>& nodes )
	{
		for ( S32 i = 0; i < nodes.Size(); ++i )
		{
			if( nodes[i] != NULL )
			{
				m_nodes.Insert( nodes[i] );
			}
		}
	}
	
	C_ListFilter( const Btl::Vector<Aftk::C_SceneNode*>& nodes )
	{
		for ( S32 i = 0; i < nodes.Size(); ++i )
		{
			if( nodes[i] != NULL )
			{
				m_nodes.Insert( nodes[i] );
			}
		}
	}	

	virtual bool IsValidNode( const Aftk::C_SceneNode* node ) const
	{
		return m_nodes.Find( node ) != m_nodes.End();
	}

private:
	Btl::Set<const Aftk::C_SceneNode*>	m_nodes;
};

//
// C_Scene::C_CameraFovSource
//
	
class C_Scene::C_CameraFovCurveSource: public C_Scene::C_CurveSource
{
public:
	C_CameraFovCurveSource(const Aftk::C_SceneCameraNode& cameraNode):
		m_cameraNode(cameraNode)
	{
	}
	
	virtual void Evaluate(F32* outValue) const
	{
		*outValue = m_cameraNode.GetVerticalFieldOfView();
	}
	
	virtual bool HasDiscontinuity( F32 currentTime ) const
	{
		const Aftk::I_Plug* plugs[2] =
		{
			&m_cameraNode.GetFocalLengthPlug(),
			&m_cameraNode.GetVerticalFilmAperturePlug()
		};
	
		return AnyPlugHasDiscontinuity( plugs, 2, currentTime );
	}
	
private:
	const Aftk::C_SceneCameraNode&	m_cameraNode;
};



//
// C_Scene
//


C_Scene::C_Scene():
	m_constraints( new C_ExportConstraints ),
	m_aftkScene(NULL)
{
}

C_Scene::~C_Scene()
{
}

C_Status C_Scene::Import( Aftk::C_Scene& aftkScene, const C_SceneConfig& sceneConfig )
{
	C_NameFilter nameFilter( sceneConfig );
	return Import( aftkScene, sceneConfig, nameFilter );
}

C_Status C_Scene::Import( Aftk::C_Scene& aftkScene, const Btl::Vector<Aftk::C_SceneNode*>& sceneNodes, const C_SceneConfig& sceneConfig )
{
	C_ListFilter filter( sceneNodes );
	return Import( aftkScene, sceneConfig, filter );
}

C_Status C_Scene::Import( Aftk::C_Scene& aftkScene, const C_SceneConfig& sceneConfig, const I_Filter& filter )
{
	m_aftkScene = &aftkScene;
	m_sceneConfig = sceneConfig;
		
	m_nodes.Clear();
	m_constraints->Clear();
	m_referenceTransforms.Clear();
	m_customScalarCurves.Clear();
	m_customVector3Curves.Clear();
	m_customVector4Curves.Clear();
	
	// Set number of frames.
	// 1.5 is to ensure final frame is included and round up non-integer end times.
	F32 sceneDuration = aftkScene.GetDuration();
	F32 offset = 1.5f;
	m_numFrames = static_cast< U32 >( Maths::Max( 0, static_cast<S32>( offset + ( sceneDuration * m_sceneConfig.oversampling ) ) ) );
	m_frameRate = aftkScene.GetFrameRate();

	// Configure conversion from Aftk scene to game's coordinate system.
	m_sceneToGameTransform.SetSceneToGameMatrix( sceneConfig.scaleFactor, sceneConfig.flipX, sceneConfig.flipY, sceneConfig.flipZ);
	
	// Firstly, remove any Z-flip that may have been introduced.
	Aftk::Matrix44 sceneMatrix = aftkScene.GetRootSceneNodePtr()->GetWorldMatrix();
	RemoveAftkMatrixZFlip( sceneMatrix );
	aftkScene.GetRootSceneNodePtr()->SetWorldMatrix( sceneMatrix );

	const Aftk::C_SceneNode* pWorldOriginNode = FindCustomWorldOriginNode(aftkScene, sceneConfig);

	if (pWorldOriginNode || sceneConfig.HasNonZeroWorldOrigin())
	{
		if (pWorldOriginNode)
		{
			LOGINFO(gLog, "Using node '%s' as world origin.", pWorldOriginNode->GetName().c_str());

			// Apply inverse of new world origin node's world matrix.
			sceneMatrix = pWorldOriginNode->GetWorldMatrixInverse();
		}
		else if (sceneConfig.HasNonZeroWorldOrigin())
		{
			// Convert world origin from game space into scene space.
			MathsStorage::Float3 sceneWorldOrigin = m_sceneToGameTransform.PointToSceneSpace(sceneConfig.worldOrigin);

			sceneMatrix[3][0] = -sceneWorldOrigin.x;
			sceneMatrix[3][1] = -sceneWorldOrigin.y;
			sceneMatrix[3][2] = -sceneWorldOrigin.z;

			LOGINFO(gLog, "Using custom world origin (%.3f, %.3f, %.3f)", sceneWorldOrigin.x, sceneWorldOrigin.y, sceneWorldOrigin.z);
		}
		aftkScene.GetRootSceneNodePtr()->SetWorldMatrix( sceneMatrix );
	}

	C_Status status;
	
	status = VerifyNodeNamesAreUnique( aftkScene, sceneConfig );
	
	if ( !status ) return status;
	
	// Build hierarchy, list of nodes.
	const Aftk::C_SceneNode* rootNode = GetRootNode( aftkScene, sceneConfig );
	
	if ( NULL == rootNode )
	{
		return C_Status( false, "Cannot find scene root node" );
	}
	
	BuildHierarchy( rootNode, -1, sceneConfig, filter );
	
	status = ImportConstraints( sceneConfig );
	if ( !status ) return status;

	Btl::Vector<const Aftk::C_SceneNode*> missingInfluenceNodes;
	ProcessInfluenceNodes(&missingInfluenceNodes);
	
	if (missingInfluenceNodes.Size() > 0)
	{
		for (S32 i = 0; i < missingInfluenceNodes.Size(); ++i)
		{
			LOGWARN(gLog, "Active influence node '%s' is not part of animation scene.", missingInfluenceNodes[i]->GetName().c_str());
		}
	}

	//DebugPrintNodeHierarchy( gLog );
	
	if ( sceneConfig.importAnimatedNodeAttributes )
	{
		status = ImportAnimatedNodeAttributes( sceneConfig );
		if ( !status ) return status;
	}
	
	if ( sceneConfig.importAnimatedMaterials )
	{
		status = ImportAnimatedMaterialParameters( sceneConfig );
		if ( !status ) return status;
	}
	
	//DebugPrintCustomCurves( gLog );
	
	status = ImportReferenceTransforms( sceneConfig );
	if ( !status ) return status;
	
	DebugPrintReferenceTransforms( gLog );
	
	return C_Status( true );
}

const C_SceneToGameTransform& C_Scene::GetSceneToGameTransform() const
{
	return m_sceneToGameTransform;
}

const C_SceneConfig& C_Scene::GetSceneConfig() const
{
	return m_sceneConfig;
}

U32 C_Scene::GetNumFrames() const
{
	return m_numFrames;
}

F32 C_Scene::GetFrameRate() const
{
	return m_frameRate;
}

void C_Scene::SetCurrentFrame( U32 frame )
{
	F32 time = ( m_aftkScene->GetStartTime() + ( frame / m_sceneConfig.oversampling ) );
	m_aftkScene->SetCurrentTime( time );
}

U32 C_Scene::GetNumNodes() const
{
	return m_nodes.Size();
}

S32 C_Scene::GetNodeParentIndex( U32 nodeIndex ) const
{
	return m_nodes[nodeIndex].parentIndex;
}

U32 C_Scene::GetNodeDepth( U32 nodeIndex ) const
{
	U32 depth = 0;
	S32 parentIndex = GetNodeParentIndex(nodeIndex);
	while (parentIndex >= 0)
	{
		++depth;
		parentIndex = GetNodeParentIndex(parentIndex);		
	}
	return depth;
}
	
U32 C_Scene::GetMaxNodeChainLength( U32 nodeIndex ) const
{
	const U32 numNodes = GetNumNodes();
	for ( U32 i = nodeIndex + 1; i < numNodes; ++i )
	{
		m_nodes[i].chainLength = 0;
	}
	
	m_nodes[nodeIndex].chainLength = 1;
	
	U32 maxChainLength = 1;
	
	for ( U32 i = nodeIndex + 1; i < numNodes; ++i )
	{
		S32 parentIndex = GetNodeParentIndex(i);
		
		if ( parentIndex >= 0 && U32(parentIndex) >= nodeIndex )
		{
			U32 parentChainLength = m_nodes[parentIndex].chainLength;
			if ( parentChainLength > 0 )
			{
				m_nodes[i].chainLength = parentChainLength + 1;
				
				maxChainLength = Maths::Max( parentChainLength + 1, maxChainLength );
			}
		}
	}
	
	return maxChainLength;
}

F32 C_Scene::GetMaxNodeChainDistance( U32 nodeIndex ) const
{
	const U32 numNodes = GetNumNodes();
	for ( U32 i = nodeIndex + 1; i < numNodes; ++i )
	{
		m_nodes[i].inChain = false;
	}
	
	m_nodes[nodeIndex].inChain = true;
	m_nodes[nodeIndex].distanceFromChainRoot = 0.f;
	
	F32 maxChainDistance = 0.f;
	
	for ( U32 i = nodeIndex + 1; i < numNodes; ++i )
	{
		S32 parentIndex = GetNodeParentIndex( i );
		
		if ( parentIndex >= 0 && U32(parentIndex) >= nodeIndex )
		{
			if ( m_nodes[parentIndex].inChain )
			{
				m_nodes[i].inChain = true;
				
				Maths::Mat43 nodeBasePoseMatrix;
				GetLocalNodeBindPoseTransform( i, nodeBasePoseMatrix );
				
				m_nodes[i].distanceFromChainRoot =
					m_nodes[parentIndex].distanceFromChainRoot
					+ Maths::GetF32( Maths::Length( Maths::RowW(nodeBasePoseMatrix) ) );
				
				maxChainDistance = Maths::Max( m_nodes[i].distanceFromChainRoot, maxChainDistance );
			}
		}
	}
	
	return maxChainDistance;
}

const CHAR8* C_Scene::GetNodeName( U32 nodeIndex ) const
{
	return m_nodes[nodeIndex].name.Chars();
}

S32 C_Scene::GetNodeIndexByAftkSceneNode( const Aftk::C_SceneNode* aftkSceneNode ) const
{
	if ( NULL == aftkSceneNode )
	{
		return -1;
	}
	const U32 numNodes = GetNumNodes();
	for ( U32 i = 0; i < numNodes; ++i )
	{
		if ( aftkSceneNode == m_nodes[i].aftkSceneNode )
		{
			return i;
		}
	}
	return -1;
}

S32 C_Scene::GetNodeIndexByNearestAftkSceneNode( const Aftk::C_SceneNode* aftkSceneNode ) const
{
	if (!aftkSceneNode)
	{
		return -1;
	}
	S32 nodeIndex = GetNodeIndexByAftkSceneNode( aftkSceneNode );
	const Aftk::C_SceneNode* node = aftkSceneNode->GetParentPtr();	
	
	while ( nodeIndex < 0 && node )
	{
		nodeIndex = GetNodeIndexByAftkSceneNode( node );
		node = node->GetParentPtr();
	}
	return nodeIndex;
}

void C_Scene::GetCurrentLocalNodeTransform( U32 nodeIndex, Animation::C_JointTransform& outLocalTransform ) const
{
	Animation::C_JointTransform nodeWorldTransform;
	GetCurrentWorldNodeTransform( nodeIndex, nodeWorldTransform );
	
	S32 parentIndex = GetNodeParentIndex( nodeIndex );
	
	Animation::C_JointTransform parentWorldTransform;
	parentWorldTransform.SetIdentity();
	
	if ( parentIndex >= 0 )
	{
		GetCurrentWorldNodeTransform( parentIndex, parentWorldTransform );
	}
	
	outLocalTransform = nodeWorldTransform * parentWorldTransform.Inverse();
}

void C_Scene::GetCurrentLocalNodeTransform( U32 nodeIndex, Animation::C_ObjectTransform& outLocalTransform ) const
{
	Animation::C_ObjectTransform nodeWorldTransform;
	GetCurrentWorldNodeTransform( nodeIndex, nodeWorldTransform );
	
	S32 parentIndex = GetNodeParentIndex( nodeIndex );
	
	Animation::C_ObjectTransform parentWorldTransform;
	parentWorldTransform.SetIdentity();
	
	if ( parentIndex >= 0 )
	{
		GetCurrentWorldNodeTransform( parentIndex, parentWorldTransform );
	}
	
	outLocalTransform = nodeWorldTransform * parentWorldTransform.Inverse();
}

void C_Scene::GetCurrentLocalNodeTransform( U32 nodeIndex, Maths::Mat43& outLocalTransform ) const
{
	Maths::Mat43 nodeWorldTransform;
	GetCurrentWorldNodeTransform( nodeIndex, nodeWorldTransform );
	
	S32 parentIndex = GetNodeParentIndex( nodeIndex );
	
	Maths::Mat43 parentWorldTransform = Maths::Const<Maths::Mat43>::Identity();

	if ( parentIndex >= 0 )
	{
		GetCurrentWorldNodeTransform( parentIndex, parentWorldTransform );
	}
	
	outLocalTransform = nodeWorldTransform * Maths::Inverse(parentWorldTransform);

}

void C_Scene::GetCurrentWorldNodeTransform( U32 nodeIndex, Animation::C_JointTransform& outWorldTransform ) const
{
	Maths::Mat43 nodeWorldTransform;
	GetCurrentWorldNodeTransform( nodeIndex, nodeWorldTransform );
	outWorldTransform = Animation::C_JointTransform( nodeWorldTransform );
}

void C_Scene::GetCurrentWorldNodeTransform( U32 nodeIndex, Animation::C_ObjectTransform& outWorldTransform ) const
{
	Maths::Mat43 nodeWorldTransform;
	GetCurrentWorldNodeTransform( nodeIndex, nodeWorldTransform );
	outWorldTransform = Animation::C_ObjectTransform( nodeWorldTransform );
}

void C_Scene::GetCurrentWorldNodeTransform( U32 nodeIndex, Maths::Mat43& outWorldTransform ) const
{
	const C_Node& node = m_nodes[nodeIndex];
	Maths::Mat43 m43;
	if (node.IsAftkSceneRootNode())
	{
		// Aftk root node always has the identity transform. 
		// It may have an optional world origin offset applied which we want child nodes to inherit
		// in their world matrices.
		m43 = Maths::Const<Maths::Mat43>::Identity();
	}
	else
	{
		ConvertAftkMatrix( m43, node.aftkSceneNode->GetWorldMatrix() );
	}
	outWorldTransform = m_sceneToGameTransform.ToGameSpace( m43 );
}

void C_Scene::GetLocalNodeBindPoseTransform( U32 nodeIndex, Animation::C_JointTransform& outLocalTransform ) const
{
	Animation::C_JointTransform nodeWorldTransform;
	GetWorldNodeBindPoseTransform( nodeIndex, nodeWorldTransform );
	
	S32 parentIndex = GetNodeParentIndex( nodeIndex );
	if ( parentIndex >= 0 )
	{
		Animation::C_JointTransform parentWorldTransform;
		GetWorldNodeBindPoseTransform( parentIndex, parentWorldTransform );
		outLocalTransform = nodeWorldTransform * parentWorldTransform.Inverse();
	}
	else
	{
		outLocalTransform = nodeWorldTransform;
	}
}

void C_Scene::GetLocalNodeBindPoseTransform( U32 nodeIndex, Animation::C_ObjectTransform& outLocalTransform ) const
{
	Animation::C_ObjectTransform nodeWorldTransform;
	GetWorldNodeBindPoseTransform( nodeIndex, nodeWorldTransform );
	
	S32 parentIndex = GetNodeParentIndex( nodeIndex );
	
	Animation::C_ObjectTransform parentWorldTransform;
	parentWorldTransform.SetIdentity();
	
	if ( parentIndex >= 0 )
	{
		GetWorldNodeBindPoseTransform( parentIndex, parentWorldTransform );
	}
	
	outLocalTransform = nodeWorldTransform * parentWorldTransform.Inverse();
}

void C_Scene::GetLocalNodeBindPoseTransform( U32 nodeIndex, Maths::Mat43& outLocalTransform ) const
{
	Maths::Mat43 nodeWorldTransform;
	GetWorldNodeBindPoseTransform( nodeIndex, nodeWorldTransform );
	
	S32 parentIndex = GetNodeParentIndex( nodeIndex );
	
	Maths::Mat43 parentWorldTransform = Maths::Const<Maths::Mat43>::Identity();
	
	if ( parentIndex >= 0 )
	{
		GetWorldNodeBindPoseTransform( parentIndex, parentWorldTransform );
	}
	
	outLocalTransform = nodeWorldTransform * Maths::Inverse(parentWorldTransform);
}

void C_Scene::GetInverseWorldNodeBindPoseTransform( U32 nodeIndex, Animation::C_JointTransform& outInverseWorldTransform ) const
{
	Maths::Mat43 inverseWorldBindPoseMatrix;
	GetInverseWorldNodeBindPoseTransform( nodeIndex, inverseWorldBindPoseMatrix );
	outInverseWorldTransform = Animation::C_JointTransform( inverseWorldBindPoseMatrix );
}

void C_Scene::GetInverseWorldNodeBindPoseTransform( U32 nodeIndex, Animation::C_ObjectTransform& outInverseWorldTransform ) const
{
	Maths::Mat43 inverseWorldBindPoseMatrix;
	GetInverseWorldNodeBindPoseTransform( nodeIndex, inverseWorldBindPoseMatrix );
	outInverseWorldTransform = Animation::C_ObjectTransform( inverseWorldBindPoseMatrix );
}

void C_Scene::GetInverseWorldNodeBindPoseTransform( U32 nodeIndex, Maths::Mat43& outInverseWorldTransform ) const
{
	const C_Node& node = m_nodes[nodeIndex];
	Aftk::Matrix44 invBindPoseMatrix;

	// Use identity INVERSE bind pose for anything that isn't a joint. This is a trick to make sure
	// that the skinning matrix calculation code generates the right kind of matrices for skinned
	// meshes and non-skinned meshes/locators/etc. without having any special cases.
	bool useBindPose = false;

	if (node.isInfluenceNode || node.aftkSceneNode->HasType(Aftk::E_ObjType_SceneJoint))
	{
		useBindPose = true;
	}
	
	if (useBindPose)		
	{
		Aftk::Matrix44 bindPoseMatrix = node.aftkSceneNode->GetBasePose();
		RemoveAftkMatrixZFlip( bindPoseMatrix );
		UtMaths::Invert4x4(&invBindPoseMatrix, bindPoseMatrix);
	}
	else
	{
		UtMaths::SetIdentity(&invBindPoseMatrix);
	}
	
	Maths::Mat43 m43;
	ConvertAftkMatrix( m43, invBindPoseMatrix );
	
	outInverseWorldTransform = m_sceneToGameTransform.ToGameSpace( m43 );
}



void C_Scene::GetWorldNodeBindPoseTransform( U32 nodeIndex, Animation::C_JointTransform& outWorldTransform ) const
{
	Maths::Mat43 worldBindPoseMatrix;
	GetWorldNodeBindPoseTransform( nodeIndex, worldBindPoseMatrix );
	outWorldTransform = Animation::C_JointTransform( worldBindPoseMatrix );
}

void C_Scene::GetWorldNodeBindPoseTransform( U32 nodeIndex, Animation::C_ObjectTransform& outWorldTransform ) const
{
	Maths::Mat43 worldBindPoseMatrix;
	GetWorldNodeBindPoseTransform( nodeIndex, worldBindPoseMatrix );
	outWorldTransform = Animation::C_ObjectTransform( worldBindPoseMatrix );
}

void C_Scene::GetWorldNodeBindPoseTransform( U32 nodeIndex, Maths::Mat43& outWorldTransform ) const
{
	const C_Node& node = m_nodes[nodeIndex];
		
	Aftk::Matrix44 bindPoseMatrix;

	#if 0
	const Aftk::E_ObjType sceneNodeType = node.aftkSceneNode->GetType();
	if ( sceneNodeType != Aftk::E_ObjType_SceneNode &&
		 sceneNodeType != Aftk::E_ObjType_SceneJoint
	)
	{
		// Use current world matrix for nodes that won't be used for skinning.
		bindPoseMatrix = node.aftkSceneNode->GetWorldMatrix();
	}	
	else
	#endif
	
	bool useBindPose = true;
	
/*
	if (node.aftkSceneNode->HasType(Aftk::E_ObjType_SceneMesh))
	{
		const Aftk::C_SceneMeshNode* meshNode = static_cast<const Aftk::C_SceneMeshNode*>(node.aftkSceneNode);
		if (!meshNode->IsSkinned())
		{
			// Use identity bind pose for non-skinned meshes.
			// FIXME: actually use identity INVERSE bind pose, but keep original!
			useBindPose = false;
		}
	}
*/
	
	if (useBindPose)		
	{
		bindPoseMatrix = node.aftkSceneNode->GetBasePose();
		RemoveAftkMatrixZFlip( bindPoseMatrix );
	}
	else
	{
		UtMaths::SetIdentity(&bindPoseMatrix);
	}
	
	Maths::Mat43 m43;
	ConvertAftkMatrix( m43, bindPoseMatrix );
	
	outWorldTransform = m_sceneToGameTransform.ToGameSpace( m43 );
}


// Returns true if plug represents a keyframed channel i.e. it is ultimately connected to
// an animation curve that is driven by time and has more than one key.
bool IsKeyframedChannel( const Aftk::I_Plug* plug )
{
	if ( !plug )
	{
		return false;
	}
	
	const Aftk::I_Plug* currPlug = plug;
	
	
	
	// Follow source connections until 
	
	const Aftk::I_Object* src = currPlug->GetSourceConnectionPtr();
	
	while ( src != NULL )
	{
		if ( src->HasType( Aftk::E_ObjType_AnimationCurve ) )
		{
			const Aftk::C_AnimationCurveNode* animCurveNode = static_cast< const Aftk::C_AnimationCurveNode* >( src );
			
			// Is this animation curve driven directly by time?
			if ( animCurveNode->IsTimeInput() )
			{
				// Yes, so consider it keyframed if there are one or more keys.
				return animCurveNode->GetNumKeys() > 0;
			}
			else
			{
				// No, so follow the animation curve input.
				currPlug = &animCurveNode->GetInputPlug();
			}
		}
		else if ( src->HasType( Aftk::E_ObjType_IPlug ) )
		{
			currPlug = static_cast<const Aftk::I_Plug*>( src );
		}
		else
		{
			// Unrecognised node type.
			return false;
		}
		
		src = currPlug->GetSourceConnectionPtr();
	}
	
	return false;
}

bool C_Scene::NodeHasRotationKeys( U32 nodeIndex ) const
{
	if ( nodeIndex >= U32(m_nodes.Size()) ) return false;
	if ( m_nodes[nodeIndex].aftkSceneNode == NULL ) return false;
	
	const Aftk::C_NullPlug& rotatePlug = m_nodes[nodeIndex].aftkSceneNode->GetRotatePlug();
	
	if ( rotatePlug.GetNumChildren() != 3 )
	{
		return false;
	}
	
	return IsKeyframedChannel( rotatePlug.GetChildPtr( 0 ) )
		|| IsKeyframedChannel( rotatePlug.GetChildPtr( 1 ) ) 
		|| IsKeyframedChannel( rotatePlug.GetChildPtr( 2 ) );
}

bool C_Scene::NodeHasTranslationKeys( U32 nodeIndex ) const
{
	if ( nodeIndex >= U32(m_nodes.Size()) ) return false;
	if ( m_nodes[nodeIndex].aftkSceneNode == NULL ) return false;

	if ( m_nodes[nodeIndex].parentIndex < 0 && m_sceneConfig.HasNonZeroWorldOrigin() )
	{
		return true;
	}
	
	const Aftk::C_NullPlug& translatePlug = m_nodes[nodeIndex].aftkSceneNode->GetTranslatePlug();
	
	if ( translatePlug.GetNumChildren() != 3 )
	{
		return false;
	}
	
	return IsKeyframedChannel( translatePlug.GetChildPtr( 0 ) )
		|| IsKeyframedChannel( translatePlug.GetChildPtr( 1 ) ) 
		|| IsKeyframedChannel( translatePlug.GetChildPtr( 2 ) );
}

bool C_Scene::NodeHasScaleKeys( U32 nodeIndex ) const
{
	if ( nodeIndex >= U32(m_nodes.Size()) ) return false;
	if ( m_nodes[nodeIndex].aftkSceneNode == NULL ) return false;
	
	const Aftk::C_NullPlug& scalePlug = m_nodes[nodeIndex].aftkSceneNode->GetScalePlug();
	
	if ( scalePlug.GetNumChildren() != 3 )
	{
		return false;
	}
	
	return IsKeyframedChannel( scalePlug.GetChildPtr( 0 ) )
		|| IsKeyframedChannel( scalePlug.GetChildPtr( 1 ) ) 
		|| IsKeyframedChannel( scalePlug.GetChildPtr( 2 ) );
}

bool C_Scene::NodeHasRotationConstraint( U32 nodeIndex ) const
{
	return false;
}

bool C_Scene::NodeHasTranslationConstraint( U32 nodeIndex ) const
{
	return false;
}

bool C_Scene::NodeHasScaleConstraint( U32 nodeIndex ) const
{
	return false;
}



bool C_Scene::NodeHasRotationDiscontinuity( U32 nodeIndex ) const
{
	if (nodeIndex >= (U32)m_nodes.Size()) return false;
	if (m_nodes[nodeIndex].aftkSceneNode == NULL) return false;
	
	const Aftk::C_NullPlug& rotationPlug = m_nodes[nodeIndex].aftkSceneNode->GetRotatePlug();
	
	if (rotationPlug.GetNumChildren() != 3) return false;
	
	const Aftk::I_Plug* plugs[3];
	plugs[0] = rotationPlug.GetChildPtr(0);
	plugs[1] = rotationPlug.GetChildPtr(1);
	plugs[2] = rotationPlug.GetChildPtr(2);
	
	return AnyPlugHasDiscontinuity(plugs, 3, m_aftkScene->GetCurrentTime());
}

bool C_Scene::NodeHasTranslationDiscontinuity( U32 nodeIndex ) const
{
	if (nodeIndex >= (U32)m_nodes.Size()) return false;
	if (m_nodes[nodeIndex].aftkSceneNode == NULL) return false;
	
	const Aftk::C_NullPlug& translationPlug = m_nodes[nodeIndex].aftkSceneNode->GetTranslatePlug();
	
	if (translationPlug.GetNumChildren() != 3) return false;
	
	const Aftk::I_Plug* plugs[3];
	plugs[0] = translationPlug.GetChildPtr(0);
	plugs[1] = translationPlug.GetChildPtr(1);
	plugs[2] = translationPlug.GetChildPtr(2);
	
	return AnyPlugHasDiscontinuity(plugs, 3, m_aftkScene->GetCurrentTime());
}

bool C_Scene::NodeHasScaleDiscontinuity( U32 nodeIndex ) const
{
	if (nodeIndex >= (U32)m_nodes.Size()) return false;
	if (m_nodes[nodeIndex].aftkSceneNode == NULL) return false;
	
	const Aftk::C_NullPlug& scalePlug = m_nodes[nodeIndex].aftkSceneNode->GetScalePlug();
	
	if (scalePlug.GetNumChildren() != 3) return false;
	
	const Aftk::I_Plug* plugs[3];
	plugs[0] = scalePlug.GetChildPtr(0);
	plugs[1] = scalePlug.GetChildPtr(1);
	plugs[2] = scalePlug.GetChildPtr(2);
	
	return AnyPlugHasDiscontinuity(plugs, 3, m_aftkScene->GetCurrentTime());
}

bool C_Scene::IsHelperNode( U32 nodeIndex ) const
{
	if (nodeIndex >= U32(m_nodes.Size())) return false;
	return m_nodes[nodeIndex].isHelper;
}

bool C_Scene::NodeHasNamedParent(U32 nodeIndex, const C_PatternList& parentNamePatternList) const
{
	S32 parentIndex = GetNodeParentIndex(nodeIndex);

	while (parentIndex >= 0)
	{
		if (parentNamePatternList.Match(GetNodeName(parentIndex)))
		{
			return true;
		}
		parentIndex = GetNodeParentIndex(parentIndex);
	}
	return false;
}

bool C_Scene::IsHighFidelityNode(U32 nodeIndex) const
{
	return m_nodes[nodeIndex].isHighFidelity;
}

bool C_Scene::NodeHasNamedParent(U32 nodeIndex, const CHAR8* parentNamePattern) const
{
	S32 parentIndex = GetNodeParentIndex(nodeIndex);

	while (parentIndex >= 0)
	{
		if (Animation::StringUtil::Match(GetNodeName(parentIndex), parentNamePattern))
		{
			return true;
		}
		parentIndex = GetNodeParentIndex(parentIndex);
	}
	return false;
}

void C_Scene::DebugPrintNodeHierarchy( Debug::Logger& logger ) const
{
	LOGINFO( logger, "======= Scene Hierarchy ======\n" );
	for ( S32 i = 0; i < m_nodes.Size(); ++i )
	{
	
		S32 numParents = 0;
		S32 parentIndex = GetNodeParentIndex( i );
		while ( parentIndex >= 0 )
		{
			++numParents;
			parentIndex = GetNodeParentIndex( parentIndex );
		}
		
		Util::C_String indent = "";
		for ( S32 j = 0; j < numParents; ++j )
		{
			indent += " ";
		}
			
		LOGINFO( logger, "%s[%3d] %s (%d)\n", indent.Chars(), i, m_nodes[ i ].name.Chars(), 0 ); // GetMaxJointChainLength( i ) );
	}
	LOGINFO( logger, "==============================\n" );
}

U32 C_Scene::GetNumCustomScalars() const
{
	return m_customScalarCurves.Size();
}

U32 C_Scene::GetNumCustomVector3s() const
{
	return m_customVector3Curves.Size();
}

U32 C_Scene::GetNumCustomVector4s() const
{
	return m_customVector4Curves.Size();
}

const CHAR8* C_Scene::GetCustomScalarName( U32 index ) const
{
	return m_customScalarCurves[ index ].name.Chars();
}

const CHAR8* C_Scene::GetCustomVector3Name( U32 index ) const
{
	return m_customVector3Curves[ index ].name.Chars();
}

const CHAR8* C_Scene::GetCustomVector4Name( U32 index ) const
{
	return m_customVector4Curves[ index ].name.Chars();
}

F32 C_Scene::GetCurrentCustomScalarValue( U32 index ) const
{
	F32 value = 0.f;
	m_customScalarCurves[ index ].GetCurrentValue( &value );
	return value;
}

void C_Scene::GetCurrentCustomVector3Value( U32 index, F32 outValue[3] ) const
{
	m_customVector3Curves[ index ].GetCurrentValue( outValue );
}

void C_Scene::GetCurrentCustomVector4Value( U32 index, F32 outValue[4] ) const
{
	m_customVector4Curves[ index ].GetCurrentValue( outValue );
}

bool C_Scene::DoesCustomScalarHaveDiscontinuity( U32 index ) const
{
	BIZ_ASSERT( index < (U32)m_customScalarCurves.Size() );
	return m_customScalarCurves[index].HasDiscontinuity(m_aftkScene->GetCurrentTime());
}

bool C_Scene::DoesCustomVector3HaveDiscontinuity( U32 index ) const
{
	BIZ_ASSERT( index < (U32)m_customVector3Curves.Size() );
	return m_customVector3Curves[index].HasDiscontinuity(m_aftkScene->GetCurrentTime());
}

bool C_Scene::DoesCustomVector4HaveDiscontinuity( U32 index ) const
{
	BIZ_ASSERT( index < (U32)m_customVector4Curves.Size() );
	return m_customVector4Curves[index].HasDiscontinuity(m_aftkScene->GetCurrentTime());
}

void C_Scene::DebugPrintCustomCurves( Debug::Logger& logger ) const
{
	LOGINFO( logger, "======== Custom Curves =======\n" );
	for ( S32 i = 0; i < m_customScalarCurves.Size(); ++i )
	{
		LOGINFO( logger, "Scalar  [%3d] %s\n", i, m_customScalarCurves[i].name.Chars() );
	}
	for ( S32 i = 0; i < m_customVector3Curves.Size(); ++i )
	{
		LOGINFO( logger, "Vector3 [%3d] %s\n", i, m_customVector3Curves[i].name.Chars() );
	}
	for ( S32 i = 0; i < m_customVector4Curves.Size(); ++i )
	{
		LOGINFO( logger, "Vector4 [%3d] %s\n", i, m_customVector4Curves[i].name.Chars() );
	}		
	LOGINFO( logger, "==============================\n" );
}

//
// Reference transforms. These are typically static world space transforms that are not part of the node hierarchy.
//
U32 C_Scene::GetNumReferenceTransforms() const
{
	return m_referenceTransforms.Size();
}

const CHAR8* C_Scene::GetReferenceTransformName( U32 refTransformIndex ) const
{
	return m_referenceTransforms[ refTransformIndex ].name.Chars();
}

bool C_Scene::IsReferenceTransformAnimated( U32 refTransformIndex ) const
{
	const Aftk::C_SceneNode* node = m_referenceTransforms[ refTransformIndex ].aftkSceneNode;
	
	const Aftk::C_NullPlug* parentPlugs[3] = { 0, 0, 0 };
	
	parentPlugs[0] = &node->GetRotatePlug();
	parentPlugs[1] = &node->GetTranslatePlug();
	parentPlugs[2] = &node->GetScalePlug();
		
	bool animated = false;
	
	for ( U32 i = 0; i < 3; ++i )
	{
		if ( parentPlugs[i] == 0 )
		{
			continue;
		}
		
		for ( U32 j = 0; j < 3; ++j )
		{
			animated = animated || IsKeyframedChannel( parentPlugs[i]->GetChildPtr(j) );
		}
	}
	return animated;
}

void C_Scene::GetCurrentWorldReferenceTransform( U32 refTransformIndex, Animation::C_JointTransform& outWorldTransform ) const
{
	Maths::Mat43 m;
	GetCurrentWorldReferenceTransform( refTransformIndex, m );
	outWorldTransform = Animation::C_JointTransform( m );
}

void C_Scene::GetCurrentWorldReferenceTransform( U32 refTransformIndex, Animation::C_ObjectTransform& outWorldTransform ) const
{
	Maths::Mat43 m;
	GetCurrentWorldReferenceTransform( refTransformIndex, m );
	outWorldTransform = Animation::C_ObjectTransform( m );
}

void C_Scene::GetCurrentWorldReferenceTransform( U32 refTransformIndex, Maths::Mat43& outWorldTransform ) const
{
	const C_ReferenceTransform& rt = m_referenceTransforms[refTransformIndex];
	Maths::Mat43 m43;
	ConvertAftkMatrix( m43, rt.aftkSceneNode->GetWorldMatrix() );
	outWorldTransform = m_sceneToGameTransform.ToGameSpace( m43 );
}

void C_Scene::DebugPrintReferenceTransforms( Debug::Logger& logger ) const
{
	if (0 == m_referenceTransforms.Size())
	{
		return;
	}
	LOGINFO( logger, "====== Reference Transforms ======\n" );
	for ( S32 i = 0; i < m_referenceTransforms.Size(); ++i )
	{
		Animation::C_JointTransform jt;
		GetCurrentWorldReferenceTransform( i, jt );
		
		const Maths::Vec4F32 q = Maths::GetVec4F32(jt.GetRotation());
		const Maths::Vector3 t = jt.GetTranslation();
	
		LOGINFO( logger, "[%3d] %s Q:(%.3f, %.3f, %.3f, %.3f) T:(%.3f, %.3f, %.3f)",
			i,
			m_referenceTransforms[ i ].name.Chars(),
			Maths::GetF32(Maths::X(q)),
			Maths::GetF32(Maths::Y(q)),
			Maths::GetF32(Maths::Z(q)),
			Maths::GetF32(Maths::W(q)),
			Maths::GetF32(Maths::X(t)),
			Maths::GetF32(Maths::Y(t)),
			Maths::GetF32(Maths::Z(t))
		 );
	}
	LOGINFO( logger, "==================================\n" );
}

const C_ExportConstraints& C_Scene::GetConstraints() const
{
	return *m_constraints;
}

inline U32 Hash(const Btl::String& str)
{
	return Btl::Hash(str.Chars());
}

class StringHashKeyOp
{
public:
	//Hashing function.
	static U32 GetHash(const Btl::String& key)
	{
		return Btl::HashRaw((const U8*)key.Chars(), key.Length());
	}

	static bool Equal(const Btl::String& key1, const Btl::String& key2)
	{
		if (key1 == key2)
			return true;

		return false;
	}
};

C_Status C_Scene::VerifyNodeNamesAreUnique( const Aftk::C_Scene& scene, const C_SceneConfig& sceneConfig ) const
{
	Btl::OpenHashSet<Btl::String, StringHashKeyOp> names;
	Btl::OpenHashSet<Btl::String, StringHashKeyOp> duplicateNames;
	
	bool foundDuplicates = false;
	
	Btl::String errorMsg = "Cannot import scene because it contains duplicate node names";
	
	if (sceneConfig.removeNamespaces)
	{
		errorMsg += " after namespace removal";
	}
	errorMsg += ":\n\n";
	
	Btl::String nodeName;
	for ( Aftk::C_SceneNodeConstIterator sceneNodeIter(&scene); sceneNodeIter; ++sceneNodeIter )
	{
		const Aftk::C_SceneNode* sceneNode = sceneNodeIter;
		nodeName = GetCleanAftkSceneNodeName(sceneNode, sceneConfig).Chars();
		
		if (names.Find(nodeName) == names.End())
		{
			names.Insert(nodeName);
		}
		else
		{
			foundDuplicates = true;
			if (duplicateNames.Find(nodeName) == duplicateNames.End())
			{
				errorMsg += "    - ";
				errorMsg += nodeName;
				errorMsg += "\n";
				
				duplicateNames.Insert(nodeName);
			}
		}
	}
	
	if (foundDuplicates)
	{
		return C_Status(false, errorMsg);
	}
	return C_Status(true);
}

const Aftk::C_SceneNode* C_Scene::GetRootNode( const Aftk::C_Scene& scene, const C_SceneConfig& sceneConfig ) const
{
	for ( Aftk::C_SceneNodeConstIterator sceneNodeIter(&scene); sceneNodeIter; ++sceneNodeIter )
	{
		const Aftk::C_SceneNode* sceneNode = sceneNodeIter;
			
		if ( sceneConfig.rootNodeNames.Match( GetCleanAftkSceneNodeName(sceneNode, sceneConfig ) ) )
		{
			return sceneNode;
		}
	}
		
	// Couldn't find a valid root node - just use the scene root.
	return scene.GetRootSceneNodePtr();
}

const Aftk::C_SceneNode* C_Scene::FindCustomWorldOriginNode( const Aftk::C_Scene& scene, const C_SceneConfig& sceneConfig )
{
	for ( Aftk::C_SceneNodeConstIterator sceneNodeIter(&scene); sceneNodeIter; ++sceneNodeIter )
	{
		const Aftk::C_SceneNode* sceneNode = sceneNodeIter;
			
		if ( sceneConfig.worldOriginNodeNames.Match( GetCleanAftkSceneNodeName(sceneNode, sceneConfig ) ) )
		{
			return sceneNode;
		}
	}
	return NULL;
}

void C_Scene::BuildHierarchy( const Aftk::C_SceneNode* root, S32 parentIndex, const C_SceneConfig& sceneConfig, const I_Filter& filter )
{
	if ( !root )
	{
		return;
	}

	const Aftk::C_SceneNode* currentNode = root;
	const S32 firstIndex = m_nodes.Size();
	
	// The root node is assumed to have no siblings.
	bool ignoreSiblings = parentIndex < 0;
	
	// Add sibling joints.
	while ( currentNode )
	{
		if ( filter.IsValidNode( currentNode ) )
		{
			AddNode( currentNode, parentIndex, sceneConfig );
		}
		
		if ( ignoreSiblings )
		{
			break;
		}
		currentNode = currentNode->GetNextPtr();
	}
	
	// Recursively add child nodes of siblings.
	currentNode = root;
	S32 currentIndex = firstIndex;
	
	while ( currentNode )
	{
		const bool isValidNode = filter.IsValidNode( currentNode );
		const Aftk::C_SceneNode* childNode = currentNode->GetChildPtr();
	
		if ( childNode )
		{
			if ( isValidNode )
			{
				BuildHierarchy( childNode, currentIndex, sceneConfig, filter );
			}
			else
			{
				// Not a valid node - pass in parent index.
				BuildHierarchy( childNode, parentIndex, sceneConfig, filter );
			}
		}
		
		if ( isValidNode )
		{
			++currentIndex;
		}
		
		if ( ignoreSiblings )
		{
			break;
		}
		
		currentNode = currentNode->GetNextPtr();		
	}
}



bool C_Scene::IsValidNode( const Aftk::C_SceneNode* node, const C_SceneConfig& sceneConfig ) const
{
	if ( NULL == node ) return false;

	// Ignore constraints.
	if ( node->HasType( Aftk::E_ObjType_ISceneConstraint ) )
	{
		return false;
	}
	
	// Ignore excluded nodes
	Util::C_String nodeName = GetCleanAftkSceneNodeName( node, sceneConfig );
	if ( sceneConfig.excludedNodeNames.Match( nodeName ) )
	{
		//LOGINFO( gLog, "Node '%s' is excluded from skeleton\n", node->GetName().c_str() );
		return false;
	}
	
	// Ignore reference marker nodes as these are extracted in a separate pass.
	if ( sceneConfig.referenceNodeNames.Match( nodeName ) )
	{
		return false;
	}

	return true;
}

void C_Scene::AddNode( const Aftk::C_SceneNode* aftkSceneNode, S32 parentIndex, const C_SceneConfig& sceneConfig )
{
	if ( aftkSceneNode == NULL ) return;
	
	m_nodes.PushBack( C_Node( aftkSceneNode, GetCleanAftkSceneNodeName( aftkSceneNode, sceneConfig ), parentIndex ) );
	m_nodes.Back().isHelper = sceneConfig.helperNodeNames.Match(m_nodes.Back().name);

	bool isHighFidelity = false;
	if (parentIndex >= 0)
	{
		if (m_nodes[parentIndex].isHighFidelity)
		{
			isHighFidelity = true;
		}
	}
	if (!isHighFidelity)
	{
		isHighFidelity = sceneConfig.highFidelityRootNodeNames.Match(m_nodes.Back().name);
	}
	m_nodes.Back().isHighFidelity = isHighFidelity;
	m_nodes.Back().isModelElement = IsModelElement(m_nodes.Back().name, sceneConfig);
}

//Util::C_String C_Scene::GetCleanAftkSceneNodeName( const Aftk::C_SceneNode* node, const C_SceneConfig& sceneConfig ) const
//{
//	if ( node == NULL ) return Util::C_String();
//	Util::C_String name( node->GetName().c_str() );
//	return name.Lower();
//}

// Import and process driven joints.
C_Status C_Scene::ImportConstraints( const C_SceneConfig& sceneConfig )
{
	m_constraints->Clear();
	
	for ( S32 i = 0; i < m_nodes.Size(); ++i )
	{
		const Aftk::C_SceneNode* sceneNode = m_nodes[i].aftkSceneNode;
		bool ok = m_constraints->ImportConstraintsFromAftkSceneNode( *sceneNode, *this );
		
		if ( !ok )
		{
			return C_Status(false);
		}
	}
	
	C_Status result = m_constraints->Sort( *this );
	return result;
}

bool IsScalarPlug( const Aftk::I_Plug& plug )
{
	switch (plug.GetDataType())
	{
		case Aftk::E_GenericAttributeType_Boolean:
		case Aftk::E_GenericAttributeType_U8:
		case Aftk::E_GenericAttributeType_U16:
		case Aftk::E_GenericAttributeType_U32:
		case Aftk::E_GenericAttributeType_U64:
		case Aftk::E_GenericAttributeType_S8: 
		case Aftk::E_GenericAttributeType_S16:
		case Aftk::E_GenericAttributeType_S32:
		case Aftk::E_GenericAttributeType_S64:
		case Aftk::E_GenericAttributeType_F32:
		case Aftk::E_GenericAttributeType_F64:
			return true;
			
		default:
			break;
	}
	return false;
}

bool IsDescendedFrom(const Aftk::C_SceneNode& node, const Aftk::C_SceneNode* root)
{
	if (root == NULL) return true;
	
	const Aftk::C_SceneNode* n = &node;
	while (n != NULL && n != root)
	{
		n = n->GetParentPtr();
	}
	return n == root;
}

C_Status C_Scene::ImportAnimatedNodeAttributes( const C_SceneConfig& sceneConfig )
{
	// Scan all nodes in the Aftk scene, examining their dynamic plugs (i.e. Maya attributes or MotionBuilder custom properties).
	// Only specifically excluded nodes are skipped to allow some means of stripping out unnecessary curves, if necessary.
	const Aftk::C_SceneNode* rootNode = GetRootNode(*m_aftkScene, sceneConfig);
	
	for (Aftk::C_SceneNodeConstIterator nodeIter(m_aftkScene); nodeIter; ++nodeIter)
	{
		const Aftk::C_SceneNode& sceneNode = *nodeIter;
		
		bool partOfRootHierarchy = IsDescendedFrom(sceneNode, rootNode);
		
		const Util::C_String nodeName = GetCleanAftkSceneNodeName( &sceneNode, sceneConfig );
		if ( sceneConfig.excludedNodeNames.Match( nodeName ) )
		{
			// Skip node that has been specifically excluded.
			continue;
		}	
		
		Util::C_String cleanNodeName = GetCleanAftkSceneNodeName(&sceneNode, sceneConfig);
		
		// Export keyframed visibility channels.
		{
			const Aftk::I_Plug* plug = &sceneNode.GetVisibilityPlug();

			if (IsScalarPlug(*plug) && IsKeyframedChannel(plug) && IsModelElement(cleanNodeName, sceneConfig))
			{
				Util::C_String curveName = Util::C_String::Format( "%s.%s", cleanNodeName.Chars(), plug->GetName().c_str() );
				m_customScalarCurves.PushBack( C_CustomScalarCurve( plug, curveName ) );
			}
		}	
		
		// If node is a camera, export FOV and depth of field curves.
		if (sceneNode.HasType(Aftk::E_ObjType_SceneCamera))
		{
			const Aftk::C_SceneCameraNode& cameraNode = static_cast<const Aftk::C_SceneCameraNode&>(sceneNode);
			
			Util::C_String curveName = Util::C_String::Format( "%s.fov", cleanNodeName.Chars() );
			
			m_customScalarCurves.PushBack(C_CustomScalarCurve(curveName, new C_CameraFovCurveSource(cameraNode)));
		}
	
		//
		// Export attributes on dynamic plugs.
		//
		const U32 numDynamicPlugs = sceneNode.GetNumDynamicPlugs();

		if ( numDynamicPlugs == 0 )
		{
			// Skip nodes with no dynamic plugs.
			continue;
		}
			
		bool firstPlugSeen = true;
		
		for ( U32 plugIndex = 0; plugIndex < numDynamicPlugs; ++plugIndex )
		{
			const Aftk::C_DynamicPlug* plug = sceneNode.GetDynamicPlugPtr( plugIndex );
			
			Aftk::E_GenericAttributeType attrType = plug->GetDataType();
			
			// TODO: expand to support the following types:
			// Vector4F32, Vector4F64, Vector3F32, Vector3F64, F32, F64.
			
			Util::C_String attributeName = plug->GetName().c_str();
			
			if (sceneConfig.excludedAttributeNames.Match(attributeName))
			{
				// Skip ignored plugs.
				continue;
			}
				
			if ( !IsScalarPlug( *plug ) )
			{
				// Skip non-scalar attributes.
				continue;
			}
			
			if ( !IsKeyframedChannel( plug ) )
			{
				// Skip non-animated attributes.
				continue;
			}
			
			if ( firstPlugSeen )
			{
				//LOGDEBUG( gLog, "== Node %s\n", sceneNode.GetName().c_str() );
				firstPlugSeen = false;
			}
			
			volatile Aftk::E_ObjType sourceType = plug->GetSourceConnectionPtr()->GetType();
			//LOGDEBUG( gLog, "    %s (source: %s)\n", plug->GetName().c_str(), plug->GetSourceConnectionPtr()->GetName().c_str() );
			
			Util::C_String curveName = Util::C_String::Format( "%s.%s", cleanNodeName.Chars(), plug->GetName().c_str() );
			
			m_customScalarCurves.PushBack( C_CustomScalarCurve( plug, curveName ) );
		}
	}

	return C_Status(true);
}

// Find a material node plug by name. Case sensitive.
const Aftk::C_DynamicPlug* FindMaterialPlug( const Aftk::C_MaterialNode& materialNode, const CHAR8* plugName )
{
	for ( U32 i = 0; i < materialNode.GetNumPlugs(); ++i )
	{
		const Aftk::C_DynamicPlug* plug = materialNode.GetPlugPtr( i );
		if ( plug->GetName() == plugName )
		{
			return plug;
		}
	}
	return NULL;
}

const Aftk::C_ShaderNode* GetConnectedShaderNode( const Aftk::I_Plug& plug )
{
	if ( plug.IsConnected() )
	{
		const Aftk::I_Object* sourceConnection = plug.GetSourceConnectionPtr();
		if (sourceConnection->HasType(Aftk::E_ObjType_Shader))
			return static_cast<const Aftk::C_ShaderNode*>( sourceConnection );
			
		if ( sourceConnection->HasType(Aftk::E_ObjType_IPlug) )
		{
			const Aftk::I_Plug* intermediatePlug = static_cast<const Aftk::I_Plug*>( sourceConnection );
			
			if ( intermediatePlug->GetTopLevelParentPtr()->HasType( Aftk::E_ObjType_Shader ) )
			{
				return static_cast<const Aftk::C_ShaderNode*>( intermediatePlug->GetTopLevelParentPtr() );
			}
		}
	}

	return NULL;
}



bool IsVectorPlug( const Aftk::I_Plug& plug )
{
	bool isVector = true;
	
	Aftk::E_GenericAttributeType dataType = plug.GetDataType();
	if ( plug.IsCompound() )
	{
		dataType = plug.GetCompoundType();
	}

	switch ( dataType )
	{
		case Aftk::E_GenericAttributeType_Vector3F32:
		case Aftk::E_GenericAttributeType_Vector3F64:
		case Aftk::E_GenericAttributeType_Vector4F32:
		case Aftk::E_GenericAttributeType_Vector4F64:
			break;
			
		// TODO: handle all the various integer variations?
			
		default:
			isVector = false;
			break;
	}
	return isVector;
}

// Returns a list of plugs for each element of the vector value represented by plug.
bool GetVectorElementPlugs( const Aftk::I_Plug& plug, Btl::Vector<const Aftk::I_Plug*>& outPlugs )
{
	outPlugs.Clear();
	
	if ( plug.IsCompound() )
	{
		bool unknownType = false;
		
		switch ( plug.GetCompoundType() )
		{
			case Aftk::E_GenericAttributeType_Vector3F32:
			case Aftk::E_GenericAttributeType_Vector3F64:
				// Vector3.
				//LOGDEBUG( gLog, "Vector3" );
				break;

			case Aftk::E_GenericAttributeType_Vector4F32:
			case Aftk::E_GenericAttributeType_Vector4F64:
				// Vector4.
				//LOGDEBUG( gLog, "Vector4" );
				break;
				
			default:
				unknownType = true;
				//LOGDEBUG( gLog, "Unknown type" );
				break;
		}		
		
		if ( unknownType )
		{
			// Look for plug with RGB and A (alpha) inputs.
			if ( plug.GetNumChildren() != 2 )
			{
				return false;
			}
			
			const Aftk::I_Plug* rgbPlug = NULL;
			const Aftk::I_Plug* alphaPlug = NULL;
			
			if ( plug.GetChildPtr(0)->IsCompound() )
			{
				rgbPlug = plug.GetChildPtr(0);
			}
			else
			{
				alphaPlug = plug.GetChildPtr(0);
			}

			if ( plug.GetChildPtr(1)->IsCompound() )
			{
				rgbPlug = plug.GetChildPtr(1);
			}
			else
			{
				alphaPlug = plug.GetChildPtr(1);
			}
			
			if ( rgbPlug == NULL || alphaPlug == NULL )
			{
				return false;
			}

			if ( !IsVectorPlug( *rgbPlug ) )
			{
				return false;
			}
			
			//LOGDEBUG( gLog, "RBGA plug" );
			
			for ( U32 i = 0; i < rgbPlug->GetNumChildren(); ++i )
			{
				outPlugs.PushBack( rgbPlug->GetChildPtr(i) );
			}
			
			outPlugs.PushBack( alphaPlug );
		}
		else
		{
			for ( U32 i = 0; i < plug.GetNumChildren(); ++i )
			{
				outPlugs.PushBack( plug.GetChildPtr(i) );
			}
			return true;
		}
	}
	else
	{
		if ( IsScalarPlug( plug ) )
		{
			//LOGDEBUG( gLog, "Scalar plug" );
			outPlugs.PushBack( &plug );
			return true;
		}
	}
	return false;
}


C_Status C_Scene::ImportAnimatedMaterialParameters( const C_SceneConfig& sceneConfig )
{
	typedef Btl::Set<const Aftk::C_MaterialResource*> C_MaterialSet;
	
	C_MaterialSet materialSet;

	const Aftk::C_SceneNode* rootNode = GetRootNode(*m_aftkScene, sceneConfig);
	
	// Build a set of all materials referenced by mesh nodes (including skinned ones) in the scene.
	for (Aftk::C_SceneNodeConstIterator nodeIter(m_aftkScene); nodeIter; ++nodeIter)
	{
		const Aftk::C_SceneNode& sceneNode = *nodeIter;

		// Only consider mesh materials.
		if ( !sceneNode.HasType( Aftk::E_ObjType_SceneMesh ) )
		{
			continue;
		}
		
		bool partOfRootHierarchy = IsDescendedFrom(sceneNode, rootNode);
		
		const Util::C_String nodeName = GetCleanAftkSceneNodeName( &sceneNode, sceneConfig );
		if ( sceneConfig.excludedNodeNames.Match( nodeName ) )
		{
			// Skip meshes that have been specifically excluded.
			continue;
		}
		
		// Insert materials from each polygon into the material set.
		const Aftk::C_SceneMeshNode& meshNode = static_cast<const Aftk::C_SceneMeshNode&>( sceneNode );
		
		for ( U32 lodLevel = 0; lodLevel < meshNode.GetNumLodLevels(); ++lodLevel )
		{
			for ( U32 polyIndex = 0; polyIndex < meshNode.GetNumPolygons(lodLevel); ++polyIndex )
			{
				const Aftk::C_Polygon& poly = meshNode.GetPolygon( lodLevel, polyIndex );
				
				const Aftk::C_MaterialResource* materialResource = meshNode.GetPolygonMaterialMappingPtr(poly.GetMaterialMappingIndex());
				
				if ( materialResource != NULL )
				{
					materialSet.Insert( materialResource );
				}
			}
		}
	}
	
	// For each material, find its animated parameters.
	for ( C_MaterialSet::Iterator materialIter = materialSet.Begin(); materialIter != materialSet.End(); ++materialIter )
	{
		const Aftk::C_MaterialResource* materialResource = *materialIter;
		const Aftk::C_MaterialNode& materialNode = materialResource->GetContent();
		
		//LOGDEBUG( gLog, "Material '%s'", materialNode.GetName().c_str() );
		
		const Aftk::C_DynamicPlug* surfaceShaderPlug = FindMaterialPlug( materialNode, "mayaSurfaceShader" );
		
		if ( surfaceShaderPlug == NULL )
		{
			continue;
		}
		
		const Aftk::C_ShaderNode* shaderNode = GetConnectedShaderNode( *surfaceShaderPlug );
		
		if ( shaderNode == NULL )
		{
			continue;
		}
		
		//LOGDEBUG( gLog, "Has shader node '%s'", shaderNode->GetName().c_str() );
		
		Btl::Vector<const Aftk::I_Plug*> vectorElementPlugs;
		for ( U32 i = 0; i < shaderNode->GetNumPlugs(); ++i )
		{
			const Aftk::I_Plug* plug = shaderNode->GetPlugPtr( i );
			Util::C_String plugName = plug->GetName().c_str();
			
			//LOGDEBUG( gLog, "[%2d] Static %splug '%s' connected to '%s'", i, plug->IsCompound() ? "compound " : "", plug->GetName().c_str(), plug->GetConnectionRoot()->GetName().c_str() );
						
			
			vectorElementPlugs.Clear();
			GetVectorElementPlugs( *plug, vectorElementPlugs );
			
			if ( vectorElementPlugs.Size() > 0 )
			{
				bool animated = false;
				for ( S32 j = 0; j < vectorElementPlugs.Size(); ++j )
				{
					if ( IsKeyframedChannel( vectorElementPlugs[j] ) )
					{
						animated = true;
						break;
					}
				}
				
				if ( animated )
				{
					Util::C_String curveName = Util::C_String::Format( "%s.%s", materialNode.GetName().c_str(), plugName.Chars() );
				
					switch ( vectorElementPlugs.Size() )
					{
						case 1:
							m_customScalarCurves.PushBack( C_CustomScalarCurve( vectorElementPlugs[0], curveName ) );
							break;
						
						case 2:
							break;
						
						case 3:
							m_customVector3Curves.PushBack( C_CustomVector3Curve( &vectorElementPlugs[0], curveName ) );
							break;
						
						case 4:
							m_customVector4Curves.PushBack( C_CustomVector4Curve( &vectorElementPlugs[0], curveName ) );
							break;
							
						default:
							break;
					}				
				}
			}
		}

		for ( U32 i = 0; i < shaderNode->GetNumDynamicPlugs(); ++i )
		{
			const Aftk::C_DynamicPlug* plug = shaderNode->GetDynamicPlugPtr( i );
			//LOGDEBUG( gLog, "[%2d] Dynamic plug '%s' connected to '%s'", i, plug->GetName().c_str(), plug->GetConnectionRoot()->GetName().c_str() );
		}

	}

	return C_Status(true);
}

C_Status C_Scene::ImportReferenceTransforms( const C_SceneConfig& sceneConfig )
{
	m_referenceTransforms.Clear();

	for ( Aftk::C_SceneNodeConstIterator iter( m_aftkScene ); iter; ++iter )
	{
		const Aftk::C_SceneNode& sceneNode = *iter;
		
		Util::C_String name = GetCleanAftkSceneNodeName( &sceneNode, sceneConfig );
		
		if ( sceneConfig.referenceNodeNames.Match( name ) )
		{
			m_referenceTransforms.PushBack( C_ReferenceTransform( &sceneNode, name ) );
		}
	}

	return C_Status( true );
}

void C_Scene::ProcessInfluenceNodes(Btl::Vector<const Aftk::C_SceneNode*>* outMissingInfluenceNodes)
{
	// Mark all influence nodes as joints, in order to handle XSI scenes where there is no specific joint type.
	for (Aftk::C_SceneNodeConstIterator nodeIter(m_aftkScene); nodeIter; ++nodeIter)
	{
		const Aftk::C_SceneNode& node = *nodeIter;

		if (node.GetNumInfluenceClusters() <= 0)
		{
			continue;
		}

		for (U32 clusterIndex = 0; clusterIndex < node.GetNumInfluenceClusters(); ++clusterIndex)
		{
			const Aftk::C_InfluenceClusterNode* influenceCluster = node.GetInfluenceClusterPtr(clusterIndex);
			const U32 numEnvelopes = influenceCluster->GetNumEnvelopes();

			for (U32 envIndex = 0; envIndex < numEnvelopes; ++envIndex)
			{
				const Aftk::C_WeightEnvelope& weightEnvelope = influenceCluster->GetEnvelope(envIndex);
				const Aftk::C_SceneNode* influenceNode = weightEnvelope.GetInfluenceNodePtr();

				S32 nodeIndex = GetNodeIndexByAftkSceneNode(influenceNode);
				if (nodeIndex >= 0)
				{
					// Mark node as being an influence node.
					m_nodes[nodeIndex].isInfluenceNode = true;
				}
				else if (outMissingInfluenceNodes)
				{
					// Found an influence node, but it's not in the animation scene - this could be an error, so build a list.
					bool alreadyInserted = false;
					for (S32 i = 0; i < outMissingInfluenceNodes->Size() && !alreadyInserted; ++i)
					{
						if ((*outMissingInfluenceNodes)[i] == influenceNode)
						{
							alreadyInserted = true;
						}
					}
					
					if (!alreadyInserted)
					{
						outMissingInfluenceNodes->PushBack(influenceNode);
					}
				}
			}
		}
	}
}


//
// C_Scene::C_PlugCurveSource::C_CurveSource
//

void C_Scene::C_PlugCurveSource::Evaluate( F32* outValue ) const
{
	for ( S32 i = 0; i < plugs.Size(); ++i )
	{
		const Aftk::I_Plug* plug = plugs[i];
		BIZ_ASSERT( plug );
		
		F32 value = 0.f;
		
		if ( plug )
		{
			switch ( plug->GetDataType() )
			{
				case Aftk::E_GenericAttributeType_Boolean:
				{
					bool x;
					plug->GetValue(x);
					value = x ? 1.f : 0.f;
					break;
				}
								
				case Aftk::E_GenericAttributeType_U8:
				{
					U8 x;
					plug->GetValue(x);
					value = static_cast<F32>(x);
					break;
				}
				case Aftk::E_GenericAttributeType_U16:
				{
					U16 x;
					plug->GetValue(x);
					value = static_cast<F32>(x);
					break;
				}
				case Aftk::E_GenericAttributeType_U32:
				{
					U32 x;
					plug->GetValue(x);
					value = static_cast<F32>(x);
					break;
				}
					
				case Aftk::E_GenericAttributeType_U64:
				{
					U64 x;
					plug->GetValue(x);
					value = static_cast<F32>(x);
					break;
				}

				case Aftk::E_GenericAttributeType_S8: 
				{
					S8 x;
					plug->GetValue(x);
					value = static_cast<F32>(x);
					break;
				}
				
				case Aftk::E_GenericAttributeType_S16:
				{
					S16 x;
					plug->GetValue(x);
					value = static_cast<F32>(x);
					break;
				}
				
				case Aftk::E_GenericAttributeType_S32:
				{
					S32 x;
					plug->GetValue(x);
					value = static_cast<F32>(x);
					break;
				}
				
				case Aftk::E_GenericAttributeType_F32:
					plug->GetValue( value );
					break;
					
				case Aftk::E_GenericAttributeType_F64:
				{
					F64 tmpValue = 0.0;
					plug->GetValue( tmpValue );
					value = static_cast< F32 >( tmpValue );
					break;
				}
				
				
				default:
					BIZ_ASSERT( false );
					break;
			}
		}
		outValue[i] = value;
	}
	
}

bool C_Scene::C_PlugCurveSource::HasDiscontinuity( F32 currentTime ) const
{
	return AnyPlugHasDiscontinuity( &plugs[0], plugs.Size(), currentTime );
}


} // end AnimationTools

#include <system/serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_MemoryStream.h>

namespace Maths
{
	template < typename T >
	void Serialise( T& out, const Maths::Mat43& m )
	{
		Maths::Serialise( m, out );
	}

	template < typename T >
	void Serialise( T& out, const Maths::Vector3& v )
	{
		Maths::Serialise( v, out );
	}
}

namespace AnimationTools
{

C_Status SerialiseSkeleton( const C_Scene& scene, Sys::I_BinaryStream& stream )
{
	Sys::BinarySerialiserAutoEndian out( stream );
	
	const U32 kCurrentVersion = 7;
	
	out << U32( kCurrentVersion );
	
	const U32 numNodes = scene.GetNumNodes();
	
	const U32 kMaxNodes = 32767;
	if ( numNodes > kMaxNodes )
	{
		return C_Status( false, Btl::String::Format( "Scene has too many nodes (has %d, needs to be less than %d)", numNodes, kMaxNodes ) );
	}
	
	
	out << U32( numNodes );
	
	// Write parent indices.
	for ( U32 i = 0; i < numNodes; ++i )
	{
		S16 parentIndex = scene.GetNodeParentIndex( i );
		out << parentIndex;
	}

	// Write local base pose transforms as C_JointTransforms.
	for ( U32 i = 0; i < numNodes; ++i )
	{
		Animation::C_JointTransform localBasePoseTransform;
		scene.GetLocalNodeBindPoseTransform( i, localBasePoseTransform );
		out << localBasePoseTransform;
	}
	
	// Write inverse world base pose transforms.
	for ( U32 i = 0; i < numNodes; ++i )
	{
		typedef Animation::C_JointTransform BindPoseTransform;
	
		BindPoseTransform invWorldBasePoseTransform;
		scene.GetInverseWorldNodeBindPoseTransform( i, invWorldBasePoseTransform );
		
		Maths::Mat43 invWorldBasePoseMatrix = invWorldBasePoseTransform.AsMat43();
		out << invWorldBasePoseMatrix;
	}
	
	// Write joint constraints (see C_ExportConstraints).
	out << scene.GetConstraints();
	
	// Write node names.
	out << U32( numNodes );
	for ( U32 i = 0; i < numNodes; ++i )
	{
		out << Util::C_String( scene.GetNodeName( i ) );
	}
	
	return C_Status( true );
}

C_Status DeserialiseSkeleton( Animation::C_Skeleton& outSkeleton, Sys::I_BinaryStream& stream )
{
	Sys::BinaryDeserialiserAutoEndian in( stream );
	
	in >> outSkeleton;
	
	return C_Status( true );
	
}


} // end Animation
