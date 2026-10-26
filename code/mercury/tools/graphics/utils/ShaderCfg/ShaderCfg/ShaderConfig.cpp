
#include "stdafx.h"

#include <System/DebugLog.h>

#include <core/containers.h>
#include <core/utility/endian.h>
#include <system/Serialise.h>
#include <system/serialise/C_FileBlock.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

#include "ShaderConfig.h"

namespace ShaderCfg
{

Debug::Logger &g_Log = Debug::GetLogger("mercury.tools.ShaderCfg");

// C_ShaderDefine

C_Define::C_Define() :
	m_VertexShader( false ),
	m_PixelShader( false )
{
}

C_Define::~C_Define()
{
}

void C_Define::Dump()
{
	LOGINFO(g_Log, "Define : %s - Define = %s, Value = %s\n   Vertex:%d, Pixel:%d, TextId:%s\n", m_Name.Chars(), m_Define.Chars(), m_Value.Chars(), m_VertexShader, m_PixelShader, m_TextId.Chars() );
	LOGINFO(g_Log, "\n");
}

// C_Fragment

C_Fragment::C_Fragment()
{
}

C_Fragment::~C_Fragment()
{
}

void C_Fragment::Dump()
{
	LOGINFO(g_Log, "Fragment[%d] : %s\n", m_Index, m_Name.Chars());

	for(S32 i=0; i<m_DefineList.Size(); i++)
	{
		S32 index = m_DefineList[i];
		LOGINFO(g_Log, "   Define : %s\n", m_pShaderConfig->m_DefineList[index].m_Name.Chars() );
	}
}


// C_ShaderConfig

C_ShaderConfig::C_ShaderConfig()
{
}

C_ShaderConfig::~C_ShaderConfig()
{
}

C_Define* C_ShaderConfig::LookupDefine( const Util::C_String& name )
{
	for(S32 i=0; i<m_DefineList.Size(); i++)
	{
		C_Define* pDefine = &m_DefineList[i];

		if(pDefine)
		{
			if(pDefine->m_Name == name)
				return pDefine;
		}
	}
	return NULL;
}

C_Fragment* C_ShaderConfig::LookupFragment( const Util::C_String& name )
{
	for(S32 i=0; i<m_FragmentList.Size(); i++)
	{
		C_Fragment* pFragment = &m_FragmentList[i];

		if(pFragment)
		{
			if(pFragment->m_Name == name)
				return pFragment;
		}
	}
	return NULL;
}

void C_ShaderConfig::Dump()
{
	for(S32 i=0; i<m_DefineList.Size(); i++)
	{
		m_DefineList[i].Dump();
	}

	for(S32 i=0; i<m_FragmentList.Size(); i++)
	{
		m_FragmentList[i].Dump();
	}
}

} // namespace ShaderCfg