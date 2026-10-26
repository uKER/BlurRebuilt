#ifndef XTANIMATIONTREE_H
#define XTANIMATIONTREE_H

#include <Game/GameActors/Drivers/XtAnimationTreeListener.h>
#include <Xt/XtGame.h>
#include <animation/common/BlendModes.h>
#include <animation/pose/PoseMirror.h>	// For E_MirrorAxis
#include <resource_utils/CycleResourceDef.h>
#include <btl/containers/list.h>
#include <btl/containers/vector.h>

//
// Sorcery/Xt interface for animation tree.
//

namespace AnimationTree
{

	class C_Tree;
	class C_Node;
	class C_Parameter;

	class C_XtNode;
	class C_XtActionId;
	class C_XtParameterId;
	class C_XtFixedParameterId;
	class C_XtIkTargetId;
	class C_XtRootNode;
	class C_IkSetTargetNode;

	class C_XtTree
	{
	public:
		XtSz8						name;
		C_XtRootNode*				root;
		XtArray<C_XtNode>			nodes;
		XtArray<C_XtActionId>		actionIds;
		XtArray<C_XtParameterId>	parameterIds;
		XtArray<C_XtParameterId>	fixedParameterIds;
		XtArray<C_XtIkTargetId>		ikTargetIds;
		
		bool Build(C_Tree& tree) const;
		
		static void XtCallback(XtEventType et, XtRecord* record, C_XtTree* tree, XtCallbackContext* context);
		
		static void AddListener(C_XtAnimTreeListener* listener);
		static void RemoveListener(C_XtAnimTreeListener* listener);
		
		static const C_XtTree* Find(const CHAR8* name);
				
	private:
		typedef Btl::List<C_XtAnimTreeListener> ListenerList;
		typedef Btl::Vector<C_XtTree*> TreeVector;
		static ListenerList m_listeners;
		static TreeVector m_trees;
	};

	//
	// Actions.
	//
	class C_XtActionId
	{
	public:
		XtSz8		name;
	};

	//
	// Parameters.
	//

	class C_XtParameterId
	{
	public:
		XtSz8		name;
	};

	class C_XtParameterBase
	{
	public:
		virtual C_Parameter Build(C_Tree& tree) const=0;
	};

	class C_XtParameter: public C_XtParameterBase
	{
	public:
		C_XtParameterId*	source;
		S32					feedbackParameterIndex;
		bool				enableFeedback;

		virtual C_Parameter Build(C_Tree& tree) const;
	};

	class C_XtConstantParameter: public C_XtParameterBase
	{
	public:
		F32	value;
		
		virtual C_Parameter Build(C_Tree& tree) const;
	};

	class C_XtSmoothedParameter: public C_XtParameter
	{
	public:
		F32	smoothingFactor;
		
		virtual C_Parameter Build(C_Tree& tree) const;
	};
	
	//
	// Fixed parameters.
	//
	class C_XtFixedParameterId
	{
	public:
		XtSz8		name;
	};

	//
	// Animation cycles.
	//

	enum E_XtIkConstraints
	{
		E_XtIkConstraints_None = 0,
		E_XtIkConstraints_BothHandsSteering,
		E_XtIkConstraints_RightHandSteeringLeftHandFree,
		E_XtIkConstraints_LeftHandSteeringRightHandFree,
		E_XtIkConstraints_BothHandsFree
	};

	class C_XtCycle: public ResourceUtils::C_XtCycleResourceDef
	{
	public:
		E_XtIkConstraints		ikConstraints;

	};

	class C_XtCycleGroupItem
	{
	public:
		XtHandle<C_XtCycle>		cycle;
	};

	class C_XtCycleGroup
	{
	public:
		XtArray<C_XtCycleGroupItem>	cycles;
	};


	//
	// Nodes.
	//

	class C_XtNode
	{
	public:

		virtual C_Node* Build(C_Tree& tree) const = 0;
	};

	class C_XtRootNode: public C_XtNode
	{
	public:
		virtual C_Node* Build(C_Tree& tree) const;
		
		C_XtNode*	input;
	};


