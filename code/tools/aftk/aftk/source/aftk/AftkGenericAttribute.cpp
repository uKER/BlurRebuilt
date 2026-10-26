#include <stdafx.h>

using namespace Aftk;

C_GenericAttribute::C_GenericAttribute() 
{
	OneTimeInit();
}

C_GenericAttribute::~C_GenericAttribute()
{
	// Free any allocated memory
	Clear();
}

// Perform deep copying of this node;
C_GenericAttribute::C_GenericAttribute(const C_GenericAttribute &rhs) 
{
	OneTimeInit();
	*this = rhs;
}

void C_GenericAttribute::SetValue(const C_GenericAttribute& rhs)
{
	switch (rhs.m_type) {
	case E_GenericAttributeType_U8: {
		U8 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_U16: {
		U16 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_U32: {
		U32 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_U64: {
		U64 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_S8: {
		S8 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_S16: {
		S16 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_S32: {
		S32 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_S64: {
		S64 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_F32: {
		F32 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_F64: {
		F64 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}

	case E_GenericAttributeType_Vector4F64: {
		Vector4F64 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector4F32: {
		Vector4F32 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector4S64: {
		Vector4S64 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}	
	case E_GenericAttributeType_Vector4S32: {
		Vector4S32 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector4S16: {
		Vector4S16 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}	
	case E_GenericAttributeType_Vector4S8: {
		Vector4S8 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector4U64: {
		Vector4U64 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector4U32: {
		Vector4U32 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector4U16: {
		Vector4U16 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}	
	case E_GenericAttributeType_Vector4U8: {
		Vector4U8 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}

	case E_GenericAttributeType_Vector3F64: {
		Vector3F64 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector3F32: {
		Vector3F32 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector3S64: {
		Vector3S64 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}	
	case E_GenericAttributeType_Vector3S32: {
		Vector3S32 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}	
	case E_GenericAttributeType_Vector3S16: {
		Vector3S16 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}	
	case E_GenericAttributeType_Vector3S8: {
		Vector3S8 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector3U64: {
		Vector3U64 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector3U32: {
		Vector3U32 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector3U16: {
		Vector3U16 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}	
	case E_GenericAttributeType_Vector3U8: {
		Vector3U8 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}

	case E_GenericAttributeType_Vector2F64: {
		Vector2F64 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector2F32: {
		Vector2F32 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector2S64: {
		Vector2S64 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}	
	case E_GenericAttributeType_Vector2S32: {
		Vector2S32 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}	
	case E_GenericAttributeType_Vector2S16: {
		Vector2S16 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}	
	case E_GenericAttributeType_Vector2S8: {
		Vector2S8 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector2U64: {
		Vector2U64 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector2U32: {
		Vector2U32 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Vector2U16: {
		Vector2U16 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}	
	case E_GenericAttributeType_Vector2U8: {
		Vector2U8 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}

	case E_GenericAttributeType_Matrix44F32: {
		Matrix44F32 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_Boolean: {
		bool value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_GenericAttributeType_String: {
		SetValue(*(std::string*)rhs.m_pValue);
		break;
	}
	case E_GenericAttributeType_BlobData: {
		SetValue(*(C_BlobData*)rhs.m_pValue);
		break;
	}
	default:
		m_pValue = NULL;
		m_type = E_GenericAttributeType_Null;
		break;
	}

	m_type = rhs.m_type;
}

const C_GenericAttribute& C_GenericAttribute::operator =(const C_GenericAttribute &rhs)
{
	Clear();

	m_name = rhs.m_name;

	SetValue(rhs);

	return *this;
}

bool C_GenericAttribute::operator==(const C_GenericAttribute &rhs) const
{
	if (m_name == rhs.m_name &&
		m_type == rhs.m_type) {
		switch (m_type) {
			case E_GenericAttributeType_U8: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(U8)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_U16: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(U16)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_U32: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(U32)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_U64: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(U64)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_S8: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(S8)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_S16: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(S16)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_S32: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(S32)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_S64: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(S64)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_F32: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(F32)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_F64: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(F64)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector4F64: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector4F64)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector4F32: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector4F32)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector4S64: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector4S64)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector4S32: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector4S32)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector4S16: {
				if (!memcmp(m_pValue, rhs.m_pValue, sizeof(Vector4S16)) != 0)
					return false;

				break;
			}
			case E_GenericAttributeType_Vector4S8: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector4S8)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector4U64: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector4U64)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector4U32: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector4U32)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector4U16: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector4U16)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector4U8: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector4U8)) != 0)
					return false;
				break;
			}

			case E_GenericAttributeType_Vector3F64: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector3F64)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector3F32: 
			{
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector3F32)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector3S64: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector3S64)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector3S32: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector3S32)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector3S16: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector3S16)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector3S8: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector3S8)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector3U64: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector3U64)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector3U32: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector3U32)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector3U16: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector3U16)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector3U8: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector3U8)) != 0)
					return false;
				break;
			}

			case E_GenericAttributeType_Vector2F64: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector2F64)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector2F32: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector2F32)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector2S64: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector2S64)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector2S32: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector2S32)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector2S16: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector2S16)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector2S8: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector2S8))!= 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector2U64: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector2U64)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector2U32: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector2U32)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector2U16: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector2U16)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Vector2U8: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Vector2U8)) != 0)
					return false;
				break;
			}

			case E_GenericAttributeType_Matrix44F32: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(Matrix44F32)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_Boolean: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(bool)) != 0)
					return false;
				break;
			}
			case E_GenericAttributeType_String: {
				std::string* pLhs = (std::string*)m_pValue;
				std::string* pRhs = (std::string*)rhs.m_pValue;
				if (*pLhs != *pRhs) {
					return false;
				}
				break;
			}
			case E_GenericAttributeType_BlobData: {
				C_BlobData *pLhs = (C_BlobData*)m_pValue;
				C_BlobData *pRhs = (C_BlobData*)rhs.m_pValue;
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

bool C_GenericAttribute::operator!= (const C_GenericAttribute &rhs) const 
{
	if ( *this == rhs ) {
		return false;
	} else {
		return true;
	}
}

bool C_GenericAttribute::operator<(const C_GenericAttribute &rhs) const 
{
	if (memcmp((const void*)this, (const void*) &rhs, sizeof(C_GenericAttribute)) < 0)
	{
		return true;
	} else {
		return false;
	}
}

void C_GenericAttribute::SetName(const std::string &name) 
{
	m_name = name;
}

// Overloaded set operations
void C_GenericAttribute::SetValue(U8 value) 
{
	if (m_type == E_GenericAttributeType_U8)
	{
		(*(U8*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<U8>::Allocate();
	(*(U8*)m_pValue) = value;
#else
	m_pValue = new U8(value);
#endif
	m_type = E_GenericAttributeType_U8;
}

void C_GenericAttribute::SetValue(U16 value) 
{
	if (m_type == E_GenericAttributeType_U16)
	{
		(*(U16*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<U16>::Allocate();
	(*(U32*)m_pValue) = value;
#else
	m_pValue = new U16(value);
#endif
	m_type = E_GenericAttributeType_U16;
}

void C_GenericAttribute::SetValue(U32 value)
{
	if (m_type == E_GenericAttributeType_U32)
	{
		(*(U32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<U32>::Allocate();
	(*(U32*)m_pValue) = value;
#else
	m_pValue = new U32(value);
#endif
	m_type = E_GenericAttributeType_U32;
}

void C_GenericAttribute::SetValue(U64 value)
{
	if (m_type == E_GenericAttributeType_U64)
	{
		(*(U64*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<U64>::Allocate();
	(*(U64*)m_pValue) = value;
#else
	m_pValue = new U64(value);
#endif
	m_type = E_GenericAttributeType_U64;
}


void C_GenericAttribute::SetValue(S8 value) 
{
	if (m_type == E_GenericAttributeType_S8)
	{
		(*(S8*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<S8>::Allocate();
	(*(S8*)m_pValue) = value;
#else
	m_pValue = new S8(value);
#endif
	m_type = E_GenericAttributeType_S8;
}

void C_GenericAttribute::SetValue(S16 value)
{
	if (m_type == E_GenericAttributeType_S16)
	{
		(*(S16*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<S16>::Allocate();
	(*(S16*)m_pValue) = value;
#else
	m_pValue = new S16(value);
#endif
	m_type = E_GenericAttributeType_S16;
}

void C_GenericAttribute::SetValue(S32 value)
{
	if (m_type == E_GenericAttributeType_S32)
	{
		(*(S32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<S32>::Allocate();
	(*(S32*)m_pValue) = value;
#else
	m_pValue = new S32(value);
#endif
	m_type = E_GenericAttributeType_S32;
}

void C_GenericAttribute::SetValue(S64 value)
{
	if (m_type == E_GenericAttributeType_S64)
	{
		(*(S64*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<S64>::Allocate();
	(*(S64*)m_pValue) = value;
#else
	m_pValue = new S64(value);
#endif
	m_type = E_GenericAttributeType_S64;
}

void C_GenericAttribute::SetValue(F32 value) 
{
	if (m_type == E_GenericAttributeType_F32)
	{
		(*(F32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<F32>::Allocate();
	(*(F32*)m_pValue) = value;
#else
	m_pValue = new F32(value);
#endif
	m_type = E_GenericAttributeType_F32;
}

void C_GenericAttribute::SetValue(F64 value)
{
	if (m_type == E_GenericAttributeType_F64)
	{
		(*(F64*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<F64>::Allocate();
	(*(F64*)m_pValue) = value;
#else
	m_pValue = new F64(value);
#endif
	m_type = E_GenericAttributeType_F64;
}

void C_GenericAttribute::SetValue(const Vector4F64& value) 
{
	if (m_type == E_GenericAttributeType_Vector4F64)
	{
		(*(Vector4F64*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector4F64>::Allocate();
	(*(Vector4F64*)m_pValue) = value;
#else
	m_pValue = new Vector4F64(value);
#endif
	m_type = E_GenericAttributeType_Vector4F64;
}

void C_GenericAttribute::SetValue(const Vector4F32& value) 
{
	if (m_type == E_GenericAttributeType_Vector4F32)
	{
		(*(Vector4F32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector4F32>::Allocate();
	(*(Vector4F32*)m_pValue) = value;
#else
	m_pValue = new Vector4F32(value);
#endif
	m_type = E_GenericAttributeType_Vector4F32;
}

void C_GenericAttribute::SetValue(const Vector4S64& value) 
{
	if (m_type == E_GenericAttributeType_Vector4S64)
	{
		(*(Vector4S64*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector4S64>::Allocate();
	(*(Vector4S64*)m_pValue) = value;
#else
	m_pValue = (void *)new Vector4S64(value);
#endif
	m_type = E_GenericAttributeType_Vector4S64;
}

void C_GenericAttribute::SetValue(const Vector4S32& value) 
{
	if (m_type == E_GenericAttributeType_Vector4S32)
	{
		(*(Vector4S32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector4S32>::Allocate();
	(*(Vector4S32*)m_pValue) = value;
#else
	m_pValue = new Vector4S32(value);
#endif
	m_type = E_GenericAttributeType_Vector4F64;
}

void C_GenericAttribute::SetValue(const Vector4S16& value) 
{
	if (m_type == E_GenericAttributeType_Vector4S16)
	{
		(*(Vector4S16*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector4S16>::Allocate();
	(*(Vector4S16*)m_pValue) = value;
#else
	m_pValue = new Vector4S16(value);
#endif
	m_type = E_GenericAttributeType_Vector4S16;
}

void C_GenericAttribute::SetValue(const Vector4S8& value) 
{
	if (m_type == E_GenericAttributeType_Vector4S8)
	{
		(*(Vector4S8*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector4S8>::Allocate();
	(*(Vector4S16*)m_pValue) = value;
#else
	m_pValue = new Vector4S8(value);
#endif
	m_type = E_GenericAttributeType_Vector4S8;
}

void C_GenericAttribute::SetValue(const Vector4U64& value) 
{
	if (m_type == E_GenericAttributeType_Vector4U64)
	{
		(*(Vector4U64*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector4U64>::Allocate();
	(*(Vector4U64*)m_pValue) = value;
#else
	m_pValue = new Vector4U64(value);
#endif
	m_type = E_GenericAttributeType_Vector4U64;
}

void C_GenericAttribute::SetValue(const Vector4U32& value) 
{
	if (m_type == E_GenericAttributeType_Vector4U32)
	{
		(*(Vector4U32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector4U64>::Allocate();
	(*(Vector4U32*)m_pValue) = value;
#else
	m_pValue = new Vector4U32(value);
#endif
	m_type = E_GenericAttributeType_Vector4U32;
}

void C_GenericAttribute::SetValue(const Vector4U16& value) 
{
	if (m_type == E_GenericAttributeType_Vector4U16)
	{
		(*(Vector4U16*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector4U16>::Allocate();
	(*(Vector4U16*)m_pValue) = value;
#else
	m_pValue = new Vector4U16(value);
#endif
	m_type = E_GenericAttributeType_Vector4U16;
}

void C_GenericAttribute::SetValue(const Vector4U8& value) 
{
	if (m_type == E_GenericAttributeType_Vector4U8)
	{
		(*(Vector4U8*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector4U8>::Allocate();
	(*(Vector4U8*)m_pValue) = value;
#else
	m_pValue = new Vector4U8(value);
#endif
	m_type = E_GenericAttributeType_Vector4U8;
}


void C_GenericAttribute::SetValue(const Vector3F64& value) 
{
	if (m_type == E_GenericAttributeType_Vector3F64)
	{
		(*(Vector3F64*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector3F64>::Allocate();
	(*(Vector3F64*)m_pValue) = value;
#else
	m_pValue = new Vector3F64(value);
#endif
	m_type = E_GenericAttributeType_Vector3F64;
}

void C_GenericAttribute::SetValue(const Vector3F32& value) 
{
	if (m_type == E_GenericAttributeType_Vector3F32)
	{
		(*(Vector3F32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector3F32>::Allocate();
	(*(Vector3F32*)m_pValue) = value;
#else
	m_pValue = new Vector3F32(value);
#endif
	m_type = E_GenericAttributeType_Vector3F32;
}

void C_GenericAttribute::SetValue(const Vector3S64& value) 
{
	if (m_type == E_GenericAttributeType_Vector3S64)
	{
		(*(Vector3S64*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector3S64>::Allocate();
	(*(Vector3S64*)m_pValue) = value;
#else
	m_pValue = new Vector3S64(value);
#endif
	m_type = E_GenericAttributeType_Vector3S64;
}

void C_GenericAttribute::SetValue(const Vector3S32& value) 
{
	if (m_type == E_GenericAttributeType_Vector3S32)
	{
		(*(Vector3S32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector3S32>::Allocate();
	(*(Vector3S32*)m_pValue) = value;
#else
	m_pValue = new Vector3S32(value);
#endif
	m_type = E_GenericAttributeType_Vector3S32;
}

void C_GenericAttribute::SetValue(const Vector3S16& value) 
{
	if (m_type == E_GenericAttributeType_Vector3S16)
	{
		(*(Vector3S16*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector3S16>::Allocate();
	(*(Vector3S16*)m_pValue) = value;
#else
	m_pValue = new Vector3S16(value);
#endif
	m_type = E_GenericAttributeType_Vector3S16;
}

void C_GenericAttribute::SetValue(const Vector3S8& value) 
{
	if (m_type == E_GenericAttributeType_Vector3S8)
	{
		(*(Vector3S8*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector3S8>::Allocate();
	(*(Vector3S8*)m_pValue) = value;
#else
	m_pValue = new Vector3S8(value);
#endif
	m_type = E_GenericAttributeType_Vector3S8;
}

void C_GenericAttribute::SetValue(const Vector3U64& value) 
{
	if (m_type == E_GenericAttributeType_Vector3U64)
	{
		(*(Vector3U64*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector3S8>::Allocate();
	(*(Vector3U64*)m_pValue) = value;
#else
	m_pValue = new Vector3U64(value);
#endif
	m_type = E_GenericAttributeType_Vector3U64;
}

void C_GenericAttribute::SetValue(const Vector3U32& value) 
{
	if (m_type == E_GenericAttributeType_Vector3U32)
	{
		(*(Vector3U32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector3U32>::Allocate();
	(*(Vector3U32*)m_pValue) = value;
#else
	m_pValue = new Vector3U32(value);
#endif
	m_type = E_GenericAttributeType_Vector3U32;
}

void C_GenericAttribute::SetValue(const Vector3U16& value) 
{
	if (m_type == E_GenericAttributeType_Vector3U16)
	{
		(*(Vector3U16*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector3U16>::Allocate();
	(*(Vector3U16*)m_pValue) = value;
#else
	m_pValue = new Vector3U16(value);
#endif
	m_type = E_GenericAttributeType_Vector3U16;
}

void C_GenericAttribute::SetValue(const Vector3U8& value) 
{
	if (m_type == E_GenericAttributeType_Vector3U8)
	{
		(*(Vector3U8*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector3U8>::Allocate();
	(*(Vector3U8*)m_pValue) = value;
#else
	m_pValue = new Vector3U8(value);
#endif
	m_type = E_GenericAttributeType_Vector3U8;
}

void C_GenericAttribute::SetValue(const Vector2F64& value) 
{
	if (m_type == E_GenericAttributeType_Vector2F64)
	{
		(*(Vector2F64*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector2F64>::Allocate();
	(*(Vector2F64*)m_pValue) = value;
#else
	m_pValue = new Vector2F64(value);
#endif
	m_type = E_GenericAttributeType_Vector2F64;
}

void C_GenericAttribute::SetValue(const Vector2F32& value) 
{
	if (m_type == E_GenericAttributeType_Vector2F32)
	{
		(*(Vector2F32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector2F32>::Allocate();
	(*(Vector2F32*)m_pValue) = value;
#else
	m_pValue = new Vector2F32(value);
#endif
	m_type = E_GenericAttributeType_Vector2F32;
}

void C_GenericAttribute::SetValue(const Vector2S64& value) 
{
	if (m_type == E_GenericAttributeType_Vector2S64)
	{
		(*(Vector2S64*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector2S64>::Allocate();
	(*(Vector2S64*)m_pValue) = value;
#else
	m_pValue = new Vector2S64(value);
#endif
	m_type = E_GenericAttributeType_Vector2S64;
}

void C_GenericAttribute::SetValue(const Vector2S32& value) 
{
	if (m_type == E_GenericAttributeType_Vector2S32)
	{
		(*(Vector2S32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector2S32>::Allocate();
	(*(Vector2S32*)m_pValue) = value;
#else
	m_pValue = new Vector2S32(value);
#endif
	m_type = E_GenericAttributeType_Vector2S32;
}

void C_GenericAttribute::SetValue(const Vector2S16& value) 
{
	if (m_type == E_GenericAttributeType_Vector2S16)
	{
		(*(Vector2S16*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector2S16>::Allocate();
	(*(Vector2S16*)m_pValue) = value;
#else
	m_pValue = new Vector2S16(value);
#endif
	m_type = E_GenericAttributeType_Vector2S16;
}

void C_GenericAttribute::SetValue(const Vector2S8& value) 
{
	if (m_type == E_GenericAttributeType_Vector2S8)
	{
		(*(Vector2S8*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector2S8>::Allocate();
	(*(Vector2S8*)m_pValue) = value;
#else
	m_pValue = new Vector2S8(value);
#endif
	m_type = E_GenericAttributeType_Vector2S8;
}

void C_GenericAttribute::SetValue(const Vector2U32& value) 
{
	if (m_type == E_GenericAttributeType_Vector2U32)
	{
		(*(Vector2U32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector2U32>::Allocate();
	(*(Vector2U32*)m_pValue) = value;
#else
	m_pValue = new Vector2U32(value);
#endif
	m_type = E_GenericAttributeType_Vector2U32;
}

void C_GenericAttribute::SetValue(const Vector2U64& value) 
{
	if (m_type == E_GenericAttributeType_Vector2U64)
	{
		(*(Vector2U64*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector2U64>::Allocate();
	(*(Vector2U64*)m_pValue) = value;
#else
	m_pValue = new Vector2U64(value);
#endif
	m_type = E_GenericAttributeType_Vector2U64;
}

void C_GenericAttribute::SetValue(const Vector2U16& value) 
{
	if (m_type == E_GenericAttributeType_Vector2U16)
	{
		(*(Vector2U16*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector2U16>::Allocate();
	(*(Vector2U64*)m_pValue) = value;
#else
	m_pValue = new Vector2U16(value);
#endif
	m_type = E_GenericAttributeType_Vector2U16;
}

void C_GenericAttribute::SetValue(const Vector2U8& value) 
{
	if (m_type == E_GenericAttributeType_Vector2U8)
	{
		(*(Vector2U8*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Vector2U8>::Allocate();
	(*(Vector2U8*)m_pValue) = value;
#else
	m_pValue = new Vector2U8(value);
#endif
	m_type = E_GenericAttributeType_Vector2U8;
}

void C_GenericAttribute::SetValue(const Matrix44F32& value) 
{
	if (m_type == E_GenericAttributeType_Matrix44F32)
	{
		(*(Matrix44F32*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<Matrix44F32>::Allocate();
	(*(Matrix44F32*)m_pValue) = value;
#else
	m_pValue = new Matrix44F32(value);
#endif
	m_type = E_GenericAttributeType_Matrix44F32;
}


void C_GenericAttribute::SetValue(bool value) 
{
	if (m_type == E_GenericAttributeType_Boolean)
	{
		(*(bool*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_Allocator<bool>::Allocate();
	(*(bool*)m_pValue) = value;
#else
	m_pValue = new bool(value);
#endif
	m_type = E_GenericAttributeType_Boolean;
}

void C_GenericAttribute::SetValue(const std::string &value) 
{
	if (m_type == E_GenericAttributeType_String)
	{
		(*(std::string*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_ArrayAllocator<std::string>::Allocate(1);
	(*(std::string*)m_pValue) = value;
#else
	m_pValue = new std::string(value);
#endif
	m_type = E_GenericAttributeType_String;
}

void C_GenericAttribute::SetValue(const char* value)
{
	if (m_type == E_GenericAttributeType_String)
	{
		(*(std::string*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_ArrayAllocator<std::string>::Allocate(1);
	(*(std::string*)m_pValue) = value;
#else
	m_pValue = new std::string(value);
#endif
	m_type = E_GenericAttributeType_String;
}

void C_GenericAttribute::SetValue(const C_BlobData &value)
{
	if (m_type == E_GenericAttributeType_BlobData)
	{
		(*(C_BlobData*)m_pValue) = value;
		return;
	}
	Clear();
#ifdef AFTK_USE_MEMORY_POOL
	m_pValue = C_ArrayAllocator<C_BlobData>::Allocate(1);
	(*(C_BlobData*)m_pValue) = value;
#else
	m_pValue = new C_BlobData(value);
#endif
	m_type = E_GenericAttributeType_BlobData;

}

const std::string& C_GenericAttribute::GetName() const 
{
	return m_name;
}

E_GenericAttributeType C_GenericAttribute::GetDataType() const 
{
	return m_type;
}

void C_GenericAttribute::Clear() 
{
	if (m_pValue) 
	{
		switch (m_type)
		{
			case E_GenericAttributeType_U8:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<U8>::Free((U8*)m_pValue);
#else
				delete (U8*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_U16:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<U16>::Free((U16*)m_pValue);
#else
				delete (U16*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_U32:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<U32>::Free((U32*)m_pValue);
#else
				delete (U32*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_U64:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<U64>::Free((U64*)m_pValue);
#else
				delete (U64*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_S8:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<S8>::Free((S8*)m_pValue);
#else
				delete (S8*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_S16:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<S16>::Free((S16*)m_pValue);
#else
				delete (S16*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_S64:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<S64>::Free((S64*)m_pValue);
#else
				delete (S64*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_S32:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<S32>::Free((S32*)m_pValue);
#else
				delete (S32*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_F32:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<F32>::Free((F32*)m_pValue);
#else
				delete (F32*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_F64:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<F64>::Free((F64*)m_pValue);
#else
				delete (F64*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector4F32:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector4F32>::Free((Vector4F32*)m_pValue);
#else
				delete (Vector4F32*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector3F32:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector3F32>::Free((Vector3F32*)m_pValue);
#else
				delete (Vector3F32*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector2F32:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector2F32>::Free((Vector2F32*)m_pValue);
#else
				delete (Vector2F32*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Matrix44F32:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Matrix44F32>::Free((Matrix44F32*)m_pValue);
#else
				delete (Matrix44F32*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Boolean:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<bool>::Free((bool*)m_pValue);
#else
				delete (bool*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_String:
			{
#ifdef AFTK_USE_MEMORY_POOL
//				delete (std::string*)m_pValue;
				C_ArrayAllocator<std::string>::Free((std::string*)m_pValue, 1);
#else
				delete (std::string*)m_pValue;
#endif
				break;
			}
				
			case E_GenericAttributeType_Vector2F64:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector2F64>::Free((Vector2F64*)m_pValue);
#else
				delete (Vector2F64*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector2S64:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector2S64>::Free((Vector2S64*)m_pValue);
#else
				delete (Vector2S64*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector2S32:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector2S32>::Free((Vector2S32*)m_pValue);
#else
				delete (Vector2S32*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector2S16:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector2S16>::Free((Vector2S16*)m_pValue);
#else
				delete (Vector2S16*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector2S8:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector2S8>::Free((Vector2S8*)m_pValue);
#else
				delete (Vector2S8*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector2U64:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector2U64>::Free((Vector2U64*)m_pValue);
#else
				delete (Vector2U64*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector2U32:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector2U32>::Free((Vector2U32*)m_pValue);
#else
				delete (Vector2U32*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector2U16:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector2U16>::Free((Vector2U16*)m_pValue);
#else
				delete (Vector2U16*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector2U8:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector2U8>::Free((Vector2U8*)m_pValue);
#else
				delete (Vector2U8*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector3F64:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector3F64>::Free((Vector3F64*)m_pValue);
#else
				delete (Vector3F64*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector3S64:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector3S64>::Free((Vector3S64*)m_pValue);
#else
				delete (Vector3S64*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector3S32:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector3S32>::Free((Vector3S32*)m_pValue);
#else
				delete (Vector3S32*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector3S16:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector3S16>::Free((Vector3S16*)m_pValue);
#else
				delete (Vector3S16*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector3S8:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector2S8>::Free((Vector2S8*)m_pValue);
#else
				delete (Vector2S8*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector3U64:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector2U64>::Free((Vector2U64*)m_pValue);
#else
				delete (Vector2U64*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector3U32:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector3U32>::Free((Vector3U32*)m_pValue);
#else
				delete (Vector3U32*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector3U16:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector3U16>::Free((Vector3U16*)m_pValue);
#else
				delete (Vector3U16*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector3U8:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector3U8>::Free((Vector3U8*)m_pValue);
#else
				delete (Vector3U8*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector4F64:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector4F64>::Free((Vector4F64*)m_pValue);
#else
				delete (Vector4F64*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector4S64:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector4S64>::Free((Vector4S64*)m_pValue);
#else
				delete (Vector4S64*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector4S32:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector4S32>::Free((Vector4S32*)m_pValue);
#else
				delete (Vector4S32*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector4S16:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector4S16>::Free((Vector4S16*)m_pValue);
#else
				delete (Vector4S16*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector4S8:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector4S8>::Free((Vector4S8*)m_pValue);
#else
				delete (Vector4S8*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector4U64:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector4U64>::Free((Vector4U64*)m_pValue);
#else
				delete (Vector4U64*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector4U32:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector4U32>::Free((Vector4U32*)m_pValue);
#else
				delete (Vector4U32*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector4U16:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector4U16>::Free((Vector4U16*)m_pValue);
#else
				delete (Vector4U16*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Vector4U8:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_Allocator<Vector4U8>::Free((Vector4U8*)m_pValue);
#else
				delete (Vector4U8*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_BlobData:
			{
#ifdef AFTK_USE_MEMORY_POOL
				C_ArrayAllocator<C_BlobData>::Free((C_BlobData*)m_pValue, 1);
#else
				delete (C_BlobData*)m_pValue;
#endif
				break;
			}

			case E_GenericAttributeType_Null:
			default:
				break;
		}
	}

	m_pValue = 0;
	m_type = E_GenericAttributeType_Null;
}

U32 C_GenericAttribute::GetHash() const
{
	switch (m_type) {
	case E_GenericAttributeType_U8: {
		U8 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<U8>::GetHash(value);
	}
	case E_GenericAttributeType_U16: {
		U16 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<U16>::GetHash(value);
	}
	case E_GenericAttributeType_U32: {
		U32 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<U32>::GetHash(value);
	}
	case E_GenericAttributeType_U64: {
		U64 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<U64>::GetHash(value);
	}
	case E_GenericAttributeType_S8: {
		S8 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<S8>::GetHash(value);
	}
	case E_GenericAttributeType_S16: {
		S16 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<S16>::GetHash(value);
	}
	case E_GenericAttributeType_S32: {
		S32 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<S32>::GetHash(value);
	}
	case E_GenericAttributeType_S64: {
		S64 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<S64>::GetHash(value);
	}
	case E_GenericAttributeType_F32: {
		F32 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<F32>::GetHash(value);
	}
	case E_GenericAttributeType_F64: {
		F64 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<F64>::GetHash(value);
	}

	case E_GenericAttributeType_Vector4F64: {
		Vector4F64 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector4F64>::GetHash(value);
	}
	case E_GenericAttributeType_Vector4F32: {
		Vector4F32 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector4F32>::GetHash(value);
	}
	case E_GenericAttributeType_Vector4S64: {
		Vector4S64 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector4S64>::GetHash(value);
	}	
	case E_GenericAttributeType_Vector4S32: {
		Vector4S32 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector4S32>::GetHash(value);
	}
	case E_GenericAttributeType_Vector4S16: {
		Vector4S16 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector4S16>::GetHash(value);
	}	
	case E_GenericAttributeType_Vector4S8: {
		Vector4S8 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector4S8>::GetHash(value);
	}
	case E_GenericAttributeType_Vector4U64: {
		Vector4U64 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector4U64>::GetHash(value);
	}
	case E_GenericAttributeType_Vector4U32: {
		Vector4U32 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector4U32>::GetHash(value);
	}
	case E_GenericAttributeType_Vector4U16: {
		Vector4U16 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector4U16>::GetHash(value);
	}	
	case E_GenericAttributeType_Vector4U8: {
		Vector4U8 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector4U8>::GetHash(value);
	}

	case E_GenericAttributeType_Vector3F64: {
		Vector3F64 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector3F64>::GetHash(value);
	}
	case E_GenericAttributeType_Vector3F32: {
		Vector3F32 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector3F32>::GetHash(value);
	}
	case E_GenericAttributeType_Vector3S64: {
		Vector3S64 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector3S64>::GetHash(value);
	}	
	case E_GenericAttributeType_Vector3S32: {
		Vector3S32 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector3S32>::GetHash(value);
	}	
	case E_GenericAttributeType_Vector3S16: {
		Vector3S16 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector3S16>::GetHash(value);
	}	
	case E_GenericAttributeType_Vector3S8: {
		Vector3S8 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector3S8>::GetHash(value);
	}
	case E_GenericAttributeType_Vector3U64: {
		Vector3U64 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector3U64>::GetHash(value);
	}
	case E_GenericAttributeType_Vector3U32: {
		Vector3U32 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector3U32>::GetHash(value);
	}
	case E_GenericAttributeType_Vector3U16: {
		Vector3U16 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector3U16>::GetHash(value);
	}	
	case E_GenericAttributeType_Vector3U8: {
		Vector3U8 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector3U8>::GetHash(value);
	}

	case E_GenericAttributeType_Vector2F64: {
		Vector2F64 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector2F64>::GetHash(value);
	}
	case E_GenericAttributeType_Vector2F32: {
		Vector2F32 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector2F32>::GetHash(value);
	}
	case E_GenericAttributeType_Vector2S64: {
		Vector2S64 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector2S64>::GetHash(value);
	}	
	case E_GenericAttributeType_Vector2S32: {
		Vector2S32 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector2S32>::GetHash(value);
	}	
	case E_GenericAttributeType_Vector2S16: {
		Vector2S16 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector2S16>::GetHash(value);
	}	
	case E_GenericAttributeType_Vector2S8: {
		Vector2S8 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector2S8>::GetHash(value);
	}
	case E_GenericAttributeType_Vector2U64: {
		Vector2U64 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector2U64>::GetHash(value);
	}
	case E_GenericAttributeType_Vector2U32: {
		Vector2U32 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector2U32>::GetHash(value);
	}
	case E_GenericAttributeType_Vector2U16: {
		Vector2U16 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector2U16>::GetHash(value);
	}	
	case E_GenericAttributeType_Vector2U8: {
		Vector2U8 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Vector2U8>::GetHash(value);
	}

	case E_GenericAttributeType_Matrix44F32: {
		Matrix44F32 value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<Matrix44F32>::GetHash(value);
	}
	case E_GenericAttributeType_Boolean: {
		bool value;
		GetValue(value);
		return Ut::HashKeyOpByteArray<bool>::GetHash(value);
	}
	case E_GenericAttributeType_String: {
		return Ut::HashKeyOpString::GetHash(((std::string*)m_pValue)->c_str());
	}
	case E_GenericAttributeType_BlobData: {
		//Generate hash using first 1/2/4 bytes of blob
		void* pData = ((C_BlobData*)m_pValue)->GetDataPtr();
		U32 size = ((C_BlobData*)m_pValue)->GetSize();
		if (size > 3)
			return Ut::HashKeyOpByteArray<U32>::GetHash(*((U32*)pData));
		else if (size > 1)
			return Ut::HashKeyOpByteArray<U16>::GetHash(*((U16*)pData));
		else if (size > 0)
			return Ut::HashKeyOpByteArray<U8>::GetHash(*((U8*)pData));
		else
			return 0;
	}
	default:
		return 0;
		break;
	}
}

void C_GenericAttribute::Output(const C_GenericAttribute& genericAttribute)
{
	/*
	switch (genericAttribute.GetDataType())
	{
		case E_GenericAttributeType_F64:
		{
			F64 val;
			genericAttribute.GetValue(val);
			AFTK_DO_BEGIN << (F32)val << "\n" AFTK_DO_INFO_END;
			break;
		}
		case E_GenericAttributeType_F32:
		{
			F32 val;
			genericAttribute.GetValue(val);
			AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
			break;
		}
		case E_GenericAttributeType_U8:
		{
			U8 val;
			genericAttribute.GetValue(val);
			AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
			break;
		}
		case E_GenericAttributeType_U16:
		{
			U16 val;
			genericAttribute.GetValue(val);
			AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
			break;
		}
		case E_GenericAttributeType_U32:
		{
			U32 val;
			genericAttribute.GetValue(val);
			AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
			break;
		}
		case E_GenericAttributeType_S8:
		{
			S8 val;
			genericAttribute.GetValue(val);
			AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
			break;
		}
		case E_GenericAttributeType_S16:
		{
			S16 val;
			genericAttribute.GetValue(val);
			AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
			break;
		}
		case E_GenericAttributeType_S32:
		{
			S32 val;
			genericAttribute.GetValue(val);
			AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
			break;
		}
		case E_GenericAttributeType_Vector2F32: 
		{
			Vector2F32 val;
			genericAttribute.GetValue(val);
			AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
			break;
		}
		case E_GenericAttributeType_Vector3F32: 
		{
			Vector3F32 val;
			genericAttribute.GetValue(val);
			AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
			break;
		}
		case E_GenericAttributeType_Vector4F32: 
		{
			Vector4F32 val;
			genericAttribute.GetValue(val);
			AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
			break;
		}
		case E_GenericAttributeType_String: 
		{
			std::string val;
			genericAttribute.GetValue(val);
			AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
			break;
		}

		case E_GenericAttributeType_Null:
		{
			AFTK_DO_BEGIN << "NULL\n" AFTK_DO_INFO_END;
			break;
		}

		default:
			AFTK_DO_BEGIN << "UNKNOWN" << "\n" AFTK_DO_INFO_END;
			break;
	};
	*/
	AFTK_DO_BEGIN << "Function deprecated." << "\n" AFTK_DO_INFO_END;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_GenericAttribute::OneTimeInit()
{
	m_pValue = 0;
	m_type = E_GenericAttributeType_Null;
}

#endif
