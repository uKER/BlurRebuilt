#ifndef MISCSHARED_UID_TYPETABLE_H
#define MISCSHARED_UID_TYPETABLE_H

#include <btl/allocators/pool.h>
#include <btl/utility/pair.h>



namespace UID
{

/**
@ingroup uid_internal.
@brief Chained entry in C_TypeTable.
*/
template <typename T_UID, typename T_Data>
struct C_TypeTableEntry
{
	typedef T_UID C_UID;
	typedef T_Data C_Data;

	C_UID				m_Uid;		///< UID for this entry
	C_TypeTableEntry*	m_pNext;	///< weak ptr, next entry sharing the same hash value
	C_Data				m_Data;		///< data for this entry
};

/**
@ingroup uid_internal.
@brief Base class for C_TypeTable iterators.
*/
template <class T_Entry, size_t t_Size>
class C_TypeTableIteratorBase
{
public:
	typedef T_Entry C_Entry;
	typedef typename T_Entry::C_UID C_UID;
	typedef typename T_Entry::C_Data C_Data;

	inline C_UID GetUID() const {return GetEntry().m_Uid;} ///< @returns Unique identifier for the current item if the iterator is valid.

	inline bool operator == (const C_TypeTableIteratorBase& other) const;
	inline bool operator != (const C_TypeTableIteratorBase& other) const;

protected:
	inline C_TypeTableIteratorBase();
	inline C_TypeTableIteratorBase(const C_TypeTableIteratorBase& copy);
	inline C_TypeTableIteratorBase(C_Entry* pEntry, size_t hash, C_Entry* const* ppTable);

	inline ~C_TypeTableIteratorBase();

	inline C_TypeTableIteratorBase& operator = (const C_TypeTableIteratorBase& copy);
	
	inline C_TypeTableIteratorBase& operator ++ ();
	inline C_TypeTableIteratorBase& operator += (size_t count);

	inline C_Entry& GetEntry() const; ///< @returns Current item if the iterator is valid.

private:
	C_Entry*		m_pEntry;	///< weak ptr, entry in chained hash table
	size_t			m_Hash;		///< hash value for m_pEntry
	C_Entry* const*	m_ppTable;	///< weak ptr, chained hash table

};

/**
@ingroup uid_internal.
@brief Writable iterator for C_TypeTable.
*/
template <class T_Entry, size_t t_Size>
class C_TypeTableIterator : public C_TypeTableIteratorBase<T_Entry, t_Size>
{
	typedef C_TypeTableIteratorBase<T_Entry, t_Size> C_Super;

public:
	typedef C_TypeTableIterator<T_Entry, t_Size> Iterator;

	inline C_TypeTableIterator() : C_Super() {}
	inline C_TypeTableIterator(const Iterator& copy) : C_Super(copy) {}
	inline C_TypeTableIterator(T_Entry* pEntry, size_t hash, T_Entry* const* ppTable) : C_Super(pEntry, hash, ppTable) {}

	inline ~C_TypeTableIterator() {}
	
	inline typename T_Entry::C_Data& GetData() const {return C_Super::GetEntry().m_Data;} ///< @returns Data for the current item if the iterator is valid.

	inline Iterator& operator = (const Iterator& copy) {return static_cast<Iterator&>(C_Super::operator = (copy));}

	inline Iterator& operator ++ () {return static_cast<Iterator&>(C_Super::operator ++ ());}
	inline const Iterator operator ++ (int dummy);

	inline Iterator& operator += (size_t count) {return static_cast<Iterator&>(C_Super::operator += (count));}
	inline const Iterator operator + (size_t count) const;

};

/**
@ingroup uid_internal.
@brief Read-only iterator for C_TypeTable.
*/
template <class T_Entry, size_t t_Size>
class C_TypeTableIteratorConst : public C_TypeTableIteratorBase<T_Entry, t_Size>
{
	typedef C_TypeTableIteratorBase<T_Entry, t_Size> C_Super;

public:
	typedef C_TypeTableIterator<T_Entry, t_Size> Iterator;
	typedef C_TypeTableIteratorConst<T_Entry, t_Size> ConstIterator;

