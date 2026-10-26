#ifndef XT_IMPL_XTREFLECTION_H
#define XT_IMPL_XTREFLECTION_H

#ifdef _MSC_VER
#pragma once
#endif

// Prototypes

struct XtType;
struct XtRecord;

template<typename Type> class XtHandle;

template<typename Type> XtType* XtGetType();

namespace Xt
{
	void GetRecordHandle(XtHandle<XtU8>* result, XtRecord* record, XtType* type);
};

// Helpers

namespace Xt
{

	// ReducePtr

	template<typename PtrType> struct ReducePtr
		{ typedef PtrType Type; static const int kCount = 0; };

	template<typename PtrType> struct ReducePtr<PtrType*>
		{ typedef typename ReducePtr<PtrType>::Type Type; static const int kCount = ReducePtr<PtrType>::kCount + 1; };

};

// XT_STATIC_ASSERT

template<bool x> struct XT_STATIC_ASSERT_FAILED;
template<> struct XT_STATIC_ASSERT_FAILED<true> { };
template<size_t x> struct XT_STATIC_ASSERT_TEST { };

#define XT_STATIC_ASSERT_IMPL2(expr, line)	typedef XT_STATIC_ASSERT_TEST<sizeof(XT_STATIC_ASSERT_FAILED<(bool)(expr)>)> XT_STATIC_ASSERT_TYPE_##line;
#define XT_STATIC_ASSERT_IMPL(expr, line)	XT_STATIC_ASSERT_IMPL2(expr, line)
#define XT_STATIC_ASSERT(expr)				XT_STATIC_ASSERT_IMPL(expr, __LINE__)

// XT_DECLARE_PRIVATE_COPY_CONSTRUCTORS

#define XT_DECLARE_PRIVATE_COPY_CONSTRUCTORS(type) private: type(const type&); void operator=(const type&);

namespace XtUt
{
	typedef XtU8	YesTrait;
	typedef XtU16	NoTrait;

	template<typename Base, typename Derived>
	struct IsBaseOfImpl
	{
		struct Conversions
		{
			operator const volatile Base*() const;
			operator const volatile Derived*();
		};

		template<typename Type> static YesTrait Test(const volatile Derived*, Type);
		static NoTrait Test(const volatile Base*, int);

		static const bool value = (sizeof(Test(Conversions(), 0)) == sizeof(YesTrait));
	};

	template<typename Lhs, typename Rhs>
	struct IsSameAsImpl
	{
		static const volatile Lhs* MakeLhs();
		static const volatile Rhs* MakeRhs();

		template<typename Type> static YesTrait Test(Type*, Type*);
		static NoTrait Test(...);

		static const bool value = (sizeof(Test(MakeLhs(), MakeRhs())) == sizeof(YesTrait));
	};

	template<typename Base, typename Derived>
	struct IsBaseOf
	{
		static const bool value = IsBaseOfImpl<Base, Derived>::value || IsSameAsImpl<Base, Derived>::value;
	};

	template<typename Lhs, typename Rhs>
	struct IsSameAs
	{
		static const bool value = IsSameAsImpl<Lhs, Rhs>::value;
	};
}

//
// Reflection types
//

enum XtTypeBehaviour
{
	kXtTbAtom,
	kXtTbEnum,
	kXtTbFlags,
	kXtTbStruct,

	kXtTypeBehaviourCount
};

enum XtFieldBehaviour
{
	kXtFbAtom,
	kXtFbEnum,
	kXtFbFlags,
	kXtFbStruct,
	kXtFbPtr,
	kXtFbHandle,
	kXtFbLabel,

	kXtFieldBehaviourCount,

	kXtFbImplInfer,
	kXtFbImplEnumLabel,
	kXtFbImplFlagLabel,
	kXtFbImplAtom,
	kXtFbImplBase,
	kXtFbImplCallback,
	kXtFbImplArray,
	kXtFbImplWeak,
	kXtFbImplFlagStorage,
	kXtFbImplEnd
};

