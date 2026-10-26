#include "SystemPch.h"

#include <system/serialise/I_File.h>
#include <system/serialise/BinarySerialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

#include <system/debug/metadebug/MetaDebugAttributeList.h>

namespace MetaDebug
{

extern Debug::Logger &g_Log;

// C_Attribute

C_Attribute::C_Attribute()
	: m_Type(E_AttributeType_Void)
	, m_Data(NULL)
{
}

C_Attribute::~C_Attribute()
{
	if(m_Type == E_AttributeType_String)
	{
		const char* pData = (const char*)m_Data;
		delete[]( pData );
	}
}

const Util::C_String& C_Attribute::GetName() const
{
	return m_Name;
}

void C_Attribute::SetName( const Util::C_String& name )
{
	m_Name = name;
}

E_AttributeType C_Attribute::GetType() const
{
	return m_Type;
}

S32 C_Attribute::GetValueS32() const
{
	BIZ_ASSERT( m_Type == E_AttributeType_S32 );

	return (S32&)m_Data;
}

void C_Attribute::SetValueS32( const S32 value )
{
	BIZ_ASSERT( m_Type == E_AttributeType_Void );
	(S32&)m_Data = value;
	m_Type = E_AttributeType_S32;
}

U32 C_Attribute::GetValueU32() const
{
	BIZ_ASSERT( m_Type == E_AttributeType_U32 );
	return (U32&)m_Data;
}

void C_Attribute::SetValueU32( const U32 value )
{
	BIZ_ASSERT( m_Type == E_AttributeType_Void );
	(U32&)m_Data = value;
	m_Type = E_AttributeType_U32;
}

F32 C_Attribute::GetValueF32() const
{
	BIZ_ASSERT( m_Type == E_AttributeType_F32 );
	return (F32&)m_Data;
}

void C_Attribute::SetValueF32( const F32 value )
{
	BIZ_ASSERT( m_Type == E_AttributeType_Void );
	(F32&)m_Data = value;
	m_Type = E_AttributeType_F32;
}

const char* C_Attribute::GetValueString() const
{
	BIZ_ASSERT( m_Type == E_AttributeType_String );
	return (const char*)m_Data;
}

void C_Attribute::SetValueString( const char* string )
{
	BIZ_ASSERT( m_Type == E_AttributeType_Void );
	BIZ_ASSERT( string != NULL );
	S32 length = strlen(string);
	char* pString = new char[length+1];
	strcpy(pString, string);
	m_Data = (void*)pString;
	m_Type = E_AttributeType_String;
}

Time::C_Time C_Attribute::GetValueTime() const
{
	U32 iTime = (U32&)m_Data;
	return (U64)iTime * SYS_USEC_PER_SEC;
}

void C_Attribute::SetValueTime( Time::C_Time value )
{
	BIZ_ASSERT( m_Type == E_AttributeType_Void );
	U32 iTime = (U32)(value / SYS_USEC_PER_SEC);
	(U32&)m_Data = iTime;
	m_Type = E_AttributeType_Time;
}

// C_AttributeList

C_AttributeList::C_AttributeList()
{

}

C_AttributeList::~C_AttributeList()
{


}

C_AttributeList::C_AttributeList( const Util::C_String& name, const DebugLevel::Level debugLevel )
	: C_MetaNode(name, debugLevel)
{

}

E_Type C_AttributeList::GetType() const 
{
	return E_Type_AttributeList;
}

S32 C_AttributeList::GetAttributeCount() const
{
	return m_AttributeList.Size();
}

const C_Attribute* C_AttributeList::GetAttributeByIndex(S32 index) const
{
	return &m_AttributeList[index];
}

const C_Attribute* C_AttributeList::GetAttributeByName(const Util::C_String& name) const
{
	const S32 attributeCount = GetAttributeCount();
	for(S32 i=0; i<attributeCount; i++)
	{
		const C_Attribute* pAttribute = GetAttributeByIndex(i);
		if( pAttribute->GetName() == name )
			return pAttribute;
	}

	return NULL;
}

C_Attribute* C_AttributeList::AddAttribute()
{
	m_AttributeList.PushBack();
	return &m_AttributeList[m_AttributeList.Size() - 1];
}

void C_AttributeList::AddU32(const Util::C_String& name, U32 value)
{
	C_Attribute* pAttribute = AddAttribute();
	pAttribute->SetName(name);
	pAttribute->SetValueU32(value);	
}

void C_AttributeList::AddS32(const Util::C_String& name, S32 value)
{
	C_Attribute* pAttribute = AddAttribute();
	pAttribute->SetName(name);
	pAttribute->SetValueS32(value);	
}

void C_AttributeList::AddF32(const Util::C_String& name, F32 value)
{
	C_Attribute* pAttribute = AddAttribute();
	pAttribute->SetName(name);
	pAttribute->SetValueF32(value);	
}

void C_AttributeList::AddString(const Util::C_String& name, const Util::C_String& string)
{
	C_Attribute* pAttribute = AddAttribute();
	pAttribute->SetName(name);
	pAttribute->SetValueString(string);	
}

void C_AttributeList::AddTime(const Util::C_String& name, Time::C_Time time)
{
	C_Attribute* pAttribute = AddAttribute();
	pAttribute->SetName(name);
	pAttribute->SetValueTime(time);	
}

void C_AttributeList::ReadData(  Sys::I_File* pFile )
{
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pFile);

	ar >> m_AttributeList;
}

void C_AttributeList::WriteData(  Sys::I_File* pFile ) const
{
	Sys::BinarySerialiserAutoEndian ar(*pFile);

	ar << m_AttributeList;
}

void C_AttributeList::DumpData(const Util::C_String& prefix ) const
{
	const S32 attributeCount = GetAttributeCount();
	for(S32 i=0; i<attributeCount; i++)
	{
		const C_Attribute* pAttribute = GetAttributeByIndex(i);
		switch(pAttribute->GetType())
		{
			case E_AttributeType_Void:
			LOGDEBUG(g_Log, "%s   [%d]Void %s\n", prefix.Chars(), i, pAttribute->GetName().Chars());
			break;

			case E_AttributeType_S32:
				LOGDEBUG(g_Log, "%s   [%d]S32 %s = %d\n", prefix.Chars(), i, pAttribute->GetName().Chars(), pAttribute->GetValueS32());
			break;

			case E_AttributeType_U32:
				LOGDEBUG(g_Log, "%s   [%d]U32 %s = %d\n", prefix.Chars(), i, pAttribute->GetName().Chars(), pAttribute->GetValueU32());
			break;

			case E_AttributeType_F32:
				LOGDEBUG(g_Log, "%s   [%d]F32 %s = %f\n", prefix.Chars(), i, pAttribute->GetName().Chars(), pAttribute->GetValueF32());
			break;

			case E_AttributeType_String:
				LOGDEBUG(g_Log, "%s   [%d]String %s = %s\n", prefix.Chars(), i, pAttribute->GetName().Chars(), pAttribute->GetValueString());
			break;

			case E_AttributeType_Time:
				LOGDEBUG(g_Log, "%s   [%d]Time %s = %s\n", prefix.Chars(), i, pAttribute->GetName().Chars(), Time::FormatDate_TimeString(pAttribute->GetValueTime()).Chars());
			break;
		}
	}
}

} // namespace MetaDebug

