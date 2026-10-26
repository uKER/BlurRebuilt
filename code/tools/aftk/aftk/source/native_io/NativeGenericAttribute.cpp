#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeGenericAttribute.h"
#include "NativeUtility.h"
#include "NativeIObject.h"

using namespace Aftk;

C_Status C_NativeGenericAttribute::Serialise(
	C_NativeDiskWriter* pDiskWriter,
	C_NativeFtk* pNativeFtk, 
	const C_GenericAttribute* pGenericAttribute,
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_GENERIC_ATTRIBUTE);
	}
	{
		//Write out attribute name
		*pDiskWriter << pGenericAttribute->GetName();

		switch (pGenericAttribute->GetDataType())
		{
			case E_GenericAttributeType_U8: 
			{
				U32 type = NATIVE_TYPE_ID_U8;
				*pDiskWriter << type;
				U8 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_U16: 
			{
				U32 type = NATIVE_TYPE_ID_U16;
				*pDiskWriter << type;
				U16 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_U32: 
			{
				U32 type = NATIVE_TYPE_ID_U32;
				*pDiskWriter << type;
				U32 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_U64: 
			{
				U32 type = NATIVE_TYPE_ID_U64;
				*pDiskWriter << type;
				U64 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_S8:
			{
				U32 type = NATIVE_TYPE_ID_S8;
				*pDiskWriter << type;
				S8 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_S16: 
			{
				U32 type = NATIVE_TYPE_ID_S16;
				*pDiskWriter << type;
				S16 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_S32: 
			{
				U32 type = NATIVE_TYPE_ID_S32;
				*pDiskWriter << type;
				S32 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_S64: 
			{
				U32 type = NATIVE_TYPE_ID_S64;
				*pDiskWriter << type;
				S64 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_F32: 
			{
				U32 type = NATIVE_TYPE_ID_F32;
				*pDiskWriter << type;
				F32 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_F64: 
			{
				U32 type = NATIVE_TYPE_ID_F64;
				*pDiskWriter << type;
				F64 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_Vector4F64: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR4F64;
				*pDiskWriter << type;
				Vector4F64 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_Vector4F32: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR4F32;
				*pDiskWriter << type;
				Vector4F32 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}	
			case E_GenericAttributeType_Vector4S64: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR4S64;
				*pDiskWriter << type;
				Vector4S64 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}	
			case E_GenericAttributeType_Vector4S32: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR4S32;
				*pDiskWriter << type;
				Vector4S32 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}			
			case E_GenericAttributeType_Vector4S16: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR4S16;
				*pDiskWriter << type;
				Vector4S16 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}			
			case E_GenericAttributeType_Vector4S8: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR4S8;
				*pDiskWriter << type;
				Vector4S8 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}	
			case E_GenericAttributeType_Vector4U64: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR4U64;
				*pDiskWriter << type;
				Vector4U64 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}	
			case E_GenericAttributeType_Vector4U32: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR4U32;
				*pDiskWriter << type;
				Vector4U32 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}			
			case E_GenericAttributeType_Vector4U16: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR4U16;
				*pDiskWriter << type;
				Vector4U16 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_Vector4U8: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR4U8;
				*pDiskWriter << type;
				Vector4U8 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}

			case E_GenericAttributeType_Vector3F64: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR3F64;
				*pDiskWriter << type;
				Vector3F64 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_Vector3F32: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR3F32;
				*pDiskWriter << type;
				Vector3F32 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_Vector3S64: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR3S64;
				*pDiskWriter << type;
				Vector3S64 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_Vector3S32: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR3S32;
				*pDiskWriter << type;
				Vector3S32 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}			
			case E_GenericAttributeType_Vector3S16: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR3S16;
				*pDiskWriter << type;
				Vector3S16 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}			
			case E_GenericAttributeType_Vector3S8: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR3S8;
				*pDiskWriter << type;
				Vector3S8 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}	
			case E_GenericAttributeType_Vector3U64: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR3U64;
				*pDiskWriter << type;
				Vector3U64 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_Vector3U32: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR3U32;
				*pDiskWriter << type;
				Vector3U32 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}			
			case E_GenericAttributeType_Vector3U16: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR3U16;
				*pDiskWriter << type;
				Vector3U16 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_Vector3U8: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR3U8;
				*pDiskWriter << type;
				Vector3U8 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}

			case E_GenericAttributeType_Vector2F64: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR2F64;
				*pDiskWriter << type;
				Vector2F64 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_Vector2F32: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR2F32;
				*pDiskWriter << type;
				Vector2F32 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}	
			case E_GenericAttributeType_Vector2S64: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR2S64;
				*pDiskWriter << type;
				Vector2S64 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_Vector2S32: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR2S32;
				*pDiskWriter << type;
				Vector2S32 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}			
			case E_GenericAttributeType_Vector2S16: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR2S16;
				*pDiskWriter << type;
				Vector2S16 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}			
			case E_GenericAttributeType_Vector2S8: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR2S8;
				*pDiskWriter << type;
				Vector2S8 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}			
			case E_GenericAttributeType_Vector2U64: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR2U64;
				*pDiskWriter << type;
				Vector2U64 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}	
			case E_GenericAttributeType_Vector2U32: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR2U32;
				*pDiskWriter << type;
				Vector2U32 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}	
			case E_GenericAttributeType_Vector2U16: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR2U16;
				*pDiskWriter << type;
				Vector2U16 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_Vector2U8: 
			{
				U32 type = NATIVE_TYPE_ID_VECTOR2U8;
				*pDiskWriter << type;
				Vector2U8 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}

			case E_GenericAttributeType_Matrix44F32: 
			{
				U32 type = NATIVE_TYPE_ID_MATRIX44F32;
				*pDiskWriter << type;
				Matrix44 value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_Boolean:
			{
				U32 type = NATIVE_TYPE_ID_BOOLEAN;
				*pDiskWriter << type;
				bool value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_String: 
			{
				U32 type = NATIVE_TYPE_ID_STRING;
				*pDiskWriter << type;
				std::string value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			case E_GenericAttributeType_BlobData: 
			{
				U32 type = NATIVE_TYPE_ID_BLOB;
				*pDiskWriter << type;
				C_BlobData value;
				pGenericAttribute->GetValue(value);
				*pDiskWriter << value;
				break;
			}
			default:
				U32 type = NATIVE_TYPE_ID_NULL;
				*pDiskWriter << type;
				break;
		}
	}
	if (useOuterBlockHeaders)
	{
		pDiskWriter->EndBlock();
	}

	return C_Status::E_Status_Success;
}

