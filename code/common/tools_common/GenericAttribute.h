#ifndef TU_GENERIC_ATTRIBUTE_H
#define TU_GENERIC_ATTRIBUTE_H

#define TU_GENERIC_ATTRIBUTE_EPSILON 1e-6f

class GenericAttribute
{
public:
	enum Type
	{
		ATTR_NULL		= 0,
		ATTR_U8			= 1,
		ATTR_U16		= 2,
		ATTR_U32		= 3,
		ATTR_U64		= 4,
		ATTR_S8			= 5,
		ATTR_S16		= 6,
		ATTR_S32		= 7,
		ATTR_S64		= 8,
		ATTR_F32		= 11,
		ATTR_F64		= 12,
		ATTR_BOOLEAN	= 17,
		ATTR_STRING		= 18,
		
		ATTR_COUNT
	};

	///\brief Constructor.
	///
	GenericAttribute();

	///\brief Destructor.
	///
	~GenericAttribute();

	///\brief Copy constructor.
	///
	///@param rhs Class to copy.
	GenericAttribute(const GenericAttribute &rhs); 

	///\brief assignment operator.
	///
	///@param rhs Class to copy.
	///@return Reference to this class.
	const GenericAttribute& operator=(const GenericAttribute &rhs);

	///\brief Equivilance test operator.
	///
	///@param rhs The class to test against.
	///@return bool Test result.
	bool operator==(const GenericAttribute &rhs) const;

	///\brief Negated equivilance test operator.
	///
	///@param rhs The class to test against.
	///@return bool Test result.
	bool operator!= (const GenericAttribute &rhs) const;

	///\brief Less than operator.
	///
	///This test is required in order to be able to store generic attributes within a sorted container.
	///
	///@param rhs The class to test against
	///@return bool Test result.
	bool operator<(const GenericAttribute &rhs) const;

	///\brief Sets the name of this attribute.
	///
	///@param name Attribute name.
	void SetName(const std::string &name);

	///\brief Overwrites the contents of the generic attribute using another generic attribute.
	///
	///@param rhs The generic attribute to copy.
	void SetValue(const GenericAttribute& rhs);

	///\brief Set the value of this attribute.
	///
	///@param value Attribute value.
	void SetValue(U8 value);

	///\brief Set the value of this attribute.
	///
	///@param value Attribute value.
	void SetValue(U16 value);

	///\brief Set the value of this attribute.
	///
	///@param value Attribute value.
	void SetValue(U32 value);

	///\brief Set the value of this attribute.
	///
	///@param value Attribute value.
	void SetValue(U64 value);

	///\brief Set the value of this attribute.
	///
	///@param value Attribute value.
	void SetValue(S8 value);

	///\brief Set the value of this attribute.
	///
	///@param value Attribute value.
	void SetValue(S16 value);

	///\brief Set the value of this attribute.
	///
	///@param value Attribute value.
	void SetValue(S32 value);

	///\brief Set the value of this attribute.
	///
	///@param value Attribute value.
	void SetValue(S64 value);

	///\brief Set the value of this attribute.
	///
	///@param value Attribute value.
	void SetValue(F32 value);

	///\brief Set the value of this attribute.
	///
	///@param value Attribute value.
	void SetValue(F64 value);

	///\brief Set the value of this attribute.
	///
	///@param value Attribute value.
	void SetValue(bool value);

	///\brief Set the value of this attribute.
	///
	///@param value Attribute value.
	void SetValue(const char* value);

	///\brief Set the value of this attribute.
	///
	///@param value Attribute value.
	void SetValue(const std::string& value);

	///\brief Get the name of this attribute.
	///
	///@return Attribute name;
	const std::string& Name() const;

	///\brief Get the name of this attribute.
	///
	///@return Attribute name;
	std::string& Name();

	///\brief Get the value of this attribute.
	///
	///@param value Reference to variable to be filled with this attribute's value.
	template <class T>
	void Value(T& value) const
	{
		value = *(T*)m_pValue;
	}

