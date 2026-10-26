#include "stdafx.h"
#include "AnimationTree.h"
#include <animation/common/MemoryBlock.h>
#include <animation/deferred/CommandList.h>
#include <animation/common/StringId.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/ik/IkChain.h>
#include <system/debug/I_DebugRender.h>
#include <animation/cycle/Cycle.h>
#include <resource_utils/I_ResourceBag.h>
#include <resource_utils/ResourceBagUtils.h>

namespace AnimationTree
{

static const char* kNodeTypeNames[] =
{
	"Unknown",
	"Cycle",
	"Select",
	"CycleGroup",
	"StateMachine",
	"TimeController",
	"Blend",
	"LayeredBlend",
	"IkSetTarget",
	"IkEnable",
	"IkApply",
	"Mirror",
	"Trigger"
};	
BIZ_STATIC_ASSERT((sizeof(kNodeTypeNames) / sizeof(kNodeTypeNames[0])) == E_NodeType_Count);


//
// C_DebugRenderContext
//

class C_DebugRenderContext
{
public:
	C_DebugRenderContext(DbUtil::I_DebugRender* debugRender, const C_Tree& tree, const Maths::HVec2& pos, F32 textSize):
		m_startPos(pos),
		m_colour(255, 255, 255, 255),
		m_debugRender(debugRender),
		m_tree(tree),
		m_textSize(textSize),
		m_numLines(0),
		m_depth(0)
	{
	}

	
	void Begin()
	{
		++m_depth;
	}
	
	void End()
	{
		--m_depth;
	}
	
	void PrintLine(const Util::C_String& str)
	{
		m_debugRender->Draw2dText(GetPos(), str, m_colour, m_textSize);
		m_numLines++;
	}
	
	Maths::HVec2 GetPos() const
	{
		return m_startPos + Maths::HVecY(m_numLines * m_textSize * 1.1f) + Maths::HVecX(m_depth * 40.f);
	}
	
	const C_Tree& Tree() const
	{
		return m_tree;
	}
	
private:
	const Maths::HVec2		m_startPos;
	Maths::RGBA				m_colour;
	DbUtil::I_DebugRender*	m_debugRender;
	const C_Tree&			m_tree;
	F32						m_textSize;
	U32						m_numLines;
	U32						m_depth;
};

//
// Collects and displays sizes of all cycles used by an animation tree.
//
class C_CycleStatsVisitor: public C_Visitor
{
public:
	C_CycleStatsVisitor()
	{
	}

	void Begin()
	{
		m_cycles.Clear();
		m_cycles.Reserve(150);
		m_totalSize = 0;
	}

	void End()
	{
		// Sort by size.
		SortByDescSize();
	}

	virtual bool Visit(C_CycleNode& cycleNode)
	{
		const Animation::C_Cycle* cycle = cycleNode.GetCycle();
		if (cycle)
		{
			m_cycles.PushBack(CycleInfo(cycle));
			m_totalSize += m_cycles.Back().size;
		}
		return true;
	}

	virtual bool Visit(C_CycleGroupNode& cycleGroupNode)
	{
		for (U32 i = 0, count = cycleGroupNode.GetNumCycles(); i < count; ++i)
		{
			const Animation::C_Cycle* cycle = cycleGroupNode.GetCycle(i);
			if (cycle)
			{
				m_cycles.PushBack(CycleInfo(cycle));
				m_totalSize += m_cycles.Back().size;
			}
		}
		return true;
	}

	void SortByDescSize()
	{
		bool swapped = false;
		do
		{
			swapped = false;
			for (S32 i = 0, count = m_cycles.Size() - 1; i < count; ++i)
			{
				if (m_cycles[i].size < m_cycles[i+1].size)
				{
					Btl::Swap(m_cycles[i], m_cycles[i+1]);
					swapped = true;
				}
			}
		} while (swapped);
	}

	void DebugRender(DbUtil::I_DebugRender* debugRender, const Maths::HVec2& pos, F32 textSize)
	{
		Util::C_String str;
		Maths::HVec2 p = pos;
		Maths::HVec2 colStart = pos;
		Maths::RGBA textColour(0xff, 0xff, 0xff, 0xff);
		Maths::RGBA largeColour(0xa0, 0x20, 0x00, 0xff);
		Maths::RGBA medColour(0x80, 0x80, 0x00, 0xff);
		Maths::RGBA smallColour(0x00, 0x80, 0x00, 0xff);
		//Maths::RGBA largeColour(0xf0, 0x40, 0x40, 0xff);
		//Maths::RGBA medColour(0xf0, 0xf0, 0x00, 0xff);
		//Maths::RGBA smallColour(0x00, 0xf0, 0x00, 0xff);
		F32 colWidth = 300.f;
		const U32 kSmallSize = 8 * 1024;
		const U32 kMediumSize = 20 * 1024;

		debugRender->Draw2dText(p, str.Printf("Total %d cycles, %.1fK", m_cycles.Size(), m_totalSize / 1024.f), textColour, textSize);
		p += Maths::HVecY(textSize);

		U32 sizeSum = 0;

		for (U32 i = 0, count = m_cycles.Size(); i < count; ++i)
		{
			Maths::RGBA bgColour;
			if (m_cycles[i].size <= kSmallSize)
			{
				bgColour = smallColour;
			}
			else if (m_cycles[i].size <= kMediumSize)
			{
				bgColour = medColour;
			}
			else
			{
				bgColour = largeColour;
			}
			textColour = bgColour;
			bgColour = Maths::RGBA(0,0,0,0x80);
			debugRender->Draw2dBar(p, Maths::HVec2(colWidth - 10.f, textSize), bgColour);

			sizeSum += m_cycles[i].size;

			str.Printf("%d %d%% %.1fK %s", i + 1, (S32)((100.f * sizeSum) / m_totalSize), m_cycles[i].size / 1024.f, m_cycles[i].cycle->GetName().GetString());
			debugRender->Draw2dText(p, str, textColour, textSize);
			p += Maths::HVecY(textSize);

			if (p.Y() > 650.f)
			{
				colStart += Maths::HVecX(colWidth);
				p = colStart;
			}
		}
	}

private:
	struct CycleInfo
	{
		CycleInfo(const Animation::C_Cycle* inCycle):
			cycle(inCycle),
			size(inCycle->GetMemoryUsage())
		{
		}
		const Animation::C_Cycle*	cycle;
		U32							size;

	};

	DbUtil::I_DebugRender*	m_debugRender;
	Btl::Vector<CycleInfo>	m_cycles;
	U32						m_totalSize;
};

//
// Gathers information about tree memory usage.
//
class C_TreeMemoryVisitor: public C_Visitor
{
public:
	C_TreeMemoryVisitor():
		m_totalSize(0)
	{
		for (U32 i = 0; i < E_NodeType_Count; ++i)
		{
			m_memoryUsedByNodeType[i] = 0;
			m_nodeCounts[i] = 0;
		}
	}

	void Begin()
	{
		for (U32 i = 0; i < E_NodeType_Count; ++i)
		{
			m_memoryUsedByNodeType[i] = 0;
			m_nodeCounts[i] = 0;
		}
		m_totalSize = 0;
	}

	void End()
	{
	}

	template <typename T> bool AddNodeSize(const T& node)
	{
		m_memoryUsedByNodeType[node.GetType()] += sizeof(T);
		m_totalSize += sizeof(T);
		m_nodeCounts[node.GetType()]++;
		return true;
	}

	virtual bool Visit(C_CycleNode& cycleNode)					{ return AddNodeSize(cycleNode); }
	virtual bool Visit(C_SelectNode& selectNode)				{ return AddNodeSize(selectNode); }
	virtual bool Visit(C_CycleGroupNode& cycleGroupNode)		{ return AddNodeSize(cycleGroupNode); }
	virtual bool Visit(C_StateMachineNode& stateMachineNode)	{ return AddNodeSize(stateMachineNode); }
	virtual bool Visit(C_LayeredBlendNode& layeredBlendNode)	{ return AddNodeSize(layeredBlendNode); }
	virtual bool Visit(C_BlendNode& blendNode)					{ return AddNodeSize(blendNode); }
	virtual bool Visit(C_TimeControllerNode& timeControllerNode){ return AddNodeSize(timeControllerNode); }
	virtual bool Visit(C_IkSetTargetNode& ikSetTargetNode)		{ return AddNodeSize(ikSetTargetNode); }
	virtual bool Visit(C_IkApplyNode& ikApplyNode)				{ return AddNodeSize(ikApplyNode); }
	virtual bool Visit(C_IkEnableNode& ikEnableNode)			{ return AddNodeSize(ikEnableNode); }
	virtual bool Visit(C_MirrorNode& mirrorNode)				{ return AddNodeSize(mirrorNode); }
	virtual bool Visit(C_TriggerNode& triggerNode)				{ return AddNodeSize(triggerNode); }

	void DebugRender(DbUtil::I_DebugRender* debugRender, const Maths::HVec2& pos, F32 textSize)
	{
		Util::C_String str;
		Maths::HVec2 p = pos;
		Maths::HVec2 colStart = pos;
		Maths::RGBA textColour(0xff, 0xff, 0xff, 0xff);
		F32 colWidth = 300.f;
		const U32 kSmallSize = 8 * 1024;
		const U32 kMediumSize = 20 * 1024;

		debugRender->Draw2dText(p, str.Printf("Total %d nodes, %.2fK", GetTotalNodes(), GetTotalSize() / 1024.f), textColour, textSize);
		p += Maths::HVecY(textSize);

		static const U32 kNodeTypeSizes[E_NodeType_Count] =
		{
			0,
			sizeof(C_CycleNode), 
			sizeof(C_SelectNode),
			sizeof(C_CycleGroupNode),
			sizeof(C_StateMachineNode),
			sizeof(C_TimeControllerNode),
			sizeof(C_BlendNode),
			sizeof(C_LayeredBlendNode),
			sizeof(C_IkSetTargetNode),
			sizeof(C_IkEnableNode),
			sizeof(C_IkApplyNode),
			sizeof(C_MirrorNode),
			sizeof(C_TriggerNode)
		};

		BIZ_STATIC_ASSERT(sizeof(kNodeTypeSizes)/sizeof(kNodeTypeSizes[0]) == E_NodeType_Count);

		for (U32 i = 0; i < E_NodeType_Count; ++i)
		{
			str.Printf("%.2fK %d %d %s", m_memoryUsedByNodeType[i] / 1024.f, m_nodeCounts[i], kNodeTypeSizes[i], kNodeTypeNames[i]);
			debugRender->Draw2dText(p, str, textColour, textSize);
			p += Maths::HVecY(textSize);

			if (p.Y() > 650.f)
			{
				colStart += Maths::HVecX(colWidth);
				p = colStart;
			}
		}
	}

private:

