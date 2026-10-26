#ifndef ANIMATIONTREE_H
#define ANIMATIONTREE_H

#include <animation/cycle/CycleInstance.h>
#include <animation/common/BlendModes.h>
#include <animation/common/MemoryBlock.h>
#include <animation/common/StringId.h>
#include <animation/resource/CycleResource.h>
#include <animation/pose/PoseMirror.h>
#include <resource_utils/ResourceUtilsDeclare.h>
#include <btl/containers/vector.h>
#include <btl/containers/bitvector.h>
#include <btl/utility/string.h>
#include <btl/allocators/stackallocator.h>

namespace DbUtil
{
	class I_DebugRender;
}

namespace ResourceUtils
{
	class I_ResourceBag;
}

class C_ResourceBag;

namespace Animation
{
	class C_MemoryBlockReader;
	class C_MemoryBlockWriter;
	class C_Skeleton;
	class C_CommandList;
	class C_IkChain;
}

namespace AnimationTree
{

//
// Internal interface.
//

enum E_NodeType
{
	E_NodeType_Unknown	= 0,
	E_NodeType_Cycle,
	E_NodeType_Select,
	E_NodeType_CycleGroup,
	E_NodeType_StateMachine,
	E_NodeType_TimeController,
	E_NodeType_Blend,
	E_NodeType_LayeredBlend,
	E_NodeType_IkSetTarget,
	E_NodeType_IkEnable,
	E_NodeType_IkApply,
	E_NodeType_Mirror,
	E_NodeType_Trigger,
	E_NodeType_Count
};

class C_Node;
class C_CycleNode;
class C_SelectNode;
class C_CycleGroupNode;
class C_StateMachineNode;
class C_TimeControllerNode;
class C_BlendNode;
class C_LayeredBlendNode;
class C_IkSetTargetNode;
class C_IkEnableNode;
class C_IkApplyNode;
class C_MirrorNode;
class C_TriggerNode;


/**
An instance of an animation tree.

TODO: to save memory, move strings to context objects used during tree construction to resolve strings onto indices.

*/
class C_Tree
{
public:
	static const S32	kInvalidActionIndex		= 0;
	static const S32	kInvalidParameterIndex	= 0;


	C_Tree();
	~C_Tree();

	/// Sets the resource bag to be used for loading resources associated with this tree.
	void SetResourceBag(ResourceUtils::I_ResourceBag& resourceBag);

	/// Returns the resource bag to be used for loading resources associated with this tree.
	ResourceUtils::I_ResourceBag& GetResourceBag() const;

	/// Notifies tree nodes that the resource bag has been loaded and its contents can be safely queried. Called
	/// by the owner of the resource bag.
	void OnResourceBagLoaded();
	
	//
	// Varying parameters.
	//
	
	/// Adds a named parameter to the tree and returns its index.
	S32 AddParameter(const CHAR8* name);
	
	/// Finds a named parameter, returning its index or -1 no matching parameter found.
	S32 FindParameter(const CHAR8* name) const;
	
	/// Sets the value of a parameter.
	FORCE_INLINE void SetParameter(S32 index, F32 value)
	{
		m_parameterValues[index] = value;
	}
	
	/// Gets the current value of a parameter. If index is less than zero, then returns zero.
	FORCE_INLINE F32 GetParameter(S32 index) const
	{
		return m_parameterValues[index];
	}
	
	//
	// Fixed parameters.
	//
	
	/// Adds a named fixed parameter to the tree and returns its index.
	S32 AddFixedParameter(const CHAR8* name);
	
	/// Finds a named fixed parameter, returning its index or -1 no matching parameter found.
	S32 FindFixedParameter(const CHAR8* name) const;
	
	/// Sets the value of a fixed parameter.
	void SetFixedParameter(S32 index, F32 value);
	
	/// Gets the current value of a fixed parameter. If index is less than zero, then returns zero.
	F32 GetFixedParameter(S32 index) const;	

	/// Returns the number of fixed parameters.
	U32 GetNumFixedParameters() const;

	//
	// Feedback parameters written back by arbitrary anim tree nodes.
	//
	void SetFeedbackParameter(U32 index, F32 value);

	F32 GetFeedbackParameter(U32 index) const;

	U32 GetNumFeedbackParameters() const		{ return kMaxFeedbackParameters; }
	
	//
	// Actions.
	//
	
	/// Adds a named action to the tree and returns its index.
	S32 AddAction(const CHAR8* name);
	
	/// Finds a named action, returning its index or -1 if no matching action found.
	S32 FindAction(const CHAR8* name);
	
	/// Returns named of action.
	const CHAR8* GetActionName(S32 index) const;
	
	/// Clears all action requests.
	void ClearAllActionRequests();
	
	/// Requests an action from any active state machines.
	void RequestAction(S32 index)
	{
		m_actionRequests.SetBit(index);
		m_actionCompleted.ClearBit(index);
	}
	
	/// Returns true if the given action was requested and if index is less than zero,
	/// then returns true if any action was requested.
	bool ActionRequested(S32 index) const
	{
		return m_actionRequests.GetBitValue(index);
	}

	/// Marks an action as having been completed.
	void CompleteAction(S32 index)
	{
		m_actionCompleted.SetBit(index);
		m_actionRequests.ClearBit(index);
	}
	
	/// Returns true if an action has been completed after evaluation.
	bool IsActionComplete(S32 index) const
	{
		return m_actionCompleted.GetBitValue(index);
	}

