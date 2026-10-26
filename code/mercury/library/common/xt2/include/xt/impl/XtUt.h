#ifndef BIZARRE_XT_UT_H
#define BIZARRE_XT_UT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace XtUt
{

	// XtUt::Block

	struct Block
	{
		XtU8* start;
		XtU8* cursor;
		XtU8* end;

	public:
		inline Block() :
			start(0),
			cursor(0),
			end(0)
		{
		}

		inline Block(XtU8* start_, XtU8* cursor_, XtU8* end_) :
			start(start_),
			cursor(cursor_),
			end(end_)
		{
		}

		inline Block(XtU8* start_, size_t capacity) :
			start(start_),
			cursor(start_),
			end(start_ + capacity)
		{
		}

		inline ~Block()
		{
			delete [] start;
		}
	};

	// XtUt::WeakBlock

	struct WeakBlock : public Block
	{
		inline WeakBlock() :
			Block()
		{
		}

		inline WeakBlock(XtU8* start_, XtU8* cursor_, XtU8* end_) :
			Block(start_, cursor_, end_)
		{
		}

		inline WeakBlock(XtU8* start_, size_t capacity) :
			Block(start_, capacity)
		{
		}

		inline ~WeakBlock()
		{
			start = 0;
		}
	};

	// XtUt::Container

	template<typename Type, typename Behaviour> struct ContainerIterator;
	template<typename Type, typename Behaviour> struct ConstContainerIterator;

	template<typename Type, typename Behaviour>
	struct Container
	{
		Type*	elements;
		size_t	count;
		size_t	capacity;

	public:
		typedef Container<Type, Behaviour>				ContainerType;
		typedef Type									ElementType;
		typedef ContainerIterator<Type, Behaviour>		Iterator;
		typedef ConstContainerIterator<Type, Behaviour>	ConstIterator;

	public:
		inline Container() :
			elements(0),
			count(0),
			capacity(0)
		{
		}

		inline ~Container()
		{
			FreeElements(this);
		}

	public:
		inline Type&		operator[](size_t index)		{ assert(index < count); return elements[index]; }
		inline const Type&	operator[](size_t index) const	{ assert(index < count); return elements[index]; }

	private:
		Container(const Container<Type, Behaviour>&);
		void operator=(const Container<Type, Behaviour>&);
	};

	// XtUt::Array

	template<typename Type>
	struct ArrayBehaviour
	{
		static inline Type* Allocate(size_t capacity)
		{
			return XtUt::AllocateAsBytes<Type>(capacity);
		}

		static inline Type* Construct(Type* object, size_t count)
		{
			return XtUt::ConstructN<Type>(object, count);
		}

		static inline Type* CopyConstruct(Type* object, const Type& toCopy)
		{
			return XtUt::Construct<Type>(object, toCopy);
		}

		static inline void Dispose(const Type* object, size_t count)
		{
		}

		static inline void Destruct(Type* object, size_t count)
		{
			XtUt::DestructN<Type>(object, count);
		}

		static inline void Free(Type* object)
		{
			XtUt::FreeAsBytes<Type>(object);
		}

		static inline void Abandon(Type* elements, size_t index, size_t count)
		{
		}
	};

	template<typename Type>
	struct Array : public Container< Type, ArrayBehaviour< Type > >
	{
	};

	// XtUt::List

	template<typename Type>
	struct ListBehaviour : public ArrayBehaviour<Type*>
	{
		static inline void Dispose(Type*const* object, size_t count)
		{
			for(size_t i = 0; i < count; i++)
				delete object[i];
		}

		static inline void Abandon(Type** elements, size_t index, size_t count)
		{
			Free(elements);
		}
	};

	template<typename Type> struct ListIterator;

	template<typename Type>
	struct List : public Container< Type*, ListBehaviour< Type > >
	{
		typedef ListIterator<Type> Iterator;
	};

	// XtUt::Set

	template<typename Type, typename Pred> struct SetIterator;

	template<typename Type, typename Pred = LessThan<Type> >
	struct Set
	{
		Array<Type>	elements;
		Pred		predicate;

	public:
		typedef SetIterator<Type, Pred> Iterator;

	public:
		inline Set(Pred predicate_ = Pred()) :
			predicate(predicate_)
		{
		}

		inline ~Set()
		{
			Free(this);
		}

	public:
		inline Type* operator[](const Type& value)
		{
			return At(this, value);
		}

	private:
		Set(const Set<Type, Pred>&);
		void operator=(const Set<Type, Pred>&);
	};

	// XtUt::Dictionary

	template<typename Kt, typename Vt, typename Pred = LessThan<Kt> > struct DictionaryIterator;

	template<typename Kt, typename Vt, typename Pred = LessThan<Kt> >
	struct Dictionary
	{
	public:
		typedef Kt									KeyType;
		typedef Vt									ValueType;
		typedef DictionaryIterator<Kt, Vt, Pred>	Iterator;

	public:
		struct Pair
		{
			KeyType				key;
			mutable ValueType*	value;

		public:
			inline Pair() :
				key(),
				value()
			{
			}

			inline Pair(const Pair& pair) :
				key(pair.key),
				value(pair.value)
			{
				pair.value = 0;
			}

			inline explicit Pair(const KeyType& key_) :
				key(key_),
				value()
			{
			}

			inline Pair(const KeyType& key_, ValueType* value_) :
				key(key_),
				value(value_)
			{
			}

			inline ~Pair()
			{
				delete value;
			}

		public:
			inline operator const KeyType&() const
			{
				return key;
			}

		public:
			inline void operator=(const Pair& pair)
			{
				assert(value == 0);

				key			= pair.key;
				value		= pair.value;

				pair.value	= 0;
			}
		};

	public:
		Set<Pair, Pred> pairs;

	public:
		inline Dictionary(Pred predicate_ = Pred()) :
			pairs(predicate_)
		{
		}

	public:
		inline ValueType* operator[](const KeyType& key)
		{
			return At(this, key);
		}

	private:
		Dictionary(const Dictionary<KeyType, ValueType, Pred>&);
		void operator=(const Dictionary<KeyType, ValueType, Pred>&);
	};

	// XtUt::Ptr

	template<typename Type>
	struct Ptr
	{
		mutable Type* object;

	public:
		inline Ptr(Type* object_ = 0) :
			object(object_)
		{
		}

		inline Ptr(const Ptr<Type>& ptr) :
			object(ptr.object)
		{
			ptr.object = 0;
		}

		inline ~Ptr()
		{
			delete object;
		}

	public:
		inline Ptr<Type>& operator=(const Ptr<Type>& ptr)
		{
			if (this != &ptr)
			{
				delete object;

				object		= ptr.object;
				ptr.object	= 0;
			}

			return *this;
		}

	public:
		inline Type*		operator->()		{ return object; }
		inline const Type*	operator->() const	{ return object; }

	public:
		inline operator Type*()				{ return object; }
		inline operator const Type*() const	{ return object; }
	};

	// XtUt::Link

	template<typename Type> struct LinkList;

	template<typename Type>
	struct Link
	{
		Type*		object;
		Link<Type>*	next;
		Link<Type>*	prev;

	public:
		inline Link(Type* object_) :
			object(object_),
			next(this),
			prev(this)
		{
			assert(object != 0);
		}

		inline Link(Type* object_, LinkList<Type>* list) :
			object(object_),
			next(this),
			prev(this)
		{
			assert(object != 0);

			AttachBefore(list, this);
		}

		inline ~Link()
		{
			Detach(this);
		}

	private:
		inline Link(LinkList<Type>*) :
			object(0),
			next(this),
			prev(this)
		{
		}

	public:
		inline Type*		operator->()		{ return object; }
		inline const Type*	operator->() const	{ return object; }

	public:
		inline operator Type*()				{ return object; }
		inline operator const Type*() const	{ return object; }

	template<typename> friend struct LinkList;
	};

	// XtUt::LinkList

	template<typename Type> struct LinkListIterator;
	template<typename Type> struct LinkListReverseIterator;
	template<typename Type> struct LinkListConstIterator;
	template<typename Type> struct LinkListConstReverseIterator;

	template<typename Type>
	struct LinkList
	{
		Link<Type> elements;

	public:
		typedef LinkListIterator<Type>				Iterator;
		typedef LinkListReverseIterator<Type>		ReverseIterator;
		typedef LinkListConstIterator<Type>			ConstIterator;
		typedef LinkListConstReverseIterator<Type>	ConstReverseIterator;

	public:
		inline LinkList() :
			elements(this)
		{
		}

		inline ~LinkList()
		{
			Free(this);
		}
	};

	// XtUt::WeakAnchor

	template<typename Type>
	struct WeakAnchor
	{
	public:
		struct Ref
		{
			Type*	object;
			int		refCount;

		public:
			inline Ref(Type* object_) :
				object(object_),
				refCount(1)
			{
			}

		public:
			inline void Acquire()
			{
				refCount++;
			}

			inline void Release()
			{
				if (--refCount <= 0)
					delete this;
			}
		};

	public:
		Ref* ref;

	public:
		inline WeakAnchor(Type* object) :
			ref(new Ref(object))
		{
		}

		inline ~WeakAnchor()
		{
			if (ref)
			{
				ref->object = 0;
				ref->Release();
			}
		}

	private:
		WeakAnchor(const WeakAnchor<Type>&);
		void operator=(const WeakAnchor<Type>&);
	};

	// XtUt::WeakRef

	template<typename Type>
	struct WeakRef
	{
		typename WeakAnchor<Type>::Ref* ref;

	public:
		inline WeakRef() :
			ref(0)
		{
		}

		inline WeakRef(const WeakRef<Type>& wr) :
			ref(wr.ref)
		{
			if (ref)
				ref->Acquire();
		}

		inline WeakRef(const WeakAnchor<Type>& anchor) :
			ref(anchor.ref)
		{
			if (ref)
				ref->Acquire();
		}

		inline WeakRef(const WeakAnchor<Type>* anchor) :
			ref(anchor ? anchor->ref : 0)
		{
			if (ref)
				ref->Acquire();
		}

		inline ~WeakRef()
		{
			if (ref)
				ref->Release();
		}

	public:
		inline WeakRef<Type>& operator=(const WeakRef<Type>& wr)
		{
			if (wr.ref)
				wr.ref->Acquire();

			if (ref)
				ref->Release();

			ref = wr.ref;

			return *this;
		}

		inline WeakRef<Type>& operator=(const WeakAnchor<Type>& anchor)
		{
			if (anchor.ref)
				anchor.ref->Acquire();

			if (ref)
				ref->Release();

			ref = anchor.ref;

			return *this;
		}

		inline WeakRef<Type>& operator=(const WeakAnchor<Type>* anchor)
		{
			if (anchor && anchor->ref)
				anchor->ref->Acquire();

			if (ref)
				ref->Release();

			ref = anchor ? anchor->ref : 0;

			return *this;
		}

	public:
		inline Type*		Get()		{ return ref ? ref->object : 0; }
		inline const Type*	Get() const	{ return ref ? ref->object : 0; }

	public:
		inline Type*		operator->()		{ return Get(); }
		inline const Type*	operator->() const	{ return Get(); }

	public:
		inline operator Type*()				{ return Get(); }
		inline operator const Type*() const	{ return Get(); }
	};

} // namespace XtUt

