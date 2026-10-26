#ifndef __SYSTEM_DEBUG_METADEBUGATTRIBUTELIST_H__
#define __SYSTEM_DEBUG_METADEBUGATTRIBUTELIST_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Utility/string.h>
#include <btl/Containers/Vector.h>
#include <system/Debug/I_DebugRender.h>
#include <system/Debug/metadebug/MetaDebug.h>
#include <system/time/Time.h>

namespace MetaDebug
{

enum E_AttributeType
{
	E_AttributeType_Void = 0,
	E_AttributeType_U32,
	E_AttributeType_S32,
	E_AttributeType_F32,
	E_AttributeType_String,
	E_AttributeType_Time,
	E_AttributeType_Count 
};

// C_Attribute

class C_Attribute
{
friend class C_AttributeList;
public:
	C_Attribute();
	~C_Attribute();

	E_AttributeType GetType() const;

	const Util::C_String& GetName() const;
	void SetName( const Util::C_String& name );

	U32 GetValueU32() const;
	void SetValueU32( const U32 value );

	S32 GetValueS32() const;
	void SetValueS32( const S32 value );

	F32 GetValueF32() const;
	void SetValueF32( const F32 value );

	const char* GetValueString() const;
	void SetValueString( const char* string );

	Time::C_Time GetValueTime() const;
	void SetValueTime( Time::C_Time value );

	template <typename Archive>
	void Serialise(Archive& ar) const;

	template <typename Archive>
	void Deserialise(Archive& ar);

private:
	E_AttributeType m_Type;
	void* m_Data;
	Util::C_String m_Name;
};

// C_AttributeList

class C_AttributeList : public C_MetaNode
{
public:
	C_AttributeList();
	C_AttributeList( const Util::C_String& name, const DebugLevel::Level debugLevel );
	~C_AttributeList();

	E_Type GetType() const OVERRIDE;

	S32 GetAttributeCount() const;

	const C_Attribute* GetAttributeByIndex(S32 index) const;
	const C_Attribute* GetAttributeByName(const Util::C_String& name) const;

	inline const C_Attribute* operator[](S32 index) const
	{
		return GetAttributeByIndex(index);
	}

	inline const C_Attribute* operator[](const char* name) const
	{
		return GetAttributeByName(name);
	}

	C_Attribute* AddAttribute();
	void AddU32(const Util::C_String& name, U32 value);
	void AddS32(const Util::C_String& name, S32 value);
	void AddF32(const Util::C_String& name, F32 value);
	void AddString(const Util::C_String& name, const Util::C_String& string);
	void AddTime(const Util::C_String& name, Time::C_Time time);

	void ReadData(  Sys::I_File* pFile ) OVERRIDE;
	void WriteData(  Sys::I_File* pFile ) const OVERRIDE;
	void DumpData(const Util::C_String& prefix ) const OVERRIDE;

private:
	Ut::Vector<C_Attribute> m_AttributeList;

};


template <typename Archive>
void C_Attribute::Serialise(Archive& ar) const
{
	ar << (U32)m_Type;
	ar << m_Name;
	switch(m_Type)
	{
		case E_AttributeType_Void: 
			break;
		case E_AttributeType_S32: 
			ar << GetValueS32(); 			
			break;
		case E_AttributeType_U32: 
			ar << GetValueU32(); 
			break;
		case E_AttributeType_F32: 
			ar << GetValueF32(); 
			break;
		case E_AttributeType_String: 
			ar << Util::C_String(GetValueString()); 
			break;
		case E_AttributeType_Time: 
			ar << GetValueTime(); 
			break;


	}
}

template <typename Archive>
void C_Attribute::Deserialise(Archive& ar)
{
	U32 type;
	ar >> type;
	ar >> m_Name;
	switch(type)
	{
	case E_AttributeType_Void: 
		{
		}
		break;
	case E_AttributeType_S32: 
		{
			S32 value;
			ar >> value;
			SetValueS32(value);
		}
		break;
	case E_AttributeType_U32: 
		{
			U32 value;
			ar >> value;
			SetValueU32(value);
		}
		break;
	case E_AttributeType_F32: 
		{
			F32 value;
			ar >> value;
			SetValueF32(value);
		}
		break;
	case E_AttributeType_String: 
		{
			Util::C_String value;
			ar >> value;
			SetValueString(value);
		}
		break;
	
	case E_AttributeType_Time: 
		{
			Time::C_Time value;
			ar >> value;
			SetValueTime(value);
		}
		break;
	}
}




} // namespace MetaDebug


#endif