	/// Clears the action completed flag.
	void AcknowledgeActionComplete(S32 index)
	{
		m_actionCompleted.ClearBit(index);
	}

	//
	// IK chains.
	// 
	
	U32 GetNumIkChains() const;
	
	S32 AddIkChain(const CHAR8* name, Animation::C_IkChain& ikChain);
	
	S32 FindIkChain(const CHAR8* name) const;
	
	Animation::C_IkChain* GetIkChain(S32 ikChainIndex) const;

	
	//
	// IK targets.
	//

	S32 AddIkTarget(const CHAR8* name, S32 ikChainIndex, U32 ikChainTargetIndex);
	
	S32 FindIkTarget(const CHAR8* name) const;
	
	Animation::C_IkChain* GetIkTargetChain(S32 targetId) const;

	U32 GetIkTargetIndex(S32 targetId) const;

	const CHAR8* GetIkTargetName(S32 targetId) const;

	U32 GetNumIkTargets() const;

	void ClearIkTargetWeights();
	F32 GetIkTargetWeight(S32 targetId) const;
	void BlendIkTargetWeight(S32 targetId, Animation::E_BlendMode blendMode, F32 targetWeight, F32 blendWeight);
	
	//
	// Animations.
	//
	
	/// Gets the specified animation index, used to select specified animations in cycle group nodes.
	U32 GetAnimationIndex(U32 instance) const;
	
	/// Sets the specified animation index, used to select specific animations in cycle group nodes.
	void SetAnimationIndex(U32 instance, U32 animIndex);
	
	//
	// Tree construction.
	//
	
	/// Begin constructing a tree. Must be called before any nodes are created.
	void BeginBuild();

	/// Finish constructing the tree.
	void EndBuild();

	/// Clears all parameters as well as the tree.
	void Clear();
	
	/// Clears out just the tree structure, destroying all nodes but preserving parameters.
	void ClearTree();
	
	/// Sets root of tree.
	void SetRoot(C_Node* node);
	
	/// Gets current tree root node.
	C_Node* GetRoot();
		
	/// Creates a cycle node.
	C_CycleNode* CreateCycleNode();
	
	/// Creates a select node.
	C_SelectNode* CreateSelectNode();

	/// Creates a cycle group node.
	C_CycleGroupNode* CreateCycleGroupNode();
	
	/// Creates a state machine node.
	C_StateMachineNode* CreateStateMachineNode();
	
	/// Creates a time controller node.
	C_TimeControllerNode* CreateTimeControllerNode();
	
	/// Creates a 1d blend node.
	C_BlendNode* CreateBlendNode();
	
	/// Creates a layered blend node.
	C_LayeredBlendNode* CreateLayeredBlendNode();
	
	/// Creates an IK set target node.
	C_IkSetTargetNode* CreateIkSetTargetNode();

	/// Creates an IK enable node.
	C_IkEnableNode* CreateIkEnableNode();
	
	/// Creates an IK apply node.
	C_IkApplyNode* CreateIkApplyNode();
	
	/// Creates a mirror node.
	C_MirrorNode* CreateMirrorNode();

	/// Creates a trigger node.
	C_TriggerNode* CreateTriggerNode();
	
	void SetGlobalMirrorEnabled(bool enabled);
	
	bool IsGlobalMirrorEnabled() const;
	
	//
	// Evaluation.
	//

	/// Evaluates the blend tree, generating a list of animation commands in the specified command list.
	bool Evaluate(Animation::C_CommandList& commandList, const Animation::C_Skeleton* skeleton, const Animation::C_Pose& pose, F32 deltaTime);
	
	//
	// Serialisation.
	//
	
	/// Saves animation tree state.
	void SaveState(Animation::C_MemoryBlockWriter& out) const;
	
	/// Loads animation tree state.
	bool LoadState(Animation::C_MemoryBlockReader& in);
	
	#ifndef CFG_GOLD

	enum E_DebugRenderMode
	{
		E_DebugRenderMode_ActiveTree	= 0,
		E_DebugRenderMode_CycleStats,
		E_DebugRenderMode_NodeMemoryStats
	};

	/// Renders debug information.
	void DebugRender(DbUtil::I_DebugRender* debugRender, const Maths::HVec2& pos, F32 textSize, E_DebugRenderMode mode) const;

	#endif

	//
	// Random number generation.
	// 
	void SetRandomSeed(U32 seed)	{ m_randomSeed = seed; }
	U32 Rand()		{ m_randomSeed *= 16807; return m_randomSeed; }
	F32 FRand()		{ return (Rand() & 0xfffff) / static_cast<F32>(0xfffff); }

private:
	void Destroy();
	
	template <typename T>
	T* CreateNode()
	{
		T* n = m_nodeAllocator.Allocate<T>();
		m_nodes.PushBack(n);
		m_nodeMemoryUsed += sizeof(T);
		return n;
	}
	
	static const U32 kMaxAnimIndices = 1;
	static const U32 kMaxFeedbackParameters = 2;
	
	struct IkChain
	{
		Animation::C_IkChain*	ikChain;
		Btl::String				name;
	};
	
	struct IkTarget
	{
		S32				ikChainIndex;
		U32				targetIndex;
		F32				weight;
		F32				totalBlendWeight;
		Btl::String		name;
	};

	struct Parameter
	{
		Parameter() {}
		Parameter(const CHAR8* inName, F32 inValue): name(inName), value(inValue) {}

