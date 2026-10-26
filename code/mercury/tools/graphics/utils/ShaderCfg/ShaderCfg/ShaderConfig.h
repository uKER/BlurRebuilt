#ifndef SHADER_CONFIG_H
#define SHADER_CONFIG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/Containers.h>
#include <System/DebugLog.h>
#include <System/Xml/XmlBlockParser.h>
#include <system/Serialise.h>

#include <system/serialise/TextSerialise.h>

namespace ShaderCfg
{

extern Debug::Logger &g_Log;

class C_ShaderConfig;
class C_ShaderRegistry;
class C_ShaderConfigXmlParser;
class C_ShaderRegistryXmlParser;

// C_Define

class C_Define
{
public:
	C_Define();
	~C_Define();

	void Destroy() {}

	void Dump();

public:
	Util::C_String	m_Name;
	S32				m_Index;

	Util::C_String	m_Define;
	Util::C_String	m_Value;

	bool			m_VertexShader;
	bool			m_PixelShader;

	Util::C_String	m_TextId;
};

// C_Condition

enum E_ConditionType
{
	E_ConditionType_None = 0,
	E_ConditionType_Required,
	E_ConditionType_Mutex,
	E_ConditionType_Exclude
};

class C_Condition
{
public:
	E_ConditionType			m_Type;
	Util::C_String			m_Id;
};

// C_Fragment

class C_Fragment
{
public:
	C_Fragment();
	~C_Fragment();

	void Destroy() {}

	void Dump();

public:
	C_ShaderConfig*	m_pShaderConfig;

	Util::C_String	m_Name;

	S32				m_Index;

	Ut::Vector<C_Condition>			m_ConditionList;

	Ut::Vector<S32>					m_DefineList;
};

// C_ShaderConfig

class C_ShaderConfig
{	
	friend class C_ShaderConfigXmlParser;
public:

	C_ShaderConfig();
	~C_ShaderConfig();

	void Dump();

	C_Define* LookupDefine( const Util::C_String& name );
	C_Fragment* LookupFragment( const Util::C_String& name );

public:
	Ut::Vector<C_Define>			m_DefineList;

	Ut::Vector<C_Fragment>			m_FragmentList;
};


} // namespace ShaderCfg

#endif // SHADER_CONFIG_H
