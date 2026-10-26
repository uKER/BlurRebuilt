#ifndef EDITOR_LIB_SELECTION_H
#define EDITOR_LIB_SELECTION_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_Selection
	{
	public:
		enum E_Mode
		{
			E_Mode_Unknown,
			E_Mode_Object,
			E_Mode_Component,

			E_Mode_Count
		};

		struct C_Item
		{
			C_NodeId id;
			Ut::Vector<U32> components;
		};

		typedef Ut::UtVector<Ut::DynamicVector<C_Item> > ItemArray;
		typedef Ut::UtMap<C_NodeId, U32> ItemMap;

		C_Selection();
		~C_Selection();
		C_Selection(const C_Selection& selection);
		const C_Selection& operator=(const C_Selection& selection);

		bool operator==(const C_Selection& rhs) const;

		bool operator!=(const C_Selection& rhs) const;

		void Add(E_Mode mode, C_NodeId id);

		void Add(E_Mode mode, C_NodeId id, const Ut::Vector<U32>& components);

		void Toggle(E_Mode mode, C_NodeId id);

		void Toggle(E_Mode mode, C_NodeId id, const Ut::Vector<U32>& components);

		void Remove(E_Mode mode, C_NodeId id);

		void Remove(E_Mode mode, C_NodeId id, const Ut::Vector<U32>& components);

		void Clear();

		const C_Item* Find(C_NodeId id) const;

		const C_Item* Find(C_NodeId id, U32 component) const;

		const ItemArray& GetItems() const;

		E_Mode GetMode() const;

	private:
		C_Item* _Find(C_NodeId id);

		E_Mode m_mode;
		ItemArray m_itemArray;
		mutable ItemMap m_itemMap;
	};
}

#endif