		Btl::String	name;
		F32			value;
	};

	static const U32	kMaxParameters		= 20;
	static const U32	kMaxFixedParameters	= 4;
	static const U32	kMaxIkTargets		= 8;
	static const U32	kMaxIkChains		= 4;
	static const U32	kMaxActions			= 24;

	Btl::Vector<C_Node*>			m_nodes;
	C_Node*							m_root;

	Btl::Vector<F32, Btl::VectorParameters_Static<kMaxParameters> >				m_parameterValues;
	Btl::Vector<Btl::String>													m_parameterNames;
	Btl::Vector<Parameter, Btl::VectorParameters_Static<kMaxFixedParameters> >	m_fixedParameters;

	Btl::Vector<Btl::String>		m_actionNames;

	typedef Btl::BitVectorTemplate<Btl::BitVectorBase<Btl::Vector<U8, Btl::VectorParameters_Static<kMaxActions> > > > ActionBitVector;

	ActionBitVector					m_actionRequests;
	ActionBitVector					m_actionCompleted;

	Btl::Vector<IkTarget, Btl::VectorParameters_Static<kMaxIkTargets> >	m_ikTargets;
	Btl::Vector<IkChain, Btl::VectorParameters_Static<kMaxIkChains> >	m_ikChains;
	
	Btl::DynamicStackAllocator		m_nodeAllocator;
	U32								m_nodeMemoryUsed;
	U32								m_randomSeed;
	ResourceUtils::I_ResourceBag*	m_pResourceBag;
	U32								m_animIndices[kMaxAnimIndices];
	F32								m_feedbackParameters[kMaxFeedbackParameters];
	bool							m_isGlobalMirrorEnabled;
};

class C_Context
{
public:
	C_Context(C_Tree& inTree,
			Animation::C_CommandList& inCommandList,
			const Animation::C_Skeleton* inSkeleton,
			const Animation::C_Pose& inPose,
			U32 inUpdateCounter,
			F32 inDeltaTime):
		tree(inTree),
		commandList(inCommandList),
		skeleton(inSkeleton),
		pose(inPose),
		updateCounter(inUpdateCounter),
		deltaTime(inDeltaTime)
	{
	}
	
	C_Tree&						tree;
	const Animation::C_Pose&	pose;
	Animation::C_CommandList&	commandList;
	const Animation::C_Skeleton* const	skeleton;
	const U32					updateCounter;
	const F32					deltaTime;
private:
};

class C_UpdateCounter
{
public:
	C_UpdateCounter():
		m_counter(0)
	{
	}
	
	void Reset()
	{
		m_counter = 0;
	}
	
	void Set(U32 updateCounter)
	{
		m_counter = updateCounter;
	}
	
	/// Returns true if counter updated. Activated is set to true
	/// if there wasn't an update in the previous frame.
	bool Update(const C_Context& context, bool* pActivated = NULL)
	{
		bool notUpdated = (m_counter != context.updateCounter);
		if (pActivated != NULL)
		{
			bool notUpdatedLastFrame = (m_counter != (context.updateCounter - 1));
			*pActivated = notUpdatedLastFrame;
		}
		m_counter = context.updateCounter;
		return notUpdated;
	}
	
	void LoadState(Animation::C_MemoryBlockReader& in);
	void SaveState(Animation::C_MemoryBlockWriter& out) const;
	
private:
	U32		m_counter;
};

/**

*/
class C_Parameter
{
public:
	C_Parameter():
		m_pGetValueFunc(&C_Parameter::GetConstantParameter),
		m_value(0.f),
		m_smoothingFactor(1.f),
		m_index(C_Tree::kInvalidParameterIndex),
		m_feedbackParameterIndex(-1)
	{}
	
	C_Parameter(F32 value):
		m_pGetValueFunc(&C_Parameter::GetConstantParameter),
		m_value(value),
		m_index(C_Tree::kInvalidParameterIndex),
		m_feedbackParameterIndex(-1)
	{
	}
	
	C_Parameter(const C_Tree& tree, const CHAR8* name, F32 smoothingFactor = 1.f, S32 feedbackParameterIndex = -1):
		m_pGetValueFunc(&C_Parameter::GetVaryingParameter),
		m_value(0.f),
		m_smoothingFactor(smoothingFactor),
		m_feedbackParameterIndex(feedbackParameterIndex)
	{
		m_index = tree.FindParameter(name);
	}

	inline F32 GetValue(C_Context& context)
	{
		return (this->*m_pGetValueFunc)(context);
	}

	bool IsConstant() const					{ return m_pGetValueFunc == &C_Parameter::GetConstantParameter; }
	F32 GetConstantValue() const			{ return IsConstant() ? m_value : 0.f; }

	bool FeedbackEnabled() const			{ return m_feedbackParameterIndex >= 0; }
	U32 GetFeedbackParameterIndex() const	{ return m_feedbackParameterIndex; }
	
	void SaveState(Animation::C_MemoryBlockWriter& out) const;
	void LoadState(Animation::C_MemoryBlockReader& in);

private:

	F32 GetConstantParameter(C_Context& context)
	{
		return m_value;
	}

