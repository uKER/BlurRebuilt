#include "stdafx.h"
#include <Game/GameActors/Drivers/XtAnimationTree.h>
#include <Game/GameActors/Drivers/AnimationTree.h>
#include <animation/pose/PoseMirror.h>

XT_TYPE(Animation::E_BlendMode, "AnimBlendMode")
	XT_OPTION(Animation::E_BlendMode_Accumulate, "Accumulate")
	XT_OPTION(Animation::E_BlendMode_Additive, "Additive")
	XT_OPTION(Animation::E_BlendMode_Lerp, "Lerp")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtTree, "AnimTree")
	XT_CALLBACK(AnimationTree::C_XtTree::XtCallback)
	XT_FIELD(name, "Name")
	XT_FIELD(root, "Root")
	XT_FIELD(nodes, "Nodes")
	XT_FIELD(actionIds, "Actions")
	XT_FIELD(parameterIds, "ParameterIds")
	XT_FIELD(fixedParameterIds, "FixedParameterIds")
	XT_FIELD(ikTargetIds, "IkTargetIds")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtActionId, "AnimActionId")
	XT_FIELD(name, "Name")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtParameterId, "AnimParameterId")
	XT_FIELD(name, "Name")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtFixedParameterId, "AnimFixedParameterId")
	XT_FIELD(name, "Name")
XT_END_TYPE

XT_ABSTRACT_TYPE(AnimationTree::C_XtParameterBase, "AnimParameterBase")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtParameter, "AnimParameter")
	XT_BASE(AnimationTree::C_XtParameterBase)
	XT_FIELD(source, "Source")
	XT_FIELD(feedbackParameterIndex, "FeedbackParameterIndex")
	XT_FIELD(enableFeedback, "EnableFeedback")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtConstantParameter, "AnimConstantParameter")
	XT_BASE(AnimationTree::C_XtParameterBase)
	XT_FIELD(value, "Value")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtSmoothedParameter, "AnimSmoothedParameter")
	XT_BASE(AnimationTree::C_XtParameter)
	XT_FIELD(smoothingFactor, "SmoothingFactor")
XT_END_TYPE

XT_ABSTRACT_TYPE(AnimationTree::C_XtNode, "AnimTreeNode")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtRootNode, "AnimRootNode")
	XT_BASE(AnimationTree::C_XtNode)
	XT_FIELD(input, "In")
XT_END_TYPE

XT_TYPE(AnimationTree::E_XtIkConstraints, "AnimIkConstraints")
	XT_OPTION(AnimationTree::E_XtIkConstraints_None, "None")
	XT_OPTION(AnimationTree::E_XtIkConstraints_BothHandsSteering, "BothHandsSteering")
	XT_OPTION(AnimationTree::E_XtIkConstraints_RightHandSteeringLeftHandFree, "RightHandSteeringLeftHandFree")
	XT_OPTION(AnimationTree::E_XtIkConstraints_LeftHandSteeringRightHandFree, "LeftHandSteeringRightHandFree")
	XT_OPTION(AnimationTree::E_XtIkConstraints_BothHandsFree, "BothHandsFree")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtCycle, "AnimCycle")
	XT_BASE(ResourceUtils::C_XtCycleResourceDef)
	XT_FIELD(ikConstraints, "IkConstraints")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtCycleGroupItem, "AnimCycleGroupItem")
	XT_FIELD(cycle, "Cycle")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtCycleGroup, "AnimCycleGroup")
	XT_FIELD(cycles, "Cycles")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtCycleNode, "AnimCycleNode")
	XT_BASE(AnimationTree::C_XtNode)
	XT_FIELD(cycle, "Cycle")
	XT_FIELD(timeScale, "TimeScale")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtFixedParameterOverrideNode, "AnimFixedParameterOverrideNode")
	XT_BASE(AnimationTree::C_XtNode)
	XT_FIELD(fixedParameter, "FixedParameter")
	XT_FIELD(overrideValue, "OverrideValue")
	XT_FIELD(input, "Input")
XT_END_TYPE

XT_TYPE(AnimationTree::E_XtSelectMode, "AnimSelectMode")
	XT_OPTION(AnimationTree::E_XtSelectMode_Random, "Random")
	XT_OPTION(AnimationTree::E_XtSelectMode_Sequential, "Sequential")
	XT_OPTION(AnimationTree::E_XtSelectMode_AnimIndex, "AnimIndex")
	XT_OPTION(AnimationTree::E_XtSelectMode_FixedParameter, "FixedParameter")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtSelectNode, "AnimSelectNode")
	XT_BASE(AnimationTree::C_XtNode)
	XT_FIELD(selectMode, "SelectMode")
	XT_FIELD(fixedParameter, "FixedParameter")
	XT_FIELD(inputs, "Inputs")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtCycleGroupNode, "AnimCycleGroupNode")
	XT_BASE(AnimationTree::C_XtNode)
	XT_FIELD(cycleGroup, "CycleGroup")
	XT_FIELD(selectMode, "SelectMode")
	XT_FIELD(fixedParameter, "FixedParameter")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtStateMachineNode, "AnimStateMachineNode")
	XT_BASE(AnimationTree::C_XtNode)
	XT_FIELD(initialState, "InitialState")
	//XT_FIELD(states, "States")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtState, "AnimState")
	XT_FIELD(name, "Name")
	XT_FIELD(action, "Action")
	XT_FIELD(input, "Input")
	XT_FIELD(transitions, "Transitions")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtTransition, "AnimTransition")
	XT_FIELD(nextState, "NextState")
	XT_FIELD(blendTime, "BlendTime")
	XT_FIELD(conditions, "Conditions")
