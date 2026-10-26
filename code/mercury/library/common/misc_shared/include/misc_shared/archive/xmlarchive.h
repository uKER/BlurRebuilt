#ifndef ARCHIVE_XML_ARCHIVE_H
#define ARCHIVE_XML_ARCHIVE_H
#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_PC
#pragma warning(disable:4530) // warning C4530: C++ exception handler used, but unwind semantics are not enabled. 
#endif

#include "serialisationinfo.h"
#include "core/utility/endian.h"
#include "system/serialise/i_binarystream.h"
#include "btl/utility/types.h"
#include <sstream>

namespace Arch
{
	
class XmlOutputArchive
{
public:
	XmlOutputArchive(Sys::I_BinaryStream& out) : m_stream(out), m_tab(0), m_name(0)
	{
		m_version = 0;
		BeginElement("archive");
		*this << MakeNvp("Version", m_version);
	}

	~XmlOutputArchive()
	{
		EndElement("archive");
	}

	/// Does nothing.
	bool Write(const void *pData, U32 nBytes) {return true;}
	inline void Align(U32 bytes) {}

#define _XML_OUTPUT_STREAM_MACRO(TYPE, STYPE) \
	XmlOutputArchive& operator << (TYPE& t) {SerialiseAtom<TYPE, STYPE>(t); return *this;} \
	XmlOutputArchive& operator << (const TYPE& t) {SerialiseAtom<TYPE, STYPE>(t); return *this;}

	_XML_OUTPUT_STREAM_MACRO(U8, S32)
	_XML_OUTPUT_STREAM_MACRO(S8, S32)
	_XML_OUTPUT_STREAM_MACRO(char, S32)
	_XML_OUTPUT_STREAM_MACRO(bool, S32)
	_XML_OUTPUT_STREAM_MACRO(U16, S32)
	_XML_OUTPUT_STREAM_MACRO(S16, S32)
	_XML_OUTPUT_STREAM_MACRO(U32, S32)
	_XML_OUTPUT_STREAM_MACRO(S32, S32)
	_XML_OUTPUT_STREAM_MACRO(F32, F32)
	_XML_OUTPUT_STREAM_MACRO(F64, F64)
	_XML_OUTPUT_STREAM_MACRO(U64, S64)
	_XML_OUTPUT_STREAM_MACRO(S64, S64)

#undef _XML_OUTPUT_STREAM_MACRO

	XmlOutputArchive& operator << (char* t) {SerialiseAtom<char*, char*>(t); return *this;}
	XmlOutputArchive& operator << (const char* t) {SerialiseAtom<const char*, char*>(t); return *this;}

	template<typename T>
	XmlOutputArchive& operator << (T& obj)
	{
		typedef typename Btl::StripConst<T>::Result T_NC;

		std::stringstream att;

		const char* type = Arch::SerialisationInfo<T_NC>::GetGUID();
		
		if (type != 0) 
		{
			if (att.str().size() > 0) att << " ";
			att << "type = \"" << type << "\"";
		}

		if (m_name != 0) 
		{
			if (att.str().size() > 0) att << " ";
			att << "name = \"" << m_name << "\"";
		}

		if (Arch::SerialisationInfo<T_NC>::GetSerialisationType() != Arch::SERIALISATION_TYPE_PRIMITIVE)
		{
			if (att.str().size() > 0) att << " ";
			att << "version = \"" << (U32) Arch::SerialisationInfo<T_NC>::GetVersion() << "\"";
		}

		m_name = 0;
		U32 currentVersion = GetVersion();
		SetVersion(Arch::SerialisationInfo<T_NC>::GetVersion());
		BeginElement("object", att.str().c_str());

		Serialise(*this, const_cast<T_NC&>(obj));

		EndElement("object");
		SetVersion(currentVersion);

		return *this;
	}

	// Pointers treated as a primitive type.
	template<typename T>
	XmlOutputArchive& operator << (T* obj)
	{
		BeginElement("pointer");
		typedef typename Btl::StripConst<T>::Result T_NC;
		Serialise(*this, const_cast<T_NC*&>(obj));
		EndElement("pointer");
		return *this;
	}

	template<typename T>
	XmlOutputArchive& operator << (const Nvp<T>& obj)
	{
		m_name = obj.name;
		*this << obj.value;
		return *this;
	}

