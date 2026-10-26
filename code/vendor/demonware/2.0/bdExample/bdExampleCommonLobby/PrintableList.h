// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef PRINTABLE_LIST_H
#define PRINTABLE_LIST_H

template <typename T, bdUInt N>
class PrintableList
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		PrintableList();

		~PrintableList();

		bdBool add(const T& entry);

		void clear();

		bdUInt getSize() const;

		T& operator[] (const bdUInt i);

		void print(bdString& string) const;

	protected:

		T m_entries[N];
		bdUInt m_size;
};

#include <bdExample/bdExampleCommonLobby/PrintableList.inl>

#endif // PRINTABLE_LIST_H