	U32 GetTotalNodes() const
	{
		U32 totalNodes = 0;
		for (U32 i = 0; i < E_NodeType_Count; ++i)
		{
			totalNodes += m_nodeCounts[i];
		}
		return totalNodes;
	}

	U32 GetTotalSize() const
	{
		return m_totalSize;
	}

	U32		m_memoryUsedByNodeType[E_NodeType_Count];
	U32		m_nodeCounts[E_NodeType_Count];
	U32		m_totalSize;
	U32		m_totalNodes;
};

//
// Visits tree nodes and notifies them that their resources have been loaded.
//
class C_OnResourceBagLoadedVisitor: public C_Visitor
{
public:
	C_OnResourceBagLoadedVisitor(ResourceUtils::I_ResourceBag& resourceBag):
	  m_resourceBag(resourceBag)
  {
  }

	virtual bool Visit(C_CycleNode& cycleNode)					{ cycleNode.OnResourceBagLoaded(m_resourceBag); return true; }
	virtual bool Visit(C_CycleGroupNode& cycleGroupNode)		{ cycleGroupNode.OnResourceBagLoaded(m_resourceBag); return true; }

private:
	ResourceUtils::I_ResourceBag&	m_resourceBag;
};

//
// C_Tree
//

C_Tree::C_Tree():
	m_isGlobalMirrorEnabled(false),
	m_nodeMemoryUsed(0),
	m_pResourceBag(NULL)
{
	Clear();
}

C_Tree::~C_Tree()
{
	Destroy();
}


void C_Tree::SetResourceBag(ResourceUtils::I_ResourceBag& resourceBag)
{
	m_pResourceBag = &resourceBag;
}

ResourceUtils::I_ResourceBag& C_Tree::GetResourceBag() const
{
	BIZ_ASSERT(m_pResourceBag);
	return *m_pResourceBag;
}

void C_Tree::OnResourceBagLoaded()
{
	if (!m_root) return;

	C_OnResourceBagLoadedVisitor visitor(*m_pResourceBag);
	m_root->Accept(visitor);
}

//
// Varying parameters
//

S32 C_Tree::AddParameter(const CHAR8* name)
{
	S32 index = FindParameter(name);
	if (index > 0)
	{
		return index;
	}
	m_parameterNames.PushBack(Btl::String(name));
	m_parameterValues.PushBack(0.f);
	return m_parameterValues.Size() - 1;
}

S32 C_Tree::FindParameter(const CHAR8* name) const
{
	for (S32 i = 1, count = m_parameterNames.Size(); i < count; ++i)
	{
		if (!stricmp(m_parameterNames[i].Chars(), name))
		{
			return i;
		}
	}
	return 0;
}

//
// Fixed parameters
//
S32 C_Tree::AddFixedParameter(const CHAR8* name)
{
	S32 index = FindFixedParameter(name);
	if (index >= 0)
	{
		return index;
	}
	m_fixedParameters.PushBack(Parameter(name, 0.f));
	return m_fixedParameters.Size() - 1;
}

S32 C_Tree::FindFixedParameter(const CHAR8* name) const
{
	for (S32 i = 0, count = m_fixedParameters.Size(); i < count; ++i)
	{
		if (!stricmp(m_fixedParameters[i].name.Chars(), name))
		{
			return i;
		}
	}
	return -1;
}

void C_Tree::SetFixedParameter(S32 index, F32 value)
{
	if (index >= 0)
	{
		m_fixedParameters[index].value = value;
	}
}

F32 C_Tree::GetFixedParameter(S32 index) const
{
	if (index >= 0)
	{
		return m_fixedParameters[index].value;
	}
	return 0.f;
}

U32 C_Tree::GetNumFixedParameters() const
{
	return m_fixedParameters.Size();
}

//
// Feedback parameters.
//
void C_Tree::SetFeedbackParameter(U32 index, F32 value)
{
	if (index < kMaxFeedbackParameters)
	{
		m_feedbackParameters[index] = value;
	}
}

F32 C_Tree::GetFeedbackParameter(U32 index) const
{
	if (index < kMaxFeedbackParameters)
	{
		return m_feedbackParameters[index];
	}
	return 0.f;
}

//
// Actions
//

S32 C_Tree::AddAction(const CHAR8* name)
{
	S32 index = FindAction(name);
	if (index > 0) return index;
	m_actionNames.PushBack(name);
	index = m_actionNames.Size() - 1;
	BIZ_ASSERT(index > 0);
	m_actionRequests.PushBack(false);
	m_actionCompleted.PushBack(false);
	return index;
}

S32 C_Tree::FindAction(const CHAR8* name)
{
	for (S32 i = 1, count = m_actionNames.Size(); i < count; ++i)
	{
		if (!stricmp(m_actionNames[i].Chars(), name))
		{
			return i;
		}
	}
	return 0;
}

const CHAR8* C_Tree::GetActionName(S32 index) const
{
	BIZ_ASSERT(index >= 0);
	return m_actionNames[index].Chars();
}


void C_Tree::ClearAllActionRequests()
{
	m_actionRequests.ClearBits();
}

//
// IK chains.
//

U32 C_Tree::GetNumIkChains() const
{
	return m_ikChains.Size();
}

S32 C_Tree::AddIkChain(const CHAR8* name, Animation::C_IkChain& ikChain)
{
	S32 index = m_ikChains.Size();
	m_ikChains.PushBack();
	m_ikChains.Back().ikChain = &ikChain;
	m_ikChains.Back().name = name;
	return index;
}

S32 C_Tree::FindIkChain(const CHAR8* name) const
{
	for (S32 i = 0, count = m_ikChains.Size(); i < count; ++i)
	{
		if (!stricmp(m_ikChains[i].name.Chars(), name))
		{
			return i;
		}
	}
	return -1;
}

Animation::C_IkChain* C_Tree::GetIkChain(S32 ikChainIndex) const
{
	return ikChainIndex >= 0 ? m_ikChains[ikChainIndex].ikChain : NULL;
}

//
// IK targets.
//

S32 C_Tree::AddIkTarget(const CHAR8* name, S32 ikChainIndex, U32 ikChainTargetIndex)
{
	BIZ_ASSERT(ikChainIndex >= 0);
	if (ikChainIndex < 0) return -1;

	S32 index = m_ikTargets.Size();
	m_ikTargets.PushBack();
	IkTarget& t = m_ikTargets.Back();
	
	t.ikChainIndex = ikChainIndex;
	t.targetIndex = ikChainTargetIndex;
	t.name = name;
	
	return index;
}

S32 C_Tree::FindIkTarget(const CHAR8* name) const
{
	if (name)
	{
		for (U32 i = 0, count = m_ikTargets.Size(); i < count; ++i)
		{
			if (!stricmp(m_ikTargets[i].name.Chars(), name))
			{
				return i;
			}
		}
	}
	return -1;
}

Animation::C_IkChain* C_Tree::GetIkTargetChain(S32 targetId) const
{
	if (targetId < 0 || targetId >= m_ikTargets.Size())
	{
		return NULL;
	}
	return m_ikChains[m_ikTargets[targetId].ikChainIndex].ikChain;
}

U32 C_Tree::GetIkTargetIndex(S32 targetId) const
{
	if (targetId < 0 || targetId >= m_ikTargets.Size())
	{
		return 0;
	}
	return m_ikTargets[targetId].targetIndex;
}

const CHAR8* C_Tree::GetIkTargetName(S32 targetId) const
{
	if (targetId < 0 || targetId >= m_ikTargets.Size())
	{
		return "";
	}
	return m_ikTargets[targetId].name.Chars();
}

void C_Tree::ClearIkTargetWeights()
{
	for (U32 i = 0, count = m_ikTargets.Size(); i < count; ++i)
	{
		m_ikTargets[i].weight = 0.f;
		m_ikTargets[i].totalBlendWeight = 0.f;
	}
}

F32 C_Tree::GetIkTargetWeight(S32 targetId) const
{
	return targetId >= 0 ? m_ikTargets[targetId].weight : 0.f;
}

void C_Tree::BlendIkTargetWeight(S32 targetId, Animation::E_BlendMode blendMode, F32 targetWeight, F32 blendWeight)
{
	if (targetId >= 0 && blendWeight > 0.f)
	{
		F32 currentWeight = m_ikTargets[targetId].weight;
		F32 totalBlendWeight = m_ikTargets[targetId].totalBlendWeight;
		switch (blendMode)
		{
			default:	
			case Animation::E_BlendMode_Accumulate:
				totalBlendWeight += blendWeight;
				currentWeight += (targetWeight - currentWeight) * blendWeight / totalBlendWeight;
				break;

			case Animation::E_BlendMode_Lerp:
				currentWeight += (targetWeight - currentWeight) * blendWeight;
				totalBlendWeight = blendWeight;
				break;

			case Animation::E_BlendMode_Additive:
				currentWeight += targetWeight * blendWeight;
				totalBlendWeight += blendWeight;
				break;
		}
		m_ikTargets[targetId].weight = currentWeight;
		m_ikTargets[targetId].totalBlendWeight = totalBlendWeight;

		BIZ_ASSERT(m_ikTargets[targetId].weight <= 1.f);
	}
}

U32 C_Tree::GetNumIkTargets() const
{
	return m_ikTargets.Size();
}


//
// Animations
//

U32 C_Tree::GetAnimationIndex(U32 instance = 0) const
{
	assert(instance < kMaxAnimIndices);
	return m_animIndices[instance];
}

void C_Tree::SetAnimationIndex(U32 instance, U32 animIndex)
{
	if (instance < kMaxAnimIndices)
	{
		m_animIndices[instance] = animIndex;
	}
	else
	{
		assert(false);
	}
}

void C_Tree::BeginBuild()
{
	ClearTree();
}

void C_Tree::EndBuild()
{
}

void C_Tree::Clear()
{
	ClearTree();
	m_parameterValues.Clear();
	m_parameterNames.Clear();
	m_fixedParameters.Clear();
	m_ikTargets.Clear();
	for (U32 i = 0; i < kMaxFeedbackParameters; ++i)
	{
		m_feedbackParameters[i] = 0.f;
	}
	m_randomSeed = 0x7abceda2;

	// Add null parameter and actions at 0 indices.
	m_parameterNames.PushBack(Btl::String("<null>"));
	m_parameterValues.PushBack(0.f);
	m_actionNames.PushBack(Btl::String("<null>"));
	m_actionRequests.PushBack(false);
	m_actionCompleted.PushBack(false);
}

void C_Tree::ClearTree()
{
	Destroy();
}

void C_Tree::SetRoot(C_Node* node)
{
	m_root = node;
}

C_Node* C_Tree::GetRoot()
{
	return m_root;
}

C_CycleNode* C_Tree::CreateCycleNode()
{
	return CreateNode<C_CycleNode>();
}

C_SelectNode* C_Tree::CreateSelectNode()
{
	return CreateNode<C_SelectNode>();
}

C_CycleGroupNode* C_Tree::CreateCycleGroupNode()
{
	return CreateNode<C_CycleGroupNode>();
}

C_StateMachineNode* C_Tree::CreateStateMachineNode()
{
	return CreateNode<C_StateMachineNode>();
}

C_TimeControllerNode* C_Tree::CreateTimeControllerNode()
{
	return CreateNode<C_TimeControllerNode>();
}

C_BlendNode* C_Tree::CreateBlendNode()
{
	return CreateNode<C_BlendNode>();
}

C_LayeredBlendNode* C_Tree::CreateLayeredBlendNode()
{
	return CreateNode<C_LayeredBlendNode>();
}

C_IkSetTargetNode* C_Tree::CreateIkSetTargetNode()
{
	return CreateNode<C_IkSetTargetNode>();
}

C_IkApplyNode* C_Tree::CreateIkApplyNode()
{
	return CreateNode<C_IkApplyNode>();
}

C_IkEnableNode* C_Tree::CreateIkEnableNode()
{
	return CreateNode<C_IkEnableNode>();
}

C_MirrorNode* C_Tree::CreateMirrorNode()
{
	return CreateNode<C_MirrorNode>();
}

C_TriggerNode* C_Tree::CreateTriggerNode()
{
	return CreateNode<C_TriggerNode>();
}

void C_Tree::SetGlobalMirrorEnabled(bool enabled)
{
	m_isGlobalMirrorEnabled = enabled;
}

bool C_Tree::IsGlobalMirrorEnabled() const
{
	return m_isGlobalMirrorEnabled;
}

bool C_Tree::Evaluate(Animation::C_CommandList& commandList, const Animation::C_Skeleton* skeleton, const Animation::C_Pose& pose, F32 deltaTime)
{
	if (m_root == NULL) return true;
	
	PERF_EVENT("Animation.AnimTree.Evaluate");

	m_randomSeed ^= commandList.GetUpdateCounter();

	// Reset IK target weights.
	for (U32 i = 0, count = m_ikChains.Size(); i < count; ++i)
	{
		if (m_ikChains[i].ikChain)
		{
			m_ikChains[i].ikChain->ClearTargetWeights();
		}
	}
	
	C_Context context(*this, commandList, skeleton, pose, commandList.GetUpdateCounter(), deltaTime);
	m_root->Evaluate(context, Animation::E_BlendMode_Accumulate, 1.f);
	
	ClearAllActionRequests();
	
	return true;
}

void C_Tree::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	U32 numNodes = m_nodes.Size();
	out << numNodes;