enum XtAtomType
{
	kXtNotAtom,

	kXtBool,
	kXtI8,
	kXtI16,
	kXtI32,
	kXtI64,
	kXtU8,
	kXtU16,
	kXtU32,
	kXtU64,
	kXtF32,
	kXtF64,
	kXtSz8,
	kXtSz16,
	kXtLocId,

	kXtAtomTypeCount
};

enum XtEventType
{
	kXtEtConstruct,
	kXtEtDestruct,
	kXtEtUser = 1000
};

enum XtComponentBehaviour
{
	kXtCbObject,
	kXtCbPtr,
	kXtCbHandle,

	kXtComponentBehaviourCount
};

struct XtRecord;

struct XtCallbackContext
{
	XtU32 tag;

public:
	XtCallbackContext(XtU32 tag_) :
		tag(tag_)
	{
	}
};

typedef void (*XtConstructCallback)(void*);
typedef void (*XtEventCallback)(XtEventType, XtRecord*, void*, XtCallbackContext*);
typedef void (*XtDestructCallback)(void*);

struct XtBase;
struct XtField;

struct XtType
{
	const char*			name;
	XtU32				size;
	XtBase*				bases;
	XtField*			fields;

	XtU32				fieldCount	: 11;
	XtU32				isAbstract	: 1;
	XtU32				baseCount	: 4;
	XtTypeBehaviour		behaviour	: 3;
	XtAtomType			atom		: 5;
	XtU32				alignment	: 8;

	XtConstructCallback	construct;
	XtEventCallback		event;
	XtDestructCallback	destruct;
};

struct XtBase
{
	XtType*	type;
	XtU32	offset;
};

struct XtField
{
	const char*			name;
	XtType*				type;
	XtIntPtr			value;
	XtU32				offset		: 24;
	XtU32				array		: 1;
	XtFieldBehaviour	behaviour	: 7;
};

struct XtSchema
{
	int			typeCount;
	XtType*		types;

	int			baseCount;
	XtBase*		bases;

	int			fieldCount;
	XtField*	fields;

	XtEndian	endian;
	XtU8*		strings;

XT_DECLARE_PRIVATE_COPY_CONSTRUCTORS(XtSchema)
};

//
// Database types
//

extern XtType gXtNullType;

struct XtRecord
{
	XtU16	salt;
	XtU16	strongRef;
	XtU32	weakRef;
	void*	object;
	XtId	id;
	XtType*	type;

public:
	XtRecord() :
		salt(1),
		strongRef(0),
		weakRef(0),
		object(0),
		id(0),
		type(&gXtNullType)
	{
	}

XT_DECLARE_PRIVATE_COPY_CONSTRUCTORS(XtRecord)
};

struct XtDb
{
	XtSchema*			schema;
	size_t				capacity;
	size_t				count;
	XtRecord*			records;
	XtRecord**			index;
	XtRecord*			free_;
	XtCallbackContext*	context;

public:
	XtDb() :
		schema(0),
		capacity(0),
		count(0),
		records(0),
		index(0),
		free_(0),
		context(0)
	{
	}

	~XtDb()
	{
		delete [] records;
		delete [] index;
	}

XT_DECLARE_PRIVATE_COPY_CONSTRUCTORS(XtDb)
};

const int kXtPackRefFlag_Strong = 0x0001;

struct XtPackRef
{
	XtU32		flags;
	XtRecord*	record;

public:
	XtPackRef() :
		flags(0),
		record(0)
	{
	}
};

struct XtPack
{
	size_t		count;
	XtPackRef*	records;

public:
	XtPack() :
		count(0),
		records(0)
	{
	}

	~XtPack()
	{
		delete [] records;
	}

XT_DECLARE_PRIVATE_COPY_CONSTRUCTORS(XtPack)
};

extern XtRecord gXtNullRecord;