	F32 GetVaryingParameter(C_Context& context)
	{
		F32 value = m_value;
		bool activated = false;
		if (m_updateCounter.Update(context, &activated))
		{
			F32 currentValue = context.tree.GetParameter(m_index);
			F32 smoothingFactor = activated ? 1.f : m_smoothingFactor;
			value += (currentValue - value) * smoothingFactor;
		
			if (m_feedbackParameterIndex >= 0)
			{
				context.tree.SetFeedbackParameter(m_feedbackParameterIndex, value);
			}
			m_value = value;
		}
		return value;
	}

	F32 (C_Parameter::*m_pGetValueFunc)(C_Context&);
	C_UpdateCounter	m_updateCounter;
	F32				m_value;
	F32				m_smoothingFactor;
	S16				m_index;
	S16				m_feedbackParameterIndex;
};

class C_CyclePlayer
{
public:
	C_CyclePlayer();
	
	//void Initialise(const Animation::C_CycleResource* cycleResource, F32 timeScale, Animation::E_LoopMode loopMode);

	void SetTimeScale(F32 timeScale);
	void SetLoopMode(Animation::E_LoopMode loopMode);
	void SetCycleResource(const Animation::C_CycleResource* pCycleResource);

	void SetPhase(const C_Context& context, F32 phase);
	
	// Returns true if activated.
	void Update(C_Context& context);
	
	void Evaluate(Animation::C_CommandList& commandList, const Animation::C_Pose& pose, Animation::E_BlendMode blendMode, F32 weight);
	
	F32 GetPhase() const				{ return m_phase / 65535.f; }
	F32 GetFrequency() const			{ return m_frequency / 511.f; }
	bool Activated() const				{ return m_activated; }
	
	const Animation::C_StringId GetCycleName() const;
	const Animation::C_Cycle* GetCycle() const;
	
	void SaveState(Animation::C_MemoryBlockWriter& out) const;
	void LoadState(Animation::C_MemoryBlockReader& in);

	
private:
	bool AcquireCycle();

	Animation::C_CycleInstance			m_cycleInstance;
	C_UpdateCounter						m_updateCounter;
	const Animation::C_CycleResource*	m_cycleResource;
	U16									m_phase;			// 0-65535
	S16									m_frequency;		// 1:6:9? 64Hz maximum, 0.002 minimum. 512seconds duration
	S16									m_timeScale;		// Packed 1:3:10 fixed point, with sign.
	U16									m_loopMode : 4;
	U16									m_activated : 1;
};

class C_DebugRenderContext;

class C_Visitor
{
public:
	virtual ~C_Visitor() {}

	virtual bool Visit(C_CycleNode& cycleNode)					{ return true; }
	virtual bool Visit(C_SelectNode& selectNode)				{ return true; }
	virtual bool Visit(C_CycleGroupNode& cycleGroupNode)		{ return true; }
	virtual bool Visit(C_StateMachineNode& stateMachineNode)	{ return true; }
	virtual bool Visit(C_LayeredBlendNode& layeredBlendNode)	{ return true; }
	virtual bool Visit(C_BlendNode& blendNode)					{ return true; }
	virtual bool Visit(C_TimeControllerNode& timeControllerNode){ return true; }
	virtual bool Visit(C_IkSetTargetNode& ikSetTargetNode)		{ return true; }
	virtual bool Visit(C_IkEnableNode& ikEnableNode)			{ return true; }
	virtual bool Visit(C_IkApplyNode& ikApplyNode)				{ return true; }
	virtual bool Visit(C_MirrorNode& ikApplyNode)				{ return true; }
	virtual bool Visit(C_TriggerNode& triggerNode)				{ return true; }
};

/**
Simple helper class that sets IK weights for a selection of IK targets.
*/
class C_IkConstraints
{
public:
	C_IkConstraints();

	void Reset();
	void SetIkTargetEnabled(const CHAR8* name, const C_Tree& animTree, bool enabled);
	void EnableIkTarget(S32 ikTargetId);
	void DisableIkTarget(S32 ikTargetId);
	void Apply(C_Tree& tree, Animation::E_BlendMode blendMode, F32 blendWeight);

	bool IsIkTargetEnabled(S32 ikTargetId) const	{ return 0 != ((m_flags & m_mask) & (1 << ikTargetId)); }
private:
	U16		m_flags;
	U16		m_mask;
};


/**
Base interface for animation tree nodes.
*/
class C_Node
{
public:
	virtual ~C_Node() {}
	
	/// Sets the phase (normalised time) of the node.
	virtual void SetPhase(C_Context& context, F32 phase) {}
	
	/// Gets the frequency (cycles per second or one over duration) of the node.
	virtual F32 GetFrequency(C_Context& context) { return 0.f; }
	
	/// Evaluates the node, updating the command list in the context object.
	virtual void Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight) {}

	/// Saves node state.
	virtual void SaveState(Animation::C_MemoryBlockWriter& out) const {}
	
	/// Loads node state.
	virtual void LoadState(Animation::C_MemoryBlockReader& in) {}
	
#ifndef CFG_GOLD
	/// Renders debug information.
	virtual void DebugRender(C_DebugRenderContext& context) const;
#endif
	
	/// Returns the type of the node.
	virtual E_NodeType GetType() const		{ return E_NodeType_Unknown; }

	/// Accepts a visitor.
	virtual bool Accept(C_Visitor& visitor)	{ return true; }
};

/**
Cycle playback node.
*/
class C_CycleNode: public C_Node
{
public:
	C_CycleNode();

	/// Initialises the cycle node to use the specified cycle and loop mode.
	void Create(ResourceUtils::I_ResourceBag& resourceBag, const Util::C_String& cycleFilename, Animation::E_LoopMode loopMode, F32 timeScale = 1.f);

