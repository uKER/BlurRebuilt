#include "stdafx.h"
#include <animation/tools/ExportConstraints.h>
#include "ConstraintHelpers.h"
#include <animation/common/JointTransform.h>
#include <animation/tools/Scene.h>
#include <system/Serialise.h>
#include <system/serialise/TextSerialise.h>
#include <Aftk/AftkEuler.h>

namespace AnimationTools
{

const U32 kMaxConstraintsOfSameType = 255;

#if CFG_DEBUG
#define DBGLOG( ... )	LOGINFO( gLog, __VA_ARGS__ )
#else
#define DBGLOG( ... )
#endif

//
// C_ExportConstraints
//

void ConvertToJointTransform( Animation::C_JointTransform& jt, const Aftk::Matrix44& m, const C_SceneToGameTransform& sceneToGameTransform )
{
	Maths::Mat43 m43;
	ConvertAftkMatrix( m43, m );
	jt = Animation::C_JointTransform( sceneToGameTransform.ToGameSpace( m43 ) );
}

namespace
{
	Debug::Logger& gLog = Debug::GetLogger( "mercury.animation" );
}

void C_ExportConstraints::Clear()
{
	m_aimConstraints.Clear();
	m_orientationConstraints.Clear();
	m_positionConstraints.Clear();
	m_drivenKeyRotationConstraints.Clear();
	m_drivenKeyTranslationConstraints.Clear();
	m_dynamicAimJoints.Clear();
	m_dynamicSliderJoints.Clear();
	m_evalSequence.Clear();
}

bool C_ExportConstraints::ImportConstraintsFromAftkSceneNode(
	const Aftk::C_SceneNode& sceneNode,
	const C_Scene& scene
)
{
	m_sceneToGameTransform = scene.GetSceneToGameTransform();
	m_scene = &scene;

	//
	// Import any rotation constraints.
	//
	
	//DBGLOG( "\n======== %s ========\n", sceneNode.GetName().c_str() );

	C_AimConstraintHelper aimConstraintHelper( sceneNode );
	C_OrientationConstraintHelper orientationConstraintHelper( sceneNode );
	C_DrivenRotationKeyHelper drivenRotationKeyHelper( sceneNode );

	// NB. There are no dynamic joints in Nightwing skeletons until the move to the new tech has been completed.
#ifndef NIGHTWING_BRANCH_COMPATIBLE
	const bool isDynamicAimJoint = scene.GetSceneConfig().dynamicAimJointNames.Match( sceneNode.GetName().c_str() );
	const bool isDynamicSliderJoint = scene.GetSceneConfig().dynamicSliderJointNames.Match( sceneNode.GetName().c_str() );
	
	if ( isDynamicAimJoint )
	{
		return ImportDynamicAimJoint( sceneNode, aimConstraintHelper );
	}
	
	if ( isDynamicSliderJoint )
	{
		return ImportDynamicSliderJoint( sceneNode );
	}
#endif
	
	//
	// Import translation constraints first since aim constraints
	// may have to do calculations based on the world space position
	// of a constrained joint.
	//
	C_PositionConstraintHelper positionConstraintHelper( sceneNode );
	C_DrivenTranslationKeyHelper drivenTranslationKeyHelper( sceneNode );
	
	if ( positionConstraintHelper.IsValid() )
	{
		if ( !ImportPositionConstraint( positionConstraintHelper ) )
		{
			return false;
		}
	}
	else if ( drivenTranslationKeyHelper.IsValid() )
	{
		DBGLOG( "Found driven translation key for node '%s'", drivenTranslationKeyHelper.GetSceneNode().GetName().c_str() );
		
		ImportDrivenKeyConstraint( drivenTranslationKeyHelper, E_TransformChannel_Translation );
	}
	
	//
	// Import rotation constraints next.
	//
	if ( aimConstraintHelper.IsValid() )
	{
		if ( !ImportAimConstraint( aimConstraintHelper ) )
		{
			return false;
		}
	}
	else if ( orientationConstraintHelper.IsValid() )
	{
		if ( !ImportOrientationConstraint( orientationConstraintHelper ) )
		{
			return false;
		}
	}
	else if ( drivenRotationKeyHelper.IsValid() )
	{
		DBGLOG( "Found driven rotation key for node '%s'", drivenRotationKeyHelper.GetSceneNode().GetName().c_str() );
		
		ImportDrivenKeyConstraint( drivenRotationKeyHelper, E_TransformChannel_Rotation );
	}
			
	
	//DBGLOG( "======== %s END ========\n", sceneNode.GetName().c_str() );
		
	// Everything went smoothly.
	return true;
}

struct C_Edge
{
	C_Edge() {}
	C_Edge( U32 inFrom, U32 inTo ):
		from( inFrom ),
		to( inTo )
	{}
	U16	from;
	U16	to;
};

namespace
{