struct XtComponent
{
	XtU8*	data;
	XtType*	type;
	XtU16	count;
	XtU16	behaviour;
};

struct XtRecordHeader
{
	XtU8* top;
	XtU32 count;
	XtU32 size;
};

template<typename Type>
class XtHandle
{
public:
	XtHandle() : _record(&gXtNullRecord), _salt(0), _offset(0)
	{
		XT_STATIC_ASSERT(Xt::ReducePtr<Type>::kCount <= 0); // <Type> must not be a pointer
	}

	XtHandle(XtRecord* record, XtU16 salt, XtU16 offset) : _record(record ? record : &gXtNullRecord), _salt(record ? salt : 0), _offset(record ? offset : 0) { }

	explicit XtHandle(XtRecord* record) : _record(&gXtNullRecord), _salt(0), _offset(0)
	{
		Xt::GetRecordHandle((XtHandle<XtU8>*)this, record, XtGetType<Type>());
	}

	template<typename CopyType>
	XtHandle(const XtHandle<CopyType>& rhs) : _record(rhs._record), _salt(rhs._salt), _offset(rhs._offset + ((int)(XtIntPtr)static_cast<Type*>((CopyType*)1) - 1))
	{
		XT_STATIC_ASSERT((XtUt::IsBaseOf<Type, CopyType>::value)); // Use XtCast() to up cast XtHandles
	}

public:
	template<typename CopyType>
	XtHandle<Type>& operator=(const XtHandle<CopyType>& rhs)
	{
		XT_STATIC_ASSERT((XtUt::IsBaseOf<Type, CopyType>::value)); // Use XtCast() to up cast XtHandles

		_record	= rhs._record;
		_salt	= rhs._salt;
		_offset	= rhs._offset + ((int)(XtIntPtr)static_cast<Type*>((CopyType*)1) - 1);

		return *this;
	}

public:
	Type* Object() { return (Type*)((const XtHandle<Type>*)this)->Object(); }

	const Type* Object() const
	{
		if ((_record->salt != _salt) || !_record->object)
			return 0;

		return (Type*)((XtU8*)_record->object + _offset);
	}

	XtRecord*	Record()	const { return _record->salt == _salt ? _record : 0;		}
	XtId		Id()		const { return _record->salt == _salt ? _record->id : 0;	}
	XtU16		Salt()		const { return _salt;	}
	XtU16		Offset()	const { return _offset;	}

public:
	void Clear()
	{
		_record	= &gXtNullRecord;
		_salt	= 0;
		_offset	= 0;
	}

public:
	operator Type*()				{ return Object(); }
	operator const Type*() const	{ return Object(); }
	operator Type&()				{ Type*			object = Object(); assert(object != 0); return *object;	}
	operator const Type&() const	{ const Type*	object = Object(); assert(object != 0); return *object;	}
	Type* operator->()				{ Type*			object = Object(); assert(object != 0); return object;	}
	const Type* operator->() const	{ const Type*	object = Object(); assert(object != 0); return object;	}

private:
	XtRecord*	_record;
	XtU16		_salt;
	XtU16		_offset;

template<typename> friend class XtHandle;
friend void XtCastImpl(const XtHandle<XtU8>* handle, XtHandle<XtU8>* result, XtType* fromType, XtType* toType, int offset);
};

//
// Annotation types
//

template<typename Type>
struct XtArray
{
	Type* elements;
	XtI32 count;

public:
	XtArray() :
		elements(0),
		count(0)
	{
		XT_STATIC_ASSERT(Xt::ReducePtr<Type>::kCount <= 1); // <Type> may have at most one level of indirection
	}

