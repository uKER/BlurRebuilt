#include "stdafx.h"
#include "AnimationGraph.h"
#include "ResourceCache.h"
#include <animation/common/MemoryBlock.h>
#include <animation/deferred/CommandList.h>
#include <animation/deferred/CommandProcessor.h>
#include <animation/cycle/Cycle.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/pose/Pose.h>
#include <btl/containers/bitvector.h>

//
// Register animation tools types.
//
XT_TYPE(AnimationTools::C_SceneConfig, "SceneConfig" )
	XT_FIELD(scaleFactor, "ScaleFactor")
XT_END_TYPE

XT_TYPE(AnimationTools::C_CycleParameters, "CycleParameters" )
	XT_FIELD(maxGlobalRotationError, "MaxGlobalRotationError")
	XT_FIELD(maxGlobalTranslationError, "MaxGlobalTranslationError")
	XT_FIELD(maxRelativeCustomCurveError, "MaxRelativeCustomCurveError" )
	XT_FIELD(maxHighFidelityRotationError, "MaxHighFidelityRotationError")
	XT_FIELD(maxHighFidelityTranslationError, "MaxHighFidelityTranslationError")
	XT_FIELD(removeTranslationChannelsMatchingBasePose, "RemoveTranslationChannelsMatchingBasePose")
	XT_FIELD(removeChannelsWithNoKeys, "RemoveChannelsWithNoKeys")
XT_END_TYPE

//
// Register animation types.
//
XT_TYPE(Animation::E_LoopMode, "LoopMode")
	XT_OPTION(Animation::E_LoopMode_Once,	"Once")
	XT_OPTION(Animation::E_LoopMode_Continuous,	"Continuous")
XT_END_TYPE

//
// Register graph types.
//


XT_TYPE(AnimationGraph::C_ConstantVar, "ConstantVar")
	XT_BASE(AnimationGraph::C_InputVar)
	XT_FIELD(m_value, "Value")
XT_END_TYPE

XT_ABSTRACT_TYPE(AnimationGraph::C_InputVar, "InputVar")
	XT_FIELD(m_name, "Name")
XT_END_TYPE

XT_TYPE(AnimationGraph::C_SineWaveVar, "SineWaveVar")
	XT_BASE(AnimationGraph::C_InputVar)
	XT_FIELD(m_frequency,	"Frequency")
	XT_FIELD(m_scale,		"Scale")
	XT_FIELD(m_offset,		"Offset")
	XT_FIELD(m_phaseOffset,	"PhaseOffset")
	XT_FIELD(m_clamp,		"Clamp")
	XT_FIELD(m_clampRange,	"ClampRange")
XT_END_TYPE

XT_TYPE(AnimationGraph::C_TimelineVar, "TimelineVar")
	XT_BASE(AnimationGraph::C_InputVar)
XT_END_TYPE

/*
XT_TYPE(AnimationGraph::E_ControllerInputType, "ControllerInputType")
	XT_OPTION(AnimationGraph::E_ControllerInputType_ForwardBack, "ForwardBack")
	XT_OPTION(AnimationGraph::E_ControllerInputType_LeftRight, "LeftRight")
	XT_OPTION(AnimationGraph::E_ControllerInputType_Action1, "Action1")
	XT_OPTION(AnimationGraph::E_ControllerInputType_Action2, "Action2")
	XT_OPTION(AnimationGraph::E_ControllerInputType_Heading, "Heading")
XT_END_TYPE

XT_TYPE(AnimationGraph::C_ControllerInputVar, "ControllerInputVar")
	XT_BASE(AnimationGraph::C_InputVar)
	XT_FIELD(m_input, "Input")
XT_END_TYPE
*/

XT_TYPE(Animation::E_BlendMode, "BlendMode")
	//XT_OPTION(Animation::E_BlendMode_Replace, "Replace")
	XT_OPTION(Animation::E_BlendMode_Accumulate, "Accumulate")
	XT_OPTION(Animation::E_BlendMode_Lerp, "Lerp")
	XT_OPTION(Animation::E_BlendMode_Additive, "Additive")
XT_END_TYPE

XT_ABSTRACT_TYPE(AnimationGraph::C_Node, "Node")
XT_END_TYPE

XT_TYPE(AnimationGraph::C_ResultNode, "ResultNode")
	XT_BASE(AnimationGraph::C_Node)
	XT_FIELD(m_input,			"Input")
XT_END_TYPE

XT_TYPE(AnimationGraph::E_FieldMaskFlags, "FieldMaskFlags")
	XT_FLAG_STORAGE(XtU32)
	XT_FLAG(AnimationGraph::E_FieldMaskFlags_TransformRotation, "TransformRotation")
	XT_FLAG(AnimationGraph::E_FieldMaskFlags_TransformTranslation, "TransformTranslation")
	XT_FLAG(AnimationGraph::E_FieldMaskFlags_TransformScale, "TransformScale")
	XT_FLAG(AnimationGraph::E_FieldMaskFlags_TrajectoryRotation, "TrajectoryRotation")
	XT_FLAG(AnimationGraph::E_FieldMaskFlags_TrajectoryTranslation, "TrajectoryTranslation")
	XT_FLAG(AnimationGraph::E_FieldMaskFlags_TrajectoryDeltaRotation, "TrajectoryDeltaRotation")
	XT_FLAG(AnimationGraph::E_FieldMaskFlags_TrajectoryDeltaTranslation, "TrajectoryDeltaTranslation")
	XT_FLAG(AnimationGraph::E_FieldMaskFlags_CustomScalar, "CustomScalar")
	XT_FLAG(AnimationGraph::E_FieldMaskFlags_CustomVector3, "CustomVector3")
	XT_FLAG(AnimationGraph::E_FieldMaskFlags_CustomVector4, "CustomVector4")