	for (S32 i = 0; i < m_nodes.Size(); ++i)
	{
		U8 type = m_nodes[i]->GetType();
		out << type;
		m_nodes[i]->SaveState(out);
	}
}

bool C_Tree::LoadState(Animation::C_MemoryBlockReader& in)
{
	S32 numNodes;
	in >> numNodes;
	
	S32 readCount = Maths::Min(numNodes, m_nodes.Size());

	for (S32 i = 0; i < readCount; ++i)
	{
		U8 type;
		in >> type;
		if (type != m_nodes[i]->GetType())
		{
			return false;
		}
		m_nodes[i]->LoadState(in);
	}
	
	return true;
}

#ifndef CFG_GOLD
void C_Tree::DebugRender(DbUtil::I_DebugRender* debugRender, const Maths::HVec2& pos, F32 textSize, E_DebugRenderMode mode) const
{
	if (NULL == debugRender || m_root == NULL) return;

	switch (mode)
	{
		case E_DebugRenderMode_ActiveTree:
		{
			C_DebugRenderContext context(debugRender, *this, pos, 30.f);
			m_root->DebugRender(context);
		}
		break;

		case E_DebugRenderMode_CycleStats:
		{
			C_CycleStatsVisitor csv;
			csv.Begin();
			m_root->Accept(csv);
			csv.End();

			csv.DebugRender(debugRender, pos, textSize);
		}
		break;

		case E_DebugRenderMode_NodeMemoryStats:
		{
			C_TreeMemoryVisitor visitor;
			visitor.Begin();
			m_root->Accept(visitor);
			visitor.End();

			visitor.DebugRender(debugRender, pos, textSize);
		}
		break;
	}
}
#endif

void C_Tree::Destroy()
{
	for (S32 i = 0; i < m_nodes.Size(); ++i)
	{
		m_nodeAllocator.Free(m_nodes[i]);
		m_nodes[i] = NULL;
	}
	m_nodes.Clear();
	m_nodeMemoryUsed = 0;
	//m_nodeAllocator.Destroy();
	m_root = NULL;
}

//
// C_UpdateCounter
// 

void C_UpdateCounter::LoadState(Animation::C_MemoryBlockReader& in)
{
	in >> m_counter;
}

void C_UpdateCounter::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	out << m_counter;
}


//
// C_Parameter
//

void C_Parameter::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	out << m_value;
	m_updateCounter.SaveState(out);
}

void C_Parameter::LoadState(Animation::C_MemoryBlockReader& in)
{
	F32 value;
	in >> value;
	
	if (m_index != C_Tree::kInvalidParameterIndex)
	{
		m_value = value;
	}
	
	m_updateCounter.LoadState(in);
}

//
// C_CyclePlayer
//
C_CyclePlayer::C_CyclePlayer():
	m_cycleResource(NULL),
	m_phase(0),
	m_frequency(0),
	m_timeScale(0),
	m_loopMode(Animation::E_LoopMode_Continuous),
	m_activated(0)
{
}

void C_CyclePlayer::SetTimeScale(F32 timeScale)
{
	m_timeScale = static_cast<S16>(Maths::Clamped(timeScale * 1023.f, -32768.f, 32767.f));
}

void C_CyclePlayer::SetCycleResource(const Animation::C_CycleResource* pCycleResource)
{
	m_cycleResource = pCycleResource;
	m_cycleInstance.Clear();
	m_activated = 0;
	AcquireCycle();
}

void C_CyclePlayer::SetLoopMode(Animation::E_LoopMode loopMode)
{
	m_loopMode = loopMode;
}
	
void C_CyclePlayer::SetPhase(const C_Context& context, F32 phase)
{
	m_updateCounter.Update(context);
	m_phase = static_cast<U16>(phase * 65535.f);
}

void C_CyclePlayer::Update(C_Context& context)
{
	bool activated = false;
	if (m_updateCounter.Update(context, &activated))
	{
		if (!activated)
		{
			S32 nextPhase = m_phase + static_cast<S32>(m_frequency * context.deltaTime * 65535.f / 511.f);

			if (m_loopMode == Animation::E_LoopMode_Once)
			{
				nextPhase = Maths::Clamped(nextPhase, 0, 0xffff);
			}
			else
			{
				nextPhase &= 0xffff;
			}
			m_phase = static_cast<U16>(nextPhase);
		}
		else
		{
			m_phase = 0;
		}
	}
	m_activated = activated;
}

void C_CyclePlayer::Evaluate(Animation::C_CommandList& commandList, const Animation::C_Pose& pose, Animation::E_BlendMode blendMode, F32 weight)
{
	if (!m_cycleInstance.IsValid())
	{
		if (!AcquireCycle())
		{
			// Still no data, so don't do anything.
			return;
		}
	}

	if (m_cycleInstance.IsValid() && !m_cycleInstance.IsBound())
	{
		m_cycleInstance.BindToPose(pose);
	}

	const F32 evaluationTime = m_cycleInstance.GetDuration() * (m_phase / 65535.f);
	Animation::C_PoseFieldMask fieldMask;
	commandList.BlendCycle(blendMode, &m_cycleInstance, evaluationTime, weight, static_cast<Animation::E_LoopMode>(m_loopMode), fieldMask);
}

const Animation::C_StringId C_CyclePlayer::GetCycleName() const
{
	if (NULL == m_cycleResource)
	{
		return Animation::C_StringId();
	}
	if (!m_cycleResource->IsDataReady())
	{
		return Animation::C_StringId();
	}
	return m_cycleInstance.GetCycleName();
}

const Animation::C_Cycle* C_CyclePlayer::GetCycle() const
{
	if (NULL == m_cycleResource)
	{
		return NULL;
	}
	if (!m_cycleResource->IsDataReady())
	{
		return NULL;
	}
	return m_cycleResource->GetCycle();
}

void C_CyclePlayer::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	out << m_phase;
	
	const Animation::C_CycleInstance& c = m_cycleInstance;
	out << c.GetPreviousTime();
	out << c.GetPreviousTimeInFrames();
	out << c.GetPreviousUpdateCounter();
	out << c.GetPreviousTrajectoryTransform();
}

void C_CyclePlayer::LoadState(Animation::C_MemoryBlockReader& in)
{
	in >> m_phase;
	
	Animation::C_CycleInstance& c = m_cycleInstance;
	
	F32 prevTime, prevTimeInFrames;
	in >> prevTime >> prevTimeInFrames;
	c.SetPreviousTime( prevTime, prevTimeInFrames );
	
	U32 updateCounter;
	in >> updateCounter;
	c.SetPreviousUpdateCounter( updateCounter );
	
	Animation::C_JointTransform trajDelta;
	in >> trajDelta;
	
	c.SetPreviousTrajectoryTransform( trajDelta );
}