	XtArray(Type* elements_, XtI32 count_) : elements(elements_), count(count_) { }

public:
	operator Type*()				{ return elements; }
	operator const Type*() const	{ return elements; }

public:
	Type& operator[](int index)							{ assert((index >= 0) && (index < count)); return elements[index]; }
	const Type& operator[](int index) const				{ assert((index >= 0) && (index < count)); return elements[index]; }
	Type& operator[](unsigned int index)				{ assert((int)index < count); return elements[index]; }
	const Type& operator[](unsigned int index) const	{ assert((int)index < count); return elements[index]; }
#if defined(_WIN64) // [Blur2PC][x64] size_t indices; on x86 size_t is unsigned int
	Type& operator[](size_t index)						{ assert(index < (size_t)count); return elements[index]; }
	const Type& operator[](size_t index) const			{ assert(index < (size_t)count); return elements[index]; }
#endif
};

template<typename StorageType, typename FlagType>
struct XtFlags
{
	StorageType value;

public:
	XtFlags() :
		value(0)
	{
		XT_STATIC_ASSERT(
				(Xt::ReducePtr<StorageType>::kCount <= 0) &&	// <StorageType> must not be a pointer
				(Xt::ReducePtr<FlagType>::kCount <= 0)			// <FlagType> must not be a pointer
			);
	}

	XtFlags(StorageType value_) : value(value_) { }

public:
	XtFlags<StorageType, FlagType>& operator=(StorageType value_)	{ value = value_;	return *this; }
	XtFlags<StorageType, FlagType>& operator&=(StorageType value_)	{ value &= value_;	return *this; }
	XtFlags<StorageType, FlagType>& operator|=(StorageType value_)	{ value |= value_;	return *this; }

public:
	operator StorageType() const { return value; }
};

struct XtSz8
{
	const char* string;

public:
	explicit XtSz8(const char* string_ = "") : string(string_) { }

public:
	operator const char*() const { return string; }

public:
	const char* Text() const { return string; }
};

struct XtSz16
{
	const wchar_t* string;

public:
	explicit XtSz16(const wchar_t* string_ = L"") : string(string_) { }

public:
	operator const wchar_t*() const { return string; }

public:
	const wchar_t* Text() const { return string; }
};

struct XtLocId
{
	XtU32 id;

public:
	explicit XtLocId(XtU32 id_ = 0) : id(id_) { }
};

//
// Placement new
//

struct XtPlacementNew { };

inline void* operator new(size_t, const XtPlacementNew&, void* where)
{
	return where;
}

inline void operator delete(void*, const XtPlacementNew&, void* /*where*/)
{
}

//
// Annotation implementation
//

namespace Xt
{
	// Annotation

	struct AnnotationField;

	struct AnnotationType
	{
		XtType*				type;
		const char*			name;
		XtU32				size;
		AnnotationField*	fields;
		size_t				alignment;

		bool				isAbstract;
		XtConstructCallback	construct;
		XtDestructCallback	destruct;

		const char*			debugFile;
		int					debugLine;

		XtTypeBehaviour		behaviour;
	};

	struct AnnotationField
	{
		const char*		name;
		AnnotationType*	type;
		XtIntPtr		value;
		XtU32			offset;
		XtU8			array;
		XtU8			behaviour;
		XtU16			debugLine;
	};

	template<typename Type>
	struct Annotation
	{
		static AnnotationType	type;
		static AnnotationField	fields[];
	};

	// [Con/De]struction callbacks

	template<typename Type> inline void Construct(void* where)
	{
		new(XtPlacementNew(), where) Type;
	}

	template<typename Type> inline void Destruct(void* where)
	{
		((Type*)where)->~Type();
	}

	// AlignmentOf

	template<typename Type> inline size_t AlignmentOf()
	{
		struct Packed { Type a; XtU8 b; };
		return sizeof(Packed) - sizeof(Type);
	}

	// ReduceConst

	template<typename ConstType> struct ReduceConst						{ typedef ConstType Type; static const bool kConst = false;	};
	template<typename ConstType> struct ReduceConst<const ConstType>	{ typedef ConstType Type; static const bool kConst = true;	};

	// ReduceTypeImpl