XT_END_TYPE

XT_TYPE(AnimationGraph::C_CycleNode, "CycleNode")
	XT_BASE(AnimationGraph::C_Node)
	XT_FIELD(m_filename,			"Filename")
	XT_FIELD(m_subtractedFilename,	"SubtractedFilename")
	XT_FIELD(m_timeScale,			"TimeScale")
	XT_FIELD(m_phaseOffset,			"PhaseOffset")
	XT_FIELD(m_sceneConfig,			"SceneOptions")
	XT_FIELD(m_cycleParams,			"CycleOptions")
	XT_FIELD(m_version,				"Version")
	XT_FIELD(m_worldSpace,			"WorldSpace")
	XT_FIELD(m_ignoreTrajectory,	"IgnoreTrajectory")
	XT_FIELD(m_loopMode,			"LoopMode")
	XT_FIELD(m_useFieldMask,		"UseFieldMask")
	XT_FIELD(m_enabledFields,		"EnabledFields")
XT_END_TYPE


XT_TYPE(AnimationGraph::C_BlendInput, "BlendInput")
	XT_FIELD(m_input, "Input")
	XT_FIELD(m_controlValue, "ControlValue")
XT_END_TYPE

XT_TYPE(AnimationGraph::C_BlendNode, "BlendNode")
	XT_BASE(AnimationGraph::C_Node)
	XT_FIELD(m_blendMode, "BlendMode")
	XT_FIELD(m_inputs, "Inputs")
	XT_FIELD(m_controlInput, "ControlInput")
	XT_FIELD(m_wrap, "Wrap")
XT_END_TYPE

XT_TYPE(AnimationGraph::C_BlendLayer, "BlendLayer")
	XT_FIELD(blendMode, "BlendMode")
	XT_FIELD(input,	"Input")
XT_END_TYPE

XT_TYPE(AnimationGraph::C_LayeredBlendNode, "LayeredBlendNode")
	XT_BASE(AnimationGraph::C_Node)
	XT_FIELD(m_baseLayer, "BaseLayer")
	XT_FIELD(m_layers, "Layers")
XT_END_TYPE

XT_TYPE(Animation::E_MirrorAxis, "MirrorAxis")
	XT_OPTION(Animation::E_MirrorAxis_X, "X")
	XT_OPTION(Animation::E_MirrorAxis_Y, "Y")
	XT_OPTION(Animation::E_MirrorAxis_Z, "Z")
XT_END_TYPE

XT_TYPE(AnimationGraph::C_MirrorNode, "MirrorNode")
	XT_BASE(AnimationGraph::C_Node)
	XT_FIELD(m_input, "Input")
	XT_FIELD(m_mirrorAxis, "MirrorAxis")
	XT_FIELD(m_enabled, "Enabled")
XT_END_TYPE

XT_TYPE(AnimationGraph::C_TimeControllerNode, "TimeControllerNode")
	XT_BASE(AnimationGraph::C_Node)
	XT_FIELD(m_input, "Input")
	XT_FIELD(m_controlInput, "ControlInput")
	XT_FIELD(m_inputRange, "InputRange")
	XT_FIELD(m_timeRange, "TimeRange")
XT_END_TYPE

XT_TYPE(AnimationGraph::C_Graph, "Graph")
	XT_CALLBACK(&AnimationGraph::C_Graph::XtCallback)
	XT_FIELD(m_result,	"Result")
	XT_FIELD(m_nodes,	"Nodes")
	XT_FIELD(m_inputVars,	"InputVars")
XT_END_TYPE

XT_LIBRARY(AnimationGraph)
	
	XT_REGISTER_TYPE(AnimationGraph::C_InputVar)
	
	/*
	XT_REGISTER_TYPE(AnimationGraph::E_ControllerInputType)
	XT_REGISTER_TYPE(AnimationGraph::C_ControllerInputVar)
	*/
	XT_REGISTER_TYPE(AnimationGraph::E_FieldMaskFlags)

	XT_REGISTER_TYPE(AnimationGraph::C_ConstantVar)
	XT_REGISTER_TYPE(AnimationGraph::C_SineWaveVar)
	XT_REGISTER_TYPE(AnimationGraph::C_TimelineVar)
	XT_REGISTER_TYPE(Animation::E_BlendMode)
	XT_REGISTER_TYPE(Animation::E_MirrorAxis)
	XT_REGISTER_TYPE(Animation::E_LoopMode)
	XT_REGISTER_TYPE(AnimationGraph::C_Node)
	XT_REGISTER_TYPE(AnimationGraph::C_ResultNode)
	XT_REGISTER_TYPE(AnimationGraph::C_CycleNode)
	XT_REGISTER_TYPE(AnimationGraph::C_BlendInput)
	XT_REGISTER_TYPE(AnimationGraph::C_BlendNode)
	XT_REGISTER_TYPE(AnimationGraph::C_BlendLayer);
	XT_REGISTER_TYPE(AnimationGraph::C_LayeredBlendNode);
	XT_REGISTER_TYPE(AnimationGraph::C_MirrorNode);
	XT_REGISTER_TYPE(AnimationGraph::C_TimeControllerNode);
	XT_REGISTER_TYPE(AnimationGraph::C_Graph)
	
	XT_REGISTER_TYPE(AnimationTools::C_SceneConfig)
	XT_REGISTER_TYPE(AnimationTools::C_CycleParameters)