	///\brief Get the value type of this attribute.
	///
	///This might be, U32, bool etc.
	Type DataType() const;

	///\brief Clear the contents of this attribute.
	///
	void Clear();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
protected:
	void OneTimeInit();

	void SetValue(void* pValue, Type type)
	{
		m_pValue = pValue;
		m_type = type;
	}

	std::string						m_name;
	Type							m_type;
	void*							m_pValue;

	friend class NativeGenericAttribute;

#endif
};


inline GenericAttribute::GenericAttribute() 
{
	OneTimeInit();
}

inline GenericAttribute::~GenericAttribute()
{
	// Free any allocated memory
	Clear();
}

// Perform deep copying of this node;
inline GenericAttribute::GenericAttribute(const GenericAttribute &rhs) 
{
	OneTimeInit();
	*this = rhs;
}

inline void GenericAttribute::SetValue(const GenericAttribute& rhs)
{
	switch (rhs.m_type) {
	case GenericAttribute::ATTR_U8: {
		U8 value;
		rhs.Value(value);
		SetValue(value);
		break;
	}
	case GenericAttribute::ATTR_U16: {
		U16 value;
		rhs.Value(value);
		SetValue(value);
		break;
	}
	case GenericAttribute::ATTR_U32: {
		U32 value;
		rhs.Value(value);
		SetValue(value);
		break;
	}
	case GenericAttribute::ATTR_U64: {
		U64 value;
		rhs.Value(value);
		SetValue(value);
		break;
	}
	case GenericAttribute::ATTR_S8: {
		S8 value;
		rhs.Value(value);
		SetValue(value);
		break;
	}
	case GenericAttribute::ATTR_S16: {
		S16 value;
		rhs.Value(value);
		SetValue(value);
		break;
	}
	case GenericAttribute::ATTR_S32: {
		S32 value;
		rhs.Value(value);
		SetValue(value);
		break;
	}
	case GenericAttribute::ATTR_S64: {
		S64 value;
		rhs.Value(value);
		SetValue(value);
		break;
	}
	case GenericAttribute::ATTR_F32: {
		F32 value;
		rhs.Value(value);
		SetValue(value);
		break;
	}
	case GenericAttribute::ATTR_F64: {
		F64 value;
		rhs.Value(value);
		SetValue(value);
		break;
	}

	case GenericAttribute::ATTR_BOOLEAN: {
		bool value;
		rhs.Value(value);
		SetValue(value);
		break;
	}
	case GenericAttribute::ATTR_STRING: {
		SetValue(*(std::string*)rhs.m_pValue);
		break;
	}
	default:
		m_pValue = NULL;
		m_type = GenericAttribute::ATTR_NULL;
		break;
	}

	m_type = rhs.m_type;
}

inline const GenericAttribute& GenericAttribute::operator =(const GenericAttribute &rhs)
{
	Clear();

	m_name = rhs.m_name;

	SetValue(rhs);

	return *this;
}

