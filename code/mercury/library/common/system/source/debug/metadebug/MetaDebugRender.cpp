#include "SystemPch.h"

#include <system/serialise/I_File.h>
#include <system/serialise/BinarySerialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/debug/debugrender/DebugRenderItem.h>
#include <system/debug/debugrender/DebugRenderItem.inl>
#include <system/debug/metadebug/MetaDebugRender.h>


namespace MetaDebug
{

extern Debug::Logger &g_Log;

C_DebugRender::C_DebugRender()
{
}

C_DebugRender::~C_DebugRender()
{
}

C_DebugRender::C_DebugRender( const Util::C_String& name, const DebugLevel::Level debugLevel )
	: C_MetaNode(name, debugLevel)
{
}

E_Type C_DebugRender::GetType() const 
{
	return E_Type_DebugRender;
}

void C_DebugRender::DebugRender( DbUtil::I_DebugRender* pDebugRender, Maths::HMat43 worldMatrix )
{
	Maths::HMat43 m;
	m.SetIdentity();

	for(S32 i = 0; i < m_RenderItemList.Size(); i++)
	{
		m_RenderItemList[i].DebugRender( pDebugRender, m );
	}
}

void C_DebugRender::OnRender( DbUtil::C_DebugRenderItem* pDebugRenderItem )
{
	m_RenderItemList.PushBack();
	DbUtil::C_DebugRenderItem& item = m_RenderItemList.Back();	
	item = *pDebugRenderItem;
}

void C_DebugRender::ReadData(  Sys::I_File* pFile )
{
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pFile);

	U32 version;

	ar >> version;

	ar >> m_RenderItemList;
}

void C_DebugRender::WriteData(  Sys::I_File* pFile ) const
{
	Sys::BinarySerialiserAutoEndian ar(*pFile);

	ar << (U32)DbUtil::E_DebugRenderFileVersion_Current;

	ar << m_RenderItemList;
}

void C_DebugRender::DumpData(const Util::C_String& prefix ) const
{
	if(m_RenderItemList.Size())
	{
		LOGDEBUG(g_Log, "%s   (%d) Render Items\n", prefix.Chars(), m_RenderItemList.Size());	
	}
}

} // namespace MetaDebug