XT_END_TYPE

XT_ABSTRACT_TYPE(AnimationTree::C_XtTransitionCondition, "AnimTransitionCondition")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtFinishedCondition, "AnimFinishedCondition")
	XT_BASE(AnimationTree::C_XtTransitionCondition)
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtRequestedActionCondition, "AnimRequestedActionCondition")
	XT_BASE(AnimationTree::C_XtTransitionCondition)
	XT_FIELD(action, "Action")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtRangeCondition, "AnimRangeCondition")
	XT_BASE(AnimationTree::C_XtTransitionCondition)
	XT_FIELD(parameter, "Parameter")
	XT_FIELD(range, "Range")
XT_END_TYPE

XT_TYPE(AnimationTree::E_XtOperator, "AnimOperator")
	XT_OPTION(AnimationTree::E_XtOperator_EQ, "EQ")
	XT_OPTION(AnimationTree::E_XtOperator_NEQ, "NEQ")
	XT_OPTION(AnimationTree::E_XtOperator_LT, "LT")
	XT_OPTION(AnimationTree::E_XtOperator_LEQ, "LEQ")
	XT_OPTION(AnimationTree::E_XtOperator_GT, "GT")
	XT_OPTION(AnimationTree::E_XtOperator_GEQ, "GEQ")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtComparisonCondition, "AnimComparisonCondition")
	XT_BASE(AnimationTree::C_XtTransitionCondition)
	XT_FIELD(parameter1, "Parameter1")
	XT_FIELD(parameter2, "Parameter2")
	XT_FIELD(op, "Operator")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtSimpleState, "AnimSimpleState")
	XT_FIELD(entryBlendTime, "EntryBlendTime")
	XT_FIELD(exitBlendTime, "ExitBlendTime")
	XT_FIELD(entryCondition, "EntryCondition")
	XT_FIELD(secondaryEntryCondition, "SecondaryEntryCondition")
	XT_FIELD(exitCondition, "ExitCondition")
	XT_FIELD(input, "Input")
XT_END_TYPE


XT_TYPE(AnimationTree::C_XtSimpleStateMachineNode, "AnimSimpleStateMachineNode")
	XT_BASE(AnimationTree::C_XtNode)
	XT_FIELD(idle, "Idle")
	XT_FIELD(states, "States")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtLayeredBlendNode::Layer, "AnimLayer")
	XT_FIELD(blendMode, "BlendMode")
	XT_FIELD(tree, "Tree")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtLayeredBlendNode, "AnimLayeredBlendNode")
	XT_BASE(AnimationTree::C_XtNode)
	XT_FIELD(baseLayer, "BaseLayer")
	XT_FIELD(layers, "Layers")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtBlendInput, "AnimBlendInput")
	XT_FIELD(input, "Input")
	XT_FIELD(parameterValue, "ParameterValue")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtBlendNode, "AnimBlendNode")
	XT_BASE(AnimationTree::C_XtNode)
	XT_FIELD(blendMode, "BlendMode")
	XT_FIELD(syncTime, "SyncTime")
	XT_FIELD(smoothStep, "SmoothStep")
	XT_FIELD(useAbsoluteParameterValue, "UseAbsoluteParameterValue")
	XT_FIELD(parameter, "Parameter")
	XT_FIELD(blendInputs, "BlendInputs")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtTimeControllerNode, "AnimTimeControllerNode")
	XT_BASE(AnimationTree::C_XtNode)
	XT_FIELD(parameter, "Parameter")
	XT_FIELD(parameterRange, "ParameterRange")
	XT_FIELD(timeRange, "TimeRange")
	XT_FIELD(input, "Input")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtIkTargetId, "AnimIkTargetId")
	XT_FIELD(name, "Name")
XT_END_TYPE

XT_ABSTRACT_TYPE(AnimationTree::C_XtIkTargetBase, "AnimIkTargetBase")
	XT_FIELD(ikTargetId, "IkTarget")
	XT_FIELD(setPosition, "SetPosition")
	XT_FIELD(setRotation, "SetRotation")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtIkTargetFromJoint, "AnimIkTargetFromJoint")
	XT_BASE(AnimationTree::C_XtIkTargetBase)
	XT_FIELD(jointName, "JointName")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtConstantIkTarget, "AnimConstantIkTarget")
	XT_BASE(AnimationTree::C_XtIkTargetBase)
	XT_FIELD(position, "Position")
	XT_FIELD(rotation, "Rotation")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtIkSetTargetNode, "AnimIkSetTargetNode")
	XT_BASE(AnimationTree::C_XtNode)
	XT_FIELD(targets, "Targets")
	XT_FIELD(input, "Input")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtIkEnableNode, "AnimIkEnableNode")
	XT_BASE(AnimationTree::C_XtNode)
	XT_FIELD(enabledIkTargets, "EnabledIkTargets")
	XT_FIELD(disabledIkTargets, "DisabledIkTargets")
	XT_FIELD(input, "Input")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtIkApplyNode, "AnimIkApplyNode")
	XT_BASE(AnimationTree::C_XtNode)
	XT_FIELD(input, "Input")
	XT_FIELD(disableIk, "DisableIK")
XT_END_TYPE

