#ifndef DMAN_SELECTION_H
#define DMAN_SELECTION_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace DMan
{
	class C_Selection
	{
	public:
		enum E_Mode
		{
			E_Mode_Unknown,
			E_Mode_Pack,
			E_Mode_Record,
			E_Mode_Row,

			E_Mode_Count
		};

		struct C_Item
		{
			C_DomId id;
			Btl::Vector<U32> components;
		};

		typedef Btl::Vector<C_Item> ItemArray;

		C_Selection();
		~C_Selection();
		C_Selection(const C_Selection& selection);
		const C_Selection& operator=(const C_Selection& selection);

		bool operator==(const C_Selection& rhs) const;

		bool operator!=(const C_Selection& rhs) const;

		void Add(E_Mode mode, const C_DomId& id);

		void Add(E_Mode mode, const C_DomId& id, const Btl::Vector<U32>& components);

		void Toggle(E_Mode mode, const C_DomId& id);

		void Toggle(E_Mode mode, const C_DomId& id, const Btl::Vector<U32>& components);

		void Remove(E_Mode mode, const C_DomId& id);

		void Remove(E_Mode mode, const C_DomId& id, const Btl::Vector<U32>& components);

		void Clear();

		const C_Item* Find(const C_DomId& id) const;

		const C_Item* Find(const C_DomId& id, U32 component) const;

		const ItemArray& GetItems() const;

		E_Mode GetMode() const;

		C_RowSet* GetCurrentRowSet() const;

	private:
		typedef Btl::Map<C_DomId, U32> ItemMap;

		C_Item* _Find(const C_DomId& id);

		E_Mode			m_mode;
		ItemArray		m_itemArray;
		mutable ItemMap m_itemMap;
	};
}

#endif //DMAN_SELECTION_H