	class C_XtCycleNode: public C_XtNode
	{
	public:
		C_Node* Build(C_Tree& tree) const;
		
		XtHandle<C_XtCycle>		cycle;
		F32						timeScale;
	};

	class C_XtFixedParameterOverrideNode: public C_XtNode
	{
	public:
		C_Node* Build(C_Tree& tree) const;

		C_XtFixedParameterId*	fixedParameter;
		F32						overrideValue;
		C_XtNode*				input;
	};
	
	enum E_XtSelectMode
	{
		E_XtSelectMode_Random	= 0,
		E_XtSelectMode_Sequential,
		E_XtSelectMode_AnimIndex,
		E_XtSelectMode_FixedParameter
	};
	
	class C_XtSelectNode: public C_XtNode
	{
	public:
		C_Node* Build(C_Tree& tree) const;
		
		E_XtSelectMode			selectMode;
		C_XtFixedParameterId*	fixedParameter;
		XtArray<C_XtNode*>		inputs;
	};
	

	class C_XtCycleGroupNode: public C_XtNode
	{
	public:
		C_Node* Build(C_Tree& tree) const;
		
		XtHandle<C_XtCycleGroup>	cycleGroup;
		E_XtSelectMode				selectMode;
		C_XtFixedParameterId*		fixedParameter;
	};

	//
	// State machine.
	//

	class C_XtTransition;
	class C_XtState;

	class C_XtStateMachineNode: public C_XtNode
	{
	public:
		C_Node* Build(C_Tree& tree) const;
		
		C_XtState*			initialState;
		//XtArray<C_XtState*>	states;
	};

	class C_StateMachineNode;

	class C_XtState
	{
	public:
		C_XtState(): currentStateMachine(NULL) {}
		
		XtSz8					name;		// Not necessary, but useful for debugging.
		C_XtActionId*			action;
		C_XtNode*				input;
		XtArray<C_XtTransition>	transitions;
		
		mutable					S32 stateIndex;	// State index within animation tree instance.
		mutable					C_StateMachineNode* currentStateMachine; // Used to check that state is part of current state machine.
	};

	class C_XtTransitionCondition;

	class C_XtTransition
	{
	public:
		C_XtState*							nextState;
		F32									blendTime;
		XtArray<C_XtTransitionCondition*>	conditions;
	};


	class C_StateMachineNode;

	class C_XtTransitionCondition
	{
	public:
		virtual void Build(C_Tree& tree, C_StateMachineNode::C_Builder& stateMachineBuilder) const=0;
		virtual S32 GetActionIndex(C_Tree& tree) const;

		virtual U32 GetNumRequiredConditions() const		{ return 1; }
	};

	class C_XtFinishedCondition: public C_XtTransitionCondition
	{
	public:
		virtual void Build(C_Tree& tree, C_StateMachineNode::C_Builder& stateMachineBuilder) const;

		virtual U32 GetNumRequiredConditions() const		{ return 0; }
	};

	class C_XtRequestedActionCondition: public C_XtTransitionCondition
	{
	public:
		virtual void Build(C_Tree& tree, C_StateMachineNode::C_Builder& stateMachineBuilder) const;
		virtual S32 GetActionIndex(C_Tree& tree) const;

		virtual U32 GetNumRequiredConditions() const		{ return 0; }
		
		C_XtActionId*	action;
	};

	class C_XtRangeCondition: public C_XtTransitionCondition
	{
	public:
		virtual void Build(C_Tree& tree, C_StateMachineNode::C_Builder& stateMachineBuilder) const;

		virtual U32 GetNumRequiredConditions() const		{ return 2; }

		C_XtParameterBase*	parameter;
		XtRangeF32		range;
	};

	enum E_XtOperator
	{
		E_XtOperator_EQ,
		E_XtOperator_NEQ,
		E_XtOperator_LT,
		E_XtOperator_LEQ,
		E_XtOperator_GT,
		E_XtOperator_GEQ
	};

	class C_XtComparisonCondition: public C_XtTransitionCondition
	{
	public:

		virtual void Build(C_Tree& tree, C_StateMachineNode::C_Builder& stateMachineBuilder) const;