	/// Gets loaded cycle resource out of the resource bag.
	void OnResourceBagLoaded(ResourceUtils::I_ResourceBag& resourceBag);

	/// Sets IK constraints.
	void SetIkConstraints(const C_IkConstraints& ikConstraints)	{ m_ikConstraints = ikConstraints; }

	/// Sets the phase (normalised time) of the node.
	virtual void SetPhase(C_Context& context, F32 phase);
	
	/// Gets the frequency (cycles per second or one over duration) of the node.
	virtual F32 GetFrequency(C_Context& context);
	
	/// Evaluates the cycle node, generating blend cycle operations in the command list.
	virtual void Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight);
	
	virtual void SaveState(Animation::C_MemoryBlockWriter& out) const;
	virtual void LoadState(Animation::C_MemoryBlockReader& in);
	
#ifndef CFG_GOLD
	/// Renders debug information.
	virtual void DebugRender(C_DebugRenderContext& context) const;
#endif

	virtual E_NodeType GetType() const		{ return E_NodeType_Cycle; }

	virtual bool Accept(C_Visitor& visitor);

	const Animation::C_Cycle* GetCycle() const;

private:
	ResourceUtils::C_ResourceBagTag		m_cycleTag;
	C_CyclePlayer						m_cyclePlayer;
	C_IkConstraints						m_ikConstraints;
};

/**
Selection node, which picks one of its inputs according to the (integer) value of its input parameter.
*/
class C_SelectNode: public C_Node
{
public:
	enum E_SelectionType
	{
		E_SelectionType_Random	= 0,
		E_SelectionType_Sequential,
		E_SelectionType_AnimIndex,
		E_SelectionType_Count
	};
	
	C_SelectNode();
	
	void SetNumInputsHint(U32 numInputs);

	void AddInput(C_Node* input);

	/// Sets the animation selection method.
	void SetSelectionType(E_SelectionType select);

	/// Sets the phase (normalised time) of the node.
	virtual void SetPhase(C_Context& context, F32 phase);
	
	/// Gets the frequency (cycles per second or one over duration) of the node.
	virtual F32 GetFrequency(C_Context& context);
	
	/// Evaluates the cycle node, generating blend cycle operations in the command list.
	virtual void Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight);
	
	virtual void SaveState(Animation::C_MemoryBlockWriter& out) const;
	virtual void LoadState(Animation::C_MemoryBlockReader& in);
	
#ifndef CFG_GOLD
	/// Renders debug information.
	virtual void DebugRender(C_DebugRenderContext& context) const;
#endif

	virtual E_NodeType GetType() const		{ return E_NodeType_Select; }

	virtual bool Accept(C_Visitor& visitor);

private:
	void Update(C_Context& context);
	
	C_UpdateCounter			m_updateCounter;
	Btl::Vector<C_Node*>	m_inputs;
	S32						m_inputIndex;
	E_SelectionType			m_selectionType;
};


/**
Cycle group node.

On construction of the graph, the constant attributes should be used to
whittle down the cycles in the group into a minimal set. If the set has
only one animation, then a CycleNode should be created instead as it is
functionally identical. If there is more than possibility, then an
animation is picked at random, sequentially or possibly via an input variable.

Not sure if we should actually create a set of N cyclenodes?
Ideally create resource handles for each cycle node.

Should use N cycle resource handles, rather than 1.

TODO very similar to cycle node, share code?

When activated, 
*/
class C_CycleGroupNode: public C_Node
{
public:

	enum E_SelectionType
	{
		E_SelectionType_Random	= 0,
		E_SelectionType_Sequential,
		E_SelectionType_AnimIndex,
		E_SelectionType_Count
	};

	C_CycleGroupNode();

	void AddCycle(ResourceUtils::I_ResourceBag& resourceBag, const Util::C_String& cycleFilename, const C_IkConstraints& ikConstraints, F32 timeScale = 1.f, Animation::E_LoopMode loopMode = Animation::E_LoopMode_Continuous);

	U32 GetNumCycles() const;

	/// Gets loaded cycle resource out of the resource bag.
	void OnResourceBagLoaded(ResourceUtils::I_ResourceBag& resourceBag);

	const Animation::C_Cycle* GetCycle(U32 index) const;

	/// Sets the animation selection method.
	void SetSelectionType(E_SelectionType select);

	/// Sets the phase (normalised time) of the node.
	virtual void SetPhase(C_Context& context, F32 phase);
	
	/// Gets the frequency (cycles per second or one over duration) of the node.
	virtual F32 GetFrequency(C_Context& context);
	
	/// Evaluates the cycle node, generating blend cycle operations in the command list.
	virtual void Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight);
	
	virtual void SaveState(Animation::C_MemoryBlockWriter& out) const;
	virtual void LoadState(Animation::C_MemoryBlockReader& in);
	
#ifndef CFG_GOLD
	/// Renders debug information.
	virtual void DebugRender(C_DebugRenderContext& context) const;
#endif

	virtual E_NodeType GetType() const		{ return E_NodeType_CycleGroup; }

	virtual bool Accept(C_Visitor& visitor);

