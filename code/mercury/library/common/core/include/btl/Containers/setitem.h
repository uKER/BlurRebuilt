#ifndef BTL_SETITEM_H
#define BTL_SETITEM_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/utility/pair.h"

namespace Btl
{

/// @addtogroup Containers
/// @{

/////////////////////////////////////////////////////////////////
// Set Item
/////////////////////////////////////////////////////////////////

/// Semi-transparent wrapper for Set elements.
/// Converts implicitly to and from the Key type, so it will usually be invisible.
/// Occasionally it may be necessary to use the functions to convert explicitly. 
template <typename Key, typename Initialiser = NullType>
class SetItem
{
public:
	SetItem() {}
	SetItem(const SetItem& i) : key(i.key) {}
	SetItem(const Key& k) : key(k) {} ///< Implicit construction

	operator const Key& () const {return key;} ///< Can only get a const reference

	const Key& GetKey() const {return key;} ///< Returns a const reference to the key. The key cannot be modified after creation of the %SetItem.
	const Key& GetValue() const {return key;} ///< Return a reference to the value, which is the same as the key.

	void CopyValue(const SetItem& other) {BTL_ASSERT(key == other.key);}

	// comparison operators
	friend bool operator < (const SetItem& a, const SetItem& b) {return a.key < b.key;}
	friend bool operator < (const Key& a, const SetItem& b) {return a < b.key;}
	friend bool operator < (const SetItem& a, const Key& b) {return a.key < b;}

private:
	Key key;
};

/////////////////////////////////////////////////////////////////
// Map Item
/////////////////////////////////////////////////////////////////

/// The type stored in a Map or MultiMap.
/// @remarks
/// The stl uses 'pair' for this. But MapItem gives named functions and protects the key from being modified.
template <typename Key, typename Value, typename Initialiser = NullType>
class MapItem
{
public:
	typedef Key KeyType;
	typedef Value ValueType;

	MapItem() {}
	explicit MapItem(const Key& k) : key(k) {} ///< @a 
	MapItem(const Key& k, const Value& v) : key(k), value(v) {} ///< @a 
	MapItem(const MapItem& i) : key(i.key), value(i.value) {}

	/// Returns a reference to the value.
	Value& GetValue() {return value;} 

	/// Returns a const reference to the key. The key cannot be modified after creation of the %MapItem.
	const Key& GetKey() const {return key;} 
	
	/// Returns a reference to the value.
	const Value& GetValue() const {return value;} 

	void CopyValue(const MapItem& other) {BTL_ASSERT(key == other.key); value = other.value;}

	// comparison operators for tree
	friend bool operator < (const MapItem& a, const MapItem& b) {return a.key < b.key;}
	friend bool operator < (const Key& a, const MapItem& b) {return a < b.key;}
	friend bool operator < (const MapItem& a, const Key& b) {return a.key < b;}

	// Equality requires both key and value to be equal.
	// Unusually, two non-equal items can have neither one less than the other.
	bool operator == (const MapItem& other) const
	{
		return (key == other.key && value == other.value);
	}

	bool operator != (const MapItem& other) const
	{
		return !(key == other.key && value == other.value);
	}

private:
	Key key;
	Value value;
};

/// Result of inserting into a set or a map.
template <typename Iterator>
struct InsertionResult
{
	InsertionResult() {}
	InsertionResult(const Iterator& i, bool inserted_) : iterator(i), inserted(inserted_) {}

	// Iterator to the element.
	Iterator iterator;

	/// True if an insertion was made.
	bool inserted;
};

template <typename Iterator>
struct MultipleElements
{
	static const bool MULTIPLE = true;
	typedef Iterator InsertionReturnType;
	static InsertionReturnType MakeInsertionReturnType(Iterator i, bool) {return i;}
	static Iterator GetIteratorFromInsertionReturnType(const InsertionReturnType& i) {return i;}
};

template <typename Iterator>
struct UniqueElements
{
	static const bool MULTIPLE = false;
	typedef InsertionResult<Iterator> InsertionReturnType;
	static InsertionReturnType MakeInsertionReturnType(Iterator i, bool b) {return InsertionReturnType(i, b);}
	static Iterator GetIteratorFromInsertionReturnType(const InsertionReturnType& i) {return i.first;}
};

/// @}

}

#endif