XT_END_LIBRARY



namespace AnimationGraph
{

Debug::Logger& gLog = Debug::GetLogger("motionworks.animationgraph");


//
// C_Context
//

class C_Context
{
public:
	Animation::C_CommandList*		commandList;
	const Animation::C_Skeleton*	skeleton;
	C_ControllerState*				controllerState;
	U32								updateCounter;
	F32								deltaTime;
	F32								globalTime;
	F32								timelineValue;
	bool							inWorldSpace;		// Set to true if evaluation was directly in world space.
};

//
// C_SineWaveVar
//

F32 C_SineWaveVar::GetValue( const C_Context& context )
{
	if ( context.updateCounter != m_updateCounter )
	{
		m_updateCounter = context.updateCounter;
		//m_phase = fmodf( context.globalTime * m_frequency, 1.f );
		m_phase = fmodf( m_phase + context.deltaTime * m_frequency, 1.f );
		m_value = ( sinf( (m_phaseOffset + m_phase) * Maths::Pi2 ) * m_scale ) + m_offset;
		
		if ( m_clamp )
		{
			m_value = Maths::Clamped( m_value, m_clampRange.from, m_clampRange.to );
		}
		
	}
	return m_value;
};

void C_SineWaveVar::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	out << m_phase;
}

void C_SineWaveVar::LoadState(Animation::C_MemoryBlockReader& in)
{
	in >> m_phase;
}

//
// C_TimelineVar
//
F32 C_TimelineVar::GetValue( const C_Context& context )
{
	return context.timelineValue;
}


//
// C_ControllerInputVar
//

F32 C_ControllerInputVar::GetValue( const C_Context& context )
{
#if 0
	switch ( m_input )
	{
		case E_ControllerInputType_ForwardBack: return context.controllerState->forwardBack;
		case E_ControllerInputType_LeftRight: return context.controllerState->leftRight;
		case E_ControllerInputType_Action1: return context.controllerState->action1;
		case E_ControllerInputType_Action2: return context.controllerState->action2;
		default: break;
	}
	return C_InputVar::GetValue( context );
#endif
	return 0.f;
	
}



//
// C_Node
//

/*

	SetPhase - pushed down the tree, nodes can change this value as they pass it down.
	UpdateTime - pushed down, can be translated into SetPhase for nodes that control child node playback speeds.
	GetFrequency - pulled up from leaves, modified on the way.


*/

C_Node::C_Node( E_NodeType type ):
	m_phase(0.f),
	m_frequency(0.f),
	m_type(type)
{
}

C_Node::~C_Node()
{
}

void C_Node::SetPhase( const C_Context& context, F32 phase )
{
	m_phase = phase;
}

F32 C_Node::GetPhase() const
{
	return m_phase;
}

void C_Node::UpdateTime( const C_Context& context, F32 deltaTime )
{
	SetPhase( context, m_phase + GetFrequency( context ) * deltaTime );
}

F32 C_Node::GetFrequency( const C_Context& context )
{
	return m_frequency;
}

void C_Node::SetFrequency( const C_Context& context, F32 f )
{
	m_frequency = f;
}

void C_Node::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	out << m_phase;
}

void C_Node::LoadState(Animation::C_MemoryBlockReader& in)
{
	in >> m_phase;
}

//
// C_ResultNode
//

C_ResultNode::C_ResultNode(): C_Node(E_NodeType_Result)
{
}

void C_ResultNode::SetPhase( const C_Context& context, F32 phase )
{
	if ( m_input ) m_input->SetPhase( context, phase );
}

void C_ResultNode::UpdateTime( const C_Context& context, F32 deltaTime )
{
	if ( m_input ) m_input->UpdateTime( context, deltaTime );
}

void C_ResultNode::LoadResources(C_ResourceCache& rc)
{
	if ( m_input) m_input->LoadResources(rc);
}

void C_ResultNode::Evaluate( C_Context& context, Animation::E_BlendMode blendMode, F32 weight )
{
	if ( m_input )
	{
		m_input->Evaluate( context, blendMode, weight );
	}
}

F32 C_ResultNode::GetFrequency( const C_Context& context )
{
	return m_input ? m_input->GetFrequency( context ) : 0.f;
}

//
// C_CycleNode
//

C_CycleNode::C_CycleNode():
	C_Node(E_NodeType_Cycle),
	m_cycle(NULL),
	m_loopMode(Animation::E_LoopMode_Continuous),
	m_version(0)
{
}