private:
	struct CycleItem
	{
		CycleItem(): pResource(0) {}

		ResourceUtils::C_ResourceBagTag	tag;
		Animation::C_CycleResource*		pResource;
		F32								timeScale;
		C_IkConstraints					ikConstraints;
		Animation::E_LoopMode			loopMode;
	};
	
	C_CyclePlayer			m_cyclePlayer;
	C_IkConstraints			m_ikConstraints;
	Btl::Vector<CycleItem>	m_cycles;
	S32						m_currentCycleIndex;
	E_SelectionType			m_selectionType;
};

/**
Finite state machine node.

Want to include parameter conditions together with "events"
e.g. pick animations depending on parameters, broadly initiated by action

actions aka transition names, events
event name - if blank it's automatic i.e. animation played for one full cycle
condition - optional comparison between parameter and constant, or another parameter.

// Requested actions stay active until a transition for that action is completed.
// that doesn't work: an action may be implemented by multiple states.
// associate states with actions? e.g. state "completes" these actions.

animTree.RequestAction(actionIndex);

Conditions are:
	- Child node has played for one full cycle.
	- Parameter comparison with a constant or another parameter.
	- An "event" or "action" has been requested.

*/
class C_StateMachineNode: public C_Node
{
private:
	struct Condition;
	struct Transition;
	struct State;

public:
	enum E_Operator
	{
		E_Operator_EQ,
		E_Operator_NEQ,
		E_Operator_LT,
		E_Operator_LEQ,
		E_Operator_GT,
		E_Operator_GEQ,
		E_Operator_Count
	};


	class C_Builder
	{
	public:
		friend class C_StateMachineNode;

		C_Builder(C_StateMachineNode& stateMachine);

		void Reset();

		void SetMaxStates(U32 maxStates);

		S32 AddState(C_Node* input, S32 actionIndex, U32 numTransitions);
		void AddTransition(S32 fromState, S32 toState, U32 transitionIndex, F32 blendTime, U32 numConditions);
		void AddAnimationFinishedCondition();
		void AddRequestedActionCondition(S32 actionIndex);
		void AddComparisonCondition(const C_Parameter& parameter1, E_Operator op, F32 parameter2);

	private:
		C_StateMachineNode::Condition* GetCurrentCondition();
		C_StateMachineNode::Condition* NextCondition();

		C_StateMachineNode&				m_stateMachine;
		Animation::C_MemoryBlockSizer	m_sizer;
		Animation::C_RelativePtr<C_StateMachineNode::Transition>	m_pCurrentTransition;
		S32		m_currentConditionIndex;
		S32		m_currentStateIndex;
		S32		m_maxStates;
	};

	friend class C_Builder;

	C_StateMachineNode();
	
	/// Removes all states, transitions and conditions. Clears initial state index.
	void Clear();

	/// Sets the initial state.
	void SetInitialState(S32 stateIndex);
	
	/// Sets the phase (normalised time) of the current state.
	virtual void SetPhase(C_Context& context, F32 phase);	

	/// Evaluates the state machine.
	virtual void Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight);
	
	virtual void SaveState(Animation::C_MemoryBlockWriter& out) const;
	virtual void LoadState(Animation::C_MemoryBlockReader& in);

#ifndef CFG_GOLD
	/// Renders debug information.
	virtual void DebugRender(C_DebugRenderContext& context) const;
#endif

	virtual E_NodeType GetType() const		{ return E_NodeType_StateMachine; }

	virtual bool Accept(C_Visitor& visitor);

private:
	enum E_ConditionType
	{
		E_ConditionType_AnimationFinished	= 0,	// Current animation has finished playing.		[0]
		E_ConditionType_RequestedAction,			// A particular action has been requested.		[actionIndex]
		E_ConditionType_Comparison,					// Comparison between two parameters is true.	[parameter1, parameter2]
		E_ConditionType_Count,
		E_ConditionType_Invalid						// Invalid, always false.
	};

	struct Condition
	{
		Condition(): pTestFunc(&Condition::TestFalse) {}
			
		bool				IsValid() const		{ return pTestFunc != &Condition::TestFalse; }
		bool				IsInvalid() const	{ return pTestFunc == &Condition::TestFalse; }
		
		inline bool			Evaluate(C_Context& context)
		{
			return (this->*pTestFunc)(context);
		}

		bool TestEQ(C_Context& context);
		bool TestNEQ(C_Context& context);
		bool TestLT(C_Context& context);
		bool TestLEQ(C_Context& context);
		bool TestGT(C_Context& context);
		bool TestGEQ(C_Context& context);
		bool TestFalse(C_Context& context);

		// 24 bytes
		bool (Condition::*pTestFunc)(C_Context&);
		C_Parameter			parameter1;		// store floats for range comparison?
		F32					parameter2;
	};

	struct Transition
	{
		F32			blendTime;							// Duration of blend into the next state.
		Animation::C_RelativePtr<Condition>	pConditions;
		U16			numConditions				: 4;
		U16			animationFinishedCondition	: 1;	// Set if there is an animation finished condition.
		U16			requestedActionCondition	: 1;	// Set if there is a requested action condition (limited to 1).
		S16			actionIndex					: 10;
		S8			nextState;							// Index of next state.
	};

	struct State
	{
		C_Node*						input;				// Sub-tree used for this state.
		S32							actionIndex;		// Action that this state completes.
		Animation::C_RelativePtr<Transition>	pFirstTransition;
		U16							numTransitions					: 15;
		U16							hasAnimationFinishedCondition	: 1;
	};