inline bool GenericAttribute::operator==(const GenericAttribute &rhs) const
{
	if (m_name == rhs.m_name &&
		m_type == rhs.m_type) {
		switch (m_type) {
			case GenericAttribute::ATTR_U8: {
				U8* pLhs = (U8*)m_pValue;
				U8* pRhs = (U8*)rhs.m_pValue;
				if (*pLhs != *pRhs) {
					return false;
				}
				break;
			}
			case GenericAttribute::ATTR_U16: {
				U16* pLhs = (U16*)m_pValue;
				U16* pRhs = (U16*)rhs.m_pValue;
				if (*pLhs != *pRhs) {
					return false;
				}
				break;
			}
			case GenericAttribute::ATTR_U32: {
				U32* pLhs = (U32*)m_pValue;
				U32* pRhs = (U32*)rhs.m_pValue;
				if (*pLhs != *pRhs) {
					return false;
				}
				break;
			}
			case GenericAttribute::ATTR_U64: {
				U64* pLhs = (U64*)m_pValue;
				U64* pRhs = (U64*)rhs.m_pValue;
				if (*pLhs != *pRhs) {
					return false;
				}
				break;
			}
			case GenericAttribute::ATTR_S8: {
				S8* pLhs = (S8*)m_pValue;
				S8* pRhs = (S8*)rhs.m_pValue;
				if (*pLhs != *pRhs) {
					return false;
				}
				break;
			}
			case GenericAttribute::ATTR_S16: {
				S16* pLhs = (S16*)m_pValue;
				S16* pRhs = (S16*)rhs.m_pValue;
				if (*pLhs != *pRhs) {
					return false;
				}
				break;
			}
			case GenericAttribute::ATTR_S32: {
				S32* pLhs = (S32*)m_pValue;
				S32* pRhs = (S32*)rhs.m_pValue;
				if (*pLhs != *pRhs) {
					return false;
				}
				break;
			}
			case GenericAttribute::ATTR_S64: {
				S64* pLhs = (S64*)m_pValue;
				S64* pRhs = (S64*)rhs.m_pValue;
				if (*pLhs != *pRhs) {
					return false;
				}
				break;
			}
			case GenericAttribute::ATTR_F32: {
				F32* pLhs = (F32*)m_pValue;
				F32* pRhs = (F32*)rhs.m_pValue;
				if (fabsf(*pLhs - *pRhs) > TU_GENERIC_ATTRIBUTE_EPSILON) {
					return false;
				}
				break;
			}
			case GenericAttribute::ATTR_F64: {
				F64* pLhs = (F64*)m_pValue;
				F64* pRhs = (F64*)rhs.m_pValue;
				if (fabs(*pLhs - *pRhs) > TU_GENERIC_ATTRIBUTE_EPSILON) {
					return false;
				}
				break;
			}
			case GenericAttribute::ATTR_BOOLEAN: {
				bool* pLhs = (bool*)m_pValue;
				bool* pRhs = (bool*)rhs.m_pValue;
				if (*pLhs != *pRhs) {
					return false;
				}
				break;
			}
			case GenericAttribute::ATTR_STRING: {
				std::string* pLhs = (std::string*)m_pValue;
				std::string* pRhs = (std::string*)rhs.m_pValue;
				if (*pLhs != *pRhs) {
					return false;
				}
				break;
			}
			default:
				break;
		}
	} else {
		return false;
	}
	
	return true;
}

inline bool GenericAttribute::operator!= (const GenericAttribute &rhs) const 
{
	if ( *this == rhs ) {
		return false;
	} else {
		return true;
	}
}

inline bool GenericAttribute::operator<(const GenericAttribute &rhs) const 
{
	if (memcmp((const void*)this, (const void*) &rhs, sizeof(GenericAttribute)) < 0)
	{
		return true;
	} else {
		return false;
	}
}

inline void GenericAttribute::SetName(const std::string &name) 
{
	m_name = name;
}