bool C_CyclePlayer::AcquireCycle() 
{
	if (NULL == m_cycleResource)
	{
		return false;
	}
	if (!m_cycleResource->IsDataReady())
	{
		return false;
	}
	
	m_cycleInstance.Create(m_cycleResource->GetCycle(), NULL);
	m_frequency = static_cast<S16>(511.f * (m_timeScale / 1023.f) * Maths::Reciprocal(m_cycleInstance.GetDuration()));
	return true;
}



//
// C_Node
//

#ifndef CFG_GOLD

void C_Node::DebugRender(C_DebugRenderContext& context) const
{
	
	context.PrintLine(kNodeTypeNames[GetType()]);
}
#endif

template <typename TResource>
inline void AddToResourceBag(ResourceUtils::I_ResourceBag& resourceBag, const char* filename, TResource*& outResource)
{
	ResourceUtils::C_ResourceBagTag tag = MakeResourceBagTag(filename);
	resourceBag.AddResource(tag, filename, TResource::StaticGetResourceTypeId());

	if (outResource)
	{
		outResource = resourceBag.GetResource<TResource>(tag);
	}
}


//
// C_CycleNode
//

C_CycleNode::C_CycleNode()
{
}

void C_CycleNode::Create(ResourceUtils::I_ResourceBag& resourceBag, const Util::C_String& cycleFilename, Animation::E_LoopMode loopMode, F32 timeScale)
{
	m_cycleTag = MakeResourceBagTag(cycleFilename);
	resourceBag.AddResource(m_cycleTag, cycleFilename, Animation::C_CycleResource::StaticGetResourceTypeId());

	m_cyclePlayer.SetTimeScale(timeScale);
	m_cyclePlayer.SetLoopMode(loopMode);
}

void C_CycleNode::OnResourceBagLoaded(ResourceUtils::I_ResourceBag& resourceBag)
{
	m_cyclePlayer.SetCycleResource(resourceBag.GetResource<Animation::C_CycleResource>(m_cycleTag));
}

void C_CycleNode::SetPhase(C_Context& context, F32 phase)
{
	m_cyclePlayer.SetPhase(context, phase);
}

F32 C_CycleNode::GetFrequency(C_Context& context)
{
	return m_cyclePlayer.GetFrequency();
}

void C_CycleNode::Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight)
{
	m_cyclePlayer.Update(context);
	m_cyclePlayer.Evaluate(context.commandList, context.pose, blendMode, localWeight);
	m_ikConstraints.Apply(context.tree, blendMode, localWeight);
}

void C_CycleNode::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	m_cyclePlayer.SaveState(out);
}

void C_CycleNode::LoadState(Animation::C_MemoryBlockReader& in)
{
	m_cyclePlayer.LoadState(in);
}

#ifndef CFG_GOLD
void C_CycleNode::DebugRender(C_DebugRenderContext& context) const
{
	context.PrintLine(Util::C_String::Format("Cycle: %s %.3f", m_cyclePlayer.GetCycleName().GetString(), m_cyclePlayer.GetPhase()));
}
#endif

bool C_CycleNode::Accept(C_Visitor& visitor)
{
	visitor.Visit(*this);
	return true;
}

const Animation::C_Cycle* C_CycleNode::GetCycle() const
{
	return m_cyclePlayer.GetCycle();
}

//
// C_SelectNode
//

C_SelectNode::C_SelectNode():
	m_inputIndex(-1),
	m_selectionType(E_SelectionType_Random)
{
}

void C_SelectNode::SetNumInputsHint(U32 numInputs)
{
	m_inputs.Reserve(numInputs);
}

void C_SelectNode::AddInput(C_Node* input)
{
	m_inputs.PushBack(input);
}

void C_SelectNode::SetSelectionType(E_SelectionType select)
{
	m_selectionType = select;
}

void C_SelectNode::SetPhase(C_Context& context, F32 phase)
{
	Update(context);
	if (m_inputIndex >= 0)
	{
		if (m_inputs[m_inputIndex] != NULL)
		{
			m_inputs[m_inputIndex]->SetPhase(context, phase);
		}
	}
}

F32 C_SelectNode::GetFrequency(C_Context& context)
{
	F32 f = 0.f;
	if (m_inputIndex >= 0)
	{
		if (m_inputs[m_inputIndex] != NULL)
		{
			f = m_inputs[m_inputIndex]->GetFrequency(context);
		}
	}
	return f;
}

void C_SelectNode::Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight)
{
	Update(context);
	if (m_inputIndex >= 0)
	{
		if (m_inputs[m_inputIndex] != NULL)
		{
			m_inputs[m_inputIndex]->Evaluate(context, blendMode, localWeight);
		}
	}
}

void C_SelectNode::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	out << m_inputIndex;
}

void C_SelectNode::LoadState(Animation::C_MemoryBlockReader& in)
{
	in >> m_inputIndex;
	m_inputIndex = Maths::Clamped(m_inputIndex, 0, m_inputs.Size());
}

#ifndef CFG_GOLD
void C_SelectNode::DebugRender(C_DebugRenderContext& context) const
{
	context.PrintLine(Util::C_String::Format("SelectNode: input index %d", m_inputIndex));
}
#endif

void C_SelectNode::Update(C_Context& context)
{
	bool activated = false;
	bool needUpdate = m_updateCounter.Update(context, &activated);
	
	if (!needUpdate) return;
	
	if (activated)
	{
		S32 numInputs = m_inputs.Size();
		if (numInputs > 0)
		{
			switch (m_selectionType)
			{
				case E_SelectionType_Random:
					m_inputIndex = context.tree.Rand() % numInputs;
					break;
					
				case E_SelectionType_Sequential:
					m_inputIndex = (m_inputIndex + 1) % numInputs;
					break;
					
				case E_SelectionType_AnimIndex:
					m_inputIndex = context.tree.GetAnimationIndex(0) % numInputs;
					break;
				
				default:
					break;
			}
			
			if (m_inputIndex < 0)
			{
				m_inputIndex = 0;
			}
		}
		else
		{
			m_inputIndex = -1;
		}
	}
}

bool C_SelectNode::Accept(C_Visitor& visitor)
{
	if (visitor.Visit(*this))
	{
		for (U32 i = 0, count = m_inputs.Size(); i < count; ++i)
		{
			if (m_inputs[i])
			{
				m_inputs[i]->Accept(visitor);
			}
		}
	}
	return true;
}

//
// C_CycleGroupNode
//
C_CycleGroupNode::C_CycleGroupNode():
	m_currentCycleIndex(-1),
	m_selectionType(E_SelectionType_Sequential)
{}

void C_CycleGroupNode::AddCycle(ResourceUtils::I_ResourceBag& resourceBag, const Util::C_String& cycleFilename, const C_IkConstraints& ikConstraints, F32 timeScale, Animation::E_LoopMode loopMode)
{
	m_cycles.PushBack();

	m_cycles.Back().tag = MakeResourceBagTag(cycleFilename);
	resourceBag.AddResource(m_cycles.Back().tag, cycleFilename, Animation::C_CycleResource::StaticGetResourceTypeId());

	m_cycles.Back().timeScale = timeScale;
	m_cycles.Back().ikConstraints = ikConstraints;
	m_cycles.Back().loopMode = loopMode;
}

void C_CycleGroupNode::OnResourceBagLoaded(ResourceUtils::I_ResourceBag& resourceBag)
{
	for (U32 i = 0, count = m_cycles.Size(); i < count; ++i)
	{
		m_cycles[i].pResource = resourceBag.GetResource<Animation::C_CycleResource>(m_cycles[i].tag);
	}
}


U32 C_CycleGroupNode::GetNumCycles() const
{
	return m_cycles.Size();
}

const Animation::C_Cycle* C_CycleGroupNode::GetCycle(U32 index) const
{
	const Animation::C_CycleResource* res = m_cycles[index].pResource;
	if (!res) return 0;
	return res->GetCycle();
}


void C_CycleGroupNode::SetSelectionType(E_SelectionType select)
{
	m_selectionType = select;
}
	
void C_CycleGroupNode::SetPhase(C_Context& context, F32 phase)
{
	// TODO: ensure cycle is selected
	m_cyclePlayer.SetPhase(context, phase);
}
	
F32 C_CycleGroupNode::GetFrequency(C_Context& context)
{
	return m_cyclePlayer.GetFrequency();
}
	
void C_CycleGroupNode::Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight)
{
	m_cyclePlayer.Update(context);
	
	if (m_cyclePlayer.Activated())
	{
		// Pick a new cycle.
		U32 numCycles = m_cycles.Size();
		if (numCycles > 0)
		{
			switch (m_selectionType)
			{
				case E_SelectionType_Random:
					m_currentCycleIndex = context.tree.Rand() % numCycles;
					break;
					
				case E_SelectionType_Sequential:
					m_currentCycleIndex = (m_currentCycleIndex + 1) % numCycles;
					break;
					
				case E_SelectionType_AnimIndex:
					m_currentCycleIndex = context.tree.GetAnimationIndex(0) % numCycles;
					break;
					
				default:
					break;
			}
			
			if (m_currentCycleIndex >= 0)
			{
				m_cyclePlayer.SetCycleResource(m_cycles[m_currentCycleIndex].pResource);
				m_cyclePlayer.SetTimeScale(m_cycles[m_currentCycleIndex].timeScale);
				m_cyclePlayer.SetLoopMode(m_cycles[m_currentCycleIndex].loopMode);
				m_ikConstraints = m_cycles[m_currentCycleIndex].ikConstraints;
			}
		}
		else
		{
			m_currentCycleIndex = -1;
			m_ikConstraints.Reset();
		}
	}
	
	m_cyclePlayer.Evaluate(context.commandList, context.pose, blendMode, localWeight);
	m_ikConstraints.Apply(context.tree, blendMode, localWeight);
}
	
void C_CycleGroupNode::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	m_cyclePlayer.SaveState(out);
}