		C_XtParameterBase*	parameter1;
		C_XtParameterBase*	parameter2;
		E_XtOperator	op;
	};
	
	//
	// Simple state machine node. 
	//
	
	class C_XtSimpleState
	{
	public:
		F32							entryBlendTime;
		F32							exitBlendTime;
		C_XtTransitionCondition*	entryCondition;
		C_XtTransitionCondition*	secondaryEntryCondition;
		C_XtTransitionCondition*	exitCondition;
		C_XtNode*					input;
	};

	class C_XtSimpleStateMachineNode: public C_XtNode
	{
	public:
		C_XtNode*					idle;
		XtArray<C_XtSimpleState>	states;
		
		virtual C_Node* Build(C_Tree& tree) const;
	};

	//
	// Blend nodes.
	//

	class C_XtLayeredBlendNode: public C_XtNode
	{
	public:
		C_Node* Build(C_Tree& tree) const;
		
		struct Layer
		{
			Animation::E_BlendMode	blendMode;
			C_XtNode*				tree;
		};
		
		C_XtNode*		baseLayer;
		XtArray<Layer>	layers;
	};

	class C_XtBlendInput
	{
	public:
		C_XtNode*	input;
		F32			parameterValue;
	};

	class C_XtBlendNode: public C_XtNode
	{
	public:
		C_Node* Build(C_Tree& tree) const;
		
		Animation::E_BlendMode	blendMode;
		bool					syncTime;
		bool					smoothStep;
		bool					useAbsoluteParameterValue;
		C_XtParameterBase*		parameter;
		XtArray<C_XtBlendInput>	blendInputs;
	};

	//
	// Time controller node.
	//

	class C_XtTimeControllerNode: public C_XtNode
	{
	public:
		C_Node* Build(C_Tree& tree) const;
		
		C_XtParameterBase*	parameter;
		XtRangeF32			parameterRange;
		XtRangeF32			timeRange;
		C_XtNode*			input;
	};

	//
	// IK nodes.
	//
	
	class C_XtIkTargetId
	{
	public:
		XtSz8		name;
	};
	
	class C_XtIkTargetBase
	{
	public:
		virtual void Build(C_Tree& tree, C_IkSetTargetNode& node)=0;
		
		C_XtIkTargetId*		ikTargetId;
		bool				setPosition;
		bool				setRotation;
	};
	
	class C_XtIkTargetFromJoint: public C_XtIkTargetBase
	{
	public:
		virtual void Build(C_Tree& tree, C_IkSetTargetNode& node);
		XtSz8				jointName;
	};

	class C_XtConstantIkTarget: public C_XtIkTargetBase
	{
	public:
		virtual void Build(C_Tree& tree, C_IkSetTargetNode& node);
		XtVec3				position;
		XtVec3				rotation;
	};

	class C_XtIkSetTargetNode: public C_XtNode
	{
	public:
		virtual C_Node* Build(C_Tree& tree) const;
		
		XtArray<C_XtIkTargetBase*>	targets;
		C_XtNode*					input;
	};

	class C_XtIkEnableNode: public C_XtNode
	{
	public:
		virtual C_Node* Build(C_Tree& tree) const;

		XtArray<C_XtIkTargetId*>	enabledIkTargets;
		XtArray<C_XtIkTargetId*>	disabledIkTargets;
		C_XtNode*					input;
	};
	
	class C_XtIkApplyNode: public C_XtNode
	{
	public:
		virtual C_Node* Build(C_Tree& tree) const;
		C_XtNode*	input;
		bool		disableIk;
	};


	//
	// Mirroring node.
	//

	class C_XtMirrorNode: public C_XtNode
	{
	public:
		C_Node* Build(C_Tree& tree) const;
		
		C_XtNode*				input;
		Animation::E_MirrorAxis	mirrorAxis;
		bool					enabled;
		bool					isGlobalMirror;
	};

	class C_XtTriggerNode: public C_XtNode
	{
	public:
		C_Node* Build(C_Tree& tree) const;

		C_XtNode*		input;
		XtDispersionF32	delay;
		F32				blendTime;
	};

} // end AnimationTree

#endif