C_Status C_NativeGenericAttribute::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_GenericAttribute* pGenericAttribute,
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_GENERIC_ATTRIBUTE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find generic attribute block.");
		}
		pDiskReader->EnterBlock();
	}
	{
		//Read in attribute name
		{
			//Reset name if needed
			if (pGenericAttribute->m_name.length() != 0)
				pGenericAttribute->m_name.clear();
			*pDiskReader >> pGenericAttribute->m_name;
		}
		U32 typeId;
		pDiskReader->ReadBytes(&typeId, sizeof(U32), false);

		if (typeId == NATIVE_TYPE_ID_U8)
		{
			U8 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_U16)
		{
			U16 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_U64)
		{
			U32 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_U32)
		{
			U32 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_S8)
		{
			S8 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_S16)
		{
			S16 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_S64)
		{
			S32 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_S32)
		{
			S32 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_F32)
		{
			F32 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_F64)
		{
			F64 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR4F64)
		{
			Vector4F64 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR4F32)
		{
			Vector4F32 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR4S64)
		{
			Vector4S64 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR4S32)
		{
			Vector4S32 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR4S16)
		{
			Vector4S16 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR4S8)
		{
			Vector4S8 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR4U64)
		{
			Vector4U64 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR4U32)
		{
			Vector4U32 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR4U16)
		{
			Vector4U16 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR4U8)
		{
			Vector4U8 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR3F64)
		{
			Vector3F64 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR3F32)
		{
			Vector3F32 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR3S64)
		{
			Vector3S64 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR3S32) 
		{
			Vector3S32 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR3S16)
		{
			Vector3S16 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR3S8)
		{
			Vector3S8 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR3U64)
		{
			Vector3U64 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR3U32)
		{
			Vector3U32 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR3U16)
		{
			Vector3U16 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR3U8) 
		{
			Vector3U8 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR2F64) 
		{
			Vector2F64 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR2F32)
		{
			Vector2F32 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR2S64)
		{
			Vector2S64 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR2S32) 
		{
			Vector2S32 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR2S16)
		{
			Vector2S16 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR2S8)
		{
			Vector2S8 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR2U64)
		{
			Vector2U64 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR2U32)
		{
			Vector2U32 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR2U16)
		{
			Vector2U16 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_VECTOR2U8)
		{
			Vector2U8 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_MATRIX44F32)
		{
			Matrix44 value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}
		else if (typeId == NATIVE_TYPE_ID_BOOLEAN)
		{
			bool value;
			*pDiskReader >> value;
			pGenericAttribute->SetValue(value);
		}

		else if (typeId == NATIVE_TYPE_ID_STRING)
		{
			if (pGenericAttribute->m_pValue != 0)
			{
				AFTK_ASSERT(pGenericAttribute->m_type == E_GenericAttributeType_String);
				*pDiskReader >> (*(std::string*)pGenericAttribute->m_pValue);
			}
			else
			{
				E_GenericAttributeType type = E_GenericAttributeType_String;
			#ifdef AFTK_USE_MEMORY_POOL
				void* pValue = C_ArrayAllocator<std::string>::Allocate(1);
			#else
				void* pValue = new std::string;
			#endif
				*pDiskReader >> (*(std::string*)pValue);
				pGenericAttribute->SetValue(pValue, type);
			}
		}
		else if (typeId == NATIVE_TYPE_ID_BLOB)
		{
			if (pGenericAttribute->m_pValue != 0)
			{
				AFTK_ASSERT(pGenericAttribute->m_type == E_GenericAttributeType_BlobData);
				*pDiskReader >> (*(C_BlobData*)pGenericAttribute->m_pValue);
			}
			else
			{
				E_GenericAttributeType type = E_GenericAttributeType_BlobData;
			#ifdef AFTK_USE_MEMORY_POOL
				void* pValue = C_ArrayAllocator<C_BlobData>::Allocate(1);
			#else
				void* pValue = new BlobData();
			#endif
				*pDiskReader >> (*(C_BlobData*)pValue);
			}
		}
	}

	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}

	return C_Status::E_Status_Success;
}