void C_CycleGroupNode::LoadState(Animation::C_MemoryBlockReader& in)
{
	m_cyclePlayer.LoadState(in);
}
	
#ifndef CFG_GOLD
void C_CycleGroupNode::DebugRender(C_DebugRenderContext& context) const
{
	context.PrintLine(Util::C_String::Format("CycleGroup: %s %.3f", m_cyclePlayer.GetCycleName().GetString(), m_cyclePlayer.GetPhase()));
}
#endif

bool C_CycleGroupNode::Accept(C_Visitor& visitor)
{
	visitor.Visit(*this);
	return true;
}

//
// C_StateMachineNode::C_Builder
//
C_StateMachineNode::C_Builder::C_Builder(C_StateMachineNode& stateMachine):
	m_stateMachine(stateMachine)
{
	Reset();
}

void C_StateMachineNode::C_Builder::Reset()
{
	m_stateMachine.Clear();

	m_stateMachine.m_data.Destroy();
	m_stateMachine.m_numStates = 0;

	m_sizer.Reset();
	m_currentConditionIndex = 0;
	m_maxStates = 0;
	m_currentStateIndex = 0;
	
}

void C_StateMachineNode::C_Builder::SetMaxStates(U32 maxStates)
{
	m_maxStates = maxStates;
	m_sizer.Allocate(m_stateMachine.m_pStateOffsets, maxStates);
	m_stateMachine.m_data.Create(m_sizer);
}

S32 C_StateMachineNode::C_Builder::AddState(C_Node* input, S32 actionIndex, U32 numTransitions)
{
	BIZ_ASSERT(actionIndex >= AnimationTree::C_Tree::kInvalidActionIndex);
	BIZ_ASSERT(m_stateMachine.m_numStates <= m_maxStates);

	U32 currentStateIndex = m_stateMachine.m_numStates;
	m_stateMachine.m_numStates++;

	Animation::C_RelativePtr<C_StateMachineNode::State> pState;
	m_sizer.Allocate(pState, 1);

	Animation::C_RelativePtr<C_StateMachineNode::Transition> pTransitions;
	m_sizer.Allocate(pTransitions, numTransitions);

	// Reallocate block.
	m_stateMachine.m_data.Resize(m_sizer.GetSize());

	// Store state offset.
	m_stateMachine.m_pStateOffsets(m_stateMachine.m_data)[currentStateIndex] = pState.GetOffset();

	State* newState = pState(m_stateMachine.m_data);
	newState->input = input;
	newState->actionIndex = actionIndex;
	newState->numTransitions = numTransitions;
	newState->pFirstTransition = pTransitions;
	newState->hasAnimationFinishedCondition = 0;

	m_currentStateIndex = currentStateIndex;

	return currentStateIndex;
}

void C_StateMachineNode::C_Builder::AddTransition(S32 fromState, S32 toState, U32 transitionIndex, F32 blendTime, U32 numConditions)
{
	BIZ_ASSERT(fromState >= 0);
	BIZ_ASSERT(toState >= 0);

	State& state = m_stateMachine.GetState(fromState);
	
	BIZ_ASSERT(transitionIndex < state.numTransitions);
	m_pCurrentTransition = state.pFirstTransition + transitionIndex;

	m_currentConditionIndex = 0;

	Transition& t = *m_pCurrentTransition(m_stateMachine.m_data);

	t.blendTime = blendTime;
	t.numConditions = numConditions;
	t.nextState = toState;
	t.animationFinishedCondition = 0;
	t.requestedActionCondition = 0;
	t.actionIndex = 0;

	// Allocate space for conditions and resize.
	m_sizer.Allocate(t.pConditions, numConditions);
	m_stateMachine.m_data.Resize(m_sizer.GetSize());
}

void C_StateMachineNode::C_Builder::AddAnimationFinishedCondition()
{
	Transition& t = *m_pCurrentTransition(m_stateMachine.m_data);
	t.animationFinishedCondition = 1;
	m_stateMachine.GetState(m_currentStateIndex).hasAnimationFinishedCondition = 1;
}

void C_StateMachineNode::C_Builder::AddRequestedActionCondition(S32 actionIndex)
{
	BIZ_ASSERT(actionIndex < (1 << 10));
	Transition& t = *m_pCurrentTransition(m_stateMachine.m_data);
	t.requestedActionCondition = 1;
	t.actionIndex = actionIndex;
}

void C_StateMachineNode::C_Builder::AddComparisonCondition(const C_Parameter& parameter1, E_Operator op, F32 parameter2)
{
	Condition* cond = NextCondition();
	if (cond != NULL)
	{
		switch (op)
		{
			default:	
			case E_Operator_EQ:		cond->pTestFunc = &Condition::TestEQ; break;
			case E_Operator_NEQ:	cond->pTestFunc = &Condition::TestNEQ; break;
			case E_Operator_LT:		cond->pTestFunc = &Condition::TestLT; break;
			case E_Operator_LEQ:	cond->pTestFunc = &Condition::TestLEQ; break;
			case E_Operator_GT:		cond->pTestFunc = &Condition::TestGT; break;
			case E_Operator_GEQ:	cond->pTestFunc = &Condition::TestGEQ; break;
		}
		cond->parameter1 = parameter1;
		cond->parameter2 = parameter2;
	}
}

C_StateMachineNode::Condition* C_StateMachineNode::C_Builder::GetCurrentCondition()
{
	Transition& t = *m_pCurrentTransition(m_stateMachine.m_data);
	BIZ_ASSERT(m_currentConditionIndex < t.numConditions);
	return &t.pConditions(m_stateMachine.m_data)[m_currentConditionIndex];
}

C_StateMachineNode::Condition* C_StateMachineNode::C_Builder::NextCondition()
{
	Condition* c = GetCurrentCondition();
	++m_currentConditionIndex;
	return c;
}


//
// C_StateMachineNode
//

C_StateMachineNode::C_StateMachineNode():
	m_initialStateIndex(-1),
	m_currentStateIndex(-1)
{
}

void C_StateMachineNode::Clear()
{
	m_data.Destroy();
	m_numStates = 0;
}

void C_StateMachineNode::SetInitialState(S32 stateIndex)
{
	m_initialStateIndex = stateIndex;
	m_currentStateIndex = stateIndex;
	m_currentStateUpdate.Reset();
	m_nextStateIndex = -1;
	m_currentStateTime = 0.f;
	m_nextStateTime = 0.f;
	m_blendTime = 0.f;
}

void C_StateMachineNode::SetPhase(C_Context& context, F32 phase)
{
	Update(context);
	
	if (m_currentStateIndex < 0) return;
	
	const State& state = GetState(m_currentStateIndex);
	if (m_nextStateIndex < 0)
	{
		if (state.input != NULL)
		{
			state.input->SetPhase(context, phase);
		}
	}
	else
	{
		// Two active states since we're in the middle of transition.
		const State& nextState = GetState(m_nextStateIndex);
		
		if (state.input != NULL)
		{
			state.input->SetPhase(context, phase);
		}
		
		if (nextState.input != NULL)
		{
			nextState.input->SetPhase(context, phase);
		}
	}
}

void C_StateMachineNode::Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight)
{
	Update(context);

	if (m_currentStateIndex < 0) return;
	
	const State& state = GetState(m_currentStateIndex);
	
	if (m_nextStateIndex < 0)
	{
		if (state.input != NULL)
		{
			state.input->Evaluate(context, blendMode, localWeight);
		}
	}
	else
	{
		context.commandList.PushCurrentPose(localWeight);

		// Two active states since we're in the middle of transition.
		const State& nextState = GetState(m_nextStateIndex);
		
		const F32 nextBlendWeight = Maths::SmoothStep(Maths::Clamped(m_nextStateTime / m_blendTime, 0.f, 1.f));
		const F32 currBlendWeight = 1.f - nextBlendWeight;
		
		if (state.input != NULL)
		{
			state.input->Evaluate(context, Animation::E_BlendMode_Accumulate, localWeight * currBlendWeight);
		}
		
		if (nextState.input != NULL)
		{
			nextState.input->Evaluate(context, Animation::E_BlendMode_Accumulate, localWeight * nextBlendWeight);
		}

		context.commandList.PopAndBlendPose(blendMode);
	}
}

void C_StateMachineNode::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	out << S8(m_currentStateIndex);
	m_currentStateUpdate.SaveState(out);
}

void C_StateMachineNode::LoadState(Animation::C_MemoryBlockReader& in)
{
	S8 currentStateIndex;
	in >> currentStateIndex;
	if (currentStateIndex < -1) currentStateIndex = -1;
	if (currentStateIndex >= m_numStates) currentStateIndex = m_initialStateIndex;
	m_currentStateIndex = currentStateIndex;
	m_currentStateUpdate.LoadState(in);
}

#ifndef CFG_GOLD
void C_StateMachineNode::DebugRender(C_DebugRenderContext& context) const
{
	Util::C_String str;
	const CHAR8* actionName = "";
	if (m_currentStateIndex >= 0)
	{
		actionName = context.Tree().GetActionName(GetState(m_currentStateIndex).actionIndex);
	}
		
	context.PrintLine(str.Printf("StateMachine: %d %d %s", m_currentStateIndex, m_nextStateIndex, actionName));
	
	context.Begin();
	
	if (m_currentStateIndex >= 0)
	{
		const State& state = GetState(m_currentStateIndex);
		if (state.input != NULL)
		{
			state.input->DebugRender(context);
		}
	}
	if (m_nextStateIndex >= 0)
	{
		const State& state = GetState(m_nextStateIndex);
		if (state.input != NULL)
		{
			state.input->DebugRender(context);
		}
	}
	context.End();
}
#endif

bool C_StateMachineNode::Accept(C_Visitor& visitor)
{
	if (visitor.Visit(*this))
	{
		for (U32 i = 0, count = m_numStates; i < count; ++i)
		{
			if (GetState(i).input)
			{
				GetState(i).input->Accept(visitor);
			}
		}
	}
	return true;
}

const C_StateMachineNode::State& C_StateMachineNode::GetState(S32 stateIndex) const
{
	Animation::C_RelativePtr<State> pState(m_pStateOffsets(m_data)[stateIndex]);
	return *pState(m_data);
}