void C_CycleNode::LoadResources(C_ResourceCache& rc)
{
	if ( m_filename && *m_filename != 0 )
	{
		// Update options and cycle parameters
		
		AnimationTools::C_SceneConfig sceneConfig = rc.GetSceneConfig();
		AnimationTools::C_CycleParameters cycleParams = rc.GetCycleParameters();
		
		if ( m_version >= 1 )
		{
			if (m_sceneConfig.scaleFactor <= 0.f)
			{
				m_sceneConfig.scaleFactor = 1.f;
			}

			sceneConfig.scaleFactor = m_sceneConfig.scaleFactor;
				
			cycleParams.maxGlobalRotationError = m_cycleParams.maxGlobalRotationError;
			cycleParams.maxGlobalTranslationError = m_cycleParams.maxGlobalTranslationError;
			cycleParams.maxHighFidelityRotationError = m_cycleParams.maxHighFidelityRotationError;
			cycleParams.maxHighFidelityTranslationError = m_cycleParams.maxHighFidelityTranslationError;
			cycleParams.removeChannelsWithNoKeys = m_cycleParams.removeChannelsWithNoKeys;
			cycleParams.removeTranslationChannelsMatchingBasePose = m_cycleParams.removeTranslationChannelsMatchingBasePose;
			cycleParams.inWorldSpace = m_worldSpace;
		}
		
		cycleParams.maxRelativeCustomCurveError = m_cycleParams.maxRelativeCustomCurveError;

		m_sceneConfig = sceneConfig;
		m_cycleParams = cycleParams;
	
		// Extract scene options and cycle parameters.
		if ( m_subtractedFilename && *m_subtractedFilename != 0 )
		{
			m_cycle = rc.GetAdditiveCycle( (const char*) m_filename, (const char*) m_subtractedFilename, m_sceneConfig, m_cycleParams );
		}
		else
		{
			m_cycle = rc.GetCycle( (const char*) m_filename, m_sceneConfig, m_cycleParams );
		}
		
		m_cycleInstance = Animation::C_CycleInstance( m_cycle );
		
		m_frequency = m_timeScale * Maths::Reciprocal(m_cycleInstance.GetDuration());
	}
}

void C_CycleNode::Evaluate( C_Context& context, Animation::E_BlendMode blendMode, F32 weight )
{
	if ( m_cycle == NULL )
	{
		return;
	}
	
	F32 t = m_cycleInstance.GetDuration() * (GetPhase() + m_phaseOffset);
	
	Animation::C_PoseFieldMask fieldMask;
	
	if ( m_useFieldMask)
	{
		fieldMask.Enable(Animation::E_PoseFieldElement_JointTransformRotation, (m_enabledFields & E_FieldMaskFlags_TransformRotation) != 0);
		fieldMask.Enable(Animation::E_PoseFieldElement_JointTransformTranslation, (m_enabledFields & E_FieldMaskFlags_TransformTranslation) != 0);
		fieldMask.Enable(Animation::E_PoseFieldElement_JointTransformScale, (m_enabledFields & E_FieldMaskFlags_TransformScale) != 0);
		
		fieldMask.Enable(Animation::E_PoseFieldElement_TrajectoryTransformRotation, (m_enabledFields & E_FieldMaskFlags_TrajectoryRotation) != 0);
		fieldMask.Enable(Animation::E_PoseFieldElement_TrajectoryTransformTranslation, (m_enabledFields & E_FieldMaskFlags_TrajectoryTranslation) != 0);
		fieldMask.Enable(Animation::E_PoseFieldElement_TrajectoryDeltaTransformRotation, (m_enabledFields & E_FieldMaskFlags_TrajectoryDeltaRotation) != 0);
		fieldMask.Enable(Animation::E_PoseFieldElement_TrajectoryDeltaTransformTranslation, (m_enabledFields & E_FieldMaskFlags_TrajectoryDeltaTranslation) != 0);
		
		fieldMask.Enable(Animation::E_PoseFieldElement_CustomScalar, (m_enabledFields & E_FieldMaskFlags_CustomScalar) != 0);
		fieldMask.Enable(Animation::E_PoseFieldElement_CustomVector3, (m_enabledFields & E_FieldMaskFlags_CustomVector3) != 0);
		fieldMask.Enable(Animation::E_PoseFieldElement_CustomVector4XYZ, (m_enabledFields & E_FieldMaskFlags_CustomVector4) != 0);
		fieldMask.Enable(Animation::E_PoseFieldElement_CustomVector4W, (m_enabledFields & E_FieldMaskFlags_CustomVector4) != 0);
	}	
	
	if ( m_ignoreTrajectory )
	{
		fieldMask.DisableTrajectory();
	}
	
	
	context.commandList->BlendCycle( blendMode, &m_cycleInstance, t, weight, m_loopMode, fieldMask );
	
	if ( m_worldSpace )
	{
		context.inWorldSpace = true;
	}
}

void C_CycleNode::UpdateTime( const C_Context& context, F32 deltaTime )
{
	C_Node::UpdateTime( context, deltaTime * m_timeScale );
}


