// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A templated set class, which guarantees that each item is unique.

#ifndef BD_SET_H
#define BD_SET_H

// The set is implemented as a red-black tree.
#include <bdCore/bdContainers/bdRedBlackTree.h>

/// A container that guarantees that each item in it is unique.
/// A red black tree is used to ensure uniqueness.
template <typename T, typename LESS_THAN_FUNCTOR = bdLessThan<T> >
class bdSet
{
	public:

		/// Provides an iterator for use with the
		///  getIterator() and forward() methods.
		typedef bdRedBlackTreeIterator<T,LESS_THAN_FUNCTOR> Iterator;

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		inline bdSet();

		/// Copy constructor.
		inline bdSet(const bdSet& other);

		/// Destructor.
		inline ~bdSet();

		/// Get the number of elements in the set.
		/// \return The number of elements in the set.
		inline bdUInt getSize() const;

		/// Add an object into the set.
		/// This will attempt to add an object into the set. If \a object is
		/// already in the set, it will not be added again.
		/// \param	object[in]	The object to add to the set.
		/// \return
		/// -	True, if the object was inserted
		/// -	False, if the object was already in the set and was not
		///		inserted.
		inline bdBool insert(const T& object);

		/// Remove an object from the set.
		/// \param object[in]	The object to remove from the set.
		/// \return
		/// -	True, if the object existed in the set and was removed.
		/// -	False, if the object was not in the set.
		inline bdBool remove(const T& object);

		/// Determine whether an object is in the set.
		/// \param object[in]	The object to find.
		/// \return
		/// -	True, if the object was in the set.
		/// -	False, if the object was not in the set.
		inline bdBool find(const T& object) const;

		/// Get an iterator for the set.
		/// \return	An Iterator that can be used in conjunction with getAt()
		///			and forward().
		/// \warning	Do not modify the set with insert(), remove() or
		///				clear() whilst iterating.
		inline Iterator getIterator() const;

		/// Releases an iterator. This is used for debugging purposes in
		/// order to detect simultaneous iteration and modification bugs.
		/// \param iterator[in] The iterator to be released.
		void releaseIterator(Iterator iterator) const;

		/// Get an element identified by an \c Iterator from the set.
		/// \param iterator[in]	The \c Iterator to use.
		/// \return	The element that the iterator points to.
		inline const T& getAt(Iterator &iterator) const;

		/// Get an element identified by an \c Iterator, and advance the
		/// \c Iterator to point to the next element.
		/// \param iterator[in,out]	The iterator to advance.
		/// \return	The element pointed to by the iterator, prior to the
		///			iterator being advanced.
		inline const T& next(Iterator &iterator) const;

		/// Remove all elements from the set.
		inline void clear();

		/// Compare two sets for differences.
		/// This method compares two sets, \a a and \a b, and returns their
		/// differences in \a diffA and \a diffB respectively. Upon return,
		/// \a diffA will contain the elements that were in \a a but not in
		/// \a b, and \a diffB will contain the elements that were in \a b, but
		/// not in \a a.
		/// \param	a[in]			A set to compare for differences.
		/// \param	b[in]			A set to compare for differences.
		/// \param	diffA[out]	Will receive elements that are in \a a but
		///							not in \a b.
		/// \param	diffB[out]	Will receive elements that are in \a b but
		///							not in \a a.
		static void BD_CALL compare(const bdSet<T,LESS_THAN_FUNCTOR> &a,
									const bdSet<T,LESS_THAN_FUNCTOR> &b,
									bdSet<T,LESS_THAN_FUNCTOR> &diffA,
									bdSet<T,LESS_THAN_FUNCTOR> &diffB);

	protected:

#ifdef BD_DEBUG
		/// Counts how many iterators are currently being used to iterate
		/// this set. We will prevent insertion/deletion as long as there
		/// is more than 1 iterator being held at the same time.
		bdUInt m_numIterators;
#endif // BD_DEBUG

		/// A Red Black Tree is used to represent the set.
		bdRedBlackTree<T,LESS_THAN_FUNCTOR> m_tree;

};

#include <bdCore/bdContainers/bdSet.inl>

#endif // BD_SET_H