C_StateMachineNode::State& C_StateMachineNode::GetState(S32 stateIndex)
{
	Animation::C_RelativePtr<State> pState(m_pStateOffsets(m_data)[stateIndex]);
	return *pState(m_data);
}

void C_StateMachineNode::Update(C_Context& context)
{
	bool activated = false;
	bool needUpdate = m_currentStateUpdate.Update(context, &activated);
	
	if (!needUpdate) return;

	if (activated)
	{
		// If reactivated, reset to initial state.
		m_currentStateIndex = m_initialStateIndex;
		m_nextStateIndex = -1;
		m_currentStateTime = 0.f;
		m_nextStateTime = 0.;
		m_blendTime = 0.f;
	}
	
	if (m_currentStateIndex < 0)
	{
		return;
	}

	const State& state = GetState(m_currentStateIndex);
	DCBT(0, &state);

	m_currentStateTime += context.deltaTime;
		
	if (m_nextStateIndex < 0)
	{
		CheckForTransition(context);
	}
	else
	{
		UpdateDuringTransition(context);
	}
}

void C_StateMachineNode::UpdateDuringTransition(C_Context& context)
{
	m_nextStateTime += context.deltaTime;
	if (m_nextStateTime >= m_blendTime)
	{
		// Finished transition blend.
		m_currentStateIndex = m_nextStateIndex;
		m_nextStateIndex = -1;
		m_currentStateTime = m_nextStateTime;
		
		if (m_currentStateIndex >= 0)
		{
			// Signal that action has been completed.
			context.tree.CompleteAction(GetState(m_currentStateIndex).actionIndex);
		}
	}
}

// Check to see if any transition conditions are satisfied.
void C_StateMachineNode::CheckForTransition(C_Context& context)
{
	const State& state = GetState(m_currentStateIndex);

	if (state.numTransitions == 0)
	{
		return;
	}

	F32 currentStateDuration = 0.f;
	if (state.hasAnimationFinishedCondition && state.input != NULL)
	{
		// Calls to GetFrequency() are expensive - ideally need to avoid
		currentStateDuration = Maths::Reciprocal(state.input->GetFrequency(context));
	}

	F32 currentStateTime = m_currentStateTime;

	bool notSatisfied = true;
	for (U32 i = 0, numTransitions = state.numTransitions; i < numTransitions && notSatisfied; ++i)
	{
		Transition& trans = state.pFirstTransition(m_data)[i];
		const F32 transDuration = currentStateDuration - trans.blendTime;

		Condition* pTransitionConditions = trans.pConditions(m_data);

		if (trans.animationFinishedCondition)
		{
			BIZ_ASSERT(state.hasAnimationFinishedCondition);

			if (currentStateDuration > 0.f)
			{
				if (currentStateTime < transDuration)
				{
					// Not finished - skip to next transition.
					continue;
				}
			}
			// Otherwise transition immediately if source animation is invalid, otherwise we'll get stuck in this state.
		}

		if (trans.requestedActionCondition)
		{
			if (!context.tree.ActionRequested(trans.actionIndex))
			{
				// Action not requested - skip to next transition.
				continue;
			}
		}

		bool allMatch = true;
		for (U32 j = 0, numConditions = trans.numConditions; j < numConditions && allMatch; ++j)
		{
			Condition& cond = pTransitionConditions[j];
			if (!cond.Evaluate(context))
			{
				allMatch = false;
			}
		}

		if (allMatch)
		{
			// All conditions satisfied, so begin transition to new state.
			m_nextStateIndex = trans.nextState;
			m_nextStateTime = 0.f;
			m_blendTime = trans.blendTime;
			notSatisfied = false;

			// Mark actions used as conditions as having been completed.
			if (trans.requestedActionCondition && trans.actionIndex > 0)
			{
				context.tree.CompleteAction(trans.actionIndex);
			}
		}
	}
}

bool C_StateMachineNode::Condition::TestEQ(C_Context& context)
{
	const F32 value1 = parameter1.GetValue(context);
	return value1 == parameter2;
}

bool C_StateMachineNode::Condition::TestNEQ(C_Context& context)
{
	const F32 value1 = parameter1.GetValue(context);
	return value1 != parameter2;
}

bool C_StateMachineNode::Condition::TestLT(C_Context& context)
{
	const F32 value1 = parameter1.GetValue(context);
	return value1 < parameter2;
}

bool C_StateMachineNode::Condition::TestLEQ(C_Context& context)
{
	const F32 value1 = parameter1.GetValue(context);
	return value1 <= parameter2;
}

bool C_StateMachineNode::Condition::TestGT(C_Context& context)
{
	const F32 value1 = parameter1.GetValue(context);
	return value1 > parameter2;
}

bool C_StateMachineNode::Condition::TestGEQ(C_Context& context)
{
	const F32 value1 = parameter1.GetValue(context);
	return value1 >= parameter2;
}

bool C_StateMachineNode::Condition::TestFalse(C_Context& context)
{
	return false;
}

//
// C_LayeredBlendNode
//

C_LayeredBlendNode::C_LayeredBlendNode():
	m_baseLayer(NULL)
{
}
	
void C_LayeredBlendNode::Clear()
{
	m_baseLayer = NULL;
	m_layers.Clear();
}
		
void C_LayeredBlendNode::SetBaseLayer(C_Node* baseTree)
{
	m_baseLayer = baseTree;
}
	
S32 C_LayeredBlendNode::AddLayer(C_Node* layerTree, Animation::E_BlendMode blendMode)
{
	if (layerTree == NULL) return -1;
	if (m_layers.Size() >= m_layers.Capacity()) return -1;
	
	Layer newLayer;
	newLayer.childTree = layerTree;
	newLayer.blendMode = blendMode;
	
	S32 index = m_layers.Size();
	m_layers.PushBack(newLayer);
	return index;
}
	
void C_LayeredBlendNode::SetPhase(C_Context& context, F32 phase)
{
	if (m_baseLayer != NULL)
	{
		m_baseLayer->SetPhase(context, phase);
	}
}

F32 C_LayeredBlendNode::GetFrequency(C_Context& context)
{
	F32 kAbsurdlyLargeFreq = 999999.f;
	F32 minFreq = kAbsurdlyLargeFreq;
	if (m_baseLayer != NULL)
	{
		F32 f = m_baseLayer->GetFrequency(context);
		
		// if (f > 0 && f < minFreq) minFreq = f
		minFreq = Maths::Select_GT(f, 0.f, Maths::Select_LT(f, minFreq, f, minFreq), minFreq);
	}
	for (S32 i = 0, count = m_layers.Size(); i < count; ++i)
	{
		Layer& layer = m_layers[i];
		F32 f = layer.childTree->GetFrequency(context);

		// if (f > 0 && f < minFreq) minFreq = f
		minFreq = Maths::Select_GT(f, 0.f, Maths::Select_LT(f, minFreq, f, minFreq), minFreq);
	}

	minFreq = Maths::Select_GE(minFreq, kAbsurdlyLargeFreq, 0.f, minFreq);
	return minFreq;
}

	
void C_LayeredBlendNode::Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight)
{
	context.commandList.PushCurrentPose(localWeight);
	
	if (m_baseLayer != NULL)
	{
		m_baseLayer->Evaluate(context, Animation::E_BlendMode_Accumulate, 1.f);
	}
	
	for (U32 i = 0, numLayers = m_layers.Size(); i < numLayers; ++i)
	{
		Layer& layer = m_layers[i];
		layer.childTree->Evaluate(context, layer.blendMode, 1.f);
	}

	context.commandList.PopAndBlendPose(blendMode);
}

#ifndef CFG_GOLD
void C_LayeredBlendNode::DebugRender(C_DebugRenderContext& context) const
{
	C_Node::DebugRender(context);
	context.Begin();
	
	if (m_baseLayer != NULL)
	{
		m_baseLayer->DebugRender(context);
	}
	for (S32 i = 0; i < m_layers.Size(); ++i)
	{
		const Layer& layer = m_layers[i];
		
		if (layer.childTree != NULL)
		{
			layer.childTree->DebugRender(context);
		}
	}
	context.End();
}
#endif

bool C_LayeredBlendNode::Accept(C_Visitor& visitor)
{
	if (visitor.Visit(*this))
	{
		if (m_baseLayer)
		{
			m_baseLayer->Accept(visitor);
		}
		for (U32 i = 0, count = m_layers.Size(); i < count; ++i)
		{
			const Layer& layer = m_layers[i];
			layer.childTree->Accept(visitor);
		}
	}
	return true;
}

//
// C_BlendNode
//

C_BlendNode::C_BlendNode()
{
	Clear();
}

void C_BlendNode::Clear()
{
	m_inputs.Clear();
	m_blendMode = Animation::E_BlendMode_Accumulate;
	m_phase = 0;
	m_phaseUpdate.Reset();
	m_parameter = C_Parameter(0.f);
	m_activeInputs[0] = NULL;
	m_activeInputs[1] = NULL;
	m_flags = E_Flag_SyncTime;
}

void C_BlendNode::SetBlendMode(Animation::E_BlendMode blendMode)
{
	m_blendMode = blendMode;
}

void C_BlendNode::SetFlags(U32 flags)
{
	m_flags |= flags;
}

void C_BlendNode::SetParameter(const C_Parameter& parameter)
{
	m_parameter = parameter;
}

void C_BlendNode::AddInput(C_Node* input, F32 parameterValue)
{
	if (m_inputs.Size() >= m_inputs.Capacity()) return;
	
	// TODO: sort inputs
	
	BlendInput in;
	in.input = input;
	in.parameterValue = parameterValue;
	m_inputs.PushBack(in);
}

void C_BlendNode::SetPhase(C_Context& context, F32 phase)
{
	m_phaseUpdate.Update(context);
	m_phase = phase;
	
	UpdateActiveInputs(context);

	if (m_activeInputs[0] != NULL) m_activeInputs[0]->SetPhase(context, phase);
	if (m_activeInputs[1] != NULL) m_activeInputs[1]->SetPhase(context, phase);
}

