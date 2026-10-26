#ifndef OPTIONS_H
#define OPTIONS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/Containers.h>
#include <System/DebugLog.h>
#include <System/Xml/XmlBlockParser.h>
#include <system/Serialise.h>

#include <system/serialise/TextSerialise.h>

class C_OptionsConfig
{
public:
	C_OptionsConfig();
	~C_OptionsConfig();

public:
	Util::C_String					m_ConfigName;

	Util::C_String					m_DefineFormat;
	Util::C_String					m_DefineValueFormat;

	Util::C_String					m_BatchFileIn;
	Util::C_String					m_BatchFileOut;

	Util::C_String					m_BinaryFileOut;

	bool							m_EndianSwapped;
};


class C_Options : public Sys::C_XmlContent
{
public:
	C_Options();
	~C_Options();

	bool ReadXml(Util::C_String fileName);

	virtual bool StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes);

	void Dump();

public:
	Util::C_String					m_ConfigFile;
	Util::C_String					m_RegistryFile;

	Ut::Vector<C_OptionsConfig>		m_ConfigList;
};


#endif