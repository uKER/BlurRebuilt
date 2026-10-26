#pragma once

#include "utility.h"

namespace Ut
{

template <typename T>
class Name_t
{
public:
	Name_t(char* name, const T& object) : m_name(name), m_object(object) {}

	char* Name() const {return m_name;}
	const T& Object() const {return m_object;}

private:
	char* m_name;
	const T& m_object;
};

template <typename T>
Name_t<T> Name(char* name, const T& object)
{
	return Name_t<T>(name, object);
}

/////////////////////////////////////////////////////////

template <typename Stream>
class OutputStream
{
public:
	OutputStream(Stream& out) : m_out(out) {}

	// general template
	template <typename T>
	OutputStream& operator & (const T& t)
	{
		// version number - default to writing 0
		U32 version = SerialisationVersion<T>::RESULT;
		if (version > -1) m_out << version;

		*this << t;

		return *this;
	}

	template <typename T>
	OutputStream& Write(const T& t)
	{
		*this OutputStream::<< y;
	}

	//////////////////////////////////////////////////
	// specialisations

	template <typename T>
	OutputStream& operator << (Ut::Name_t<T> t)
	{
		return (*this << t.Object()); 
	}

	template <typename T>
	OutputStream& operator << (const T* p)
	{
		if (p)
		{
			m_out << U32(1);
			*this << *p;
		}
		else
		{
			m_out << U32(0);
		}

		return *this;
	}

	template <typename T>
	OutputStream& operator << (T* p)
	{
		return (*this << const_cast<const T*>(p));
	}

	///////////////////////////////////////////////////
	// basic types

	OutputStream& operator << (const U32& t)
	{
		m_out << t;
		return *this;
	}

	OutputStream& operator << (const S32& t)
	{
		m_out << t;
		return *this;
	}

	OutputStream& operator << (const U16& t)
	{
		m_out << t;
		return *this;
	}

	OutputStream& operator << (const S16& t)
	{
		m_out << t;
		return *this;
	}

	OutputStream& operator << (const U8& t)
	{
		m_out << t;
		return *this;
	}

	OutputStream& operator << (const S8& t)
	{
		m_out << t;
		return *this;
	}

	OutputStream& operator << (const float& t)
	{
		m_out << t;
		return *this;
	}

private:
	Stream& m_out;
};

///////////////////////////////////////////////////////////////////

template <typename T>
struct SerialisationVersion
{
	const static int RESULT = 0;
};

#define SERIALISATION_VERSION(Type, version) template <> struct Ut::SerialisationVersion<Type> {const static int RESULT = version;};
#define BASIC_SERIALISATION(Type) template <> struct Ut::SerialisationVersion<Type> {const static int RESULT = -1;};

template <typename T>
struct SerialisationID
{
	const static int RESULT = -1;
};

#define SERIALISATION_ID(Type, ID) template <> struct Ut::SerialisationID<Type> {const static int RESULT = ID;};


}