F32 C_BlendNode::GetFrequency(C_Context& context)
{
	UpdateActiveInputs(context);
	
	F32 freq = 0.f;
	F32 totalWeight = 0.f;
	
	if (m_activeInputs[0] != NULL)
	{
		freq += m_activeWeights[0] * m_activeInputs[0]->GetFrequency(context);
		totalWeight += m_activeWeights[0];
	}	
	
	if (m_activeInputs[1] != NULL)
	{
		freq += m_activeWeights[1] * m_activeInputs[1]->GetFrequency(context);
		totalWeight += m_activeWeights[1];
	}	
	
	freq *= Maths::Reciprocal(totalWeight);
	
	return freq;
}

void C_BlendNode::Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight)
{
	UpdateActiveInputs(context);
	
	if (m_flags & E_Flag_SyncTime)
	{
		if (m_phaseUpdate.Update(context))
		{
			F32 frequency = GetFrequency(context);
			m_phase += frequency * context.deltaTime;
			m_phase = fmodf(m_phase, 1.f);

			if (m_activeInputs[0] != NULL) m_activeInputs[0]->SetPhase(context, m_phase);
			if (m_activeInputs[1] != NULL) m_activeInputs[1]->SetPhase(context, m_phase);
		}
	}
	
	F32 totalInputWeight = 0.f;
	if (m_activeInputs[0] != NULL) totalInputWeight += m_activeWeights[0];
	if (m_activeInputs[1] != NULL) totalInputWeight += m_activeWeights[1];

	if (blendMode != m_blendMode)
	{
		context.commandList.PushCurrentPose(localWeight);
	}

	// Evaluate active inputs.
	if (m_activeInputs[0] != NULL)
	{
		m_activeInputs[0]->Evaluate(context, m_blendMode, localWeight * m_activeWeights[0]);
	}
	
	if (m_activeInputs[1] != NULL)
	{
		m_activeInputs[1]->Evaluate(context, m_blendMode, localWeight * m_activeWeights[1]);
	}
	
	if (blendMode != m_blendMode)
	{
		context.commandList.PopAndBlendPose(blendMode);
	}
}

void C_BlendNode::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	out << m_phase;
	m_parameter.SaveState(out);
}

void C_BlendNode::LoadState(Animation::C_MemoryBlockReader& in)
{
	in >> m_phase;
	m_parameter.LoadState(in);
}

#ifndef CFG_GOLD
void C_BlendNode::DebugRender(C_DebugRenderContext& context) const
{
	C_Node::DebugRender(context);
	context.Begin();
	
	if (m_activeInputs[0] != NULL)
	{
		m_activeInputs[0]->DebugRender(context);
	}
	if (m_activeInputs[1] != NULL)
	{
		m_activeInputs[1]->DebugRender(context);
	}
	context.End();
}
#endif

bool C_BlendNode::Accept(C_Visitor& visitor)
{
	if (visitor.Visit(*this))
	{
		for (U32 i = 0, count = m_inputs.Size(); i < count; ++i)
		{
			if (m_inputs[i].input)
			{
				m_inputs[i].input->Accept(visitor);
			}
		}
	}
	return true;
}

void C_BlendNode::UpdateActiveInputs(C_Context& context)
{
	if (m_activeInputUpdate.Update(context))
	{
		F32 currentParamValue = m_parameter.GetValue(context);
		
		if (m_flags & E_Flag_Absolute)
		{
			currentParamValue = Maths::Abs(currentParamValue);
		}
		
		const S32 numInputs = m_inputs.Size();
		S32 lowerIndex = -1;
		for (S32 i = 0; i < numInputs; ++i)
		{
			if (m_inputs[i].parameterValue > currentParamValue)
			{
				break;
			}
			lowerIndex = i;
		}
		S32 upperIndex = lowerIndex + 1;

		lowerIndex = Maths::Max(lowerIndex, 0);
		upperIndex = Maths::Min(upperIndex, numInputs - 1);
		
		if (upperIndex > lowerIndex)
		{
			m_activeInputs[0] = m_inputs[lowerIndex].input;
			m_activeInputs[1] = m_inputs[upperIndex].input;
			
			F32 span = m_inputs[upperIndex].parameterValue - m_inputs[lowerIndex].parameterValue;
			F32 blendFraction = (currentParamValue - m_inputs[lowerIndex].parameterValue) * Maths::Reciprocal(span);
			blendFraction = Maths::Clamped(blendFraction, 0.f, 1.f);
			
			if (m_flags & E_Flag_SmoothStep)
			{
				blendFraction = Maths::SmoothStep(blendFraction);
			}
			
			m_activeWeights[0] = 1.f - blendFraction;
			m_activeWeights[1] = blendFraction;
		}
		else
		{
			m_activeInputs[0] = m_inputs[lowerIndex].input;
			m_activeInputs[1] = NULL;
			m_activeWeights[0] = 1.f;
			m_activeWeights[1] = 1.f;
		}
	
	}
}



//
// C_TimeControllerNode
//

C_TimeControllerNode::C_TimeControllerNode():
	m_input(NULL),
	m_parameterMin(0.f),
	m_parameterMax(1.f),
	m_timeMin(0.f),
	m_timeMax(1.f)
{
}

void C_TimeControllerNode::Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight)
{
	if (NULL == m_input) return;
	
	F32 value = m_parameter.GetValue(context);
	F32 relativeValue = (value - m_parameterMin) * Maths::Reciprocal(m_parameterMax - m_parameterMin);
	F32 timeValue = m_timeMin + relativeValue * (m_timeMax - m_timeMin);
	
	// Not really time, is it? More like phase.
	m_input->SetPhase(context, timeValue);

	m_input->Evaluate(context, blendMode, localWeight);
}

void C_TimeControllerNode::SetInput(C_Node* input)
{
	m_input = input;
}

void C_TimeControllerNode::SetParameter(const C_Parameter& parameter)
{
	m_parameter = parameter;
}

void C_TimeControllerNode::SetMapping(F32 parameterMin, F32 parameterMax, F32 timeMin, F32 timeMax)
{
	m_parameterMin = parameterMin;
	m_parameterMax = parameterMax;
	m_timeMin = timeMin;
	m_timeMax = timeMax;
}

void C_TimeControllerNode::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	m_parameter.SaveState(out);
}

void C_TimeControllerNode::LoadState(Animation::C_MemoryBlockReader& in)
{
	m_parameter.LoadState(in);
}

#ifndef CFG_GOLD
void C_TimeControllerNode::DebugRender(C_DebugRenderContext& context) const
{
	C_Node::DebugRender(context);
	context.Begin();
	
	if (m_input != NULL)
	{
		m_input->DebugRender(context);
	}
	context.End();
}
#endif

bool C_TimeControllerNode::Accept(C_Visitor& visitor)
{
	if (visitor.Visit(*this))
	{
		if (m_input)
		{
			m_input->Accept(visitor);
		}
	}
	return true;
}

//
// C_IkConstraints
//
C_IkConstraints::C_IkConstraints():
	m_flags(0),
	m_mask(0)
{
}

void C_IkConstraints::Reset()
{
	m_flags = 0;
	m_mask = 0;
}

void C_IkConstraints::SetIkTargetEnabled(const CHAR8* name, const C_Tree& animTree, bool enabled)
{
	if (enabled)
	{
		EnableIkTarget(animTree.FindIkTarget(name));
	}
	else
	{
		DisableIkTarget(animTree.FindIkTarget(name));
	}
}

void C_IkConstraints::EnableIkTarget(S32 ikTargetId)
{
	if (ikTargetId >= 0)
	{
		BIZ_ASSERT(ikTargetId < 16);
		U32 flag = 1 << ikTargetId;
		m_flags |= flag;
		m_mask |= flag;
	}
}

void C_IkConstraints::DisableIkTarget(S32 ikTargetId)
{
	if (ikTargetId >= 0)
	{
		BIZ_ASSERT(ikTargetId < 16);
		U32 flag = 1 << ikTargetId;
		m_flags &= ~flag;
		m_mask |= flag;
	}
}

void C_IkConstraints::Apply(C_Tree& tree, Animation::E_BlendMode blendMode, F32 blendWeight)
{
	U32 flags = m_flags;
	U32 mask = m_mask;

	U32 ikTargetId = 0;
	while (mask)
	{
		if (mask & 1 )
		{
			F32 weight = (flags & 1) ? 1.f : 0.f;
			tree.BlendIkTargetWeight(ikTargetId, blendMode, weight, blendWeight);
		}

		++ikTargetId;
		mask >>= 1;
		flags >>= 1;
	}
}

//
// C_IkSetTargetNode
//

C_IkSetTargetNode::C_IkSetTargetNode():
	m_child(NULL),
	m_boundSkeleton(NULL)
{
}

void C_IkSetTargetNode::SetPhase(C_Context& context, F32 phase)
{
	if (m_child)
	{
		m_child->SetPhase(context, phase);
	}
}

F32 C_IkSetTargetNode::GetFrequency(C_Context& context)
{
	if (m_child)
	{
		return m_child->GetFrequency(context);
	}
	return 0.f;
}

void C_IkSetTargetNode::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	if (m_child)
	{
		m_child->SaveState(out);
	}
}

void C_IkSetTargetNode::LoadState(Animation::C_MemoryBlockReader& in)
{
	if (m_child)
	{
		m_child->LoadState(in);
	}
}

void C_IkSetTargetNode::Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight)
{
	if (NULL == m_child) return;

	if (context.skeleton != m_boundSkeleton)
	{
		if (context.skeleton != NULL)
		{
			// Bind targets onto joints.
			for (U32 i = 0, count = m_targets.Size(); i < count; ++i)
			{
				Target& t = m_targets[i];
				t.jointIndex = context.skeleton->FindJointByName(t.jointNameId);
			}
		}
		
		m_boundSkeleton = context.skeleton;
	}
	
	if (context.skeleton != NULL)
	{
		context.commandList.PushCurrentPose(localWeight);
		
		m_child->Evaluate(context, Animation::E_BlendMode_Accumulate, localWeight);
		
		for (U32 i = 0, count = m_targets.Size(); i < count; ++i)
		{
			const Target& t = m_targets[i];
			if (t.jointIndex >= 0)
			{
				Animation::C_IkChain* ikChain = context.tree.GetIkTargetChain(t.ikTargetId);
				
				if (ikChain != NULL)
				{
					U32 targetIndex = context.tree.GetIkTargetIndex(t.ikTargetId);
					
					// TODO: have offsets available and ability to use position, rotation or both.
					context.commandList.SetIkTargetTransformFromJoint(*ikChain, targetIndex, t.jointIndex);
				}
			}
		}
		
		context.commandList.PopAndBlendPose(blendMode);
	}
	else
	{
		// No skeleton for some reason...
		m_child->Evaluate(context, Animation::E_BlendMode_Accumulate, localWeight);
	}
}