private:

	const State& GetState(S32 stateIndex) const;
	State& GetState(S32 stateIndex);
	void Update(C_Context& context);
	void UpdateDuringTransition(C_Context& context);
	void CheckForTransition(C_Context& context);

	enum
	{
		kMaxStates = 8
	};

	Animation::C_MemoryBlock		m_data;
	C_UpdateCounter					m_currentStateUpdate;
	F32								m_blendTime;
	F32								m_currentStateTime;
	F32								m_nextStateTime;
	Animation::C_RelativePtr<U16>	m_pStateOffsets;
	U16								m_numStates;
	S8								m_currentStateIndex;
	S8								m_nextStateIndex;
	S8								m_initialStateIndex;
	
	
};

/**
Layered blend node.
*/
class C_LayeredBlendNode: public C_Node
{
public:
	C_LayeredBlendNode();
	
	/// Clears base layer and all other layers.
	void Clear();
		
	/// Sets the base layer.
	void SetBaseLayer(C_Node* baseTree);
	
	/// Adds an additional layer above the base layer.
	S32 AddLayer(C_Node* layerTree, Animation::E_BlendMode blendMode);
	
	/// Sets the phase (normalised time) of the base layer. Other layers are unaffected.
	virtual void SetPhase(C_Context& context, F32 phase);

	/// Gets the frequency (cycles per second or one over duration) of the node.
	virtual F32 GetFrequency(C_Context& context);
	
	/// Evaluates the node, evaluating each layer in turn from the base layer onwards.
	virtual void Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight);

#ifndef CFG_GOLD
	/// Renders debug information.
	virtual void DebugRender(C_DebugRenderContext& context) const;
#endif
	
	virtual E_NodeType GetType() const		{ return E_NodeType_LayeredBlend; }

	virtual bool Accept(C_Visitor& visitor);
		
private:
	enum
	{
		kMaxLayers = 8
	};
	struct Layer
	{
		C_Node*					childTree;
		Animation::E_BlendMode	blendMode;
	};
	
	typedef Btl::Vector<Layer, Btl::VectorParameters_Static<kMaxLayers> > LayerVector;
	
	C_Node*			m_baseLayer;
	LayerVector		m_layers;
};

/**
One dimensional blend node.
*/
class C_BlendNode: public C_Node
{
public:
	enum E_Flag
	{
		E_Flag_SyncTime		= 1 << 0,
		E_Flag_SmoothStep	= 1 << 1,	// Smooth step rather than linear steps between input values.
		E_Flag_Absolute		= 1 << 2,	// Use absolute parameter value (i.e. ignore sign)
	};

	C_BlendNode();
	
	void Clear();
	
	void SetBlendMode(Animation::E_BlendMode blendMode);
	
	void SetFlags(U32 flags);
	
	void SetParameter(const C_Parameter& parameter);
	
	void AddInput(C_Node* input, F32 parameterValue);
	
		
	/// Sets the phase of all child sub trees, implicitly synchronising their playback.
	virtual void SetPhase(C_Context& context, F32 phase);

	/// Gets the frequency (cycles per second or one over duration) of the node.
	virtual F32 GetFrequency(C_Context& context);
	
	/// Evaluates the node, evaluating each layer in turn from the base layer onwards.
	virtual void Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight);
	
	virtual void SaveState(Animation::C_MemoryBlockWriter& out) const;
	virtual void LoadState(Animation::C_MemoryBlockReader& in);

#ifndef CFG_GOLD
	/// Renders debug information.
	virtual void DebugRender(C_DebugRenderContext& context) const;
#endif

	virtual E_NodeType GetType() const		{ return E_NodeType_Blend; }

	virtual bool Accept(C_Visitor& visitor);

private:

	void UpdateActiveInputs(C_Context& context);

	enum
	{
		kMaxInputs = 4
	};
	
	struct BlendInput
	{
		C_Node*		input;
		F32			parameterValue;
		
		bool operator < (const BlendInput& rhs) const
		{
			return parameterValue < rhs.parameterValue;
		}
	};
	
	typedef Btl::Vector<BlendInput, Btl::VectorParameters_Static<kMaxInputs> > BlendInputVector;

	C_Parameter				m_parameter;
	C_UpdateCounter			m_activeInputUpdate;
	C_Node*					m_activeInputs[2];
	F32						m_activeWeights[2];
	U32						m_flags;
	BlendInputVector		m_inputs;
	C_UpdateCounter			m_phaseUpdate;
	F32						m_phase;
	Animation::E_BlendMode	m_blendMode;
};


/**
Node that controls the time of its children directly.
*/
class C_TimeControllerNode: public C_Node
{
public:
	C_TimeControllerNode();
	
	/// Evaluates the node, setting the time of its children before evaluating them.
	virtual void Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight);
	
	/// Sets the input node whose time (actually phase) will be controlled.
	void SetInput(C_Node* input);
	
	/// Sets the parameter that drives the mapping.
	void SetParameter(const C_Parameter& parameter);
	
	/// Configures the mapping from the input parameter onto time (normalised i.e. phase).
	void SetMapping(F32 parameterMin, F32 parameterMax, F32 timeMin, F32 timeMax);
	
	/// Saves state.
	virtual void SaveState(Animation::C_MemoryBlockWriter& out) const;
	
	/// Loads state.
	virtual void LoadState(Animation::C_MemoryBlockReader& in);
	
#ifndef CFG_GOLD
	/// Renders debug information.
	virtual void DebugRender(C_DebugRenderContext& context) const;
