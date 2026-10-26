#ifndef FILE_TAG_DB_SAX_H
#define FILE_TAG_DB_SAX_H

#ifdef _MSC_VER
#pragma once
#endif

namespace FileTag
{
	// callbacks for reading a file using XMLParser.cpp
	class C_TagDBSax : public Sys::C_XmlContent
	{
	public:
		virtual ~C_TagDBSax();

		bool StartDocument();

		bool EndDocument();

		bool StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes);

		bool EndElement(const Util::C_String& name);

		bool Read(const char* filename);

	protected:
		bool DeserialiseContainer(const Sys::C_XmlAttributeList& attributes);

		C_Tag* m_pCurrentTag;
		Util::C_String m_dbPath;
	};
}

#endif