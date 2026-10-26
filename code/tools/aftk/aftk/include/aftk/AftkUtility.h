#ifndef AFTK_UTILITY_H
#define AFTK_UTILITY_H

#ifdef _MSC_VER_
#pragma once
#endif

#ifdef PLATFORM_WINDOWS
#include <crtdbg.h>
#endif

#define AFTK_ASSERT(x) assert(x)

namespace Aftk
{
	template<class T>
	class HashKeyOpSet
	{
	public:
		//Hashing function.
		static unsigned int GetHash(const Ut::UtSet<T>& key)
		{
			unsigned int hash = 0;
			for (typename Ut::UtSet<T>::ConstIterator it = key.Begin(); it != key.End(); ++it)
				hash += it->GetKey();

			return hash;
		}

		static bool Equal(const Ut::UtSet<T>& key1, const Ut::UtSet<T>& key2)
		{
			return key1 == key2;
		}
	};

	template <class T>
	void InsertionSortVector3(
		Ut::UtVector< Ut::DynamicVector<T> >& v,
		Ut::UtVector< Ut::DynamicVector<U32> >& sortedIndexArray, 
		U32 axis,
		U32 low, 
		U32 high)
	{
		for (U32 p = low+1; p <= high; p++)
		{
			T tmp = v[p];
			U32 tmp2 = sortedIndexArray[p];
			U32 j;

			switch (axis)
			{
				case 0:
					for (j = p; j > 0 && tmp.X() < v[j-1].X(); j--)
					{
						v[j] = v[j-1];
						sortedIndexArray[j] = sortedIndexArray[j-1];
					}
					break;
				case 1:
					for (j = p; j > 0 && tmp.Y() < v[j-1].Y(); j--)
					{
						v[j] = v[j-1];
						sortedIndexArray[j] = sortedIndexArray[j-1];
					}
					break;
				case 2:
					for (j = p; j > 0 && tmp.Z() < v[j-1].Z(); j--)
					{
						v[j] = v[j-1];
						sortedIndexArray[j] = sortedIndexArray[j-1];
					}
					break;
				default:
					return;
			}
			v[j] = tmp;
			sortedIndexArray[j] = tmp2;
		}
	}

	U32 LinearInsert(
		Ut::UtVector< Ut::DynamicVector<Normal3> >& v,
		Normal3 var, 
		U32 low, 
		U32 high,
		F32 threshold);

	U32 LinearInsert(
		Ut::UtVector< Ut::DynamicVector<Vector3F32> >& v,
		Vector3F32 var, 
		U32 low, 
		U32 high,
		F32 threshold);

	U32 BinaryInsert(
		Ut::UtVector< Ut::DynamicVector<Vector3F32> >& v,
		Vector3F32 var, 
		U32 low, 
		U32 high,
		F32 threshold);

	U32 BinaryInsert(
		Ut::UtVector< Ut::DynamicVector<Normal3> >& v,
		Normal3 var, 
		U32 low, 
		U32 high,
		F32 threshold);

	std::string GetUniqueName(const std::string& oldName);

	//Returns the size of a data type in bytes.
	inline U32 GetDataTypeSize(E_GenericAttributeType type)
	{
		switch (type)
		{
		case E_GenericAttributeType_Null:
			return 0;

		case E_GenericAttributeType_U8:
		case E_GenericAttributeType_S8:
			return 1;

		case E_GenericAttributeType_U16:
		case E_GenericAttributeType_S16:
		case E_GenericAttributeType_Vector2S8:
		case E_GenericAttributeType_Vector2U8:
			return 2;

		case E_GenericAttributeType_Vector3S8:
		case E_GenericAttributeType_Vector3U8:
			return 3;

		case E_GenericAttributeType_U32:
		case E_GenericAttributeType_S32:
		case E_GenericAttributeType_F32:
		case E_GenericAttributeType_Boolean:
		case E_GenericAttributeType_Vector2S16:
		case E_GenericAttributeType_Vector2U16:
		case E_GenericAttributeType_Vector4S8:
		case E_GenericAttributeType_Vector4U8:
			return 4;

		case E_GenericAttributeType_Vector3S16:
		case E_GenericAttributeType_Vector3U16:
			return 6;

		case E_GenericAttributeType_U64:
		case E_GenericAttributeType_S64:
		case E_GenericAttributeType_F64:
		case E_GenericAttributeType_Vector2F32:
		case E_GenericAttributeType_Vector2S32:
		case E_GenericAttributeType_Vector2U32:
		case E_GenericAttributeType_Vector4S16:
		case E_GenericAttributeType_Vector4U16:
			return 8;

		case E_GenericAttributeType_Vector4F32:
		case E_GenericAttributeType_Vector2F64:
		case E_GenericAttributeType_Vector4S32:
		case E_GenericAttributeType_Vector4U32:
		case E_GenericAttributeType_Vector2U64:
		case E_GenericAttributeType_Vector2S64:
			return 16;

		case E_GenericAttributeType_Vector3F32:
		case E_GenericAttributeType_Vector3S32:
		case E_GenericAttributeType_Vector3U32:
			return 12;

		case E_GenericAttributeType_Vector3F64:
		case E_GenericAttributeType_Vector3U64:
		case E_GenericAttributeType_Vector3S64:
			return 24;

		case E_GenericAttributeType_Vector4F64:
		case E_GenericAttributeType_Vector4U64:
		case E_GenericAttributeType_Vector4S64:
			return 32;
		
		case E_GenericAttributeType_Matrix44F32:
			return 64;

		case E_GenericAttributeType_String:
			//Size unknown
			AFTK_ASSERT(type != E_GenericAttributeType_String);
			return 0;

		case E_GenericAttributeType_BlobData:
			//Size unknown
			AFTK_ASSERT(type != E_GenericAttributeType_BlobData);
			return 0;

		case E_GenericAttributeType_Unknown:
			//Size unknown
			AFTK_ASSERT(type != E_GenericAttributeType_Unknown);
			return 0;

		default:
			//Size unknown
			AFTK_ASSERT(false);
			return 0;
		}
	}

	bool CheckCRC(const std::string& fileName, U32 crc, C_Status& status);

	U32 CalculateCRC(const std::string& fileName, C_Status& status);

	inline U32 ColourRGBAToU32(F32 r, F32 g, F32 b, F32 a)
	{
		//Clamp values.
		if (r > 1.0f) r = 1.0f;
		if (r < 0.f) r = 0.f;
		if (g > 1.0f) r = 1.0f;
		if (g < 0.f) r = 0.f;
		if (b > 1.0f) r = 1.0f;
		if (b < 0.f) r = 0.f;
		if (a > 1.0f) r = 1.0f;
		if (a < 0.f) r = 0.f;

		//Scale values.
		r *= 255.f;
		g *= 255.f;
		b *= 255.f;
		a *= 255.f;

		//Convert to ints
		return AFTK_RGBA_TO_U32((U8)r, (U8)g, (U8)b, (U8)a);
	}

	inline Vector4F32 ColourRGBAFromU32(U32 colour)
	{
		U8 r = AFTK_R_FROM_U32(colour);
		U8 g = AFTK_G_FROM_U32(colour);
		U8 b = AFTK_B_FROM_U32(colour);
		U8 a = AFTK_A_FROM_U32(colour);

		Vector4F32 result;
		result[0] = (F32)r;
		result[1] = (F32)g;
		result[2] = (F32)b;
		result[3] = (F32)a;

		result[0] /= 255.f;
		result[1] /= 255.f;
		result[2] /= 255.f;
		result[3] /= 255.f;

		return result;
	}
}

#endif
