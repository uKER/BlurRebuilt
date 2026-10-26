#ifndef EDITOR_LIB_CLIPBOARD_H
#define EDITOR_LIB_CLIPBOARD_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_Clipboard
	{
	public:

		struct C_Item
		{
			C_NodeId id;
			//Ut::Vector<U32> components;
		};

		typedef Ut::UtVector<Ut::DynamicVector<C_Item> > ItemArray;

		C_Clipboard();
		~C_Clipboard();
		C_Clipboard(const C_Clipboard& clipboard);
		const C_Clipboard& operator=(const C_Clipboard& clipboard);

		bool operator==(const C_Clipboard& rhs) const;

		bool operator!=(const C_Clipboard& rhs) const;

		void Add(C_NodeId id);
		void Remove(C_NodeId id);
		void Clear();

		const C_Item* Find(C_NodeId id) const;

		const ItemArray& GetItems() const;

	private:
		C_Item* _Find(C_NodeId id);

		ItemArray m_itemArray;

	};
}

#endif