XT_TYPE(Animation::E_MirrorAxis, "AnimMirrorAxis")
	XT_OPTION(Animation::E_MirrorAxis_X, "X")
	XT_OPTION(Animation::E_MirrorAxis_Y, "Y")
	XT_OPTION(Animation::E_MirrorAxis_Z, "Z")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtMirrorNode, "AnimMirrorNode")
	XT_BASE(AnimationTree::C_XtNode)
	XT_FIELD(input, "Input")
	XT_FIELD(mirrorAxis, "MirrorAxis")
	XT_FIELD(enabled, "Enabled")
	XT_FIELD(isGlobalMirror, "IsGlobalMirror")
XT_END_TYPE

XT_TYPE(AnimationTree::C_XtTriggerNode, "AnimTriggerNode")
	XT_BASE(AnimationTree::C_XtNode)
	XT_FIELD(input, "Input")
	XT_FIELD(delay, "Delay")
	XT_FIELD(blendTime, "BlendTime")
XT_END_TYPE

XT_LIBRARY(AnimationTree)
	XT_REGISTER_TYPE(Animation::E_BlendMode)
	XT_REGISTER_TYPE(AnimationTree::C_XtTree)
	XT_REGISTER_TYPE(AnimationTree::C_XtActionId)
	XT_REGISTER_TYPE(AnimationTree::C_XtParameterId)
	XT_REGISTER_TYPE(AnimationTree::C_XtParameterBase)
	XT_REGISTER_TYPE(AnimationTree::C_XtParameter)
	XT_REGISTER_TYPE(AnimationTree::C_XtFixedParameterId)
	XT_REGISTER_TYPE(AnimationTree::C_XtConstantParameter)
	XT_REGISTER_TYPE(AnimationTree::C_XtSmoothedParameter)
	XT_REGISTER_TYPE(AnimationTree::C_XtNode)
	XT_REGISTER_TYPE(AnimationTree::C_XtRootNode)
	XT_REGISTER_TYPE(AnimationTree::E_XtIkConstraints)
	XT_REGISTER_TYPE(AnimationTree::C_XtCycle)
	XT_REGISTER_TYPE(AnimationTree::C_XtCycleGroupItem)
	XT_REGISTER_TYPE(AnimationTree::C_XtCycleGroup)
	XT_REGISTER_TYPE(AnimationTree::C_XtCycleNode)
	XT_REGISTER_TYPE(AnimationTree::C_XtFixedParameterOverrideNode)
	XT_REGISTER_TYPE(AnimationTree::E_XtSelectMode)
	XT_REGISTER_TYPE(AnimationTree::C_XtSelectNode)
	XT_REGISTER_TYPE(AnimationTree::C_XtCycleGroupNode)
	XT_REGISTER_TYPE(AnimationTree::C_XtBlendInput)
	XT_REGISTER_TYPE(AnimationTree::C_XtBlendNode)
	XT_REGISTER_TYPE(AnimationTree::C_XtLayeredBlendNode::Layer)
	XT_REGISTER_TYPE(AnimationTree::C_XtLayeredBlendNode)
	XT_REGISTER_TYPE(AnimationTree::C_XtState)
	XT_REGISTER_TYPE(AnimationTree::C_XtTransition)
	XT_REGISTER_TYPE(AnimationTree::C_XtTransitionCondition)
	XT_REGISTER_TYPE(AnimationTree::C_XtFinishedCondition)
	XT_REGISTER_TYPE(AnimationTree::C_XtRequestedActionCondition)
	XT_REGISTER_TYPE(AnimationTree::C_XtRangeCondition)
	XT_REGISTER_TYPE(AnimationTree::C_XtComparisonCondition)
	XT_REGISTER_TYPE(AnimationTree::E_XtOperator)
	XT_REGISTER_TYPE(AnimationTree::C_XtStateMachineNode)
	XT_REGISTER_TYPE(AnimationTree::C_XtSimpleState)
	XT_REGISTER_TYPE(AnimationTree::C_XtSimpleStateMachineNode)
	XT_REGISTER_TYPE(AnimationTree::C_XtTimeControllerNode)
	XT_REGISTER_TYPE(AnimationTree::C_XtIkTargetId)
	XT_REGISTER_TYPE(AnimationTree::C_XtIkTargetBase)
	XT_REGISTER_TYPE(AnimationTree::C_XtIkTargetFromJoint)
	XT_REGISTER_TYPE(AnimationTree::C_XtConstantIkTarget)
	XT_REGISTER_TYPE(AnimationTree::C_XtIkSetTargetNode)
	XT_REGISTER_TYPE(AnimationTree::C_XtIkEnableNode)
	XT_REGISTER_TYPE(AnimationTree::C_XtIkApplyNode)
	XT_REGISTER_TYPE(Animation::E_MirrorAxis)
	XT_REGISTER_TYPE(AnimationTree::C_XtMirrorNode)
	XT_REGISTER_TYPE(AnimationTree::C_XtTriggerNode)

XT_END_LIBRARY