	namespace ReduceTypeImpl
	{
		// RtBase

		template<typename Type_, typename AuxType_, XtFieldBehaviour kBehaviour_> struct RtBase
		{
			typedef typename ReduceConst<Type_>::Type		Type;
			typedef typename ReduceConst<AuxType_>::Type	AuxType;

			static const XtFieldBehaviour kBehaviour = kBehaviour_;
		};

		// RtData

		template<typename Type>						struct RtData							: public RtBase<Type, Type,			kXtFbImplInfer>		{ };
		template<typename Type, typename FlagsType>	struct RtData<XtFlags<Type, FlagsType> >: public RtBase<Type, FlagsType,	kXtFbFlags>			{ };
		template<typename Type>						struct RtData<XtHandle<Type> >			: public RtBase<Type, Type,			kXtFbHandle>		{ };

		// RtPtr

		template<typename Type> struct RtPtr		: public RtData<Type> { };
		template<typename Type> struct RtPtr<Type*>	: public RtData<Type> { static const XtFieldBehaviour kBehaviour = kXtFbPtr; };

		// RtArray

		template<typename Type> struct RtArray : public RtPtr<Type>
			{ static const bool kArray = false; };

		template<typename Type> struct RtArray<XtArray<Type> > : public RtPtr<typename ReduceConst<Type>::Type>
			{ static const bool kArray = true; };
	}

	// ReduceType

	template<typename Type> struct ReduceType : public ReduceTypeImpl::RtArray<typename ReduceConst<Type>::Type> { };

	// GetAnnotationType

	template<typename Type> inline AnnotationType& GetAnnotationType()
	{
		XT_STATIC_ASSERT(ReducePtr<Type>::kCount <= 0); // <Type> must not be a pointer

		return Annotation<typename ReduceConst<Type>::Type>::type;
	}

	// GetMemberValue

	template<typename MemberType> inline XtIntPtr GetMemberValue()
	{
		if (ReduceType<MemberType>::kBehaviour == kXtFbFlags)
			return (XtIntPtr)&GetAnnotationType<typename ReduceType<MemberType>::AuxType>();

		return 0;
	}

	// FaField

	template<typename ClassType, typename MemberType>
	inline AnnotationType* FaField_Type(MemberType (ClassType::*))
		{ return &GetAnnotationType<typename ReduceType<MemberType>::Type>(); }

	template<typename ClassType, typename MemberType>
	inline XtIntPtr FaField_Value(MemberType (ClassType::*))
		{ return GetMemberValue<MemberType>(); }

	template<typename ClassType, typename MemberType>
	inline int FaField_Offset(MemberType (ClassType::*member))
		{ return (int)(XtIntPtr)&(((ClassType*)0)->*member); }

	template<typename ClassType, typename MemberType>
	inline bool FaField_Array(MemberType (ClassType::*))
		{ return ReduceType<MemberType>::kArray; }

	template<typename ClassType, typename MemberType>
	inline XtFieldBehaviour FaField_Behaviour(MemberType (ClassType::*))
		{ return ReduceType<MemberType>::kBehaviour; }

	// FaBase

	template<typename ClassType, typename BaseType>
	inline int FaBase_Offset()
		{ return (int)(XtIntPtr)static_cast<BaseType*>((ClassType*)1) - 1; }

	// FaCallback

	template<typename Type>
	inline XtIntPtr CastCallback(void (*callback)(XtEventType, XtRecord*, Type*, XtCallbackContext*))
		{ return (XtIntPtr)callback; }
};

