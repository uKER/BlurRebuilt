#include "stdafx.h"
#include "ConstraintHelpers.h"
#include <btl/utility/types.h>

namespace AnimationTools
{

namespace
{
	void GetXyzPlugRoots( const Aftk::I_Plug& plug, const Aftk::I_Object* outRoots[ 3 ] )
	{
		for ( U32 axisIndex = 0; axisIndex < 3; ++axisIndex )
		{
			const Aftk::I_Object* sourceConnection = 0;
			const Aftk::I_Plug* childPlug = plug.GetChildPtr( axisIndex );
			
			if ( childPlug )
			{
			
				sourceConnection = childPlug->GetSourceConnectionPtr();
		
				while ( sourceConnection && sourceConnection->HasType( Aftk::E_ObjType_IPlug ) )
				{
					const Aftk::I_Plug* plug = static_cast< const Aftk::I_Plug* >( sourceConnection );
					sourceConnection = plug->GetConnectionRoot();
				}
			}
			outRoots[ axisIndex ] = sourceConnection;
		}
	}
	
	void GetRotationInputs( const Aftk::C_SceneNode& sceneNode, const Aftk::I_Object* outRotationInputs[ 3 ] )
	{
		GetXyzPlugRoots( sceneNode.GetRotatePlug(), outRotationInputs );
	}

	void GetTranslationInputs( const Aftk::C_SceneNode& sceneNode, const Aftk::I_Object* outTranslationInputs[ 3 ] )
	{
		GetXyzPlugRoots( sceneNode.GetTranslatePlug(), outTranslationInputs );
	}
	
	S32 FindDynamicPlug( const Aftk::C_SceneNode* sceneNode, const char* name )
	{
		if ( !sceneNode )
		{
			return -1;
		}
		
		U32 numPlugs = sceneNode->GetNumDynamicPlugs();
		
		for ( U32 i = 0; i < numPlugs; ++i )
		{
			const Aftk::C_DynamicPlug* plug = sceneNode->GetDynamicPlugPtr( i );
			
			if ( plug )
			{
				if ( plug->GetName() == name )
				{
					return i;
				}
			}
		}
		return -1;
	}

	
	MathsStorage::Float3 GetVector3PlugValue( const Aftk::C_NullPlug& plug )
	{
		Aftk::Vector3F32 value;
		plug.GetCompoundValue(value);
		return MathsStorage::Float3(value[0], value[1], value[2]);
	}

	
	bool GetPlugValue( const Aftk::C_SceneNode& sceneNode, const char* plugName, F32& outValue )
	{
		S32 plugIndex = FindDynamicPlug( &sceneNode, plugName );
		
		if ( plugIndex >= 0 )
		{
			F64 value64;
			if ( sceneNode.GetDynamicPlugPtr( plugIndex )->GetValue( value64 ) )
			{
				outValue = static_cast<F32>( value64 );
				return true;
			}
			
			F32 value32;
			if ( sceneNode.GetDynamicPlugPtr( plugIndex )->GetValue( value32 ) )
			{
				outValue = value32;
				return true;
			}
		}
		return false;
	}
	