namespace AnimationTree
{



//
// C_XtTree
//

bool C_XtTree::Build(C_Tree& tree) const
{
	tree.BeginBuild();

	if (root == NULL)
	{
		tree.EndBuild();
		return false;
	}
	
	C_Node* rootNode = root->Build(tree);
	tree.SetRoot(rootNode);

	tree.EndBuild();
	
	return true;
}

void C_XtTree::XtCallback(XtEventType et, XtRecord* record, C_XtTree* tree, XtCallbackContext* context)
{
	switch (et)
	{
		case kXtEtConstruct:
		{
			// Inform objects listening for this tree that it has been created.
			for (ListenerList::Iterator iter = m_listeners.Begin(); iter != m_listeners.End(); ++iter)
			{
				if (!stricmp(iter->GetXtAnimTreeName(), tree->name))
				{
					iter->OnXtAnimTreeCreated(record, tree);
				}
			}
			
			m_trees.PushBack(tree);
			
			break;
		}
		
		case kXtEtDestruct:
		{
			// Inform objects listening for this tree that it has been destroyed.
			for (ListenerList::Iterator iter = m_listeners.Begin(); iter != m_listeners.End(); ++iter)
			{
				if (!stricmp(iter->GetXtAnimTreeName(), tree->name))
				{
					iter->OnXtAnimTreeDestroyed(record, tree);
				}
			}
			
			bool foundTree = false;
			for (S32 i = 0, count = m_trees.Size(); i < count; ++i)
			{
				if (m_trees[i] == tree)
				{
					m_trees.EraseByIndexNonOrdered(i);
					foundTree = true;
					break;
				}				
			}
			assert(foundTree);
			
			break;
		}
			
		default:
			break;
	}
}

C_XtTree::ListenerList C_XtTree::m_listeners;
C_XtTree::TreeVector C_XtTree::m_trees;

void C_XtTree::AddListener(C_XtAnimTreeListener* listener)
{
	m_listeners.PushBack(*listener);
}

void C_XtTree::RemoveListener(C_XtAnimTreeListener* listener)
{
	for (ListenerList::Iterator iter = m_listeners.Begin(); iter != m_listeners.End(); ++iter)
	{
		if (&(*iter) == listener)
		{
			m_listeners.Erase(iter);
			return;
		}
	}
}

const C_XtTree* C_XtTree::Find(const CHAR8* name)
{
	for (S32 i = 0, count = m_trees.Size(); i < count; ++i)
	{
		if (!stricmp(m_trees[i]->name, name))
		{
			return m_trees[i];
		}				
	}
	return NULL;
}

//
// C_XtParameter
//

C_Parameter C_XtParameter::Build(C_Tree& tree) const
{
	if (source != NULL)
	{
		S32 feedbackIndex = (enableFeedback && feedbackParameterIndex >= 0) ? feedbackParameterIndex : -1;
		return C_Parameter(tree, source->name, 1.f, feedbackIndex);
	}

	return C_Parameter();
}

//
// C_XtConstantParameter
//

C_Parameter C_XtConstantParameter::Build(C_Tree& tree) const
{
	return C_Parameter(value);
}

//
// C_XtSmoothedParameter
//

C_Parameter C_XtSmoothedParameter::Build(C_Tree& tree) const
{
	if (source != NULL)
	{
		S32 feedbackIndex = (enableFeedback && feedbackParameterIndex >= 0) ? feedbackParameterIndex : -1;
		return C_Parameter(tree, source->name, smoothingFactor, feedbackIndex);
	}
	return C_Parameter();
}

//
// C_XtRootNode
//
C_Node* C_XtRootNode::Build(C_Tree& tree) const
{
	if (input == NULL) return NULL;
	
	return input->Build(tree);
}

C_IkConstraints BuildIkConstraints(E_XtIkConstraints xtIkConstraints, const C_Tree& tree)
{
	C_IkConstraints c;

	bool leftHandSteering = false;
	bool rightHandSteering = false;
	bool leftHandMisc = false;
	bool rightHandMisc = false;

	switch (xtIkConstraints)
	{
		case E_XtIkConstraints_None:
			return c;
			break;

		case E_XtIkConstraints_BothHandsSteering:
			leftHandSteering = true;
			rightHandSteering = true;
			leftHandMisc = false;
			rightHandMisc = false;
			break;
		case E_XtIkConstraints_RightHandSteeringLeftHandFree:
			leftHandSteering = false;
			leftHandMisc = true;
			rightHandSteering = true;
			rightHandMisc = false;
			break;
		case E_XtIkConstraints_LeftHandSteeringRightHandFree:
			leftHandSteering = true;
			leftHandMisc = false;
			rightHandSteering = false;
			rightHandMisc = true;
			break;
		case E_XtIkConstraints_BothHandsFree:
			leftHandSteering = false;
			leftHandMisc = true;
			rightHandSteering = false;
			rightHandMisc = true;
			break;
	}

	c.SetIkTargetEnabled("LeftHandSteering", tree, leftHandSteering);
	c.SetIkTargetEnabled("LeftHandMisc", tree, leftHandMisc);
	c.SetIkTargetEnabled("RightHandSteering", tree, rightHandSteering);
	c.SetIkTargetEnabled("RightHandMisc", tree, rightHandMisc);
	return c;
}

//
// C_XtCycleNode
//

Util::C_String GetCycleFilename(const CHAR8* filenameNoExt)
{
	return Util::C_String::Format("%s.cycle", filenameNoExt);
}

C_Node* C_XtCycleNode::Build(C_Tree& tree) const
{
	if (cycle == NULL) return NULL;
	
	C_CycleNode* n = tree.CreateCycleNode();
	
	F32 safeTimeScale = timeScale;
	if (safeTimeScale == 0.f) safeTimeScale = 1.f;
	
	n->Create(tree.GetResourceBag(), GetCycleFilename((const char*)cycle->m_FilenameMinusExt), cycle->m_Loopable ? Animation::E_LoopMode_Continuous : Animation::E_LoopMode_Once, safeTimeScale);
	n->SetIkConstraints(BuildIkConstraints(cycle->ikConstraints, tree));

	return n;
}

//
// C_XtFixedParameterOverrideNode
//

C_Node* C_XtFixedParameterOverrideNode::Build(C_Tree& tree) const
{
	if (!input) return 0;

	S32 paramIndex = -1;
	if (fixedParameter)
	{
		paramIndex = tree.FindFixedParameter(fixedParameter->name.Text());
	}

	F32 prevValue = 0.f; 
	if (paramIndex >= 0)
	{
		// Save previous parameter value and then override.
		prevValue = tree.GetFixedParameter(paramIndex);
		tree.SetFixedParameter(paramIndex, overrideValue);
	}

	C_Node* childTree = input->Build(tree);

	if (paramIndex >= 0)
	{
		// Restore previous parameter value.
		tree.SetFixedParameter(paramIndex, prevValue);
	}

	return childTree;
}

//
// C_XtSelectNode
//

C_Node* C_XtSelectNode::Build(C_Tree& tree) const
{
	if (inputs.count == 0) return NULL;
	
	C_Node* result = NULL;
	
	if (selectMode != E_XtSelectMode_FixedParameter && inputs.count > 0)
	{
		C_SelectNode* selectNode = tree.CreateSelectNode();
		
		for (S32 i = 0; i < inputs.count; ++i)
		{
			C_Node* inputNode = NULL;
			
			if (inputs[i] != NULL)
			{
				inputNode = inputs[i]->Build(tree);
			}
		
			selectNode->AddInput(inputNode);
		}
		
		switch (selectMode)
		{
			case E_XtSelectMode_Random:
				selectNode->SetSelectionType(C_SelectNode::E_SelectionType_Random);
				break;
			case E_XtSelectMode_Sequential:
				selectNode->SetSelectionType(C_SelectNode::E_SelectionType_Sequential);
				break;
			case E_XtSelectMode_AnimIndex:
				selectNode->SetSelectionType(C_SelectNode::E_SelectionType_AnimIndex);
				break;
			default:
				break;
		}	
		
		result = selectNode;
	}
	else
	{
		// If there is only a single input, just return a node for that.
		if (inputs.count == 1)
		{
			if (inputs[0] == NULL) return NULL;
			
			result = inputs[0]->Build(tree);
		}
		else
		{
			// Otherwise, use fixed parameter to select correct input.
	
			S32 inputIndex = 0;
			
			if (fixedParameter != NULL)
			{
				S32 paramIndex = tree.FindFixedParameter(fixedParameter->name.Text());
				if (paramIndex >= 0)
				{
					inputIndex = static_cast<S32>(tree.GetFixedParameter(paramIndex));
				}
			}

			inputIndex = inputIndex % inputs.count;
			
			if (inputs[inputIndex] != NULL)
			{
				result = inputs[inputIndex]->Build(tree);
			}
		
		}
	}

	return result;
}




//
// C_XtCycleGroupNode
//

C_Node* C_XtCycleGroupNode::Build(C_Tree& tree) const
{
	if (cycleGroup == NULL) return NULL;
	
	if (cycleGroup->cycles.count <= 0) return NULL;
	
	C_Node* result = NULL;
	
	// If parameter is fixed, create a single cycle node instead.
	if (selectMode != E_XtSelectMode_FixedParameter)
	{
		C_CycleGroupNode* groupNode = tree.CreateCycleGroupNode();
		
		for (S32 i = 0; i < cycleGroup->cycles.count; ++i)
		{
			if (cycleGroup->cycles[i].cycle != NULL)
			{
				groupNode->AddCycle(
					tree.GetResourceBag(), 
					GetCycleFilename(cycleGroup->cycles[i].cycle->m_FilenameMinusExt),
					BuildIkConstraints(cycleGroup->cycles[i].cycle->ikConstraints, tree),
					1.f,
					cycleGroup->cycles[i].cycle->m_Loopable ? Animation::E_LoopMode_Continuous : Animation::E_LoopMode_Once);
			}
		}
		
		switch (selectMode)
		{
			case E_XtSelectMode_Random:
				groupNode->SetSelectionType(C_CycleGroupNode::E_SelectionType_Random);
				break;
			case E_XtSelectMode_Sequential:
				groupNode->SetSelectionType(C_CycleGroupNode::E_SelectionType_Sequential);
				break;
			case E_XtSelectMode_AnimIndex:
				groupNode->SetSelectionType(C_CycleGroupNode::E_SelectionType_AnimIndex);
				break;
			default:
				break;
		}	
		
		result = groupNode;
	}
	else
	{
		S32 cycleIndex = 0;
		
		if (fixedParameter != NULL)
		{
			S32 paramIndex = tree.FindFixedParameter(fixedParameter->name.Text());
			if (paramIndex >= 0)
			{
				cycleIndex = static_cast<S32>(tree.GetFixedParameter(paramIndex));
			}
		}

		cycleIndex = Maths::Clamped(cycleIndex, 0, cycleGroup->cycles.count - 1);

		if (cycleGroup->cycles[cycleIndex].cycle != NULL)
		{
			C_CycleNode* n = tree.CreateCycleNode();
					
			n->Create(tree.GetResourceBag(), GetCycleFilename(cycleGroup->cycles[cycleIndex].cycle->m_FilenameMinusExt),
					cycleGroup->cycles[cycleIndex].cycle->m_Loopable ? Animation::E_LoopMode_Continuous : Animation::E_LoopMode_Once, 1.f);
			n->SetIkConstraints(BuildIkConstraints(cycleGroup->cycles[cycleIndex].cycle->ikConstraints, tree));
			result = n;
		}
		else
		{
			// TODO: warn
		}
	}

	return result;
}

//
// C_XtStateMachineNode
//

C_Node* C_XtStateMachineNode::Build(C_Tree& tree) const
{
	// Discover all possible states.
	Btl::Vector<const C_XtState*> states;
		
	if (initialState != NULL)
	{
		Btl::Vector<const C_XtState*> stateStack;
		stateStack.PushBack(initialState);
		states.PushBack(initialState);
			
		while (!stateStack.Empty())
		{
			const C_XtState* s = stateStack.Back();
			stateStack.PopBack();

			// Follow transitions.
			for (S32 i = 0; i < s->transitions.count; ++i)
			{
				const C_XtState* nextState = s->transitions[i].nextState;
				if (nextState != NULL)
				{
					bool notFoundState = true;
					for (S32 j = 0; j < states.Size() && notFoundState; ++j)
					{
						if (states[j] == nextState)
						{
							notFoundState = false;
						}
					}
					
					if (notFoundState)
					{
						stateStack.PushBack(nextState);
						states.PushBack(nextState);						
					}
				}
			}
		}
	}	
	
	if (0 == states.Size())
	{
		// Don't create a node if there are no states at all.
		return NULL;
	}


	C_StateMachineNode* sm = tree.CreateStateMachineNode();	
	if (sm == NULL) return NULL;

	C_StateMachineNode::C_Builder builder(*sm);

	builder.SetMaxStates(states.Size());
	
	// First pass: add all states.
	for (S32 i = 0; i < states.Size(); ++i)
	{
		const C_XtState& state = *states[i];
		
		S32 actionIndex = AnimationTree::C_Tree::kInvalidActionIndex;
		if (state.action != NULL)
		{
			actionIndex = tree.FindAction(state.action->name);
		}
		
		// Build input node.
		C_Node* inputNode = NULL;
		if (state.input != NULL)
		{
			inputNode = state.input->Build(tree);
		}
		
		// Count number of valid transitions.
		U32 numValidTransitions = 0;
		for (S32 j = 0; j < state.transitions.count; ++j)
		{
			if (state.transitions[j].nextState != NULL)
			{
				++numValidTransitions;
			}
		}
		
		state.stateIndex = builder.AddState(inputNode, actionIndex, numValidTransitions);
		state.currentStateMachine = sm;
	}
	
	// Second pass: add transitions and conditions.
	for (S32 i = 0; i < states.Size(); ++i)
	{
		const C_XtState& state = *states[i];
		for (S32 j = 0; j < state.transitions.count; ++j)
		{
			const C_XtTransition& trans = state.transitions[j];
						
			if (trans.nextState != NULL)
			{
				S32 nextStateIndex = -1;
				if (trans.nextState->currentStateMachine == sm)
				{
					nextStateIndex = trans.nextState->stateIndex;
				}

				// Count *actual* transitions required, duplicating where necessary.
				U32 numRequiredConditions = 0;
				for (S32 condIndex = 0; condIndex < trans.conditions.count; ++condIndex)
				{
					if (!trans.conditions[condIndex])
					{
						continue;
					}
					numRequiredConditions += trans.conditions[condIndex]->GetNumRequiredConditions();
				}

				builder.AddTransition(state.stateIndex, nextStateIndex, j, trans.blendTime, numRequiredConditions);
				
				for (S32 k = 0; k < trans.conditions.count; ++k)
				{
					const C_XtTransitionCondition* cond = trans.conditions[k];
					
					if (cond != NULL)
					{
						cond->Build(tree, builder);
					}
				}
			}
		}		
	}
	
	S32 initialStateIndex = -1;
	if (initialState != NULL)
	{
		initialStateIndex = initialState->stateIndex;
	}
	sm->SetInitialState(initialStateIndex);
		
	return sm;
}

S32 C_XtTransitionCondition::GetActionIndex(C_Tree& tree) const
{
	return AnimationTree::C_Tree::kInvalidActionIndex;
}

void C_XtFinishedCondition::Build(C_Tree& tree, C_StateMachineNode::C_Builder& stateMachineBuilder) const
{
	stateMachineBuilder.AddAnimationFinishedCondition();
}

void C_XtRequestedActionCondition::Build(C_Tree& tree, C_StateMachineNode::C_Builder& stateMachineBuilder) const
{
	stateMachineBuilder.AddRequestedActionCondition(GetActionIndex(tree));
}

S32 C_XtRequestedActionCondition::GetActionIndex(C_Tree& tree) const
{
	S32 actionIndex = AnimationTree::C_Tree::kInvalidActionIndex;
	if (action != NULL)
	{
		actionIndex = tree.FindAction(action->name);
	}
	return actionIndex;
}

void C_XtRangeCondition::Build(C_Tree& tree, C_StateMachineNode::C_Builder& stateMachineBuilder) const
{
	if (parameter != NULL)
	{
		C_Parameter p = parameter->Build(tree);
		
		stateMachineBuilder.AddComparisonCondition(p, C_StateMachineNode::E_Operator_GEQ, range.from);
		stateMachineBuilder.AddComparisonCondition(p, C_StateMachineNode::E_Operator_LEQ, range.to);
	}
}

void C_XtComparisonCondition::Build(C_Tree& tree, C_StateMachineNode::C_Builder& stateMachineBuilder) const
{
	if (parameter1 != NULL && parameter2 != NULL)
	{
		const C_Parameter p1 = parameter1->Build(tree);
		const C_Parameter p2 = parameter2->Build(tree);

		if (!p2.IsConstant())
		{
			LOGWARN(Debug::GetLogger("amax.animation.driver"), "State machine comparison condition second parameter is not constant");
		}
		
		C_StateMachineNode::E_Operator op2 = C_StateMachineNode::E_Operator_Count;
		switch (op)
		{
			case E_XtOperator_EQ:	op2 = C_StateMachineNode::E_Operator_EQ;  break;
			case E_XtOperator_NEQ:	op2 = C_StateMachineNode::E_Operator_NEQ; break;
			case E_XtOperator_LT:	op2 = C_StateMachineNode::E_Operator_LT;  break;
			case E_XtOperator_LEQ:	op2 = C_StateMachineNode::E_Operator_LEQ; break;
			case E_XtOperator_GT:	op2 = C_StateMachineNode::E_Operator_GT;  break;
			case E_XtOperator_GEQ:	op2 = C_StateMachineNode::E_Operator_GEQ; break;
			default: break;
		}
		stateMachineBuilder.AddComparisonCondition(p1, op2, p2.GetConstantValue());
	}
}

//
// C_XtSimpleStateMachineNode
// Simple state machine node. 
//

C_Node* C_XtSimpleStateMachineNode::Build(C_Tree& tree) const
{
	if (idle == NULL && states.count == 0)
	{
		return NULL;
	}
	
	C_StateMachineNode* sm = tree.CreateStateMachineNode();	
	if (sm == NULL) return NULL;
	
	// Add idle state.
	C_Node* idleNode = NULL;
	if (idle != NULL)
	{
		idleNode = idle->Build(tree);
	}

	C_StateMachineNode::C_Builder builder(*sm);

	builder.SetMaxStates(states.count + 1);

	S32 idleStateIndex = builder.AddState(idleNode, AnimationTree::C_Tree::kInvalidActionIndex, states.count);
	sm->SetInitialState(idleStateIndex);
	
	// Add reaction states.
	for (S32 i = 0; i < states.count; ++i)
	{
		S32 actionIndex = AnimationTree::C_Tree::kInvalidActionIndex;
		if (states[i].entryCondition != NULL)
		{
			actionIndex = states[i].entryCondition->GetActionIndex(tree);
		}
		
		if (actionIndex < (AnimationTree::C_Tree::kInvalidActionIndex + 1) && states[i].secondaryEntryCondition != NULL)
		{
			actionIndex = states[i].secondaryEntryCondition->GetActionIndex(tree);
		}
		
		U32 numEntryConditions = 0;
		if (states[i].entryCondition != NULL)
		{
			numEntryConditions += states[i].entryCondition->GetNumRequiredConditions();
		}
		if (states[i].secondaryEntryCondition != NULL)
		{
			numEntryConditions += states[i].secondaryEntryCondition->GetNumRequiredConditions();
		}

		U32 numExitConditions = 0;
		U32 numExitTransitions = 1;
		if (states[i].exitCondition != NULL)
		{
			numExitConditions += states[i].exitCondition->GetNumRequiredConditions();
		}
		else
		{
			// No way of leaving state...
			numExitTransitions = 0;
		}
		
		C_Node* stateInput = NULL;
		if (states[i].input != NULL)
		{
			stateInput = states[i].input->Build(tree);
		}

		// Add state.
		S32 reactionStateIndex = builder.AddState(stateInput, actionIndex, numExitTransitions);

		// Create transition from idle state to this reaction animation state.
		builder.AddTransition(idleStateIndex, reactionStateIndex, i, states[i].entryBlendTime, numEntryConditions);

		// Add conditions for this transition.
		if (states[i].entryCondition != NULL)
		{
			states[i].entryCondition->Build(tree, builder);
		}
		if (states[i].secondaryEntryCondition != NULL)
		{
			states[i].secondaryEntryCondition->Build(tree, builder);
		}
		
		if (numExitTransitions > 0)
		{
			// Create transition from this reaction state back to idle.
			builder.AddTransition(reactionStateIndex, idleStateIndex, 0, states[i].exitBlendTime, numExitConditions);
			
			if (states[i].exitCondition != NULL)
			{
				states[i].exitCondition->Build(tree, builder);
			}
		}
	}
	return sm;
}

//
// C_XtLayeredBlendNode
//

C_Node* C_XtLayeredBlendNode::Build(C_Tree& tree) const
{
	C_LayeredBlendNode* n = tree.CreateLayeredBlendNode();
	
	if (baseLayer != NULL)
	{
		C_Node* baseNode = baseLayer->Build(tree);
		n->SetBaseLayer(baseNode);
	}
	
	for (S32 i = 0; i < layers.count; ++i)
	{
		if (layers[i].tree != NULL)
		{
			C_Node* layerNode = layers[i].tree->Build(tree);
			n->AddLayer(layerNode, layers[i].blendMode);
		}
	}
	
	return n;
}

//
// C_XtBlendNode
//

C_Node* C_XtBlendNode::Build(C_Tree& tree) const
{
	if (blendInputs.count == 0) return NULL;
	if (parameter == NULL) return NULL;
	
	C_BlendNode* blendNode = tree.CreateBlendNode();
	
	blendNode->SetBlendMode(blendMode);
	
	U32 flags = 0;
	if (syncTime) flags |= C_BlendNode::E_Flag_SyncTime;
	if (smoothStep) flags |= C_BlendNode::E_Flag_SmoothStep;
	if (useAbsoluteParameterValue) flags |= C_BlendNode::E_Flag_Absolute;
	blendNode->SetFlags(flags);
	
	C_Parameter p = parameter->Build(tree);
	blendNode->SetParameter(p);
	
	for (S32 i = 0; i < blendInputs.count; ++i)
	{
		C_Node* inputNode = NULL;
		if (blendInputs[i].input != NULL)
		{
			inputNode = blendInputs[i].input->Build(tree);
		}
		blendNode->AddInput(inputNode, blendInputs[i].parameterValue);
	}
	
	return blendNode;
}

//
// C_XtTimeControllerNode
//

C_Node* C_XtTimeControllerNode::Build(C_Tree& tree) const
{
	if (input == NULL) return NULL;
	
	C_Node* inputNode = input->Build(tree);

	C_TimeControllerNode* n = tree.CreateTimeControllerNode();
	n->SetInput(inputNode);
	
	C_Parameter param;
	if (parameter != NULL)
	{
		param = parameter->Build(tree);
	}
				
	n->SetParameter(param);
	n->SetMapping(parameterRange.from, parameterRange.to, timeRange.from, timeRange.to);
	
	return n;
}

//
// C_XtIkTargetFromJoint
//

void C_XtIkTargetFromJoint::Build(C_Tree& tree, C_IkSetTargetNode& node)
{
	if (ikTargetId != NULL)
	{
		S32 targetId = tree.FindIkTarget(ikTargetId->name);
		if (targetId >= 0)
		{
			node.AddTarget(targetId, Animation::C_StringId(jointName));
		}
	}
}

//
// C_XtConstantIkTarget
//

void C_XtConstantIkTarget::Build(C_Tree& tree, C_IkSetTargetNode& node)
{
	// not yet supported.
}


//
// C_XtIkSetTargetNode
//

C_Node* C_XtIkSetTargetNode::Build(C_Tree& tree) const
{
	if (NULL == input)
	{
		return NULL;
	}

	C_IkSetTargetNode* n = tree.CreateIkSetTargetNode();
	
	for (U32 i = 0, count = targets.count; i < count; ++i)
	{
		if (targets[i] != NULL)
		{
			targets[i]->Build(tree, *n);
		}
	}
	
	C_Node* inputNode = input->Build(tree);	
	n->SetInput(inputNode);
	
	return n;
}

//
// C_XtIkEnableNode
//

C_Node* C_XtIkEnableNode::Build(C_Tree& tree) const
{
	C_Node* inputNode = 0;

	if (input)
	{
		inputNode = input->Build(tree);
	}

	if (enabledIkTargets.count == 0 && disabledIkTargets.count == 0)
	{
		return inputNode;
	}

	C_IkEnableNode* ikEnableNode = tree.CreateIkEnableNode();
	ikEnableNode->SetInput(inputNode);

	for (S32 i = 0; i < enabledIkTargets.count; ++i)
	{
		if (!enabledIkTargets[i])
		{
			continue;
		}

		S32 ikTargetIndex = tree.FindIkTarget(enabledIkTargets[i]->name);
		if (ikTargetIndex < 0)
		{
			continue;
		}
		ikEnableNode->EnableIkTarget(ikTargetIndex);
	}

	for (S32 i = 0; i < disabledIkTargets.count; ++i)
	{
		if (!disabledIkTargets[i])
		{
			continue;
		}

		S32 ikTargetIndex = tree.FindIkTarget(disabledIkTargets[i]->name);
		if (ikTargetIndex < 0)
		{
			continue;
		}
		ikEnableNode->DisableIkTarget(ikTargetIndex);
	}

	return ikEnableNode;
}

//
// C_XtIkApplyNode
//

C_Node* C_XtIkApplyNode::Build(C_Tree& tree) const
{
	if (NULL == input) return NULL;
	
	C_Node* inputNode = input->Build(tree);
	if (tree.GetNumIkChains() > 0 && !disableIk)
	{
		C_IkApplyNode* ikApplyNode = tree.CreateIkApplyNode();
		ikApplyNode->SetInput(inputNode);
		return ikApplyNode;
	}
	return inputNode;
}

//
// C_XtMirrorNode
//

C_Node* C_XtMirrorNode::Build(C_Tree& tree) const
{
	if (input == NULL) return NULL;
	
	bool actuallyEnabled = enabled;
	if (isGlobalMirror)
	{
		actuallyEnabled = tree.IsGlobalMirrorEnabled();	
	}
		
	if (!actuallyEnabled)
	{
		return input->Build(tree);
	}
	
	C_Node* inputNode = input->Build(tree);

	C_MirrorNode* mirrorNode = tree.CreateMirrorNode();
	mirrorNode->Create(inputNode, mirrorAxis);
	return mirrorNode;
}

//
// C_XtTriggerNode
//

C_Node* C_XtTriggerNode::Build(C_Tree& tree) const
{
	if (!input)
	{
		return 0;
	}

	C_Node* inputNode = input->Build(tree);
	
	if (!inputNode) return 0;

	C_TriggerNode* triggerNode = tree.CreateTriggerNode();

	triggerNode->Create(inputNode, delay.mean, delay.d, blendTime);
	return triggerNode;
}

} // end AnimationTree