#ifndef CFG_GOLD
void C_IkSetTargetNode::DebugRender(C_DebugRenderContext& context) const
{
	C_Node::DebugRender(context);
	context.Begin();

	if (m_child)
	{
		m_child->DebugRender(context);
	}

	context.End();
}
#endif

bool C_IkSetTargetNode::Accept(C_Visitor& visitor)
{
	if (visitor.Visit(*this))
	{
		if (m_child)
		{
			m_child->Accept(visitor);
		}
	}
	return true;
}

void C_IkSetTargetNode::SetInput(C_Node* input)
{
	m_child = input;
}

void C_IkSetTargetNode::AddTarget(S32 ikTargetId, const Animation::C_StringId& jointNameId)
{
	m_targets.PushBack();
	Target& t = m_targets.Back();
	t.ikTargetId = ikTargetId;
	t.jointNameId = jointNameId;
	t.jointIndex = -1;
}

//
// C_IkEnableNode
//

C_IkEnableNode::C_IkEnableNode():
	m_input(0)
{
}

void C_IkEnableNode::SetPhase(C_Context& context, F32 phase)
{
	if (m_input)
	{
		m_input->SetPhase(context, phase);
	}
}

F32 C_IkEnableNode::GetFrequency(C_Context& context)
{
	if (m_input)
	{
		return m_input->GetFrequency(context);
	}
	return 0.f;
}

void C_IkEnableNode::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	if (m_input)
	{
		m_input->SaveState(out);
	}
}

void C_IkEnableNode::LoadState(Animation::C_MemoryBlockReader& in)
{
	if (m_input)
	{
		m_input->LoadState(in);
	}
}

void C_IkEnableNode::Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight)
{
	if (m_input)
	{
		m_input->Evaluate(context, blendMode, localWeight);
	}

	m_constraints.Apply(context.tree, blendMode, localWeight);
}

bool C_IkEnableNode::Accept(C_Visitor& visitor)
{
	if (visitor.Visit(*this))
	{
		if (m_input)
		{
			m_input->Accept(visitor);
		}
	}
	return true;
}

#ifndef CFG_GOLD
void C_IkEnableNode::DebugRender(C_DebugRenderContext& context) const
{
	C_Node::DebugRender(context);
	context.Begin();

	if (m_input)
	{
		m_input->DebugRender(context);
	}

	U32 flag = 1;
	Util::C_String str;
	for (U32 i = 0; i <= 16; ++i)
	{
		if (m_constraints.IsIkTargetEnabled(i))
		{
			str.Printf("%s ON", context.Tree().GetIkTargetName(i));
			context.PrintLine(str);
		}
	}

	context.End();
}
#endif

void C_IkEnableNode::SetInput(C_Node* input)
{
	m_input = input;
}

void C_IkEnableNode::EnableIkTarget(S32 ikTargetId)
{
	m_constraints.EnableIkTarget(ikTargetId);
}

void C_IkEnableNode::DisableIkTarget(S32 ikTargetId)
{
	m_constraints.DisableIkTarget(ikTargetId);
}


//
// C_IkApplyNode
//
C_IkApplyNode::C_IkApplyNode():
	m_input(NULL)
{
}

void C_IkApplyNode::SetPhase(C_Context& context, F32 phase)
{
	if (m_input)
	{
		m_input->SetPhase(context, phase);
	}
}

F32 C_IkApplyNode::GetFrequency(C_Context& context)
{
	if (m_input)
	{
		return m_input->GetFrequency(context);
	}
	return 0.f;
}

void C_IkApplyNode::SaveState(Animation::C_MemoryBlockWriter& out) const
{
	if (m_input)
	{
		m_input->SaveState(out);
	}
}

void C_IkApplyNode::LoadState(Animation::C_MemoryBlockReader& in)
{
	if (m_input)
	{
		m_input->LoadState(in);
	}
}


void C_IkApplyNode::Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight)
{
	if (m_input == NULL) return;
	

	context.commandList.PushCurrentPose(localWeight);

	// Clear target weights for IK chains... 
	context.tree.ClearIkTargetWeights();

	// Evaluate pose.
	m_input->Evaluate(context, Animation::E_BlendMode_Accumulate, localWeight);

	// Set weights for all IK chains.
	// Ideally sorted by chain to minimise SPU DMA transfers.
	for (S32 i = 0, count = context.tree.GetNumIkTargets(); i < count; ++i)
	{
		Animation::C_IkChain* pIkChain = context.tree.GetIkTargetChain(i);
		if (pIkChain)
		{
			U32 targetIndex = context.tree.GetIkTargetIndex(i);
			context.commandList.SetIkTargetWeight(*pIkChain, targetIndex, context.tree.GetIkTargetWeight(i));
		}
	}
	
	// Evaluate all IK chains.
	for (S32 i = 0, count = context.tree.GetNumIkChains(); i < count; ++i)
	{
		context.commandList.EvaluateIkChain(*context.tree.GetIkChain(i));
	}
	
	context.commandList.PopAndBlendPose(blendMode);
}
	
void C_IkApplyNode::SetInput(C_Node* input)
{
	m_input = input;
}

#ifndef CFG_GOLD
void C_IkApplyNode::DebugRender(C_DebugRenderContext& context) const
{
	C_Node::DebugRender(context);
	context.Begin();
	if (m_input != NULL)
	{
		m_input->DebugRender(context);
	}
	context.End();
}
#endif

bool C_IkApplyNode::Accept(C_Visitor& visitor)
{
	if (visitor.Visit(*this))
	{
		if (m_input)
		{
			m_input->Accept(visitor);
		}
	}
	return true;
}

//
// C_MirrorNode
//

C_MirrorNode::C_MirrorNode():
	m_mirroredSkeleton(NULL),
	m_input(NULL)
{
}

void C_MirrorNode::Create(C_Node* input, Animation::E_MirrorAxis mirrorAxis)
{
	m_input = input;
	m_poseMirror.SetMirrorAxis(mirrorAxis);
}

void C_MirrorNode::Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight)
{
	if ( NULL == m_input ) return;
	
	// Evaluate into a temporary pose.
	context.commandList.PushCurrentPose(localWeight);
	m_input->Evaluate(context, Animation::E_BlendMode_Accumulate, localWeight);
	
	if (context.skeleton != m_mirroredSkeleton)
	{
		m_mirroredSkeleton = context.skeleton;
	
		if (context.skeleton != NULL)
		{
			m_poseMirror.Create(*context.skeleton);
		}
		else
		{
			m_poseMirror.Clear();
		}
	}
	
	// Mirror pose, in-place.
	if (context.skeleton)
	{
		context.commandList.MirrorPose(m_poseMirror);
	}
	
	// Blend in result.
	context.commandList.PopAndBlendPose(blendMode);
}

#ifndef CFG_GOLD
void C_MirrorNode::DebugRender(C_DebugRenderContext& context) const
{
	C_Node::DebugRender(context);
	context.Begin();
	
	if (m_input != NULL)
	{
		m_input->DebugRender(context);
	}
	context.End();
}
#endif

bool C_MirrorNode::Accept(C_Visitor& visitor)
{
	if (visitor.Visit(*this))
	{
		if (m_input)
		{
			m_input->Accept(visitor);
		}
	}
	return true;
}

//
// C_TriggerNode
//
C_TriggerNode::C_TriggerNode():
	m_input(0)
{
}
	
void C_TriggerNode::Create(C_Node* input, F32 delayMean, F32 delayVariation, F32 blendTime)
{
	BIZ_ASSERT(input);
	m_input = input;
	m_delay = delayMean;
	m_variation = Maths::Min(delayVariation, m_delay);
	m_state = E_State_Waiting;
	m_timer = m_delay;	// add random variation?
	m_blendTime = blendTime;
	m_updateCounter.Reset();
}

void C_TriggerNode::Reset(F32 unitVariation)
{
	m_state = E_State_Waiting;
	m_timer = m_delay + ((unitVariation * 2.f) - 1.f) * m_variation;
}
	
void C_TriggerNode::Evaluate(C_Context& context, Animation::E_BlendMode blendMode, F32 localWeight)
{
	BIZ_ASSERT(m_input);

	bool activated = false;
	m_updateCounter.Update(context, &activated);
	if (activated)
	{
		Reset(context.tree.FRand());
	}

	F32 blendWeight = 0.f;

	switch (m_state)
	{
		case E_State_Waiting:
			m_timer -= context.deltaTime;
			if (m_timer <= 0.f)
			{
				m_state = E_State_BlendingIn;
				m_timer = 0.f;
			}
			break;

		case E_State_BlendingIn:
			m_timer += context.deltaTime;
			if (m_timer < m_blendTime)
			{
				blendWeight = Maths::SmoothStep(m_timer / m_blendTime);
			}
			else
			{
				m_state = E_State_Playing;
				blendWeight = 1.f;
			}
			break;

		case E_State_Playing:
		{
			F32 inputDuration = Maths::Reciprocal(m_input->GetFrequency(context));
			m_timer += context.deltaTime;
			blendWeight = 1.f;
			if (m_timer > inputDuration - m_blendTime)
			{
				m_state = E_State_BlendingOut;
				m_timer = m_blendTime;
			}
			break;
		}
		case E_State_BlendingOut:
		{
			m_timer -= context.deltaTime;
			blendWeight = Maths::SmoothStep(m_timer / m_blendTime);
			if (m_timer <= 0.f)
			{
				Reset(context.tree.FRand());
			}
			break;
		}
	}

	if (m_state != E_State_Waiting)
	{
		m_input->Evaluate(context, blendMode, localWeight * blendWeight);
	}
}

bool C_TriggerNode::Accept(C_Visitor& visitor)
{
	if (visitor.Visit(*this))
	{
		if (m_input)
		{
			m_input->Accept(visitor);
		}
	}
	return true;
}

} // end AnimationTree

