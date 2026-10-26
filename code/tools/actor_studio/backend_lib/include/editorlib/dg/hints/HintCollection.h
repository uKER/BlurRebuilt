#ifndef EDITOR_LIB_HINT_COLLECTION_H
#define EDITOR_LIB_HINT_COLLECTION_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <Btl/Containers/Vector.h>
#include <EditorLib/Dg/Hints/IUpdateHint.h>
#include <EditorLib/Dg/Hints/IImmediateHint.h>

namespace EditorLib
{
	class I_DgNode;

	template<typename HINT>
	struct C_HintCollection
	{
		C_HintCollection(I_DgNode* pNode_) : pNode(pNode_) { BIZ_ASSERT(pNode); }
		~C_HintCollection() 
		{ 
			for (S32 i = 0; i < hints.Size(); ++i)
				SAFE_DELETE_PTR(hints[i]);
		}

		I_DgNode*			pNode;
		Btl::Vector<HINT*>	hints;
	};

	struct C_UpdateHintCollection : C_HintCollection<I_UpdateHint>
	{
		C_UpdateHintCollection(I_DgNode* pNode_) : C_HintCollection<I_UpdateHint>(pNode_) {}
	};

	struct C_ImmediateHintCollection : C_HintCollection<I_ImmediateHint>
	{
		C_ImmediateHintCollection(I_DgNode* pNode_) : C_HintCollection<I_ImmediateHint>(pNode_) {}
	};
}

#endif