	Debug::Logger& gLog = Debug::GetLogger( "mercury.animation" );
	
}

inline void GetLimitIfSet( Aftk::E_LimitType limitType, const Aftk::C_LimitMixin& sourceLimits, F32& outLimit )
{
	if ( sourceLimits.IsLimited( limitType ) )
	{
		outLimit = sourceLimits.GetLimit( limitType );
	}
}

//
// C_AimConstraintHelper
//

C_AimConstraintHelper::C_AimConstraintHelper( const Aftk::C_SceneNode& sceneNode ):
	m_sceneNode( sceneNode )
{
	for ( U32 i = 0; i < 3; ++i )
	{
		m_minLimits.v[ i ] = -Maths::Pi;
		m_maxLimits.v[ i ] = +Maths::Pi;
	}

	bool isValid = Import( sceneNode );
	
	if ( isValid )
	{
		assert( m_aimConstraintNode );
		
		// Get local space rotation limits, Euler angles, radians.
		if ( sceneNode.GetType() == Aftk::E_ObjType_SceneJoint )
		{
			const Aftk::C_SceneJointNode& c = static_cast<const Aftk::C_SceneJointNode&>(sceneNode);
			GetLimitIfSet( Aftk::E_LimitType_RotateMinX, c, m_minLimits.x );
			GetLimitIfSet( Aftk::E_LimitType_RotateMaxX, c, m_maxLimits.x );
			GetLimitIfSet( Aftk::E_LimitType_RotateMinY, c, m_minLimits.y );
			GetLimitIfSet( Aftk::E_LimitType_RotateMaxY, c, m_maxLimits.y );
			GetLimitIfSet( Aftk::E_LimitType_RotateMinZ, c, m_minLimits.z );
			GetLimitIfSet( Aftk::E_LimitType_RotateMaxZ, c, m_maxLimits.z );
		}		
	}
	else
	{
		m_aimConstraintNode = 0;
	}
}

bool C_AimConstraintHelper::IsValid() const
{
	return m_aimConstraintNode != 0;
}

const Aftk::C_SceneNode& C_AimConstraintHelper::GetSceneNode() const
{
	return m_sceneNode;
}

U32 C_AimConstraintHelper::GetConstrainedAxes() const
{
	return m_constrainedAxes;
}

const Aftk::C_SceneNode* C_AimConstraintHelper::GetTargetSceneNode() const
{
	BIZ_ASSERT( IsValid() );
	if ( !IsValid() )
	{
		return 0;
	}
	
	const Aftk::C_NullPlug& targetTranslate = m_aimConstraintNode->GetTargetTranslatePlug();
	const Aftk::I_Object* sourceConnection = targetTranslate.GetSourceConnectionPtr();

	const Aftk::C_SceneNode* targetSceneNode = 0;

	if ( sourceConnection && sourceConnection->HasType( Aftk::E_ObjType_IPlug ) )
	{
		const Aftk::I_Plug* plug = static_cast< const Aftk::I_Plug* >( sourceConnection );
		const Aftk::I_Object* targetObject = plug->GetParentPtr();
		
		if ( targetObject && targetObject->HasType( Aftk::E_ObjType_SceneNode ) )
		{
			targetSceneNode = static_cast< const Aftk::C_SceneNode* >( targetObject );
		}
	}
	
	return targetSceneNode;
}

/// Get the up vector type of the aim constraint.
Aftk::E_UpVectorType C_AimConstraintHelper::GetUpVectorType() const
{
	U32 upVectorType = Aftk::E_UpVectorType_Vector;
	if ( m_aimConstraintNode )
	{
		m_aimConstraintNode->GetWorldUpVectorTypePlug().GetValue( upVectorType );
	}
	return static_cast< Aftk::E_UpVectorType >( upVectorType );
}

/// Get up vector type name.
const char* C_AimConstraintHelper::GetUpVectorTypeName() const
{
	static const char* kUpVectorTypeNames[ Aftk::E_UpVectorType_Count ] =
	{
		"None",
		"SceneUp",
		"ObjectUp",
		"ObjectRotationUp",
		"Vector"
	};
	return kUpVectorTypeNames[ GetUpVectorType() ];
}


/// Get an aim constraint's up vector reference scene node. This only applies to aim constraints
/// with "Object Up" or "Object Rotation Up" up vector types.
const Aftk::C_SceneNode* C_AimConstraintHelper::GetUpVectorReferenceSceneNode() const
{
	if ( !m_aimConstraintNode )
	{
		return 0;
	}
	
	const Aftk::I_Object* source = m_aimConstraintNode->GetWorldUpMatrixPlug().GetSourceConnectionPtr();
	
	const Aftk::C_SceneNode* upVectorReference = 0;

	if ( source && source->HasType( Aftk::E_ObjType_IPlug ))
	{
		const Aftk::I_Plug* sourcePlug = static_cast< const Aftk::I_Plug* >( source );

		const Aftk::C_Node* node = sourcePlug->GetTopLevelParentPtr();
		
		if ( node && node->HasType( Aftk::E_ObjType_SceneNode ) )
		{
			upVectorReference = static_cast< const Aftk::C_SceneNode* >( node );
		}
	}
	
	return upVectorReference;
}

MathsStorage::Float3 C_AimConstraintHelper::GetRotationOffset() const
{
	Aftk::Vector3F32 offset( 0.0f, 0.0f, 0.0f );
	if ( m_aimConstraintNode )
	{
		m_aimConstraintNode->GetRotationalOffsetPlug().GetCompoundValue( offset );
	}
	return MathsStorage::Float3( offset.x, offset.y, offset.z );
}

MathsStorage::Float3 C_AimConstraintHelper::GetAimVector() const
{
	Aftk::Vector3F32 v( 1.0f, 0.0f, 0.0f );
	if ( m_aimConstraintNode )
	{
		m_aimConstraintNode->GetAimVectorPlug().GetCompoundValue( v );
	}
	return MathsStorage::Float3( v.x, v.y, v.z );
}

MathsStorage::Float3 C_AimConstraintHelper::GetLocalUpVector() const
{
	Aftk::Vector3F32 v( 0.0f, 1.0f, 0.0f );
	if ( m_aimConstraintNode )
	{
		m_aimConstraintNode->GetUpVectorPlug().GetCompoundValue( v );
	}
	return MathsStorage::Float3( v.x, v.y, v.z );
}

MathsStorage::Float3 C_AimConstraintHelper::GetReferenceUpVector() const
{
	Aftk::Vector3F32 v( 0.0f, 1.0f, 0.0f );
	if ( m_aimConstraintNode )
	{
		m_aimConstraintNode->GetWorldUpVectorPlug().GetCompoundValue( v );
	}
	return MathsStorage::Float3( v.x, v.y, v.z );
}

Aftk::E_RotationOrder C_AimConstraintHelper::GetRotationOrder() const
{
	Aftk::E_RotationOrder order = Aftk::E_RotationOrder_XYZ;
	if ( m_aimConstraintNode != NULL )
	{
		order = m_aimConstraintNode->GetRotationOrder();		
	}
	return order;
}


bool C_AimConstraintHelper::Import( const Aftk::C_SceneNode& sceneNode )
{
	const Aftk::I_Object* rotationInputs[ 3 ];
	GetRotationInputs( sceneNode, rotationInputs );
	
	U32 numConstraints = 0;
	m_constrainedAxes = 0;
	
	const Aftk::C_SceneAimConstraintNode* theAimConstraint = 0;
	for ( U32 axisIndex = 0; axisIndex < 3; ++axisIndex )
	{
		const Aftk::C_SceneAimConstraintNode* aimConstraint = 0;
		if ( rotationInputs[ axisIndex ] )
		{
			if ( rotationInputs[ axisIndex ]->GetType() == Aftk::E_ObjType_SceneAimConstraint )
			{
				aimConstraint = static_cast< const Aftk::C_SceneAimConstraintNode* >( rotationInputs[ axisIndex ] );
				
				m_constrainedAxes |= 1 << axisIndex;
				
				if ( theAimConstraint && aimConstraint != theAimConstraint )
				{
					LOGERROR( gLog, "Too many aim constraints driving joint '%s'!", sceneNode.GetName().c_str() );
					return false;
					
				}
				else
				{
					theAimConstraint = aimConstraint;
				}
			}
		}
		
		if ( aimConstraint )
		{
			++numConstraints;
		}
	}
	
	m_aimConstraintNode = theAimConstraint;
	
	if ( 0 == numConstraints )
	{
		return false;
	}
	
	BIZ_ASSERT( m_aimConstraintNode );
	
/*
	if ( numConstraints != 3 )
	{
		// Constraints with less than 3 inputs are currently not supported.
		LOGERROR( gLog, "Aim constraint '%s' must drive all 3 axes of joint '%s'", m_aimConstraintNode->GetName().c_str(), sceneNode.GetName().c_str() );
		return false;
	}
*/	
	return true;
} 



//
// C_WeightedInputMixin
//

C_WeightedInputMixin::C_WeightedInputMixin():
	m_numInputs(0)
{
}

U32 C_WeightedInputMixin::GetNumInputs() const
{
	return m_numInputs;
}

void C_WeightedInputMixin::AddInput( const Aftk::C_SceneNode* inputNode, F32 weight )
{
	BIZ_ASSERT(m_numInputs < kMaxInputs );
	m_inputNodes[m_numInputs] = inputNode;
	m_inputWeights[m_numInputs] = weight;
	++m_numInputs;
}

const Aftk::C_SceneNode* C_WeightedInputMixin::GetInputNode( U32 index ) const
{
	BIZ_ASSERT(index < m_numInputs);
	return m_inputNodes[ index ];
}

F32 C_WeightedInputMixin::GetInputWeight( U32 index ) const
{
	BIZ_ASSERT(index < m_numInputs);
	return m_inputWeights[ index ];
}

//
// C_OrientationConstraintHelper
//
C_OrientationConstraintHelper::C_OrientationConstraintHelper( const Aftk::C_SceneNode& sceneNode ):
	m_sceneNode(sceneNode),
	m_orientConstraintNode(GetOrientConstraintNode(sceneNode))
{
	for ( U32 i = 0; i < 3; ++i )
	{
		m_minLimits.v[ i ] = -Maths::Pi;
		m_maxLimits.v[ i ] = +Maths::Pi;
		m_localRotationOffset.v[ i ] = 0.f;
	}
	
	if ( m_orientConstraintNode != NULL )
	{
		const Aftk::C_SceneOrientConstraintNode* ori = m_orientConstraintNode;
	
		// Get inputs.
		for ( U32 i = 0; i < ori->GetNumInputs(); ++i )
		{
			F32 weight = 0.f;
			ori->GetTargetWeightPlug( i ).GetValue( weight );
			
			const Aftk::C_NullPlug& targetRotate = ori->GetTargetRotatePlug(i);
			const Aftk::I_Object* source = NULL; 
			
			if ( targetRotate.GetSourceConnectionPtr() != NULL )
			{
				source = targetRotate.GetConnectionRoot();
			
				LOGINFO( gLog, "Target rotate plug: %s, source: %s weight: %f\n", ori->GetTargetRotatePlug(i).GetName().c_str(), source ? source->GetName().c_str() : "null", weight );
				
				if ( source->HasType( Aftk::E_ObjType_SceneNode ) )
				{
					AddInput( static_cast<const Aftk::C_SceneNode*>(source), weight );
				}
			}
		}
		
		m_localRotationOffset = GetVector3PlugValue( ori->GetRotateOffsetPlug() );
	}
		
	if ( m_orientConstraintNode != NULL && sceneNode.GetType() == Aftk::E_ObjType_SceneJoint )
	{
		// Get local space rotation limits from target joint (Euler angles, radians).
		const Aftk::C_SceneJointNode& c = static_cast<const Aftk::C_SceneJointNode&>(sceneNode);
		GetLimitIfSet( Aftk::E_LimitType_RotateMinX, c, m_minLimits.x );
		GetLimitIfSet( Aftk::E_LimitType_RotateMaxX, c, m_maxLimits.x );
		GetLimitIfSet( Aftk::E_LimitType_RotateMinY, c, m_minLimits.y );
		GetLimitIfSet( Aftk::E_LimitType_RotateMaxY, c, m_maxLimits.y );
		GetLimitIfSet( Aftk::E_LimitType_RotateMinZ, c, m_minLimits.z );
		GetLimitIfSet( Aftk::E_LimitType_RotateMaxZ, c, m_maxLimits.z );
		
		// Negate and flip X.
		//m_minLimits.x = -m_minLimits.x;
		//m_maxLimits.x = -m_maxLimits.x;
		//Btl::Swap( m_minLimits.x, m_maxLimits.x );
	}
}

const MathsStorage::Float3& C_OrientationConstraintHelper::GetMinLimits() const
{
	return m_minLimits;
}

const MathsStorage::Float3& C_OrientationConstraintHelper::GetMaxLimits() const
{
	return m_maxLimits;
}

const MathsStorage::Float3& C_OrientationConstraintHelper::GetLocalRotationOffset() const
{
	return m_localRotationOffset;
}

bool C_OrientationConstraintHelper::IsValid() const
{
	return m_orientConstraintNode != NULL && GetNumInputs() > 0;
}

const Aftk::C_SceneNode& C_OrientationConstraintHelper::GetSceneNode() const
{
	return m_sceneNode;
}

Aftk::E_RotationOrder C_OrientationConstraintHelper::GetRotationOrder() const
{
	Aftk::E_RotationOrder order = Aftk::E_RotationOrder_XYZ;
	if ( m_orientConstraintNode != NULL )
	{
		order = m_orientConstraintNode->GetRotationOrder();		
	}
	return order;
}

const Aftk::C_SceneOrientConstraintNode* C_OrientationConstraintHelper::GetOrientConstraintNode(
	const Aftk::C_SceneNode& sceneNode
)
{
	const Aftk::I_Object* rotateSource[3];
	GetXyzPlugRoots( sceneNode.GetRotatePlug(), rotateSource );
		
	if ( rotateSource[0] != NULL &&
		rotateSource[0] == rotateSource[1] &&
		rotateSource[1] == rotateSource[2] )
	{
		if ( rotateSource[ 0 ]->GetType() == Aftk::E_ObjType_SceneOrientConstraint )
		{
			LOGINFO( gLog, "Orientation of '%s' controlled by orientation constraint '%s'\n", sceneNode.GetName().c_str(), rotateSource[0]->GetName().c_str() );
			return static_cast<const Aftk::C_SceneOrientConstraintNode*>(rotateSource[0]);
		}
	}
	return NULL;
}

//
// C_PositionConstraintHelper
//



C_PositionConstraintHelper::C_PositionConstraintHelper( const Aftk::C_SceneNode& sceneNode ):	
	m_sceneNode(sceneNode),
	m_pointConstraintNode(GetPointConstraintNode(sceneNode))
{
	for ( U32 i = 0; i < 3; ++i )
	{
		m_minLimits.v[ i ] = -FLT_MAX;
		m_maxLimits.v[ i ] = +FLT_MAX;
	}
	
	if ( m_pointConstraintNode )
	{
		// Get inputs.
		const Aftk::C_ScenePointConstraintNode* p = m_pointConstraintNode;
	
		// Get inputs.
		for ( U32 i = 0; i < p->GetNumInputs(); ++i )
		{
			F32 weight = 0.f;
			p->GetTargetWeightPlug( i ).GetValue( weight );
			
			const Aftk::C_NullPlug& targetTranslate = p->GetTargetTranslatePlug(i);
			const Aftk::I_Object* source = targetTranslate.GetConnectionRoot();
			
			LOGINFO( gLog, "Target rotate plug: %s, source: %s weight: %f\n", targetTranslate.GetName().c_str(), source ? source->GetName().c_str() : "null", weight );
			
			if ( source->HasType( Aftk::E_ObjType_SceneNode ) && source != p )
			{
				AddInput( static_cast<const Aftk::C_SceneNode*>(source), weight );
			}
		}
		
		// Get local space offset.
		const Aftk::C_NullPlug& translateOffset = p->GetTranslateOffsetPlug();
		m_localOffset = GetVector3PlugValue( translateOffset );
	}
	
	if ( m_pointConstraintNode != NULL && sceneNode.GetType() == Aftk::E_ObjType_SceneJoint )
	{
		// Get local space translation limits from target joint.
		const Aftk::C_SceneJointNode& c = static_cast<const Aftk::C_SceneJointNode&>(sceneNode);
		GetLimitIfSet( Aftk::E_LimitType_TranslateMinX, c, m_minLimits.x );
		GetLimitIfSet( Aftk::E_LimitType_TranslateMaxX, c, m_maxLimits.x );
		GetLimitIfSet( Aftk::E_LimitType_TranslateMinY, c, m_minLimits.y );
		GetLimitIfSet( Aftk::E_LimitType_TranslateMaxY, c, m_maxLimits.y );
		GetLimitIfSet( Aftk::E_LimitType_TranslateMinZ, c, m_minLimits.z );
		GetLimitIfSet( Aftk::E_LimitType_TranslateMaxZ, c, m_maxLimits.z );
	}
}

const MathsStorage::Float3& C_PositionConstraintHelper::GetMinLimits() const
{
	return m_minLimits;
}

const MathsStorage::Float3& C_PositionConstraintHelper::GetMaxLimits() const
{
	return m_maxLimits;
}

bool C_PositionConstraintHelper::IsValid() const
{
	return m_pointConstraintNode != NULL && GetNumInputs() > 0;
}

const Aftk::C_SceneNode& C_PositionConstraintHelper::GetSceneNode() const
{
	return m_sceneNode;
}

const Aftk::C_ScenePointConstraintNode* C_PositionConstraintHelper::GetPointConstraintNode(
	const Aftk::C_SceneNode& sceneNode )
{
	const Aftk::I_Object* translateSource[3];
	GetXyzPlugRoots( sceneNode.GetTranslatePlug(), translateSource );
		
	if ( translateSource[0] != NULL &&
		translateSource[0] == translateSource[1] &&
		translateSource[1] == translateSource[2] )
	{
		if ( translateSource[ 0 ]->GetType() == Aftk::E_ObjType_ScenePointConstraint )
		{
			LOGINFO( gLog, "Translation of '%s' controlled by point constraint '%s'\n", sceneNode.GetName().c_str(), translateSource[0]->GetName().c_str() );
			return static_cast<const Aftk::C_ScenePointConstraintNode*>(translateSource[0]);
		}
	}	
	
	return NULL;
}

const MathsStorage::Float3& C_PositionConstraintHelper::GetLocalOffset() const
{
	return m_localOffset;
}


//
// C_DrivenKeyHelper
//

C_DrivenKeyHelper::C_DrivenKeyHelper( const Aftk::C_SceneNode& sceneNode, const Aftk::I_Plug& plug ):
	m_sceneNode( sceneNode )
{
	m_isValid = Import( sceneNode, plug );
}

bool C_DrivenKeyHelper::IsValid() const
{
	return m_isValid;
}

const Aftk::C_SceneNode& C_DrivenKeyHelper::GetSceneNode() const
{
	return m_sceneNode;
}

bool C_DrivenKeyHelper::AxisIsConstrained( E_Axis axis ) const
{
	return m_inputNodes[ axis ] != 0;
}

const Aftk::C_SceneNode* C_DrivenKeyHelper::GetInputNode( E_Axis axis ) const
{
	return m_inputNodes[ axis ];
}

const Aftk::C_AnimationCurveNode* C_DrivenKeyHelper::GetAnimationCurveNode( E_Axis axis ) const
{
	return m_animationCurveNodes[ axis ];
}

E_AnimationInputType C_DrivenKeyHelper::GetInputType( E_Axis axis ) const
{
	return m_inputTypes[ axis ];
}

F32 C_DrivenKeyHelper::GetInputScaleFactor( E_Axis axis ) const
{
	return m_inputScaleFactors[ axis ];
}

bool C_DrivenKeyHelper::Import( const Aftk::C_SceneNode& sceneNode, const Aftk::I_Plug& plug )
{
	const Aftk::I_Object* inputNodes[ 3 ]; 
	GetXyzPlugRoots( plug, inputNodes );

	U32 numConstraints = 0;
	m_constrainedAxes = 0;
	
	for ( U32 axisIndex = 0; axisIndex < 3; ++axisIndex )
	{
		m_inputNodes[ axisIndex ] = 0;
		m_animationCurveNodes[ axisIndex ] = 0;
		m_inputScaleFactors[ axisIndex ] = 0.f;
	
		const Aftk::I_Object* inputNode = inputNodes[ axisIndex ];
		
		if ( !inputNode )
		{
			continue;
		}
	
		if ( inputNode->GetType() != Aftk::E_ObjType_AnimationCurve )
		{
			if ( numConstraints > 0 )
			{
				// Already found a driven key constraint - not allowed to mix with other types!
				LOGERROR( gLog, "Node '%s' uses more than one type of constraint", sceneNode.GetName().c_str() );
			}
			
			// Not controlled by driven keys.
			return false;
		}
						
		// Looks like a driven key.
		const Aftk::C_AnimationCurveNode* animCurveNode = static_cast< const Aftk::C_AnimationCurveNode* >( inputNode );
		
		if ( animCurveNode->IsTimeInput() )
		{
			// Driven by time, not another joint so ignore it.
			continue;
		}
		
		const Aftk::I_Object* inputSourceConnection = animCurveNode->GetInputPlug().GetSourceConnectionPtr();
		
		if ( !inputSourceConnection )
		{
			// Curve has no input.
			continue;
		}
		
		const Aftk::C_UnitConversionNode* unitConversionNode = NULL; 
		//const Aftk::I_Object* unitCurveSourceConnection = unitConversionNode->GetInputPlug().GetSourceConnectionPtr();
		
		if ( inputSourceConnection->GetType() == Aftk::E_ObjType_UnitConversion )
		{
			unitConversionNode = static_cast< const Aftk::C_UnitConversionNode* >( inputSourceConnection );
			inputSourceConnection = unitConversionNode->GetInputPlug().GetSourceConnectionPtr();
		}
		
		if ( !inputSourceConnection )
		{
			// No input into unit conversion node - ignore it.
			continue;
		}
		
		if ( !inputSourceConnection->HasType( Aftk::E_ObjType_IPlug ) )
		{
			// Not connected to a plug - ignore it.
			continue;
		}
		
		const Aftk::I_Plug* inputPlug = static_cast< const Aftk::I_Plug* >(inputSourceConnection);
		
		// 0 = X, 1 = Y, 2 = Z
		E_Axis inputAxis = E_Axis_X;
				
		if( inputPlug->GetName() == "x" )
		{
			inputAxis = E_Axis_X;
		}
		else if( inputPlug->GetName() == "y" )
		{
			inputAxis = E_Axis_Y;
		}
		else if( inputPlug->GetName() == "z" )
		{
			inputAxis = E_Axis_Z;
		}
		else
		{
			// Unknown input.
			LOGWARN( gLog, "Unrecognised driven key input '%s'", inputPlug->GetName().c_str() );
			continue;
		}
			
		bool inputScale = false;
		bool inputRotate = false;
		bool inputTranslate = false;
		
		if( !inputPlug->GetParentPtr() )
		{
			LOGWARN( gLog, "Input driven key plug '%s' has no parent", inputPlug->GetName().c_str() );
			continue;
		}
		
		const Aftk::I_Object* inputPlugParent = inputPlug->GetParentPtr();
			
		if( inputPlugParent->GetName() == "scale" )
		{
			inputScale = true;
		}
		else if( inputPlugParent->GetName() == "rotate" )
		{
			inputRotate = true;
		}
		else if( inputPlugParent->GetName() == "translate" )
		{
			inputTranslate = true;
		}
		else
		{
			// Unknown input.
			LOGWARN( gLog, "Unrecognised driven key input source '%s'", inputPlugParent->GetName().c_str() );
			continue;
		}
		
		// Find input node.
		const Aftk::C_Node* topLevelNode = inputPlug->GetTopLevelParentPtr();
		
		if ( !topLevelNode )
		{
			// No input node.
			continue;
		}
		
		if ( !topLevelNode->HasType( Aftk::E_ObjType_SceneNode ) )
		{
			// Input is not a scene node.
			LOGWARN( gLog, "Driven key input node '%s' is not a scene node", topLevelNode->GetName().c_str() );
			continue;
		}
		
		
		if ( inputScale )
		{
			static const E_AnimationInputType kScaleInputTypes[ E_Axis_Count ] =
			{
				E_AnimationInputType_ScaleX,
				E_AnimationInputType_ScaleY,
				E_AnimationInputType_ScaleZ
			};
			
			m_inputTypes[ axisIndex ] = kScaleInputTypes[ inputAxis ];
		}
		else if ( inputRotate )
		{
			static const E_AnimationInputType kRotationInputTypes[ E_Axis_Count ] =
			{
				E_AnimationInputType_RotationX,
				E_AnimationInputType_RotationY,
				E_AnimationInputType_RotationZ
			};
			
			m_inputTypes[ axisIndex ] = kRotationInputTypes[ inputAxis ];			
		}
		else if ( inputTranslate )
		{
			static const E_AnimationInputType kTranslationInputTypes[ E_Axis_Count ] =
			{
				E_AnimationInputType_TranslationX,
				E_AnimationInputType_TranslationY,
				E_AnimationInputType_TranslationZ
			};
			
			m_inputTypes[ axisIndex ] = kTranslationInputTypes[ inputAxis ];
		}
		
		static const char* kInputTypeNames[ E_AnimationInputType_Count ] =
		{
			"ScaleX",
			"ScaleY",
			"ScaleZ",
			"RotationX",
			"RotationY",
			"RotationZ",
			"TranslationX",
			"TranslationY",
			"TranslationZ"
		};
		
		static const char* kAxisNames[ E_Axis_Count ] = { "X", "Y", "Z" };
		
		LOGINFO( gLog, "    %s axis %s on node '%s' driven by input node '%s' %s using curve '%s'",
			plug.GetName().c_str(),
			kAxisNames[ axisIndex ],
			sceneNode.GetName().c_str(),
			topLevelNode->GetName().c_str(),
			kInputTypeNames[ m_inputTypes[ axisIndex ] ],
			animCurveNode->GetName().c_str()
		);
		
				
		m_inputNodes[ axisIndex ] = static_cast< const Aftk::C_SceneNode* >( topLevelNode );
		m_inputScaleFactors[ axisIndex ] = unitConversionNode != NULL ? unitConversionNode->GetFactor() : 1.f;
		m_animationCurveNodes[ axisIndex ] = animCurveNode;
		
		m_constrainedAxes |= 1 << axisIndex;
		++numConstraints;
	}
	
	return numConstraints > 0;
}

//
// C_DrivenRotationKeyHelper
//
C_DrivenRotationKeyHelper::C_DrivenRotationKeyHelper( const Aftk::C_SceneNode& sceneNode ):
	C_DrivenKeyHelper( sceneNode, sceneNode.GetRotatePlug() )
{
}

//
// C_DrivenTranslationKeyHelper
//
C_DrivenTranslationKeyHelper::C_DrivenTranslationKeyHelper( const Aftk::C_SceneNode& sceneNode ):
	C_DrivenKeyHelper( sceneNode, sceneNode.GetTranslatePlug() )
{
}

} // namespace Animation