	bool IsDescendant( const C_Scene& scene, S32 nodeIndex, S32 potentialDescendantNodeIndex )
	{
		S32 parentIndex = potentialDescendantNodeIndex;
		while ( parentIndex >= 0 && parentIndex != nodeIndex )
		{
			parentIndex = scene.GetNodeParentIndex( parentIndex );
		}
		return parentIndex >= 0;	
	}
}

C_Status C_ExportConstraints::Sort( const C_Scene& scene )
{
	if ( m_evalSequence.Size() == 0 )
	{
		return C_Status(true);
	}

	const U32 numNodes = scene.GetNumNodes();
	const U32 numConstraints = m_evalSequence.Size();
	
	// Create linked lists of constraints for each node.
	Btl::Vector< S16 > nodeConstraints;
	nodeConstraints.Resize( numNodes );
	
	for ( U32 nodeIndex = 0; nodeIndex < numNodes; ++nodeIndex )
	{
		nodeConstraints[ nodeIndex ] = -1;
		for ( U32 constraintIndex = 0; constraintIndex < numConstraints; ++constraintIndex )
		{
			if ( m_evalSequence[ constraintIndex ].nodeIndex == nodeIndex )
			{
				m_evalSequence[ constraintIndex ].next = nodeConstraints[ nodeIndex ];
				nodeConstraints[ nodeIndex ] = constraintIndex;
			}
		}
	}
	
	// Build edge lists and count input edges per constraint.
	Btl::Vector< C_Edge > edgeList;
	Btl::Vector< S16 > inputCounts;
	inputCounts.Resize( numConstraints );

	for ( U32 i = 0; i < numConstraints; ++i )
	{
		inputCounts[ i ] = 0;
	}
	
	DBGLOG( "*** BEFORE\n" );
	for ( U32 constraintIndex = 0; constraintIndex < numConstraints; ++constraintIndex )
	{
		DBGLOG( "[ %d ] %d %s\n", constraintIndex, m_evalSequence[ constraintIndex ].nodeIndex, scene.GetNodeName( m_evalSequence[ constraintIndex ].nodeIndex ) );
	
		// Add constraints for first parent node.
		const C_ExportConstraintIndex& constraint = m_evalSequence[ constraintIndex ];
		
		// Add edges for any constraints controlling nodes above this one in the skeleton hierarchy.
		S32 parentnodeIndex = scene.GetNodeParentIndex( constraint.nodeIndex );
		while ( parentnodeIndex >= 0 )
		{
			S32 parentConstraintIndex = nodeConstraints[ parentnodeIndex ];
			if ( parentConstraintIndex >= 0 )
			{
				while ( parentConstraintIndex >= 0 )
				{
					edgeList.PushBack( C_Edge( parentConstraintIndex, constraintIndex ) );
					parentConstraintIndex = m_evalSequence[ parentConstraintIndex ].next;
					inputCounts[ constraintIndex ]++;
				}
				break;
			}
			parentnodeIndex = scene.GetNodeParentIndex( parentnodeIndex );
		}
		
		// Similarly, add edges for constraint inputs.
		for ( U32 inputIndex = 0; inputIndex < constraint.numInputs; ++inputIndex )
		{
			S32 nodeIndex = constraint.inputNodeIndices[ inputIndex ];
			if ( nodeIndex >= 0 )
			{
				while ( nodeIndex >= 0 )
				{
					S32 parentConstraintIndex = nodeConstraints[ nodeIndex ];
					if ( parentConstraintIndex >= 0 )
					{
						while ( parentConstraintIndex >= 0 )
						{
							edgeList.PushBack( C_Edge( parentConstraintIndex, constraintIndex ) );
							parentConstraintIndex = m_evalSequence[ parentConstraintIndex ].next;
							inputCounts[ constraintIndex ]++;
						}
						break;
					}
					nodeIndex = scene.GetNodeParentIndex( nodeIndex );
				}
			}
		}
		
		// To support legacy dynamic aim nodes that don't have a target node, add all
		// constraints as potential inputs.
	#if 0
		if ( constraint.type == E_ConstraintType_DynamicAimJoint && constraint.numInputs == 0 )
		{
			for ( U32 otherConstraintIndex = 0; otherConstraintIndex < numConstraints; ++otherConstraintIndex )
			{
				if ( constraintIndex != otherConstraintIndex )
				{
					// Ignore (already added) constraints for nodes higher up in the hierarchy and
					// don't constraints for nodes lower in the hierarchy to avoid creating cyclic dependencies.
					if ( !( IsDescendant( skeletonBuilder, m_evalSequence[ otherConstraintIndex ].nodeIndex, constraint.nodeIndex ) 
							|| IsDescendant( skeletonBuilder, constraint.nodeIndex, m_evalSequence[ otherConstraintIndex ].nodeIndex ) 
							|| m_evalSequence[ otherConstraintIndex ].type == constraint.type ) )
					{
						// Add constraint as a potential dependent.
						edgeList.PushBack( C_Edge( otherConstraintIndex, constraintIndex ) );
						inputCounts[ constraintIndex ]++;
					}
				}
			}		
		}
	#endif
	}
	
	// Topological sort.
	Btl::Vector< C_ExportConstraintIndex > sortedList;
	sortedList.Reserve( numConstraints );
	
	bool notFinished = true;
	
	U32 totalNonZeroInputs = 0;
	
	while ( notFinished )
	{
		notFinished = false;
		
		totalNonZeroInputs = 0;
		
		for ( U32 i = 0; i < numConstraints; ++i )
		{
			if ( inputCounts[ i ] == 0 )
			{
				inputCounts[ i ] = -1;
				notFinished = true;
				
				sortedList.PushBack( m_evalSequence[ i ] );
				
				// Decrement input count for all edges leading out from this constraint.
				for ( U32 j = 0, numEdges = edgeList.Size(); j < numEdges; ++j )
				{
					if ( edgeList[ j ].from == i )
					{
						inputCounts[ edgeList[ j ].to ]--;
					}
				}
			}
			else if ( inputCounts[ i ] > 0 )
			{
				totalNonZeroInputs++;
			}
		}
	}
	
	Ut::Swap( m_evalSequence, sortedList );

	static const CHAR8* constraintTypeNames[] =
	{
		"Aim",
		"Orientation",
		"Position",
		"DrivenKeyRotation",
		"DrivenKeyTranslation",
		"Custom",
		"DynamicAimJoint",
		"DynamicSliderJoint"
	};
	BIZ_STATIC_ASSERT((sizeof(constraintTypeNames)/sizeof(constraintTypeNames[0])) == Animation::E_ConstraintType_Count);


	if ( totalNonZeroInputs > 0 )
	{
		LOGERROR( Debug::GetLogger( "mercury.animation" ), "Cannot sort node constraints: there is a cycle in the dependency graph!\n" );

		for ( U32 i = 0; i < numConstraints; ++i )
		{
			if ( inputCounts[ i ] > 0 )
			{
				LOGWARN( Debug::GetLogger( "mercury.animation" ), "%s (%s, %d)\n", scene.GetNodeName( sortedList[ i ].nodeIndex ), constraintTypeNames[ sortedList[ i ].type ], i );
				// Dump inputs
				for ( U32 j = 0, numEdges = edgeList.Size(); j < numEdges; ++j )
				{
					S32 to = edgeList[ j ].to;
					if ( edgeList[ j ].from == i )
					{
					
						LOGWARN( Debug::GetLogger( "mercury.animation" ), "\tHas Input: %s (%s, %d)\n", scene.GetNodeName( sortedList[ to ].nodeIndex ), constraintTypeNames[ sortedList[ to ].type ], to );
					}
				}
			}
		}
		
		// TODO: return some kind of error
		return C_Status(false, "Cannot import constraints as there is a cycle in the dependency graph. Check Log window for details.");
	}
	
	DBGLOG( "*** AFTER\n" );
	
	for ( U32 i = 0; i < numConstraints; ++i )
	{
		C_ExportConstraint* constraint = const_cast<C_ExportConstraint*>(GetConstraint(i));
		if ( constraint )
		{
			constraint->order = i;
		}
		DBGLOG("[ %d ] %d %s (%s)\n", i, m_evalSequence[ i ].nodeIndex, scene.GetNodeName( m_evalSequence[ i ].nodeIndex ), constraintTypeNames[ m_evalSequence[ i ].type ] );
	}
	return C_Status(true);
}

const C_ExportConstraint* C_ExportConstraints::GetConstraint(U32 evalIndex) const
{
	Animation::E_ConstraintType type = m_evalSequence[evalIndex].type;
	S32 index = m_evalSequence[evalIndex].index;
	const C_ExportConstraint* c = NULL;
	switch (type)
	{
		case Animation::E_ConstraintType_Aim:
			c = &GetAimConstraint(index);
			break;
		case Animation::E_ConstraintType_Orientation:
			c = &GetOrientationConstraint(index);
			break;
		case Animation::E_ConstraintType_Position:
			c = &GetPositionConstraint(index);
			break;
		case Animation::E_ConstraintType_DrivenKeyRotation:
			c = &GetDrivenKeyRotationConstraint(index);
			break;
		case Animation::E_ConstraintType_DrivenKeyTranslation:
			c = &GetDrivenKeyTranslationConstraint(index);
			break;
		case Animation::E_ConstraintType_Custom:
			break;
		case Animation::E_ConstraintType_DynamicAimJoint:
			c = &GetDynamicAimJoint(index);
			break;
		case Animation::E_ConstraintType_DynamicSliderJoint:
			c = &GetDynamicSliderJoint(index);
			break;
			
		default:
			break;
	}
	return c;
}

// Ensures min and max bounds are the right way round.
void FixBounds( MathsStorage::Float3& mins, MathsStorage::Float3& maxs )
{
	for ( U32 i = 0; i < 3; ++i )
	{
		if ( mins.v[i] > maxs.v[i] )
		{
			F32 tmp = mins.v[i];
			mins.v[i] = maxs.v[i];
			maxs.v[i] = tmp;
		}		
	}	
}


bool C_ExportConstraints::ImportAimConstraint(
	const C_AimConstraintHelper& aimConstraintHelper
)
{
	BIZ_ASSERT( aimConstraintHelper.IsValid() );

	if (m_aimConstraints.Size() > kMaxConstraintsOfSameType)
	{
		LOGWARN(gLog, "Reached maximum number of aim constraints (%d)", kMaxConstraintsOfSameType);
		return false;
	}
	
	const Aftk::C_SceneNode& sceneNode = aimConstraintHelper.GetSceneNode();
	const Aftk::C_SceneNode* targetSceneNode = aimConstraintHelper.GetTargetSceneNode();
	
	C_ExportAimConstraint constraint;
	C_ExportConstraintIndex constraintIndex;
	constraintIndex.type = Animation::E_ConstraintType_Aim;
	constraintIndex.index = m_aimConstraints.Size();
	
	Aftk::E_UpVectorType aftkUpVectorType = aimConstraintHelper.GetUpVectorType();
	
	const Aftk::C_SceneNode* upVectorReferenceNode = aimConstraintHelper.GetUpVectorReferenceSceneNode();

	DBGLOG( "=====\n" );
	DBGLOG( "[%s] Aim constraint found with up vector type: %s, target node: %s, reference node: %s, axes: %c%c%c\n",
		sceneNode.GetName().c_str(),
		aimConstraintHelper.GetUpVectorTypeName(),
		targetSceneNode ? targetSceneNode->GetName().c_str() : "none",
		upVectorReferenceNode ? upVectorReferenceNode->GetName().c_str() : "none",
		aimConstraintHelper.GetConstrainedAxes() & 1 ? 'X' : '.',
		aimConstraintHelper.GetConstrainedAxes() & 2 ? 'Y' : '.',
		aimConstraintHelper.GetConstrainedAxes() & 4 ? 'Z' : '.'
	);
	
	constraint.aimVector = m_sceneToGameTransform.NormalToGameSpace( aimConstraintHelper.GetAimVector() );
	constraint.localUpVector = m_sceneToGameTransform.NormalToGameSpace( aimConstraintHelper.GetLocalUpVector() );
	constraint.referenceUpVector = m_sceneToGameTransform.NormalToGameSpace( aimConstraintHelper.GetReferenceUpVector() );
	
	DBGLOG( "Rotational offset: (%.3f, %.3f, %.3f), aimVector: (%.3f, %.3f, %.3f), localUpVector: (%.3f, %.3f, %.3f), refUpVector: (%.3f, %.3f, %.3f)\n",
		constraint.rotationOffset.x, constraint.rotationOffset.y, constraint.rotationOffset.z,
		constraint.aimVector.x, constraint.aimVector.y, constraint.aimVector.z,
		constraint.localUpVector.x, constraint.localUpVector.y, constraint.localUpVector.z,
		constraint.referenceUpVector.x, constraint.referenceUpVector.y, constraint.referenceUpVector.z
	);
	
	// Convert local rotation offset to a quaternion.
	ConvertEulerRotationToQuaternion(
		aimConstraintHelper.GetRotationOffset(),
		aimConstraintHelper.GetRotationOrder(),
		constraint.rotationOffset
	);

	if ( !targetSceneNode )
	{
		// No target node... report an error?
		return false;
	}

	constraint.targetNodeIndex = m_scene->GetNodeIndexByNearestAftkSceneNode( targetSceneNode );
		
	BIZ_ASSERT( constraint.targetNodeIndex >= 0 );
	if ( constraint.targetNodeIndex < 0 )
	{
		LOGERROR( gLog, "Aim constraint target node '%s' is outside skeleton hierarchy.\n", targetSceneNode->GetName().c_str() );
		return false;
	}
		
	// Calculate target's position in world space.
	Animation::C_JointTransform targetWorldTransform;
	ConvertToJointTransform( targetWorldTransform, targetSceneNode->GetWorldMatrix(), m_sceneToGameTransform );
	
	Animation::C_JointTransform targetJointWorldTransform;
	m_scene->GetCurrentWorldNodeTransform( constraint.targetNodeIndex, targetJointWorldTransform );
	
	// Calculate target's position in target reference joint space.
	targetWorldTransform *= targetJointWorldTransform.Inverse();
	
	Maths::Store( targetWorldTransform.GetTranslation(), &constraint.localTargetOffset.x );
	
	DBGLOG( "Target '%s' using joint '%s' with offset: (%.3f, %.3f, %.3f)\n",
		targetSceneNode->GetName().c_str(),
		m_scene->GetNodeName( constraint.targetNodeIndex ),
		constraint.localTargetOffset.x,
		constraint.localTargetOffset.y,
		constraint.localTargetOffset.z
	);
	
	constraint.upVectorReferenceNodeIndex = -1;
	//constraint.upVectorReferenceOffset;
	
	if ( upVectorReferenceNode )
	{
		constraint.upVectorReferenceNodeIndex = m_scene->GetNodeIndexByNearestAftkSceneNode( upVectorReferenceNode );
		
		// Calculate target's position in world space.
		Animation::C_JointTransform referenceWorldTransform;
		ConvertToJointTransform( referenceWorldTransform, upVectorReferenceNode->GetWorldMatrix(), m_sceneToGameTransform );
		
		Animation::C_JointTransform referenceJointWorldTransform;
		m_scene->GetCurrentWorldNodeTransform( constraint.upVectorReferenceNodeIndex, referenceJointWorldTransform );
		
		//referenceWorldTransform.RightMultiplyByInverse( referenceJointWorldTransform );
		referenceWorldTransform *= referenceJointWorldTransform.Inverse();
				
		Maths::Store( referenceWorldTransform.GetTranslation(), &constraint.upVectorReferenceOffset.x );
	}
	
	constraint.upVectorType = Animation::E_UpVectorType_ModelSpace;

	switch ( aftkUpVectorType )
	{
		case Aftk::E_UpVectorType_None:
			LOGERROR( gLog, "Up vectors of type 'None' are not supported for aim constraints.\n" );
			return false;
			break;
		case Aftk::E_UpVectorType_SceneUp:
			// Get up vector from scene???
			constraint.upVectorType = Animation::E_UpVectorType_ModelSpace;
			break;
			
		case Aftk::E_UpVectorType_Vector:
			constraint.upVectorType = Animation::E_UpVectorType_ModelSpace;
			break;
		
		case Aftk::E_UpVectorType_ObjectUp:
			constraint.upVectorType = Animation::E_UpVectorType_AimAtReferenceJoint;
			break;
		
		case Aftk::E_UpVectorType_ObjectRotationUp:
			constraint.upVectorType = Animation::E_UpVectorType_ReferenceJointSpace;
			break;
	}
	
	constraint.axisFlags = aimConstraintHelper.GetConstrainedAxes();
	constraint.nodeIndex = m_scene->GetNodeIndexByAftkSceneNode( &sceneNode );
	
	DBGLOG( "MinRot: (%g, %g, %g) MaxRot: (%g, %g, %g)\n",
		aimConstraintHelper.GetMinLimits().x,
		aimConstraintHelper.GetMinLimits().y,
		aimConstraintHelper.GetMinLimits().z,
		aimConstraintHelper.GetMaxLimits().x,
		aimConstraintHelper.GetMaxLimits().y,
		aimConstraintHelper.GetMaxLimits().z
	);
	
	
	MathsStorage::Float3 minRotationLimits, maxRotationLimits;
	ConvertToGameEulerAngles( aimConstraintHelper.GetMinLimits(), aimConstraintHelper.GetRotationOrder(), minRotationLimits );
	ConvertToGameEulerAngles( aimConstraintHelper.GetMaxLimits(), aimConstraintHelper.GetRotationOrder(), maxRotationLimits );
	
	//FixBounds( minRotationLimits, maxRotationLimits );
		
	// Store constraint limits as sines of half the limit angle. This makes
	// it possible to apply the limits directly to a local space quaternion.
	for ( U32 i = 0; i < 3; ++i )
	{
		const F32 minAngle = minRotationLimits.v[ i ];
		const F32 maxAngle = maxRotationLimits.v[ i ];
		
		constraint.minLimits.v[ i ] = sinf( minAngle / 2.f );
		constraint.maxLimits.v[ i ] = sinf( maxAngle / 2.f );
	}
	
	FixBounds( constraint.minLimits, constraint.maxLimits );
	
	constraintIndex.nodeIndex = constraint.nodeIndex;
	constraintIndex.AddInput( constraint.targetNodeIndex );
	constraintIndex.AddInput( constraint.upVectorReferenceNodeIndex );
		
	m_evalSequence.PushBack( constraintIndex );

	m_aimConstraints.PushBack( constraint );
	
	return true;
}

bool C_ExportConstraints::ImportOrientationConstraint(
	const C_OrientationConstraintHelper& orientationConstraintHelper
)
{
	BIZ_ASSERT( orientationConstraintHelper.IsValid() );

	if (m_orientationConstraints.Size() > kMaxConstraintsOfSameType)
	{
		LOGWARN(gLog, "Reached maximum number of orientation constraints (%d)", kMaxConstraintsOfSameType);
		return false;
	}
	
	const Aftk::C_SceneNode& sceneNode = orientationConstraintHelper.GetSceneNode();
		
	const U32 numInputs = orientationConstraintHelper.GetNumInputs();
	
	if ( 0 == numInputs )
	{
		return false;
	}
	
	C_ExportOrientationConstraint constraint;
	C_ExportConstraintIndex constraintIndex;
	constraintIndex.type = Animation::E_ConstraintType_Orientation;
	constraintIndex.index = m_orientationConstraints.Size();
	
	constraint.inputNodeIndex1 = m_scene->GetNodeIndexByAftkSceneNode( orientationConstraintHelper.GetInputNode( 0 ) );
	constraint.inputNodeIndex2 = constraint.inputNodeIndex1;
	
	if ( numInputs > 1 )
	{
		constraint.inputNodeIndex2 = m_scene->GetNodeIndexByNearestAftkSceneNode( orientationConstraintHelper.GetInputNode( 1 ) );
	}

	if ( constraint.inputNodeIndex1 < 0 || constraint.inputNodeIndex2 < 0 )
	{
		LOGERROR( gLog, "Error - one or more input nodes for orientation constraint '%s' are not recognised skeleton joints.\n", sceneNode.GetName().c_str() );
		return false;
	}
	
	F32 inputWeight1 = orientationConstraintHelper.GetInputWeight( 0 );
	F32 inputWeight2 = inputWeight1;
	
	if ( numInputs > 1 )
	{
		inputWeight2 = orientationConstraintHelper.GetInputWeight( 1 );
	}
	
	DBGLOG( "=====\n" );
	
	if ( numInputs > 1 )
	{
		DBGLOG( "[%s] Orientation constraint found with input node 1: %s, weight 1: %f, input node 2: %s, weight 2: %f\n",
			sceneNode.GetName().c_str(),
			orientationConstraintHelper.GetInputNode( 0 )->GetName().c_str(),
			inputWeight1,
			orientationConstraintHelper.GetInputNode( 1 )->GetName().c_str(),
			inputWeight2
		);
	}
	else
	{
		DBGLOG( "[%s] Orientation constraint found with single input node: %s\n",
			sceneNode.GetName().c_str(),
			orientationConstraintHelper.GetInputNode( 0 )->GetName().c_str()
		);
	}
	
	DBGLOG( "MinRot: (%g, %g, %g) MaxRot: (%g, %g, %g)\n",
		orientationConstraintHelper.GetMinLimits().x,
		orientationConstraintHelper.GetMinLimits().y,
		orientationConstraintHelper.GetMinLimits().z,
		orientationConstraintHelper.GetMaxLimits().x,
		orientationConstraintHelper.GetMaxLimits().y,
		orientationConstraintHelper.GetMaxLimits().z
	);	
	DBGLOG( "LocalRotationOffset: (%g, %g, %g)\n",
		orientationConstraintHelper.GetLocalRotationOffset().x,
		orientationConstraintHelper.GetLocalRotationOffset().y,
		orientationConstraintHelper.GetLocalRotationOffset().z
	);
	
	
	constraint.blendFraction = 0.f;
	
	MathsStorage::Float3 minRotationLimits, maxRotationLimits;
	ConvertToGameEulerAngles( orientationConstraintHelper.GetMinLimits(), orientationConstraintHelper.GetRotationOrder(), minRotationLimits );
	ConvertToGameEulerAngles( orientationConstraintHelper.GetMaxLimits(), orientationConstraintHelper.GetRotationOrder(), maxRotationLimits );
	
	//FixBounds( minRotationLimits, maxRotationLimits );
	
	// Store constraint limits as sines of half the limit angle. This makes
	// it possible to apply the limits directly to a local space quaternion.
	for ( U32 i = 0; i < 3; ++i )
	{
		const F32 minAngle = minRotationLimits.v[ i ];
		const F32 maxAngle = maxRotationLimits.v[ i ];
		
		constraint.minLimits.v[ i ] = sinf( minAngle / 2.f );
		constraint.maxLimits.v[ i ] = sinf( maxAngle / 2.f );
	}
	
	FixBounds( constraint.minLimits, constraint.maxLimits );
	
	// Calculate a rotation matrix from the local orientation offset and convert to a quaternion.
	ConvertEulerRotationToQuaternion(
		orientationConstraintHelper.GetLocalRotationOffset(),
		orientationConstraintHelper.GetRotationOrder(),
		constraint.localRotationOffset
	);
	
	if ( numInputs > 1 )
	{
		F32 invDenom = Maths::Reciprocal( static_cast< F32 >( inputWeight1 + inputWeight2 ) );
		constraint.blendFraction = static_cast< F32 >( inputWeight2 * invDenom );
	}
	
	constraint.nodeIndex = m_scene->GetNodeIndexByAftkSceneNode( &orientationConstraintHelper.GetSceneNode() );
	constraintIndex.nodeIndex = constraint.nodeIndex;
	constraintIndex.AddInput( constraint.inputNodeIndex1 );
	constraintIndex.AddInput( constraint.inputNodeIndex2 );
		
	m_evalSequence.PushBack( constraintIndex );
	m_orientationConstraints.PushBack( constraint );	
	
	return true;
}

bool C_ExportConstraints::ImportPositionConstraint(
	const C_PositionConstraintHelper& positionConstraintHelper
)
{
	if (m_positionConstraints.Size() > kMaxConstraintsOfSameType)
	{
		LOGWARN(gLog, "Reached maximum number of position constraints (%d)", kMaxConstraintsOfSameType);
		return false;
	}


	const Aftk::C_SceneNode& sceneNode = positionConstraintHelper.GetSceneNode();
	const U32 numInputs = positionConstraintHelper.GetNumInputs();
	const Aftk::C_SceneNode* inputNode1 = positionConstraintHelper.GetInputNode( 0 );
	const Aftk::C_SceneNode* inputNode2 = inputNode1;
	
	if ( numInputs > 1 )
	{
		inputNode2 = positionConstraintHelper.GetInputNode( 1 );
	}
	
	C_ExportPositionConstraint constraint;
	C_ExportConstraintIndex constraintIndex;
	constraintIndex.type = Animation::E_ConstraintType_Position;
	constraintIndex.index = m_positionConstraints.Size();
	
	constraint.inputNodeIndex1 = m_scene->GetNodeIndexByNearestAftkSceneNode( inputNode1 );
	constraint.inputNodeIndex2 = m_scene->GetNodeIndexByNearestAftkSceneNode( inputNode2 );
	
	if ( constraint.inputNodeIndex1 < 0 || constraint.inputNodeIndex2 < 0 )
	{
		LOGERROR( gLog, "Cannot find corresponding skeleton joints for one or more input nodes to position constraint '%s'\n", sceneNode.GetName().c_str() );
		return false;
	}

	Animation::C_JointTransform input1Offset = GetOffsetTransform(inputNode1);
	Animation::C_JointTransform input2Offset = GetOffsetTransform(inputNode2);
	
	// Store the difference offsets.
	Maths::Store( input1Offset.GetTranslation(), &constraint.inputNodeOffset1.x );
	Maths::Store( input2Offset.GetTranslation(), &constraint.inputNodeOffset2.x );
	
	F32 inputWeight1 = positionConstraintHelper.GetInputWeight( 0 );
	F32 inputWeight2 = inputWeight1;
	
	if ( numInputs > 1 )
	{
		inputWeight2 = positionConstraintHelper.GetInputWeight( 1 );
	}
	
	DBGLOG( "=====\n" );
	DBGLOG( "[%s] Position constraint found with input node 1: %s, weight 1: %f, input node 2: %s, weight 2: %f\n",
		positionConstraintHelper.GetSceneNode().GetName().c_str(),
		inputNode1->GetName().c_str(),
		inputWeight1,
		inputNode2->GetName().c_str(),
		inputWeight2
	);
	DBGLOG( "Input joint 1: '%s', offset (%f, %f, %f), joint 2: '%s', offset (%f, %f, %f)\n",
		m_scene->GetNodeName( constraint.inputNodeIndex1 ),
		constraint.inputNodeOffset1.x,
		constraint.inputNodeOffset1.y,
		constraint.inputNodeOffset1.z,
		m_scene->GetNodeName( constraint.inputNodeIndex2 ),
		constraint.inputNodeOffset2.x,
		constraint.inputNodeOffset2.y,
		constraint.inputNodeOffset2.z
	);
	DBGLOG( "MinTrans: (%g, %g, %g) MaxTrans: (%g, %g, %g)\n",
		positionConstraintHelper.GetMinLimits().x,
		positionConstraintHelper.GetMinLimits().y,
		positionConstraintHelper.GetMinLimits().z,
		positionConstraintHelper.GetMaxLimits().x,
		positionConstraintHelper.GetMaxLimits().y,
		positionConstraintHelper.GetMaxLimits().z
	);	
	DBGLOG( "LocalOffset: (%g, %g, %g)\n",
		positionConstraintHelper.GetLocalOffset().x,
		positionConstraintHelper.GetLocalOffset().y,
		positionConstraintHelper.GetLocalOffset().z
	);	
	constraint.blendFraction = 0.f;
	
	if ( numInputs > 1 )
	{
		F32 invDenom = Maths::Reciprocal( static_cast< F32 >( inputWeight1 + inputWeight2 ) );
		constraint.blendFraction = static_cast< F32 >( inputWeight2 * invDenom );
	}
	
	constraint.nodeIndex = m_scene->GetNodeIndexByAftkSceneNode( &sceneNode );

	MathsStorage::Float3 minLimits = m_sceneToGameTransform.PointToGameSpace(positionConstraintHelper.GetMinLimits());
	MathsStorage::Float3 maxLimits = m_sceneToGameTransform.PointToGameSpace(positionConstraintHelper.GetMaxLimits());

	// Express limits in parent joint's space, in case immediate parent has been removed from skeleton.
	Animation::C_JointTransform parentOffsetTransform = GetOffsetTransform(sceneNode.GetParentPtr());

	MathsStorage::Float3 parentOffset;
	Maths::Store(parentOffsetTransform.GetTranslation(), &parentOffset.x);

	minLimits.x += parentOffset.x;
	minLimits.y += parentOffset.y;
	minLimits.z += parentOffset.z;
	maxLimits.x += parentOffset.x;
	maxLimits.y += parentOffset.y;
	maxLimits.z += parentOffset.z;

	FixBounds( minLimits, maxLimits);
	
	constraint.minLimits = minLimits;
	constraint.maxLimits = maxLimits;
	
	DBGLOG( "GameMinTrans: (%g, %g, %g) GameMaxTrans: (%g, %g, %g)\n",
		constraint.minLimits.x,
		constraint.minLimits.y,
		constraint.minLimits.z,
		constraint.maxLimits.x,
		constraint.maxLimits.y,
		constraint.maxLimits.z
	);		
	
	constraint.localOffset = m_sceneToGameTransform.ToGameSpace( positionConstraintHelper.GetLocalOffset() );
		
	constraintIndex.nodeIndex = constraint.nodeIndex;
	constraintIndex.AddInput( constraint.inputNodeIndex1 );
	constraintIndex.AddInput( constraint.inputNodeIndex2 );
	
	m_evalSequence.PushBack( constraintIndex );
	m_positionConstraints.PushBack( constraint );
	
	
	return true;
}


namespace
{
	bool SaveAnimationCurveAsCsv( const Aftk::C_AnimationCurveNode& animCurveNode, const Util::C_String& targetDir )
	{
	
		Util::C_String fileName = targetDir;
		fileName += "/";
		fileName += animCurveNode.GetName().c_str();
		fileName += ".csv";
		
		Sys::I_File* file = Sys::GetDefaultFileSystem()->Open( fileName, Sys::E_FileAccess_Write );
		
		if ( !file )
		{
			return false;
		}
		
		Sys::TextSerialiser out( *file );
		
		// time, value
		F32 startX = animCurveNode.GetStartX();
		F32 endX = animCurveNode.GetEndX();
		
		// sample into 256 sections.
		
		static const U32 kSampleCount = 16;
		static const F32 kSampleDenom = kSampleCount - 1.f;
		
		// 16 keys, 1 byte each.
		// F32 minY, maxY
		// F32 minX, maxX
		// 16 bytes.
		// + 16 bytes for each key = 32 bytes.
		// 96 bytes for 3 curves. less than a cache line...
		// problem is getting clamped curves to work at the right point: 
				
		for ( U32 i = 0; i < kSampleCount; ++i )
		{
			F32 value = 0.f;
			F32 x = startX + ( endX - startX ) * i / kSampleDenom;
			animCurveNode.Evaluate( value, x );
			
			out << x << ", " << value << "\n";
		}
				
		delete file;
		
		return true;
	}
}

bool C_ExportConstraints::ImportDrivenKeyConstraint( const C_DrivenKeyHelper& drivenKeyHelper, E_TransformChannel transformChannel )
{
	BIZ_ASSERT( drivenKeyHelper.IsValid() );
	if ( !drivenKeyHelper.IsValid() )
	{
		return false;
	}

	if (transformChannel == E_TransformChannel_Rotation && m_drivenKeyRotationConstraints.Size() > kMaxConstraintsOfSameType)
	{
		LOGWARN(gLog, "Reached maximum number of driven rotation key constraints (%d)", kMaxConstraintsOfSameType);
		return false;
	}

	if (transformChannel == E_TransformChannel_Translation && m_drivenKeyTranslationConstraints.Size() > kMaxConstraintsOfSameType)
	{
		LOGWARN(gLog, "Reached maximum number of driven translation key constraints (%d)", kMaxConstraintsOfSameType);
		return false;
	}

	const Aftk::C_SceneNode& sceneNode = drivenKeyHelper.GetSceneNode();
	
	bool flipOutputFlags[3];
	bool reflection = m_sceneToGameTransform.HasReflection();
	switch (transformChannel)
	{
		case E_TransformChannel_Rotation:
			flipOutputFlags[0] = m_sceneToGameTransform.FlipXRotation();
			flipOutputFlags[1] = m_sceneToGameTransform.FlipYRotation();
			flipOutputFlags[2] = m_sceneToGameTransform.FlipZRotation();
			break;
		
		case E_TransformChannel_Translation:
			flipOutputFlags[0] = m_sceneToGameTransform.FlipX();
			flipOutputFlags[1] = m_sceneToGameTransform.FlipY();
			flipOutputFlags[2] = m_sceneToGameTransform.FlipZ();
			break;
		
		default:
			BIZ_ASSERT(false);
			break;
	}
	
	C_ExportDrivenKey drivenKeys[ 3 ];

	for ( U32 i = 0; i < 3; ++i )
	{
		const E_Axis axis = static_cast<E_Axis>(i) ;
		const bool flipOutput = flipOutputFlags[i];
		
		F32 outputScale = m_sceneToGameTransform.GetUniformScale();
		
		if (flipOutput)
		{
			outputScale = -outputScale;
		}

		C_ExportDrivenKey& drivenKey = drivenKeys[i];
		
		drivenKey.inputNodeIndex = -1;
		
		if ( !drivenKeyHelper.AxisIsConstrained( axis ) )
		{
			continue;
		}
		
		const Aftk::C_AnimationCurveNode* animCurveNode = drivenKeyHelper.GetAnimationCurveNode( axis );
		
		if ( !animCurveNode )
		{
			BIZ_ASSERT( false );
			continue;
		}
					
		//SaveAnimationCurveAsCsv( *animCurveNode, "c:/temp/driven_key_curves" );
		
		E_AnimationInputType inputType = drivenKeyHelper.GetInputType( axis );
		
		// Convert input into scene's coordinate system.
		
		bool flipInput = false;
		
		switch ( inputType )
		{
			case E_AnimationInputType_ScaleX:
			case E_AnimationInputType_ScaleY:
			case E_AnimationInputType_ScaleZ:
				LOGERROR( gLog, "Scale inputs not supported for driven keys ('%s')", sceneNode.GetName().c_str() );
				return false;
			
			case E_AnimationInputType_RotationX:
				drivenKey.inputType = Animation::E_InputType_RotationX;
				flipInput = m_sceneToGameTransform.FlipXRotation();
				break;
			case E_AnimationInputType_RotationY:
				drivenKey.inputType = Animation::E_InputType_RotationY;
				flipInput = m_sceneToGameTransform.FlipYRotation();
				break;
			case E_AnimationInputType_RotationZ:
				drivenKey.inputType = Animation::E_InputType_RotationZ;
				flipInput = m_sceneToGameTransform.FlipZRotation();
				break;
				
			case E_AnimationInputType_TranslationX:
				drivenKey.inputType = Animation::E_InputType_TranslationX;
				flipInput = m_sceneToGameTransform.FlipX();
				break;
			case E_AnimationInputType_TranslationY:
				drivenKey.inputType = Animation::E_InputType_TranslationY;
				flipInput = m_sceneToGameTransform.FlipY();
				break;
			case E_AnimationInputType_TranslationZ:
				drivenKey.inputType = Animation::E_InputType_TranslationZ;
				flipInput = m_sceneToGameTransform.FlipZ();
				break;
				
			default:
				BIZ_ASSERT( false );
				return false;
		}
		
		drivenKey.inputNodeIndex = m_scene->GetNodeIndexByNearestAftkSceneNode( drivenKeyHelper.GetInputNode( axis ) );
		
		drivenKey.inputScaleFactor = drivenKeyHelper.GetInputScaleFactor( axis );
		
		if (flipInput)
		{
			drivenKey.inputScaleFactor *= -1.f;
		}
		
		// Now import curve.
		
		F32 minX = animCurveNode->GetStartX();
		F32 maxX = animCurveNode->GetEndX();
		
		if ( inputType >= E_AnimationInputType_RotationX && inputType <= E_AnimationInputType_RotationZ )
		{
			minX = Maths::Clamped( minX, -180.f, +180.f );
			maxX = Maths::Clamped( maxX, -180.f, +180.f );
		}
			  
		
		F32 minY = FLT_MAX;
		F32 maxY = -FLT_MAX;
		
		// First pass to determine range of Y values.
		const U32 numSamples = drivenKey.curve.GetNumKeys();
		const F32 sampleInterval = ( maxX - minX ) / ( numSamples - 1.f );
		
		for ( U32 sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex )
		{
			const F32 x = minX + sampleInterval * sampleIndex;
			F32 value = 0.f;
			animCurveNode->Evaluate( value, x );
			value *= outputScale;
			minY = Maths::Min( minY, value );
			maxY = Maths::Max( maxY, value );
		}
		
		drivenKey.curve.SetRanges( minX, maxX, minY, maxY );
		
		// Second pass to store values into curve.
		for ( U32 sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex )
		{
			const F32 x = minX + sampleInterval * sampleIndex;
			F32 value = 0.f;
			animCurveNode->Evaluate( value, x );
			value *= outputScale;
			drivenKey.curve.SetKey( sampleIndex, value );
		}
	}
	
	C_ExportDrivenKeyConstraint constraint(transformChannel);
	C_ExportConstraintIndex constraintIndex;

	constraint.nodeIndex = m_scene->GetNodeIndexByAftkSceneNode( &drivenKeyHelper.GetSceneNode() );
	constraintIndex.nodeIndex = constraint.nodeIndex;
	
	for ( U32 i = 0; i < 3; ++i )
	{
		U32 drivenKeyIndex = 0;
		if ( drivenKeys[ i ].inputNodeIndex >= 0 )
		{
			constraintIndex.AddInput( drivenKeys[ i ].inputNodeIndex );
			m_drivenKeys.PushBack( drivenKeys[ i ] );
			drivenKeyIndex = m_drivenKeys.Size();
		}
		constraint.drivenKeyIndices[ i ] = drivenKeyIndex;
	}
	
	if ( transformChannel == E_TransformChannel_Rotation )
	{
		constraintIndex.type = Animation::E_ConstraintType_DrivenKeyRotation;
		constraintIndex.index = m_drivenKeyRotationConstraints.Size();
		m_evalSequence.PushBack( constraintIndex );
		m_drivenKeyRotationConstraints.PushBack( constraint );
	}
	else
	{
		BIZ_ASSERT( transformChannel == E_TransformChannel_Translation );
		constraintIndex.type = Animation::E_ConstraintType_DrivenKeyTranslation;
		constraintIndex.index = m_drivenKeyTranslationConstraints.Size();
		m_evalSequence.PushBack( constraintIndex );
		m_drivenKeyTranslationConstraints.PushBack( constraint );
	}
	
	return true;
}

bool GetDynamicPlugValue(const Aftk::C_SceneNode& sceneNode, const char* plugName, F32* outValue)
{
	const Aftk::C_DynamicPlug* thePlug = NULL;
	for (U32 i = 0, count = sceneNode.GetNumDynamicPlugs(); i < count; ++i)
	{
		const Aftk::C_DynamicPlug* plug = sceneNode.GetDynamicPlugPtr(i);
		if (plug == NULL) continue;
		
		if (0 == _stricmp(plug->GetName().c_str(), plugName))
		{
			thePlug = plug;
			break;
		}
	}
	
	if (thePlug != NULL && outValue != NULL)
	{
		switch (thePlug->GetDataType())
		{
			case Aftk::E_GenericAttributeType_F32:
			{
				F32 plugValue = 0.f;
				thePlug->GetValue(plugValue);
				*outValue = plugValue;
				break;
			}
			
			case Aftk::E_GenericAttributeType_F64:
			{
				F64 plugValue = 0.0;
				thePlug->GetValue(plugValue);
				*outValue = static_cast<F32>(plugValue);
				break;
			}
			
			default:
				return false;
		}
	}
	
	return thePlug != NULL;
}

bool C_ExportConstraints::ImportDynamicAimJoint( const Aftk::C_SceneNode& sceneNode, const C_AimConstraintHelper& aimConstraintHelper )
{
	if (m_dynamicAimJoints.Size() > kMaxConstraintsOfSameType)
	{
		LOGWARN(gLog, "Reached maximum number of dynamic aim joints (%d)", kMaxConstraintsOfSameType);
		return false;
	}


	C_ExportDynamicAimJoint constraint;

	constraint.nodeIndex = m_scene->GetNodeIndexByAftkSceneNode( &sceneNode );
	
	if ( constraint.nodeIndex < 0 )
	{
		return false;
	}
	
	constraint.targetNodeIndex = -1;
	if ( aimConstraintHelper.IsValid() )
	{
		// Use aim constraint to set target node.
		constraint.targetNodeIndex = m_scene->GetNodeIndexByNearestAftkSceneNode(aimConstraintHelper.GetTargetSceneNode());
		GetNodeOffset(&constraint.targetNodeOffset, aimConstraintHelper.GetTargetSceneNode(), constraint.targetNodeIndex);
		
		constraint.upVector = m_sceneToGameTransform.NormalToGameSpace(aimConstraintHelper.GetLocalUpVector());
		
		const Aftk::C_SceneNode* upVectorSceneNode = aimConstraintHelper.GetUpVectorReferenceSceneNode();
		
		if (upVectorSceneNode != NULL)
		{
			constraint.upVectorNodeIndex = m_scene->GetNodeIndexByNearestAftkSceneNode(upVectorSceneNode);
			GetNodeOffset(&constraint.upVectorNodeOffset, upVectorSceneNode, constraint.upVectorNodeIndex);
		}
		else
		{
			constraint.upVectorNodeIndex = m_scene->GetNodeParentIndex(constraint.nodeIndex);
			Animation::C_JointTransform localNodeTransform;
			m_scene->GetLocalNodeBindPoseTransform(constraint.nodeIndex, localNodeTransform);
			
			Maths::Point3 offset = localNodeTransform.TransformPoint(Maths::Load((const Maths::StoragePoint3*)&constraint.upVector));

			Maths::Store(offset, (Maths::StoragePoint3*)&constraint.upVectorNodeOffset);
		}
		
		Aftk::E_UpVectorType upVectorType = aimConstraintHelper.GetUpVectorType();
		
		if (upVectorType != Aftk::E_UpVectorType_ObjectUp)
		{
			LOGERROR(gLog, "Up vector type '%s' not supported for dynamic aim joints.", aimConstraintHelper.GetUpVectorTypeName());
			return false;
		}
		
		DBGLOG( "Dynamic aim joint '%s' aiming at joint '%s' with up vector from joint '%s'\n", sceneNode.GetName().c_str(), aimConstraintHelper.GetTargetSceneNode()->GetName().c_str(), upVectorSceneNode != NULL ? upVectorSceneNode->GetName().c_str() : "none" );
	}
	else
	{
		// Not a valid aim constraint, so use default parameters.
		constraint.targetNodeOffset = MathsStorage::Float3(0.f, 0.f, 0.f);
		constraint.upVector = m_sceneToGameTransform.NormalToGameSpace(MathsStorage::Float3(0.f, -1.f, 0.f));
		
		constraint.upVectorNodeIndex = m_scene->GetNodeParentIndex(constraint.nodeIndex);
		Animation::C_JointTransform localNodeTransform;
		m_scene->GetLocalNodeBindPoseTransform(constraint.nodeIndex, localNodeTransform);
			
		Maths::Point3 offset = localNodeTransform.TransformPoint(Maths::Load((const Maths::StoragePoint3*)&constraint.upVector));

		Maths::Store(offset, (Maths::StoragePoint3*)&constraint.upVectorNodeOffset);
	}
	
	// Default limits.
	const F32 kDefaultAimLimit = 30.f;
	constraint.minHeading = -kDefaultAimLimit;
	constraint.maxHeading = +kDefaultAimLimit;
	constraint.minPitch = -kDefaultAimLimit;
	constraint.maxPitch = +kDefaultAimLimit;
	
	// Default spring constants.
	constraint.pitchSpring = 100.f;
	constraint.pitchDamping = 0.5f;
	constraint.headingSpring = 100.f;
	constraint.headingDamping = 0.5f;
	
	// Default mass.
	constraint.mass = 1.f;
	
	// Default drag (velocity damping/air resistance).
	constraint.drag = 0.f;
	
	// Default wind and buoyancy influence factors (100% default).
	constraint.windInfluence = 1.f;
	constraint.buoyancyInfluence = 1.f;
	
	// Extract parameters from scene node.
	GetDynamicPlugValue(sceneNode, "MinPitch", &constraint.minPitch);
	GetDynamicPlugValue(sceneNode, "MaxPitch", &constraint.maxPitch);
	GetDynamicPlugValue(sceneNode, "MinHeading", &constraint.minHeading);
	GetDynamicPlugValue(sceneNode, "MaxHeading", &constraint.maxHeading);
	GetDynamicPlugValue(sceneNode, "PitchSpringStrength", &constraint.pitchSpring);
	GetDynamicPlugValue(sceneNode, "PitchSpringDampening", &constraint.pitchDamping);
	GetDynamicPlugValue(sceneNode, "HeadingSpringStrength", &constraint.headingSpring);
	GetDynamicPlugValue(sceneNode, "HeadingSpringDampening", &constraint.headingDamping);
	GetDynamicPlugValue(sceneNode, "Mass", &constraint.mass);
	GetDynamicPlugValue(sceneNode, "AirDampening", &constraint.drag);
	GetDynamicPlugValue(sceneNode, "WindInfluence", &constraint.windInfluence);
	GetDynamicPlugValue(sceneNode, "BuoyancyInfluence", &constraint.buoyancyInfluence);
	
	//
	// Drag is 0.0 for no drag at all, to 1.0 which will zero velocity of joint each frame.
	//
	//constraint.drag = Maths::Clamped(1.f - constraint.drag, 0.f, 1.f);
	
	constraint.windInfluence = Maths::Max(constraint.windInfluence, 0.f);
	constraint.buoyancyInfluence = Maths::Clamped(constraint.buoyancyInfluence, 0.f, 1.f);
	
	//
	// Convert limits into degrees and flip according to coordinate system.
	//
	constraint.minHeading = Maths::DegToRad(constraint.minHeading);
	constraint.maxHeading = Maths::DegToRad(constraint.maxHeading);
	constraint.minPitch = Maths::DegToRad(constraint.minPitch);
	constraint.maxPitch = Maths::DegToRad(constraint.maxPitch);
	
	const C_SceneConfig& sceneConfig = m_scene->GetSceneConfig();
	bool flipHandedness = sceneConfig.flipX ^ sceneConfig.flipY ^ sceneConfig.flipZ;
	
	if (flipHandedness ^ sceneConfig.flipY) //flipHeading)
	{
		constraint.minHeading = -constraint.minHeading;
		constraint.maxHeading = -constraint.maxHeading;
	}
	
	if (flipHandedness ^ sceneConfig.flipX)
	{
		constraint.minPitch = -constraint.minPitch;
		constraint.maxPitch = -constraint.maxPitch;
	}
	
	// Aim axis is always assumed to be +Z.
	constraint.aimAxis = m_sceneToGameTransform.NormalToGameSpace(MathsStorage::Float3(0.f, 0.f, 1.f));
	
	C_ExportConstraintIndex constraintIndex;
	
	constraintIndex.type = Animation::E_ConstraintType_DynamicAimJoint;
	constraintIndex.index = m_dynamicAimJoints.Size();
	
	// NB. no inputs until dynamic aim joints are parsed from AftkSceneAimConstraintNodes.
	constraintIndex.nodeIndex = constraint.nodeIndex;
	constraintIndex.AddInput( constraint.targetNodeIndex );
	constraintIndex.AddInput( constraint.upVectorNodeIndex );
	
	m_evalSequence.PushBack( constraintIndex );
	m_dynamicAimJoints.PushBack( constraint );
	
	return true;
}

bool C_ExportConstraints::ImportDynamicSliderJoint( const Aftk::C_SceneNode& sceneNode )
{
	if (m_dynamicSliderJoints.Size() > kMaxConstraintsOfSameType)
	{
		LOGWARN(gLog, "Reached maximum number of dynamic slider joints (%d)", kMaxConstraintsOfSameType);
		return false;
	}

	C_ExportDynamicSliderJoint constraint;

	constraint.nodeIndex = m_scene->GetNodeIndexByAftkSceneNode( &sceneNode );
	
	if ( constraint.nodeIndex < 0 )
	{
		return false;
	}
	
	// Default parameters.
	constraint.minLimit = -1.f / m_sceneToGameTransform.GetUniformScale();
	constraint.maxLimit = +1.f / m_sceneToGameTransform.GetUniformScale();
	
	constraint.mass = 1.f;
	constraint.drag = 0.f;
	constraint.spring = 20.f;
	constraint.damping = 0.5f;

	// Default wind and buoyancy influence factors (100% default).
	constraint.windInfluence = 1.f;
	constraint.buoyancyInfluence = 1.f;
			
	// Obtain parameters from node's custom attributes.
	GetDynamicPlugValue(sceneNode, "MaxSlide", &constraint.maxLimit);
	GetDynamicPlugValue(sceneNode, "MinSlide", &constraint.minLimit);
	GetDynamicPlugValue(sceneNode, "SpringConstant", &constraint.spring);
	GetDynamicPlugValue(sceneNode, "SpringDampening", &constraint.damping);
	GetDynamicPlugValue(sceneNode, "Mass", &constraint.mass);
	GetDynamicPlugValue(sceneNode, "AirDampening", &constraint.drag);
	GetDynamicPlugValue(sceneNode, "WindInfluence", &constraint.windInfluence);
	GetDynamicPlugValue(sceneNode, "BuoyancyInfluence", &constraint.buoyancyInfluence);
	
	//
	// Drag is 0.0 for no drag at all, to 1.0 which will zero velocity of joint each frame.
	//
	//constraint.drag = Maths::Clamped(1.f - constraint.drag, 0.f, 1.f);
	constraint.windInfluence = Maths::Clamped(constraint.windInfluence, 0.f, 1.f);
	constraint.buoyancyInfluence = Maths::Clamped(constraint.buoyancyInfluence, 0.f, 1.f);
	
	// Transform limits and constraint axis into parent space.
	Maths::Mat43 localBindPose;
	m_scene->GetLocalNodeBindPoseTransform(constraint.nodeIndex, localBindPose);
	
	Maths::Normal3 localAxis = Maths::Const<Maths::Normal3>::AxisZ();
	
	Maths::Point3 parentMinLimit = m_sceneToGameTransform.PointToGameSpace(Maths::MakePoint3(Maths::MakeVF32(constraint.minLimit) * localAxis)) * localBindPose;
	Maths::Point3 parentMaxLimit = m_sceneToGameTransform.PointToGameSpace(Maths::MakePoint3(Maths::MakeVF32(constraint.maxLimit) * localAxis)) * localBindPose;
	
	Maths::Normal3 parentAxis = Maths::Normalise(m_sceneToGameTransform.NormalToGameSpace(localAxis) * localBindPose);
	
	constraint.axis = Maths::StorageVec3F32(Maths::MakeVec4F32(parentAxis)).m_Data;
	//constraint.minLimit = Maths::GetF32((parentMinLimit - Maths::RowW(localBindPose)) | parentAxis);
	//constraint.maxLimit = Maths::GetF32((parentMaxLimit - Maths::RowW(localBindPose)) | parentAxis);
	constraint.minLimit = Maths::GetF32((parentMinLimit) | parentAxis);
	constraint.maxLimit = Maths::GetF32((parentMaxLimit) | parentAxis);
	
	C_ExportConstraintIndex constraintIndex;
	
	constraintIndex.type = Animation::E_ConstraintType_DynamicSliderJoint;
	constraintIndex.index = m_dynamicSliderJoints.Size();
	
	constraintIndex.nodeIndex = constraint.nodeIndex;
	
	m_evalSequence.PushBack( constraintIndex );
	m_dynamicSliderJoints.PushBack( constraint );

	return true;
}

void C_ExportConstraints::ConvertEulerRotationToQuaternion( const MathsStorage::Float3& eulerRotation, Aftk::E_RotationOrder rotationOrder, MathsStorage::Float4& outQuaternion ) const
{
	Maths::HMat33 m;
	Aftk::EulerToMatrix(
		m,
		Maths::Load<Maths::HVec3, MathsStorage::Float3>(&eulerRotation),
		rotationOrder
	);
	m = m_sceneToGameTransform.ToGameSpace( m );
	Maths::C_Quaternion q = !Maths::C_Quaternion( m ); // negate for new maths lib.
	Maths::Store<Maths::C_Quaternion, MathsStorage::Float4>(q, &outQuaternion);
}

void C_ExportConstraints::ConvertToGameEulerAngles( const MathsStorage::Float3& eulerXyz, Aftk::E_RotationOrder rotationOrder, MathsStorage::Float3& outGameEulerXyz )
{
	Maths::HVec3 sceneEuler = Maths::Load<Maths::HVec3, MathsStorage::Float3>(&eulerXyz);
	Maths::HVec3 gameEuler = sceneEuler * m_sceneToGameTransform.GetRotationFlipVector();
	Maths::Store<Maths::HVec3, MathsStorage::Float3>(gameEuler, &outGameEulerXyz);
}

Animation::C_JointTransform C_ExportConstraints::GetOffsetTransform(const Aftk::C_SceneNode* aftkSceneNode, S32* outAnimSceneNodeIndex) const
{
	S32 animSceneNodeIndex = m_scene->GetNodeIndexByNearestAftkSceneNode(aftkSceneNode);

	if (outAnimSceneNodeIndex)
	{
		*outAnimSceneNodeIndex = animSceneNodeIndex;
	}

	if (animSceneNodeIndex < 0)
	{
		return Animation::C_JointTransform(Animation::C_JointTransform::IDENTITY);
	}

	Animation::C_JointTransform aftkSceneNodeTransform;
	ConvertToJointTransform(aftkSceneNodeTransform, aftkSceneNode->GetWorldMatrix(), m_sceneToGameTransform);

	Animation::C_JointTransform animSceneNodeTransform;
	m_scene->GetCurrentWorldNodeTransform(animSceneNodeIndex, animSceneNodeTransform);

	Animation::C_JointTransform offsetTransform = (aftkSceneNodeTransform * animSceneNodeTransform.Inverse());

	return offsetTransform;
}

void C_ExportConstraints::GetNodeOffset(MathsStorage::Float3* outOffset, const Aftk::C_SceneNode* sceneNode, S32 nodeIndex)
{
	if (sceneNode == NULL)
	{
		outOffset->x = 0.f;
		outOffset->y = 0.f;
		outOffset->z = 0.f;
		return;
	}

	Animation::C_JointTransform sceneNodeTransform;
	ConvertToJointTransform( sceneNodeTransform, sceneNode->GetWorldMatrix(), m_sceneToGameTransform );

	Animation::C_JointTransform animNodeTransform;
	
	if (nodeIndex >= 0)
	{
		m_scene->GetCurrentWorldNodeTransform( nodeIndex, animNodeTransform);
	}
	else
	{
		animNodeTransform.SetIdentity();
	}
	
	Maths::Vector3 offset = (sceneNodeTransform * animNodeTransform.Inverse()).GetTranslation();
	Maths::Store( offset, &outOffset->x );
}

} // namespace Animation

