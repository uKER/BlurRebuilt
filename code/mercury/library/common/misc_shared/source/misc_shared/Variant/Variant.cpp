#include "MiscSharedPch.h"
#include "Misc_Shared/Variant/Variant.h"
#include <sstream>

namespace Util
{

void C_Variant::FromString(const Btl::String& str, E_VarType dataType)
{
	switch (dataType) 
	{
	case E_VarType_U32: {
		U32 value;
		if (str.Length() > 2 && str(0, 2) == "0x")
		{
			value = (U32)strtoul(str.Chars()+2, 0, 16);
		}
		else
		{
			std::stringstream ss(str.Chars());
			ss >> value;
		}
		SetValue(value);
		break;
	}
	case E_VarType_U64: {
		std::stringstream ss(str.Chars());
		U64 value;
		ss >> value;
		SetValue(value);
		break;
	}
	case E_VarType_S32: {
		S32 value(0);
		if (str.Length() > 2 && str(0, 2) == "0x")
		{
			value = (S32)strtoul(str.Chars()+2, 0, 16);
		}
		else
		{
			std::stringstream ss(str.Chars());
			ss >> value;
		}
		SetValue(value);
		break;
	}
	case E_VarType_S64: {
		std::stringstream ss(str.Chars());
		S64 value;
		ss >> value;
		SetValue(value);
		break;
	}
	case E_VarType_F32: {
		std::stringstream ss(str.Chars());
		F32 value;
		ss >> value;
		SetValue(value);
		break;
	}
	case E_VarType_F64: {
		std::stringstream ss(str.Chars());
		F64 value;
		ss >> value;
		SetValue(value);
		break;
	}

	case E_VarType_Boolean: {
		std::stringstream ss(str.Chars());
		bool value;
		ss >> value;
		SetValue(value);
		break;
	}
	case E_VarType_String: 
		SetValue(str);
		break;
	case E_VarType_Matrix: 
		{
		std::stringstream ss(str.Chars());
		MathsStorage::Matrix44 value;
		for (U32 i = 0; i < 16; ++i)
			ss >> value.v[i];
		SetValue(value);
		}
		break;

	default:
		break;
	};
}

void C_Variant::ToString(Btl::String& str)
{
	switch (m_type) 
	{
	case E_VarType_U32: {
		str.Printf("%u", GetData()->u32);
		break;
	}
	case E_VarType_U64: {
		std::stringstream ss(str.Chars());
		ss << GetData()->u64;
		str = ss.str().c_str();
		break;
	}
	case E_VarType_S32: {
		str.Printf("%i", GetData()->s32);
		break;
	}
	case E_VarType_S64: {
		std::stringstream ss(str.Chars());
		ss << GetData()->s64;
		str = ss.str().c_str();
		break;
	}
	case E_VarType_F32: {
		str.Printf("%f", GetData()->f32);
		break;
	}
	case E_VarType_F64: {
		std::stringstream ss(str.Chars());
		ss << GetData()->f64;
		str = ss.str().c_str();
		break;
	}

	case E_VarType_Boolean: {
		str = GetData()->boolean ? "1" : "0";
		break;
	}
	case E_VarType_String: {
		str = *GetData()->pString;
		break;
	}
	case E_VarType_Matrix: {
		MathsStorage::Matrix44& mtx = *GetData()->pMatrix;
		str.Printf("%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", 
			mtx.v[0], mtx.v[1], mtx.v[2], mtx.v[3],
			mtx.v[4], mtx.v[5], mtx.v[6], mtx.v[7],
			mtx.v[8], mtx.v[9], mtx.v[10], mtx.v[11],
			mtx.v[12], mtx.v[13], mtx.v[14], mtx.v[15]);
		break;
	}

	default:
		str = "";
	}
}

}