void C_CycleNode::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	C_Node::SaveState(out);
	
	out << m_cycleInstance.GetPreviousTime();
	out << m_cycleInstance.GetPreviousTimeInFrames();
	out << m_cycleInstance.GetPreviousUpdateCounter();
	out << m_cycleInstance.GetPreviousTrajectoryTransform();
	
}

void C_CycleNode::LoadState(Animation::C_MemoryBlockReader& in)
{
	C_Node::LoadState(in);
	
	F32 prevTime, prevTimeInFrames;
	in >> prevTime >> prevTimeInFrames;
	m_cycleInstance.SetPreviousTime( prevTime, prevTimeInFrames );
	
	U32 updateCounter;
	in >> updateCounter;
	m_cycleInstance.SetPreviousUpdateCounter( updateCounter );
	
	Animation::C_JointTransform trajDelta;
	in >> trajDelta;
	
	m_cycleInstance.SetPreviousTrajectoryTransform( trajDelta );
}

//
// C_BlendNode
//

C_BlendNode::C_BlendNode(): C_Node(E_NodeType_Blend),
	m_activeChild0(-1),
	m_activeChild1(-1)
{
}

void C_BlendNode::LoadResources(C_ResourceCache& rc)
{
	for ( S32 i = 0; i < m_inputs.count; ++i )
	{
		if ( m_inputs[i].m_input != NULL )
		{
			m_inputs[i].m_input->LoadResources(rc);
		}
	}
}

void C_BlendNode::Evaluate( C_Context& context, Animation::E_BlendMode blendMode, F32 weight )
{
	if ( m_activeChild0 < 0 && m_activeChild1 < 0 )
	{
		return;
	}

	F32 childWeights[2] = { 0.f, 0.f };
	F32 totalChildWeight = 0.f;

	if ( m_activeChild0 >= 0 )
	{
		childWeights[0] = ( 1.f - m_alpha );
		totalChildWeight += childWeights[0];
	}
		
	if ( m_activeChild1 >= 0)
	{
		childWeights[1] = m_alpha;
		totalChildWeight += childWeights[1];
	}
	
	if ( m_blendMode != blendMode )
	{
		context.commandList->PushCurrentPose( weight );
	}

	if ( m_activeChild0 >= 0 )
	{
		m_inputs[m_activeChild0].m_input->Evaluate( context, m_blendMode, childWeights[0] );
	}
		
	if ( m_activeChild1 >= 0)
	{
		m_inputs[m_activeChild1].m_input->Evaluate( context, m_blendMode, childWeights[1] );
	}
	
	if ( m_blendMode != blendMode )
	{
		context.commandList->PopAndBlendPose( blendMode );
	}
}

void C_BlendNode::SetPhase( const C_Context& context, F32 phase )
{
	CalcBlend( context );
	C_Node::SetPhase( context, phase );
	
	if (m_activeChild0 >= 0) m_inputs[m_activeChild0].m_input->SetPhase(context, phase);
	if (m_activeChild1 >= 0) m_inputs[m_activeChild1].m_input->SetPhase(context, phase);
}

void C_BlendNode::UpdateTime( const C_Context& context, F32 deltaTime )
{
	C_Node::UpdateTime( context, deltaTime ); // calls SetPhase, calls GetFrequency.
	const F32 phase = GetPhase();
	if (m_activeChild0 >= 0) m_inputs[m_activeChild0].m_input->SetPhase(context, phase);
	if (m_activeChild1 >= 0) m_inputs[m_activeChild1].m_input->SetPhase(context, phase);
}

F32 C_BlendNode::GetFrequency( const C_Context& context )
{
	CalcBlend( context );
	
	F32 f = 0.f;
	F32 totalWeight = 0.f;
	
	if ( m_activeChild0 >= 0 )
	{
		F32 w = (1.f - m_alpha);
		f += w * m_inputs[m_activeChild0].m_input->GetFrequency( context );
		totalWeight += w;
	}
	if ( m_activeChild1 >= 0 )
	{
		F32 w = m_alpha;
		f += w * m_inputs[m_activeChild1].m_input->GetFrequency( context );
		totalWeight += w;
	}
	
	f *= Maths::Reciprocal( totalWeight );

	return f;
}

void C_BlendNode::CalcBlend( const C_Context& context )
{
	if ( m_inputs.count == 0 )
	{
		m_activeChild0 = -1;
		m_activeChild1 = -1;
		m_alpha = 0.f;
		return;
	}

	// Determine active children.
	
	F32 controlInput = m_controlInput ? m_controlInput->GetValue( context ) : 0.f;
	
	// Assuming children are sorted by their control values.
	S32 currInputIndex = m_inputs.count;
	for ( S32 i = 1; i < m_inputs.count; ++i )
	{
		if ( m_inputs[i].m_controlValue > controlInput )
		{
			currInputIndex = i;
			break;			
		}
	}
	
	S32 prevInputIndex = currInputIndex - 1;
	if ( prevInputIndex < 0 )
	{
		if ( m_wrap )
			prevInputIndex = m_inputs.count - 1;
		else
			prevInputIndex = 0;
	}
	
	if ( currInputIndex >= m_inputs.count )
	{
		currInputIndex = m_inputs.count - 1;
	}
	
	if ( currInputIndex > prevInputIndex )
	{
		m_activeChild0 = prevInputIndex;
		m_activeChild1 = currInputIndex;
		m_alpha = (controlInput - m_inputs[prevInputIndex].m_controlValue ) * Maths::Reciprocal( m_inputs[currInputIndex].m_controlValue - m_inputs[prevInputIndex].m_controlValue);
		//m_alpha = Maths::SmoothStep( m_alpha );
		m_alpha = Maths::Clamp( m_alpha, 0.f, 1.f );
	}
	else
	{
		// just one active child.
		m_activeChild0 = prevInputIndex;
		m_activeChild1 = -1;
		m_alpha = 0.f;
	}
	
	if ( m_activeChild0 >= 0 )
	{
		if ( m_inputs[m_activeChild0].m_input == NULL ) m_activeChild0 = -1;
	}
	
	if ( m_activeChild1 >= 0 )
	{
		if ( m_inputs[m_activeChild1].m_input == NULL ) m_activeChild1 = -1;
	}
	
}	