#include "XtUt.inl"

// TODO: Move this

inline XtU8 Swizzle(XtU8 v)
{
	return v;
}

inline XtU16 Swizzle(XtU16 v)
{
	return ((v & 0x00FF) << 8) | ((v & 0xFF00) >> 8);
}

inline XtU32 Swizzle(XtU32 v)
{
	return ((v & 0x000000FF) << 24) | ((v & 0x0000FF00) << 8) | ((v & 0x00FF0000) >> 8) | ((v & 0xFF000000) >> 24);
}

inline XtU64 Swizzle(XtU64 v)
{
	return (
			((v &  0x00000000000000FFULL) << 56) |
			((v &  0x000000000000FF00ULL) << 40) |
			((v &  0x0000000000FF0000ULL) << 24) |
			((v &  0x00000000FF000000ULL) << 8) |
			((v &  0x000000FF00000000ULL) >> 8) |
			((v &  0x0000FF0000000000ULL) >> 24) |
			((v &  0x00FF000000000000ULL) >> 40) |
			((v &  0xFF00000000000000ULL) >> 56)
		);
}

inline XtI8 Swizzle(XtI8 v)		{ return Swizzle((XtU8)v);	}
inline XtI16 Swizzle(XtI16 v)	{ return Swizzle((XtU16)v);	}
inline XtI32 Swizzle(XtI32 v)	{ return Swizzle((XtU32)v);	}
inline XtI64 Swizzle(XtI64 v)	{ return Swizzle((XtU64)v);	}

template<typename Type>
inline Type EndianSwizzle(Type value, XtEndian requiredEndian)
{
	if (XtGetEndian() == requiredEndian)
		return value;

	return (Type)Swizzle(value);
}

#endif // BIZARRE_XT_UT_H
