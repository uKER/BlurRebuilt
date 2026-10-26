#ifndef EDITOR_DGNODE_SELECTION_H
#define EDITOR_DGNODE_SELECTION_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Editor
{
	class C_DgNodeSelection
	{
	public:

		struct C_Item
		{
			EditorLib::I_DgNode*	pDgNode;
			Ut::Vector<U32>			components;
		};

		typedef Ut::UtVector<Ut::DynamicVector<C_Item> > ItemArray;

		C_DgNodeSelection();
		~C_DgNodeSelection();

		void Add(EditorLib::I_DgNode* pNode);

		void Add(EditorLib::I_DgNode* pNode, const Ut::Vector<U32>& components);

		void Remove(const EditorLib::I_DgNode* pNode);

		void Remove(const EditorLib::I_DgNode* pNode, const Ut::Vector<U32>& components);

		void Clear();

		const C_Item* Find(const EditorLib::I_DgNode* pNode) const;

		const C_Item* Find(const EditorLib::I_DgNode* pNode, U32 component) const;

		const ItemArray& GetItems() const;

	private:
		C_DgNodeSelection(const C_DgNodeSelection& selection);
		const C_DgNodeSelection& operator=(const C_DgNodeSelection& selection);

		typedef Ut::UtMap<const EditorLib::I_DgNode*, U32> ItemMap;

		C_Item* _Find(const EditorLib::I_DgNode* pNode);

		ItemArray m_itemArray;
		mutable ItemMap m_itemMap;
	};
}

#endif
