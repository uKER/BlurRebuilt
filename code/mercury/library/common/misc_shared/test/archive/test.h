#ifndef ARCHIVE_TEST_H
#define ARCHIVE_TEST_H
#pragma once

#include "misc_shared/archive/archive.h"

class Test
{
public:
	Test() {}
	Test(int i) {m_i = i;}

	S32 m_i;

	template <typename Archive>
	void Serialise(Archive& ar) const
	{
		ar & m_i; // Use & so that << or >> can be called as required.
	}

	// Create a Deserialise function that just calls Serialise.
	AUTO_DESERIALISE_MEMBER()
};

// 'New' version of Test with extra member and incremented version number.
class Test2
{
public:
	Test2() {m_i = -1; m_j = -1;}

	S32 m_i;
	S32 m_j;

	template <typename Archive>
	void Serialise(Archive& ar)
	{
		ar & m_i;
		if (ar.GetVersion() > 0) ar & m_j;
		else m_j = -1;
	}

	AUTO_DESERIALISE_MEMBER()
};

SERIALISATION_VERSION(Test2, 1, "Test2") // original version number defaults to 0

/////////////////////////////////////////////

class Base
{
public:
	S32 m_i;

	// The serialisation key needs to be initialised in the constructor.
	Base() {m_key = GET_SERIALISATION_KEY(Base);}

	// Used by serialisation to look up a handler.
	const Arch::SerialisationKey& GetSerialisationKey() {return m_key;}

	template <typename Archive>
	void Serialise(Archive& ar)
	{
		ar & m_i;
	}

	AUTO_DESERIALISE_MEMBER()

protected:
	Arch::SerialisationKey m_key;
};

SERIALISATION_POLYMORPHIC(Base, 0, "archive_test_base")

class Derived : public Base
{
public:
	S32 m_j;

	Derived() {m_key = GET_SERIALISATION_KEY(Derived);}

	template <typename Archive>
	void Serialise(Archive& ar)
	{
		SERIALISE_BASE_MEMBER(Base, ar);
		ar & m_j;
	}

	AUTO_DESERIALISE_MEMBER()
};

SERIALISATION_POLYMORPHIC(Derived, 0, "archive_test_derived")

///////////////////////////////////////////////

class Base2
{
public:
	S32 m_i;

	Base2() {}
	virtual ~Base2() {}

	// Alternative method: no key stored on the object, GetSerialisationKey() is virtual.
	virtual Arch::SerialisationKey GetSerialisationKey() {return GET_SERIALISATION_KEY(Base2);}

	template <typename Archive>
	void Serialise(Archive& ar)
	{
		ar & m_i;
	}

	AUTO_DESERIALISE_MEMBER()
};

SERIALISATION_POLYMORPHIC(Base2, 0, "archive_test_base2")

class Derived2 : public Base2
{
public:
	S32 m_j;

	virtual Arch::SerialisationKey GetSerialisationKey() {return GET_SERIALISATION_KEY(Derived2);}

	template <typename Archive>
	void Serialise(Archive& ar)
	{
		SERIALISE_BASE_MEMBER(Base2, ar);
		ar & m_j;
	}

	AUTO_DESERIALISE_MEMBER()
};

SERIALISATION_POLYMORPHIC(Derived2, 0, "archive_test_derived2")

///////////////////////////////////////////////

struct SimpleVector
{
	float x, y, z;
};

template <typename Archive>
void Serialise(Archive& ar, SimpleVector& t)
{
	ar & t.x;
	ar & t.y;
	ar & t.z;
}

AUTO_DESERIALISE_FREE(SimpleVector)
SERIALISATION_PRIMITIVE(SimpleVector, "SimpleVector")

#endif
