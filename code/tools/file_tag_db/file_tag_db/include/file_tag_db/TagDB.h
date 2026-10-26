#ifndef FILE_TAG_DB_TAG_DATABASE_H
#define FILE_TAG_DB_TAG_DATABASE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Sys
{
	class C_XmlDomBaseNode;
	class C_XmlDomElement;
	class C_XmlDom;
}

namespace FileTag
{
	class C_TagDBManager;

	class C_TagDB
	{
	public:
		C_TagDB(const C_TagDBManager& tagDBManager);

		~C_TagDB();

		C_Tag* AddTag(const char* pFileName);

		U32 GetNumTags() const;

		const C_Tag* GetTagPtr(U32 index) const;

		C_Tag* GetTagPtr(U32 index);

		const C_Tag* GetTagPtr(const char* pFileName) const;

		C_Tag* GetTagPtr(const char* pFileName);

		S32 GetTagIndex(const char* pFileName) const;

		///\brief Populates a file database from the given XML file.
		///
		///@param pFileName The name of the file XML file to read.
		///@return status Value indicating the success or failure of the operation.
		bool Read(const char* pFileName);

		bool Write(const char* pFileName) const;

		void Clear();

		bool DeserialiseXmlDom(const Sys::C_XmlDom& dom);

		bool SerialiseXmlDom(Sys::C_XmlDom& dom) const;

	private:
		bool DeserialiseTagXmlDom(
			const Sys::C_XmlDomElement* pElement);

		bool SerialiseTagXmlDom(
			Sys::C_XmlDomElement& tagParent,
			const C_Tag& tag) const;

		bool SerialiseContainerXmlDom(
			Sys::C_XmlDomElement* pFileParamNode, 
			const C_GenericAttribute& attr) const;

		bool DeserialiseContainerXmlDom(C_Tag& tag, 
			const Sys::C_XmlDomElement* pFileParamNode);

		Ut::Vector<C_Tag*>			m_tags;
		const C_TagDBManager&		m_tagDBManager;
	};
}

#endif