void C_BlendNode::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	C_Node::SaveState(out);
	out << m_activeChild0 << m_activeChild1;
	out << m_alpha;
}

void C_BlendNode::LoadState(Animation::C_MemoryBlockReader& in)
{
	C_Node::LoadState(in);
	in >> m_activeChild0 >> m_activeChild1;
	
	// ensure data is valid.
	if ( m_activeChild0 >= m_inputs.count ) m_activeChild0 = -1;
	if ( m_activeChild1 >= m_inputs.count ) m_activeChild1 = -1;
	
	if ( m_activeChild0 >= 0 && m_inputs[m_activeChild0].m_input == NULL ) m_activeChild0 = -1;
	if ( m_activeChild1 >= 0 && m_inputs[m_activeChild1].m_input == NULL ) m_activeChild1 = -1;
	
	in >> m_alpha;
}

//
// C_LayeredBlendNode
//

C_LayeredBlendNode::C_LayeredBlendNode():
	C_Node(E_NodeType_LayeredBlend)
{
}

void C_LayeredBlendNode::LoadResources(C_ResourceCache& rc)
{
	if ( m_baseLayer != NULL )
		m_baseLayer->LoadResources( rc );
	for (S32 i = 0; i < m_layers.count; ++i)
		if (m_layers[i].input != NULL)
			m_layers[i].input->LoadResources( rc );
}

void C_LayeredBlendNode::Evaluate( C_Context& context, Animation::E_BlendMode blendMode, F32 weight )
{
	context.commandList->PushCurrentPose( weight );
	
	if ( m_baseLayer != NULL )
	{
		m_baseLayer->Evaluate( context, Animation::E_BlendMode_Accumulate, 1.f );			
	}
	
	for (S32 i = 0; i < m_layers.count; ++i)
	{
		if (m_layers[i].input != NULL)
		{
			Animation::E_BlendMode layerBlendMode = m_layers[i].blendMode;
			m_layers[i].input->Evaluate( context, layerBlendMode, 1.f );
		}
	}

	context.commandList->PopAndBlendPose( blendMode );
}

void C_LayeredBlendNode::SetPhase( const C_Context& context, F32 phase )
{
	// Doesn't make a lot of sense, since each layer is independent.
	if (m_baseLayer != NULL)
		m_baseLayer->SetPhase(context, phase);
	for (S32 i = 0; i < m_layers.count; ++i)
		if (m_layers[i].input != NULL)
			m_layers[i].input->SetPhase( context, phase );
}

void C_LayeredBlendNode::UpdateTime( const C_Context& context, F32 deltaTime )
{
	if (m_baseLayer != NULL)
		m_baseLayer->UpdateTime( context, deltaTime );

	for (S32 i = 0; i < m_layers.count; ++i)
		if (m_layers[i].input != NULL)
			m_layers[i].input->UpdateTime( context, deltaTime );
}

F32 C_LayeredBlendNode::GetFrequency( const C_Context& context )
{
	if (m_baseLayer != NULL)
	{
		return m_baseLayer->GetFrequency(context);
	}
	return 0.f;
}

//
// C_MirrorNode
//

C_MirrorNode::C_MirrorNode():
	C_Node(E_NodeType_Mirror),
	m_input(NULL),
	m_currentSkeleton(NULL),
	m_enabled(false)
{
}

C_MirrorNode::~C_MirrorNode()
{
}

void C_MirrorNode::LoadResources(C_ResourceCache& rc)
{
	if ( m_input ) m_input->LoadResources( rc );
}

void C_MirrorNode::Evaluate( C_Context& context, Animation::E_BlendMode blendMode, F32 weight )
{
	if ( NULL == m_input ) return;
	
	// Evaluate into a temporary pose.
	context.commandList->PushCurrentPose( weight );
	m_input->Evaluate( context, Animation::E_BlendMode_Accumulate, weight );
	
	if ( m_enabled )
	{
		if ( context.skeleton != m_currentSkeleton )
		{
			m_currentSkeleton = context.skeleton;
		
			m_poseMirror.SetMirrorAxis( m_mirrorAxis );
			
			if ( context.skeleton )
			{
				m_poseMirror.Create( *context.skeleton );
			}
			else
			{
				m_poseMirror.Clear();
			}
		}
		
		// Mirror pose, in-place.
		if ( context.skeleton )
		{
			context.commandList->MirrorPose( m_poseMirror );
		}
	}
	
	// Blend in.
	context.commandList->PopAndBlendPose( blendMode );
}