#endif

	virtual E_NodeType GetType() const		{ return E_NodeType_TimeController; }

	virtual bool Accept(C_Visitor& visitor);
			
private:
	C_Node*		m_input;
	C_Parameter	m_parameter;
	F32			m_parameterMin;
	F32			m_parameterMax;
	F32			m_timeMin;
	F32			m_timeMax;
};

/*
	Clear IK targets


	Set IK Target
		Accumulates targets.
				totalWeight += weight
				target = (target * weight) / totalWeight
		
	Set IK Weights
		Target ID
		Weight

		ik_weight = new_weight

		totalWeightWeight += weight

		
*/

/**
Node that sets up IK targets from its input pose.
(Push/Evaluate child/Sample IK targets/Pop)
*/
class C_IkSetTargetNode: public C_Node
{
public:
	C_IkSetTargetNode();

	virtual void SetPhase(C_Context& context, F32 phase);
	virtual F32 GetFrequency(C_Context& context);
	virtual void SaveState(Animation::C_MemoryBlockWriter& out) const;
	virtual void LoadState(Animation::C_MemoryBlockReader& in);

	virtual void Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight);

	virtual E_NodeType GetType() const		{ return E_NodeType_IkSetTarget; }

	virtual bool Accept(C_Visitor& visitor);

#ifndef CFG_GOLD
	/// Renders debug information.
	virtual void DebugRender(C_DebugRenderContext& context) const;
#endif

	void SetInput(C_Node* input);
	void AddTarget(S32 ikTargetId, const Animation::C_StringId& jointNameId);

private:

	struct Target
	{
		Target(): ikTargetId(-1), jointIndex(-1) {}
		
		S32						ikTargetId;
		Animation::C_StringId	jointNameId;
		S16						jointIndex;
	};

	C_Node*				m_child;
	Btl::Vector<Target, Btl::VectorParameters_Static<8> >	m_targets;
	const Animation::C_Skeleton* m_boundSkeleton;
};

/**
Sets the weights of one or more IK targets.
Weight not important - more activation/deactivation?
Set of flags.

Enabled IK targets
Disabled IK targets

*/
class C_IkEnableNode: public C_Node
{
public:
	C_IkEnableNode();

	virtual void SetPhase(C_Context& context, F32 phase);
	virtual F32 GetFrequency(C_Context& context);
	virtual void SaveState(Animation::C_MemoryBlockWriter& out) const;
	virtual void LoadState(Animation::C_MemoryBlockReader& in);
	
	/// Evaluates child node and gathers IK targets.
	virtual void Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight);

	virtual E_NodeType GetType() const		{ return E_NodeType_IkEnable; }

#ifndef CFG_GOLD
	/// Renders debug information.
	virtual void DebugRender(C_DebugRenderContext& context) const;
#endif

	virtual bool Accept(C_Visitor& visitor);
	
	void SetInput(C_Node* input);
	void EnableIkTarget(S32 ikTargetId);
	void DisableIkTarget(S32 ikTargetId);

private:
	C_Node*			m_input;
	C_IkConstraints	m_constraints;
};


/**
Node that evaluates all IK chains in the animation tree.
*/
class C_IkApplyNode: public C_Node
{
public:
	C_IkApplyNode();

	virtual void SetPhase(C_Context& context, F32 phase);
	virtual F32 GetFrequency(C_Context& context);
	virtual void SaveState(Animation::C_MemoryBlockWriter& out) const;
	virtual void LoadState(Animation::C_MemoryBlockReader& in);

	virtual void Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight);
	
	void SetInput(C_Node* input);

#ifndef CFG_GOLD
	/// Renders debug information.
	virtual void DebugRender(C_DebugRenderContext& context) const;
#endif

	virtual E_NodeType GetType() const		{ return E_NodeType_IkApply; }

	virtual bool Accept(C_Visitor& visitor);

private:
	C_Node*	m_input;
};

/**
Mirrors a pose.
*/
class C_MirrorNode: public C_Node
{
public:
	C_MirrorNode();
	
	void Create(C_Node* input, Animation::E_MirrorAxis mirrorAxis);
		
	virtual void Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight);

#ifndef CFG_GOLD
	/// Renders debug information.
	virtual void DebugRender(C_DebugRenderContext& context) const;
#endif

	virtual E_NodeType GetType() const		{ return E_NodeType_Mirror; }

	virtual bool Accept(C_Visitor& visitor);

private:
	Animation::C_PoseMirror			m_poseMirror;
	const Animation::C_Skeleton*	m_mirroredSkeleton;
	C_Node*							m_input;
};

/**
Triggers one cycle of its input periodically.
*/
class C_TriggerNode: public C_Node
{
public:
	C_TriggerNode();
	
	void Create(C_Node* input, F32 delayMean, F32 delayVariation, F32 blendTime);

	void Reset(F32 unitVariation);
		
	virtual void Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight);

	virtual E_NodeType GetType() const		{ return E_NodeType_Trigger; }

	virtual bool Accept(C_Visitor& visitor);

private:
	enum E_State
	{
		E_State_Waiting	= 0,
		E_State_BlendingIn,
		E_State_Playing,
		E_State_BlendingOut
	};

	C_UpdateCounter	m_updateCounter;
	F32				m_delay;
	F32				m_variation;
	F32				m_timer;
	F32				m_blendTime;
	C_Node* 		m_input;
	E_State			m_state;
};

} // end AnimationTree

#endif