// Overloaded set operations
inline void GenericAttribute::SetValue(U8 value) 
{
	if (m_type == GenericAttribute::ATTR_U8)
	{
		(*(U8*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef TU_USE_MEMORY_POOL
	m_pValue = Allocator<U8>::Allocate();
	(*(U8*)m_pValue) = value;
#else
	m_pValue = new U8(value);
#endif
	m_type = GenericAttribute::ATTR_U8;
}

inline void GenericAttribute::SetValue(U16 value) 
{
	if (m_type == GenericAttribute::ATTR_U16)
	{
		(*(U16*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef TU_USE_MEMORY_POOL
	m_pValue = Allocator<U16>::Allocate();
	(*(U32*)m_pValue) = value;
#else
	m_pValue = new U16(value);
#endif
	m_type = GenericAttribute::ATTR_U16;
}

inline void GenericAttribute::SetValue(U32 value)
{
	if (m_type == GenericAttribute::ATTR_U32)
	{
		(*(U32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef TU_USE_MEMORY_POOL
	m_pValue = Allocator<U32>::Allocate();
	(*(U32*)m_pValue) = value;
#else
	m_pValue = new U32(value);
#endif
	m_type = GenericAttribute::ATTR_U32;
}

inline void GenericAttribute::SetValue(U64 value)
{
	if (m_type == GenericAttribute::ATTR_U64)
	{
		(*(U64*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef TU_USE_MEMORY_POOL
	m_pValue = Allocator<U64>::Allocate();
	(*(U64*)m_pValue) = value;
#else
	m_pValue = new U64(value);
#endif
	m_type = GenericAttribute::ATTR_U64;
}


inline void GenericAttribute::SetValue(S8 value) 
{
	if (m_type == GenericAttribute::ATTR_S8)
	{
		(*(S8*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef TU_USE_MEMORY_POOL
	m_pValue = Allocator<S8>::Allocate();
	(*(S8*)m_pValue) = value;
#else
	m_pValue = new S8(value);
#endif
	m_type = GenericAttribute::ATTR_S8;
}

inline void GenericAttribute::SetValue(S16 value)
{
	if (m_type == GenericAttribute::ATTR_S16)
	{
		(*(S16*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef TU_USE_MEMORY_POOL
	m_pValue = Allocator<S16>::Allocate();
	(*(S16*)m_pValue) = value;
#else
	m_pValue = new S16(value);
#endif
	m_type = GenericAttribute::ATTR_S16;
}

inline void GenericAttribute::SetValue(S32 value)
{
	if (m_type == GenericAttribute::ATTR_S32)
	{
		(*(S32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef TU_USE_MEMORY_POOL
	m_pValue = Allocator<S32>::Allocate();
	(*(S32*)m_pValue) = value;
#else
	m_pValue = new S32(value);
#endif
	m_type = GenericAttribute::ATTR_S32;
}

inline void GenericAttribute::SetValue(S64 value)
{
	if (m_type == GenericAttribute::ATTR_S64)
	{
		(*(S64*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef TU_USE_MEMORY_POOL
	m_pValue = Allocator<S64>::Allocate();
	(*(S64*)m_pValue) = value;
#else
	m_pValue = new S64(value);
#endif
	m_type = GenericAttribute::ATTR_S64;
}

inline void GenericAttribute::SetValue(F32 value) 
{
	if (m_type == GenericAttribute::ATTR_F32)
	{
		(*(F32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef TU_USE_MEMORY_POOL
	m_pValue = Allocator<F32>::Allocate();
	(*(F32*)m_pValue) = value;
#else
	m_pValue = new F32(value);
#endif
	m_type = GenericAttribute::ATTR_F32;
}

inline void GenericAttribute::SetValue(F64 value)
{
	if (m_type == GenericAttribute::ATTR_F64)
	{
		(*(F64*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef TU_USE_MEMORY_POOL
	m_pValue = Allocator<F64>::Allocate();
	(*(F64*)m_pValue) = value;
#else
	m_pValue = new F64(value);
#endif
	m_type = GenericAttribute::ATTR_F64;
}

inline void GenericAttribute::SetValue(bool value) 
{
	if (m_type == GenericAttribute::ATTR_BOOLEAN)
	{
		(*(bool*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef TU_USE_MEMORY_POOL
	m_pValue = Allocator<bool>::Allocate();
	(*(bool*)m_pValue) = value;
#else
	m_pValue = new bool(value);
#endif
	m_type = GenericAttribute::ATTR_BOOLEAN;
}

inline void GenericAttribute::SetValue(const std::string &value) 
{
	if (m_type == GenericAttribute::ATTR_STRING)
	{
		(*(std::string*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef TU_USE_MEMORY_POOL
	m_pValue = AftkArrayAllocator<std::string>::Allocate(1);
	(*(std::string*)m_pValue) = value;
#else
	m_pValue = new std::string(value);
#endif
	m_type = GenericAttribute::ATTR_STRING;
}

inline void GenericAttribute::SetValue(const char* value)
{
	if (m_type == GenericAttribute::ATTR_STRING)
	{
		(*(std::string*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef TU_USE_MEMORY_POOL
	m_pValue = AftkArrayAllocator<std::string>::Allocate(1);
	(*(std::string*)m_pValue) = value;
#else
	m_pValue = new std::string(value);
#endif
	m_type = GenericAttribute::ATTR_STRING;
}

inline const std::string& GenericAttribute::Name() const 
{
	return m_name;
}

inline std::string& GenericAttribute::Name() 
{
	return m_name;
}

inline GenericAttribute::Type GenericAttribute::DataType() const 
{
	return m_type;
}

inline void GenericAttribute::Clear() 
{
	if (m_pValue) 
	{
		switch (m_type)
		{
			case GenericAttribute::ATTR_U8:
			{
#ifdef TU_USE_MEMORY_POOL
				Allocator<U8>::Free((U8*)m_pValue);
#else
				delete (U8*)m_pValue;
#endif
				break;
			}

			case GenericAttribute::ATTR_U16:
			{
#ifdef TU_USE_MEMORY_POOL
				Allocator<U16>::Free((U16*)m_pValue);
#else
				delete (U16*)m_pValue;
#endif
				break;
			}

			case GenericAttribute::ATTR_U32:
			{
#ifdef TU_USE_MEMORY_POOL
				Allocator<U32>::Free((U32*)m_pValue);
#else
				delete (U32*)m_pValue;
#endif
				break;
			}

			case GenericAttribute::ATTR_U64:
			{
#ifdef TU_USE_MEMORY_POOL
				Allocator<U64>::Free((U64*)m_pValue);
#else
				delete (U64*)m_pValue;
#endif
				break;
			}

			case GenericAttribute::ATTR_S8:
			{
#ifdef TU_USE_MEMORY_POOL
				Allocator<S8>::Free((S8*)m_pValue);
#else
				delete (S8*)m_pValue;
#endif
				break;
			}

			case GenericAttribute::ATTR_S16:
			{
#ifdef TU_USE_MEMORY_POOL
				Allocator<S16>::Free((S16*)m_pValue);
#else
				delete (S16*)m_pValue;
#endif
				break;
			}

			case GenericAttribute::ATTR_S64:
			{
#ifdef TU_USE_MEMORY_POOL
				Allocator<S64>::Free((S64*)m_pValue);
#else
				delete (S64*)m_pValue;
#endif
				break;
			}

			case GenericAttribute::ATTR_S32:
			{
#ifdef TU_USE_MEMORY_POOL
				Allocator<S32>::Free((S32*)m_pValue);
#else
				delete (S32*)m_pValue;
#endif
				break;
			}

			case GenericAttribute::ATTR_F32:
			{
#ifdef TU_USE_MEMORY_POOL
				Allocator<F32>::Free((F32*)m_pValue);
#else
				delete (F32*)m_pValue;
#endif
				break;
			}

			case GenericAttribute::ATTR_F64:
			{
#ifdef TU_USE_MEMORY_POOL
				Allocator<F64>::Free((F64*)m_pValue);
#else
				delete (F64*)m_pValue;
#endif
				break;
			}

			case GenericAttribute::ATTR_BOOLEAN:
			{
#ifdef TU_USE_MEMORY_POOL
				Allocator<bool>::Free((bool*)m_pValue);
#else
				delete (bool*)m_pValue;
#endif
				break;
			}

			case GenericAttribute::ATTR_STRING:
			{
#ifdef TU_USE_MEMORY_POOL
				AftkArrayAllocator<std::string>::Free((std::string*)m_pValue, 1);
#else
				delete (std::string*)m_pValue;
#endif
				break;
			}
				
			case GenericAttribute::ATTR_NULL:
			default:
				break;
		}
	}

	m_pValue = 0;
	m_type = GenericAttribute::ATTR_NULL;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

inline void GenericAttribute::OneTimeInit()
{
	m_pValue = 0;
	m_type = GenericAttribute::ATTR_NULL;
}

#endif


#endif
