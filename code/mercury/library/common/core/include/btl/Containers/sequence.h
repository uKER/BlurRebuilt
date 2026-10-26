#ifndef BTL_SEQUENCE_H
#define BTL_SEQUENCE_H
#ifdef _MSC_VER
#pragma once
#endif

/////////////////////////////////////////////////////
// Sequence
/////////////////////////////////////////////////////

#include "btl/utility/types.h"

namespace Btl 
{

/// @addtogroup Containers
/// @{

/////////////////////////////////////////////////////

// Template for sequences.
// interface is functions below.
template <class T>
class Sequence_t
{ 
public:
	typedef T Iterator;

	const Iterator& Begin() const {return m_begin;}
	const Iterator& End() const {return m_end;}

	// Don't use this, it is only public because the templates below can't be friend functions
	Sequence_t(const Iterator& begin, const Iterator& end) : m_begin(begin), m_end(end) {}

private:
	Iterator m_begin;
	Iterator m_end;
};

/// A sequence of elements in a container.
/// Used to pass to algorithms such as ForEach.
/// @param begin The start of the sequence.
/// @param end The end of the sequence.
template <class Iterator>
Sequence_t<Iterator> Sequence(const Iterator& begin, const Iterator& end)
{
	return Sequence_t<Iterator>(begin, end);
}

/// A sequence of elements in a container.
/// Used to pass to algorithms such as ForEach.
/// @param c The container.
/// @remarks 
/// A container can also be used directly as a sequence of its own elements, so this is redundant except for
/// consistency with other sequence functions. However, it does guarantee that End() is called only once
/// when stepping through the sequence.
template <class Container>
Sequence_t<typename Container::Iterator> Sequence(Container& c)
{
	return Sequence_t<typename Container::Iterator>(c.Begin(), c.End());
}

/// A const sequence of elements in a container.
/// Used to pass to algorithms such as ForEach.
/// @param c The container.
template <class Container>
Sequence_t<typename Container::ConstIterator> ConstSequence(const Container& c)
{
	return Sequence_t<typename Container::ConstIterator>(c.Begin(), c.End());
}

/// A reverse sequence of elements in a container.
/// Used to pass to algorithms such as ForEach.
/// @param c The container.
template <class Container>
Sequence_t<typename Container::ReverseIterator> ReverseSequence(Container& c)
{
	return Sequence_t<typename Container::ReverseIterator>(c.ReverseBegin(), c.ReverseEnd());
}

/// A reverse const sequence of elements in a container.
/// Used to pass to algorithms such as ForEach.
/// @param c The container.
template <class Container>
Sequence_t<typename Container::ConstReverseIterator> ConstReverseSequence(const Container& c)
{
	return Sequence_t<typename Container::ConstReverseIterator>(c.ReverseBegin(), c.ReverseEnd());
}

/////////////////////////////////////////////////////
// Reverse Iterator templates

template <typename T, class Container, class ConstructFrom = T*>
class ReverseIterator_t;

// wrap iterator to run backwards
template <typename T, class Container, class ConstructFrom = T*>
class ConstReverseIterator_t
{
public:
	ConstReverseIterator_t() {}
	ConstReverseIterator_t(const ConstReverseIterator_t& c) : iterator(c.iterator) {}
	ConstReverseIterator_t& operator = (const ConstReverseIterator_t& c) {iterator = c.iterator; return *this;}
	ConstReverseIterator_t(ReverseIterator_t<T, Container, ConstructFrom>& i);

	bool operator == (const ConstReverseIterator_t& c) const {return iterator == c.iterator;}
	bool operator != (const ConstReverseIterator_t& c) const {return iterator != c.iterator;}
	bool operator < (const ConstReverseIterator_t& c) {return iterator < c.iterator;}

	const T& operator*() const {return *iterator;}
	const typename Container::ConstIterator& operator->() const {return iterator;}
	const ConstructFrom Pointer() {return iterator.Pointer();}
	typename Container::ConstIterator& Iterator() {return iterator;}

	// arithmetic
	ConstReverseIterator_t operator + (int n) const {return iterator - n;}
	ConstReverseIterator_t operator - (int n) const {return iterator + n;}
	int operator - (const ConstReverseIterator_t& c) const {return -(iterator - c.iterator);}

	ConstReverseIterator_t& operator ++() {--iterator; return *this;}
	ConstReverseIterator_t& operator --() {++iterator; return *this;}
	ConstReverseIterator_t operator ++(int) {ConstReverseIterator_t p(*this); --iterator; return p;}
	ConstReverseIterator_t operator --(int) {ConstReverseIterator_t p(*this); ++iterator; return p;}