void C_MirrorNode::SetPhase( const C_Context& context, F32 phase )
{
	if ( m_input ) m_input->SetPhase( context, phase );
}

void C_MirrorNode::UpdateTime( const C_Context& context, F32 deltaTime )
{
	if ( m_input ) m_input->UpdateTime( context, deltaTime );
}

F32 C_MirrorNode::GetFrequency( const C_Context& context )
{
	return m_input != NULL ? m_input->GetFrequency( context ) : 0.f;
}


//
// C_TimeControllerNode
// 

C_TimeControllerNode::C_TimeControllerNode():
	C_Node(E_NodeType_TimeController)
{
}

C_TimeControllerNode::~C_TimeControllerNode()
{
}
		
void C_TimeControllerNode::LoadResources(C_ResourceCache& rc)
{
	if ( m_input != NULL ) m_input->LoadResources( rc );
}

void C_TimeControllerNode::Evaluate( C_Context& context, Animation::E_BlendMode blendMode, F32 weight )
{
	if ( m_input != NULL )
	{
		m_input->Evaluate( context, blendMode, weight );
	}
}

void C_TimeControllerNode::SetPhase( const C_Context& context, F32 phase )
{
	SetChildTime( context );
}

void C_TimeControllerNode::UpdateTime( const C_Context& context, F32 deltaTime )
{
	SetChildTime( context );
}

F32 C_TimeControllerNode::GetFrequency( const C_Context& context )
{
	return 0.f; 
}

void C_TimeControllerNode::SetChildTime( const C_Context& context )
{
	if ( m_input != NULL )
	{
		F32 controlInput = 0.f;
		
		if ( m_controlInput != NULL )
		{
			controlInput = m_controlInput->GetValue( context );
		}
		
		F32 normalisedInput = ( controlInput - m_inputRange.from ) * Maths::Reciprocal( m_inputRange.to - m_inputRange.from );
		F32 normalisedTime = m_timeRange.from + normalisedInput * ( m_timeRange.to - m_timeRange.from );
		
		m_input->SetPhase( context, normalisedTime );
	}
	
}

//
// C_Graph
//


C_Graph* C_Graph::s_activeGraph = NULL;

C_Graph::C_Graph():
	m_cmdList( 256 ),
	m_result(NULL),
	m_updateCounter( 0 ),
	m_globalTime(0),
	m_timelineValue(0.f)
{
}

C_Graph::~C_Graph()
{
}

void C_Graph::UpdateControllerState( const C_ControllerState& state )
{
	m_controllerState = state;
}

void C_Graph::UpdateTime( F32 deltaTime )
{
	++m_updateCounter;
	m_deltaTime = deltaTime;
	m_globalTime += static_cast<S32>(deltaTime * 1000.f);
	
	C_Context context;
	context.skeleton = NULL;
	context.commandList = NULL;
	context.controllerState = &m_controllerState;
	context.inWorldSpace = false;
	context.updateCounter = m_updateCounter;
	context.deltaTime = m_deltaTime;
	context.globalTime = static_cast<F32>(m_globalTime / 1000.f);
	context.timelineValue = m_timelineValue;

	if ( m_result )
	{
		m_result->UpdateTime( context, deltaTime );
	}
}

void C_Graph::SetTimelineValue(F32 value)
{
	m_timelineValue = value;
}

bool C_Graph::Evaluate( const Animation::C_Skeleton* skeleton, Animation::C_Pose& pose, bool& outPoseInWorldSpace, bool enablePushPopElimination )
{
	if ( skeleton == NULL ) return true;
	if ( skeleton->GetNumJoints() == 0 ) return true;
	
	C_Context context;
	
	m_cmdList.Clear();
	m_cmdList.EnablePushPopElimination( enablePushPopElimination );
	
	context.skeleton = skeleton;
	context.commandList = &m_cmdList;
	context.controllerState = &m_controllerState;
	context.inWorldSpace = false;
	context.updateCounter = m_updateCounter;
	context.deltaTime = m_deltaTime;
	context.globalTime = static_cast<F32>(m_globalTime / 1000.f);
	
	if ( m_result )
	{
		m_result->Evaluate( context, Animation::E_BlendMode_Accumulate, 1.f );
	}
	
	m_cmdList.BindToPose( pose );
	
	Animation::C_CommandProcessor cmdProc;
	bool result = cmdProc.ExecuteCommandList( m_cmdList, pose, skeleton, NULL );
	
	outPoseInWorldSpace = context.inWorldSpace;
	
	return result;
}

void C_Graph::SaveState(Animation::C_MemoryBlock& dest) const
{
	Animation::C_MemoryBlockWriter out( dest );
	out << U32(m_cmdList.GetUpdateCounter());
	out << m_timelineValue;
	out << S32(m_nodes.count);
	for (S32 i = 0; i < m_nodes.count; ++i )
	{
		if ( m_nodes[ i ] )
		{
			out << U32(m_nodes[i]->GetType());
			m_nodes[ i ]->SaveState( out );
		}
	}
	
	out << S32(m_inputVars.count);
	for ( S32 i = 0; i < m_inputVars.count; ++i )
	{
		if ( m_inputVars[ i ] )
		{
			out << U32(m_inputVars[i]->GetType());
			m_inputVars[i]->SaveState( out );
		}
	}
}

