#ifndef XMLPARSER_H
#define XMLPARSER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Definitions.h"

namespace Sys
{

//
// C_XmlAttribute
//

struct C_XmlAttribute
{
	Util::C_String name;
	Util::C_String value;
};

//
// C_XmlAttributeList
//

class C_XmlAttributeList
{
public:
	C_XmlAttributeList();
	~C_XmlAttributeList();

	int Elements() const;

	const C_XmlAttribute* GetAttributeByIndex(int index) const;
	const C_XmlAttribute* GetAttributeByName(const char* name) const;
	C_XmlAttribute* GetAttributeByIndex(int index);
	C_XmlAttribute* GetAttributeByName(const char* name);

	const char* GetKeyByIndex(int index) const;
	const char* GetValueByIndex(int index) const;
	const char* GetValueByName(const char* name) const;

	inline const C_XmlAttribute* operator[](int index) const
	{
		return GetAttributeByIndex(index);
	}

	inline const C_XmlAttribute* operator[](const char* name) const
	{
		return GetAttributeByName(name);
	}

	void Add(const Util::C_String& name, const Util::C_String& value);
	void Clear();

	inline void SetLineNo(int lineNo)
	{
		_lineNo = lineNo;
	}

	inline int LineNo() const
	{
		return _lineNo;
	}

private:
	static const int kMaxAttributes = 64;

private:
	C_XmlAttribute	_attributes[kMaxAttributes];
	int				_noAttributes;

private:
	int				_lineNo;	
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// C_XmlContent

class C_XmlContent
{
public:
	virtual ~C_XmlContent() {};
public:
	inline virtual bool StartDocument()
	{
		return true;
	}

	inline virtual bool EndDocument()
	{
		return true;
	}

public:
	inline virtual bool StartElement(const Util::C_String& name, const C_XmlAttributeList& attributes)
	{
		return true;
	}

	inline virtual bool EndElement(const Util::C_String& name)
	{
		return true;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// XmlParse

E_XmlResult XmlParse(C_XmlContent& content, const char* document, int length);

}; //namespace Sys



#endif // XMLPARSER_H