	explicit ConstReverseIterator_t(const ConstructFrom p) : iterator(p) {}
	explicit ConstReverseIterator_t(const ConstructFrom p, const Container* c) : iterator(p, c) {}
	explicit ConstReverseIterator_t(int i, const Container* c) : iterator(i, c) {}

private:
	typename Container::ConstIterator iterator;
};

template <typename T, class Container, class ConstructFrom>
class ReverseIterator_t
{
	typedef ConstReverseIterator_t<T, Container, ConstructFrom> ConstReverseIterator;
public:
	ReverseIterator_t() {}
	ReverseIterator_t(const ReverseIterator_t& c) : iterator(c.iterator) {}
	ReverseIterator_t& operator = (const ReverseIterator_t& c) {iterator = c.iterator; return *this;}

	bool operator == (const ReverseIterator_t& c) const {return iterator == ((ReverseIterator_t&) c).Iterator();}
	bool operator != (const ReverseIterator_t& c) const {return iterator != ((ReverseIterator_t&) c).Iterator();}
	bool operator < (const ReverseIterator_t& c) {return iterator < ((ReverseIterator_t&) c).Iterator();}

	bool operator == (const ConstReverseIterator& c) const {return iterator == ((ReverseIterator_t&) c).Iterator();}
	bool operator != (const ConstReverseIterator& c) const {return iterator != ((ReverseIterator_t&) c).Iterator();}
	bool operator < (const ConstReverseIterator& c) {return iterator < ((ReverseIterator_t&) c).Iterator();}

	T& operator*() {return *iterator;}
	typename Container::Iterator& operator->() {return iterator;}
	ConstructFrom Pointer() {return iterator.Pointer();}
	typename Container::Iterator& Iterator() {return iterator;}

	// arithmetic
	ReverseIterator_t operator + (int n) const {return iterator - n;}
	ReverseIterator_t operator - (int n) const {return iterator + n;}
	
	ReverseIterator_t& operator ++() {--iterator; return *this;}
	ReverseIterator_t& operator --() {++iterator; return *this;}
	ReverseIterator_t operator ++(int) {ReverseIterator_t p(*this); --iterator; return p;}
	ReverseIterator_t operator --(int) {ReverseIterator_t p(*this); ++iterator; return p;}

	explicit ReverseIterator_t(ConstructFrom p) : iterator(p) {}
	explicit ReverseIterator_t(ConstructFrom p, Container* c) : iterator(p, c) {}
	explicit ReverseIterator_t(int i, Container* c) : iterator(i, c) {}

private:
	typename Container::Iterator iterator;
};

template <typename T, class Container, class ConstructFrom>
ConstReverseIterator_t<T, Container, ConstructFrom>::ConstReverseIterator_t(ReverseIterator_t<T, Container, ConstructFrom>& i)
{
	iterator = (typename Container::ConstIterator&) i.Iterator();
}

//////////////////////////////////////////////////////
// Counter: maintained or on demand.

class OnDemandCounter;

/// Counter policy class for list. 
/// Keeps count as elements are added and removed.
class MaintainedCounter
{
public:
	MaintainedCounter() : count(0) {}
	FORCE_INLINE void AddElement() {count++;}
	FORCE_INLINE void EraseElement() {count--; BTL_ASSERT(count >= 0);}
	FORCE_INLINE void Reset() {count = 0;}
	FORCE_INLINE void Add(int n) {count += n;}
	FORCE_INLINE void Subtract(int n) {count -= n; BTL_ASSERT(count >= 0);}

	// ignores pointer
	template <typename Container>
	FORCE_INLINE int Count(const Container& c) const {return count;} 

	template <typename Container> void Merge(MaintainedCounter& other, const Container& c);
	template <typename Container> void Merge(OnDemandCounter& other, const Container& c);

private:
	int count;
};

/// Counter policy class for list. 
/// Counts list elements when required.
class OnDemandCounter
{
public:
	// do nothing
	void AddElement() {}
	void EraseElement() {}
	void Reset() {}
	void Add(int n) { n;}
	void Subtract(int n) {}

	// get count
	template <class Container>
	int Count(const Container& c) const
	{
		int count = 0;
		
		typename Container::ConstIterator i = c.Begin();
		typename Container::ConstIterator e = c.End();

		while (i != e) {++i; count++;}

		return count;
	}

	template <typename Container> void Merge(MaintainedCounter& other, const Container& c) {}
	template <typename Container> void Merge(OnDemandCounter& other, const Container& c) {}
};

template <typename Container>
inline void MaintainedCounter::Merge(MaintainedCounter& other, const Container& c)
{
	count += other.count;
	other.count = 0;
}

template <typename Container>
inline void MaintainedCounter::Merge(OnDemandCounter& other, const Container& c)
{
	count += other.Count(c);
}

/// @}

} // end Ut

#endif