void C_Graph::LoadState(const Animation::C_MemoryBlock& src)
{
	Animation::C_MemoryBlockReader in(src);
	U32 counter;
	in >> counter;
	m_cmdList.SetUpdateCounter( counter );
	in >> m_timelineValue;
	S32 n = 0;
	in >> n;
	n = n > m_nodes.count ? m_nodes.count : n;
	for (S32 i = 0; i < n; ++i )
	{
		if ( m_nodes[ i ] != NULL )
		{
			U32 type;
			in >> type;
			if (type != m_nodes[i]->GetType())
			{
				LOGINFO(gLog, "Incompatible state\n" );
				return;
			}
			m_nodes[ i ]->LoadState( in );
		}
	}
	
	S32 numVars = 0;
	in >> numVars;
	numVars = numVars > m_inputVars.count ? m_inputVars.count : numVars;
	for (S32 i = 0; i < numVars; ++i )
	{
		if ( m_inputVars[ i ] != NULL )
		{
			U32 type;
			in >> type;
			if (type != m_inputVars[ i ]->GetType())
			{
				LOGINFO(gLog, "Incompatible state\n" );
				return;
			}
			m_inputVars[ i ]->LoadState( in );
		}
	}	
}

bool gGraphStateReady = false;
Animation::C_MemoryBlock gGraphState( 4096 );

void C_Graph::XtCallback(XtEventType eventType, XtRecord* record, C_Graph* graph, XtCallbackContext* context)
{
	switch ( eventType )
	{
		case kXtEtConstruct:
			//LOGINFO(gLog, "Constructed animation graph %X\n", ::GetCurrentThreadId() );
			
			s_activeGraph = NULL;
			
			C_ResourceCache::Instance().ClearReloadFlag();
			if ( graph->m_result && C_ResourceCache::Instance().SkeletonLoaded() )
			{
				graph->m_result->LoadResources(C_ResourceCache::Instance());
			}
			
			if (gGraphStateReady)
				graph->LoadState(gGraphState);
			
			C_ResourceCache::Instance().Purge();
			
			if ( C_ResourceCache::Instance().GetReloadFlag() )
			{
				// TODO: calculate hash or checksum for names.
				graph->UpdateCustomCurveNames();
			}		
			
			s_activeGraph = graph;
						
			break;
		case kXtEtDestruct:
			//LOGINFO(gLog, "Destroyed animation graph %X\n", ::GetCurrentThreadId() );
			graph->SaveState(gGraphState);
			gGraphStateReady = true;
			s_activeGraph = NULL;
			break;
		default:
			break;
	}
}

C_StringSet C_Graph::m_customScalarCurveNames;
C_StringSet C_Graph::m_customVector3CurveNames;
C_StringSet C_Graph::m_customVector4CurveNames;
C_Checksum16 C_Graph::m_customCurveChecksum;

void UpdateChecksum( C_Checksum16& checksum, const C_StringSet& strings )
{
	for ( C_StringSet::ConstIterator iter = strings.Begin(); iter != strings.End(); ++iter )
	{
		const Btl::String& str = iter->GetKey();
		checksum.Update( str.Chars(), str.Length() );
	}
}

void C_Graph::UpdateCustomCurveNames()
{
	m_customScalarCurveNames.Clear();
	m_customVector3CurveNames.Clear();
	m_customVector4CurveNames.Clear();
		
	Btl::String fieldName;
	
	for ( S32 i = 0; i < m_nodes.count; ++i )
	{
		if ( m_nodes[i] )
		{
			if ( m_nodes[i]->GetType() != E_NodeType_Cycle )
			{
				continue;
			}

			const C_CycleNode* cycleNode = static_cast<const C_CycleNode*>(m_nodes[i]);
			const Animation::C_Cycle* cycle = cycleNode->m_cycle;
			if ( cycle == NULL ) continue;
				
			for ( U32 j = 0; j < cycle->GetNumFields(); ++j )
			{
				fieldName = cycle->GetFieldName( j ).GetString();
				
				switch ( cycle->GetFieldType(j) )
				{
					case Animation::E_PoseFieldType_CustomVector3:
						m_customVector3CurveNames.Insert( fieldName );
						break;
						
					case Animation::E_PoseFieldType_CustomScalar:
						m_customScalarCurveNames.Insert( fieldName );
						break;
						
					case Animation::E_PoseFieldType_CustomVector4:
						m_customVector4CurveNames.Insert( fieldName );
						break;
						
					default:
						break;
				}
			}
		}
	}
	m_customCurveChecksum.Clear();
	UpdateChecksum( m_customCurveChecksum, m_customScalarCurveNames );
	UpdateChecksum( m_customCurveChecksum, m_customVector3CurveNames );
	UpdateChecksum( m_customCurveChecksum, m_customVector4CurveNames );
}




} // end AnimationGraph