#define XT_IMPL_TYPE(type_t, name, alignment, abstract, constructor, destructor) \
	namespace Xt { template<> struct Annotation< type_t > { \
		typedef type_t Type_t; static AnnotationType type; static AnnotationField fields[]; \
	}; AnnotationType Annotation< type_t >::type = { \
		0, (name), sizeof(type_t), Annotation< type_t >::fields, (alignment), \
		(abstract), (constructor), (destructor), __FILE__, __LINE__, kXtTbStruct \
	}; AnnotationField Annotation< type_t >::fields[] = {

#define XT_IMPL_FIELD(name, type, value, offset, array, behaviour) \
	{ (name), (type), (value), (offset), (array), (behaviour), __LINE__ }

#define XT_IMPL_FA_SPECIAL(name, value, behaviour) \
	XT_IMPL_FIELD(name, 0, value, 0, 0, behaviour)

#define XT_IMPL_FA_FIELD(member, name) \
	XT_IMPL_FIELD( \
		name,									Xt::FaField_Type(&Type_t::member),	Xt::FaField_Value(&Type_t::member), \
		Xt::FaField_Offset(&Type_t::member),	Xt::FaField_Array(&Type_t::member),	(XtU8)Xt::FaField_Behaviour(&Type_t::member) \
	)

#define XT_IMPL_FA_BASE(baseType) \
	XT_IMPL_FIELD(0, &Xt::GetAnnotationType< baseType >(), 0, (Xt::FaBase_Offset< Type_t, baseType >()), 0, kXtFbImplBase)

#define XT_IMPL_FA_FLAG_STORAGE(type) \
	XT_IMPL_FIELD(0, &Xt::GetAnnotationType< type >(), 0, 0, 0, kXtFbImplFlagStorage)

//
// Annotation macros
//

#define XT_LIBRARY(name)	void XtLibrary_##name() { static bool initialised = false; if (!initialised) { initialised = true; do {
#define XT_END_LIBRARY		} while(0); } }

#define XT_APPLICATION		XT_LIBRARY(Application) XT_DEPENDS_ON(Xt)
#define XT_END_APPLICATION	XT_END_LIBRARY

#define XT_DEPENDS_ON(name)		do { void XtLibrary_##name(); XtLibrary_##name(); } while(0);
#define XT_REGISTER_TYPE(type)	XtRegisterType< type >();

#define XT_TYPE(type_t, name)			XT_IMPL_TYPE(type_t, name, AlignmentOf< type_t >(), false, Construct< type_t >, Destruct< type_t >)
#define XT_ABSTRACT_TYPE(type_t, name)	XT_IMPL_TYPE(type_t, name, 1, true, 0, 0)
#define XT_END_TYPE						XT_IMPL_FA_SPECIAL(0, 0, kXtFbImplEnd) }; }

#define XT_FIELD(member, name)	XT_IMPL_FA_FIELD(member, name),
#define XT_BASE(base)			XT_IMPL_FA_BASE(base),
#define XT_ATOM(atom)			XT_IMPL_FA_SPECIAL(0,		atom,					kXtFbImplAtom),
#define XT_OPTION(value, name)	XT_IMPL_FA_SPECIAL(name,	value,					kXtFbImplEnumLabel),
#define XT_FLAG(value, name)	XT_IMPL_FA_SPECIAL(name,	value,					kXtFbImplFlagLabel),
#define XT_CALLBACK(fn)			XT_IMPL_FA_SPECIAL(0,		Xt::CastCallback((fn)),	kXtFbImplCallback),
#define XT_FLAG_STORAGE(type)	XT_IMPL_FA_FLAG_STORAGE(type),

#define XT_FRIEND template<typename> friend struct Xt::Annotation
#define XT_FRIEND_T(type_t) friend struct Xt::Annotation< type_t >

//
// Reflection
//

template<typename Type> inline XtType* XtGetType()
{
	return Xt::GetAnnotationType<Type>().type;
}

template<typename Type> inline void XtRegisterType()
{
	void XtRegisterType(Xt::AnnotationType&);

	XtRegisterType(Xt::GetAnnotationType<Type>());
}

//
// Schema
//

bool XtInitialise();
void XtShutdown();

XtSchema* XtCreateSchema(int typeCount, int baseCount, int fieldCount, int stringsLength);
void XtDestroySchema(XtSchema* schema);

XtSchema* XtGetApplicationSchema();

bool XtExportSchema(const XtSchema* schema, XtU8*& schemaData, int& schemaLength);
XtSchema* XtImportSchema(const XtU8* schemaData, int schemaLength);

//
// Crc
//

XtU32 XtCrc(const XtU8* data, int length);

//
// Helpers
//

XtType* XtFindTypeByName(const XtSchema* schema, const char* name);
XtField* XtFindFieldByName(const XtType* type, const char* name);
bool XtIsBaseOf(const XtType* base, const XtType* derived);

namespace Xt
{
	XtHandle<XtU8> XtCastImpl(const XtHandle<XtU8>* handle, XtType* fromType, XtType* toType);

	template<typename ToType, typename FromType, bool fast>
	struct XtCastT
	{
		static XtHandle<ToType> Do(const XtHandle<FromType>& handle)
		{
			return *(XtHandle<ToType>*)(&XtCastImpl(
					(XtHandle<XtU8>*)(&handle),
					handle._record->type ? handle._record->type : XtGetType<FromType>(),
					XtGetType<ToType>()
				));
		}
	};

	template<typename ToType, typename FromType>
	struct XtCastT<ToType, FromType, true>
	{
		static XtHandle<ToType> Do(const XtHandle<FromType>& handle)
		{
			return handle;
		}
	};
};

template<typename ToType, typename FromType>
XtHandle<ToType> XtCast(const XtHandle<FromType>& handle)
{
	return Xt::XtCastT<ToType, FromType, XtUt::IsBaseOf<ToType, FromType>::value>::Do(handle);
}

template<typename ToType, typename FromType>
XtHandle<ToType> XtUnsafeCast(const XtHandle<FromType>& handle)
{
	return XtHandle<ToType>(
			handle._record,
			handle._salt,
			handle._offset + ((int)(XtIntPtr)static_cast<ToType*>((FromType*)1) - 1)
		);
}

XtRecordHeader* XtGetRecordHeader(const XtRecord* record);
XtType* XtGetTypeByPtr(XtRecord* record, void* ptr);

XtU8* XtGetRecordBaseAddr(const XtRecord* record);
XtU32 XtGetRecordSize(const XtRecord* record);
XtU32 XtGetRecordFixupCount(const XtRecord* record);
void XtGenerateRecordFixups(const XtRecord* record, XtU32* fixups, XtU32 fixupCount);

inline void XtApplyRecordFixups(void* newRecord, void* originalBase, const XtU32* fixups, XtU32 fixupCount, XtU32 recordSize)
{
	XtI32 delta = (XtU8*)newRecord - (XtU8*)originalBase;

	for(XtU32 i = 0; i < fixupCount; i++)
	{
		if (fixups[i] >= recordSize)
		{
			assert(0 && "Xt fix-up data out of bounds.");
			break;
		}

		XtU8** pp = (XtU8**)((XtU8*)newRecord + fixups[i]);
		XtU8* pLocal = (*pp) + delta;
		*pp = pLocal;
	}
}

// XtDb*

XtDb* XtDbCreate(XtSchema* schema, size_t maxRecords, XtCallbackContext* context);
void XtDbDestroy(XtDb* db);
XtRecord* XtGetRecord(XtDb* db, XtId id);

// XtPack*

typedef void (*XtPackOpenCallback)(struct XtPackOpenStatus* status, void* context);

struct XtPackOpenStatus
{
	const char* message;

public:
	XtPackOpenStatus();
};

struct XtPackOpenParams
{
	XtPackOpenCallback	callback;
	void*				context;

public:
	XtPackOpenParams();
};

XtPack* XtPackOpen(XtDb* db, void* flask, size_t length, XtPackOpenParams* params = 0);
void XtPackClose(XtDb* db, XtPack* pack);

#include "Xt/XtStd.h"

#endif // XT_IMPL_XTREFLECTION_H