	template<typename T>
	XmlOutputArchive& operator << (Nvp<T>& obj)
	{
		m_name = obj.name;
		*this << obj.value;
		return *this;
	}

	void operator << (SerialisationType t)
	{
		switch (t)
		{
			case SERIALISATION_TYPE_UNKNOWN: *this << "UNKNOWN"; return;
			case SERIALISATION_TYPE_PRIMITIVE: *this << "PRIMITIVE"; return;
			case SERIALISATION_TYPE_VERSION: *this << "VERSION"; return;
			case SERIALISATION_TYPE_POLYMORPHIC: *this << "POLYMORPHIC"; return;
			default: assert(!"Serialisation type corrupt."); return;
		}
	}

	// Equivalent to << operator.
	template<typename T>
	XmlOutputArchive& operator & (T& obj)
	{
		typedef typename Btl::StripConst<T>::Result T_NC;
		*this << const_cast<T_NC&>(obj);
		return *this;
	}

	template<typename T>
	XmlOutputArchive& operator & (T* obj)
	{
		*this << obj;
		return *this;
	}

	template<typename T>
	XmlOutputArchive& operator & (const Nvp<T> obj)
	{
		m_name = obj.name;
		*this << obj.value;
		return *this;
	}

	/// The version number of the object currently being written.
	U16 GetVersion() const {return m_version;}

	bool GetEndianSwap() const {return false;}

private:

	template <typename T>
	const char* AtomTypeName() {assert(0); return "Unknown";}

	template <typename T, typename StoredType>
	void SerialiseAtom(T t)
	{
		std::stringstream att;
		att << "type = \"" << AtomTypeName<T>() << "\" ";
		if (m_name != 0) att << "name = \"" << m_name << "\" ";
		att << "value = \"" << (StoredType) t << "\"";
		WriteElement("atom", att.str().c_str());
		m_name = 0;
	} 

	void BeginElement(const char* name, const char* attributes = 0)
	{
		std::stringstream s;
		s << "<" << name;
		if (attributes != 0 && *attributes != 0) s << " " << attributes;
		s << ">";
		WriteLine(s.str());
		m_tab++;
	}

	void EndElement(const char* name)
	{
		m_tab--;
		std::stringstream s;
		s << "</" << name << ">";
		WriteLine(s.str());
	}

	void WriteElement(const char* name, const char* attributes = 0)
	{
		std::stringstream s;
		s << "<" << name;
		if (attributes != 0 && *attributes != 0) s << " " << attributes;
		s << "/>";
		WriteLine(s.str());
	}

	void WriteLine(const std::string& s)
	{
		std::stringstream stream;
		for (int i = 0; i < m_tab; i++) stream << '\t';
		stream << s;
		stream << '\n';
        m_stream.Write(stream.str().c_str(), (U32) stream.str().size());
	}

	void SetVersion(U16 i) {m_version = i;}

	Sys::I_BinaryStream& m_stream;
	int m_tab;
	const char* m_name;
	U16 m_version;
};


template <> inline const char* XmlOutputArchive::AtomTypeName<U8>() {return "U8";}
template <> inline const char* XmlOutputArchive::AtomTypeName<S8>() {return "S8";}
template <> inline const char* XmlOutputArchive::AtomTypeName<char>() {return "char";}
template <> inline const char* XmlOutputArchive::AtomTypeName<bool>() {return "bool";}
template <> inline const char* XmlOutputArchive::AtomTypeName<U16>() {return "U16";}
template <> inline const char* XmlOutputArchive::AtomTypeName<S16>() {return "S16";}
template <> inline const char* XmlOutputArchive::AtomTypeName<U32>() {return "U32";}
template <> inline const char* XmlOutputArchive::AtomTypeName<S32>() {return "S32";}
template <> inline const char* XmlOutputArchive::AtomTypeName<F32>() {return "F32";}
template <> inline const char* XmlOutputArchive::AtomTypeName<F64>() {return "F64";}
template <> inline const char* XmlOutputArchive::AtomTypeName<U64>() {return "U64";}
template <> inline const char* XmlOutputArchive::AtomTypeName<S64>() {return "S64";}
template <> inline const char* XmlOutputArchive::AtomTypeName<char*>() {return "string";}
template <> inline const char* XmlOutputArchive::AtomTypeName<const char*>() {return "string";}

}

#endif