	inline C_TypeTableIteratorConst() : C_Super() {}
	inline C_TypeTableIteratorConst(const ConstIterator& copy) : C_Super(copy) {}
	inline C_TypeTableIteratorConst(const Iterator& copy) : C_Super(copy) {}
	inline C_TypeTableIteratorConst(const T_Entry* pEntry, size_t hash, T_Entry* const* ppTable) : C_Super(pEntry, hash, ppTable) {}

	inline ~C_TypeTableIteratorConst() {}
	
	inline const typename T_Entry::C_Data& GetData() const {return C_Super::GetEntry().m_Data;} ///< @returns Data for the current item if the iterator is valid.

	inline ConstIterator& operator = (const ConstIterator& copy) {return static_cast<ConstIterator&>(C_Super::operator = (copy));}
	inline ConstIterator& operator = (const Iterator& copy) {return static_cast<ConstIterator&>(C_Super::operator = (copy));}

	inline ConstIterator& operator ++ () {return static_cast<ConstIterator&>(C_Super::operator ++ ());}
	inline const ConstIterator operator ++ (int dummy);

	inline ConstIterator& operator += (size_t count) {return static_cast<ConstIterator&>(C_Super::operator += (count));}
	inline const ConstIterator operator + (size_t count) const;

};

/// Standard T_HashPolicy for C_TypeType.
template <typename T_UID, size_t t_HashSize>
struct C_TypeTableHashPolicyDefault
{
	static const size_t kHashSize = t_HashSize;
	inline static size_t CalcHash(const T_UID uid);
};

/// T_HashPolicy for C_TypeTable where UIDs are seeded from FOURCC_HI20 values.
template <typename T_UID, size_t t_HashSize>
struct C_TypeTableHashPolicyHi20
{
	static const size_t kHashSize = t_HashSize;
	inline static size_t CalcHash(const T_UID uid);
};

template <typename T_UID, typename T_Data, class T_HashPolicy, class T_Allocator = Btl::DynamicPool< C_TypeTableEntry<T_UID, T_Data> > >
class C_TypeTable
{
	typedef C_TypeTableEntry<T_UID, T_Data> C_Entry;
	static const size_t kHashSize = T_HashPolicy::kHashSize;

public:
	typedef T_UID C_UID;
	typedef T_Data C_Data;

	typedef C_TypeTableIterator<C_Entry, kHashSize> Iterator;
	typedef C_TypeTableIteratorConst<C_Entry, kHashSize> ConstIterator;
	typedef Btl::Pair<bool, Iterator> InsertionResult;

	C_TypeTable();
	~C_TypeTable();

	void						Clear(); ///< empties the container.

	inline size_t				Size() const {return m_Size;} ///< @returns number of entries in the container.
	inline bool					Empty() const {return (m_Size == 0);} ///< @returns true if there are no entries in the container.

	InsertionResult				Insert(C_UID uid); ///< adds an entry if the UID is not already used.
	const Iterator				Erase(const Iterator& it); ///< removes the entry.

	inline const ConstIterator	Begin() const {return BeginImpl();}
	inline const ConstIterator	End() const {return EndImpl();}
	inline const ConstIterator	Find(C_UID uid) const {return FindImpl(uid);}

	inline const Iterator		Begin() {return BeginImpl();}
	inline const Iterator		End() {return EndImpl();}
	inline const Iterator		Find(C_UID uid) {return FindImpl(uid);}

private:
	T_Allocator					m_Allocator;			///< allocator for entries
	size_t						m_Size;					///< number of entries
	C_Entry*					m_pTable [kHashSize];	///< strong ptrs, chained hash table

	C_TypeTable(const C_TypeTable& copy); // can not copy
	C_TypeTable& operator = (const C_TypeTable& copy); // can not copy

	const Iterator				BeginImpl() const;
	const Iterator				EndImpl() const;
	const Iterator				FindImpl(C_UID uid) const;

};

} // namespace UID


#include <misc_shared/UID/TypeTable.inl>

#endif //  MISCSHARED_UID_TYPETABLE_H
