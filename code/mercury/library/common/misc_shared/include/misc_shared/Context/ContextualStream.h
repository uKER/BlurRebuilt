#ifndef CONTEXTUALSTREAM_5F4CCD39_6838_408F_AFE1_CC64228767A3_H
#define CONTEXTUALSTREAM_5F4CCD39_6838_408F_AFE1_CC64228767A3_H

// NOTE: This might get moved around, misc_shared probably isn't the right 
// place for it but we're not yet sure if it's fit for purpose.

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <btl/utility/pair.h>
#include <misc_shared/Context/I_Anything.h>
#include <system/serialise/I_BinaryStream.h>


namespace Sys
{

// a bunch of slightly bodgy stuff resulting from people wanting to count how 
// many bytes have been written/read when I_BinaryStream doesn't expose any 
// kind of position information...

class C_ContextualStreamPos; ///< position marker within a stream.

typedef S32 C_ContextualStreamLen; ///< length of stream between two C_ContextualStreamPos markers.

const C_ContextualStreamLen kStreamLenInvalid = -1; ///< special value for C_ContextualStreamLen when C_ContextualStreamPos markers are incompatible.

inline C_ContextualStreamLen operator - (const C_ContextualStreamPos& end, const C_ContextualStreamPos& start); ///< obtains the length between the position markers.

inline bool CompareStreamLen(const C_ContextualStreamLen& lhs, const C_ContextualStreamLen& rhs); ///< tests if the lengths are different.


/**
@brief A binary stream with extensible context.

It takes a little mental acrobatics to appreciate what this class does, so 
bear with me...

The problem being addressed is how to decouple a serialisation interface from
the context in which the serialisation will occur. Such an interface is 
defined at a low level in terms of a stream object, and implemented at higher
levels by objects that must serialise data in a context-specific manner. We
don't want to push knowledge of the context-dependent data down into the
interface, nor do we want to pull knowledge of the context specifics up into
the implementation.

We solve this by providing interfaces for serialising each context-specific 
data type, and implementing these interfaces as appropriate for the context
in which the data is being serialised. C_ContextualStream wraps a binary 
stream together with a bag of arbitrary interfaces. Stream operators can be
implemented in terms of these interfaces.

@code
// (1) in some low-level library that has no knowledge of data types or 
// contexts
class I_Serialisable
{
public:
	virtual void Serialise(C_ContextualStream& ostream) = 0;
};

// (2) in some mid-level library that provides data types but has no knowledge
// of contexts
class C_SomeDataType;

class I_SomeDataSerialiser : public Sys::C_Anything<FOURCC_U32('ISDS')>
{
public:
	virtual U32 ToU32(const C_SomeDataType& someData) = 0;
};

C_ContextualStream& operator << (C_ContextualStream& ostream, const C_SomeDataType& value)
{
	ostream << ostream.GetContextItem<I_SomeDataSerialiser>()->ToU32(value);
}

// (3) in some game code that handles runtime data but has no knowledge of 
// contexts
class C_GameEntity : public I_Serialisable
{
public:
	virtual void Serialise(C_ContextualStream& ostream)
	{
		ostream << m_SomeData;
	}
private:
	C_SomeDataType m_SomeData;
};

// (4) in some systems code that serialises for saved games but does not know 
// about game logic
class C_SomeDataSaver : public Sys::C_Anything<FOURCC_U32('CSDS'), I_SomeDataSerialiser>
{
public:
	virtual U32 ToU32(const C_SomeDataType& someData)
	{
		m_SomeDataVector.PushBack(someData):
		return m_SomeDataVector.Size() - 1;
	}
private:
	Btl::Vector<C_SomeDataType> m_SomeDataVector;
};

void SaveEntity(I_BinaryStream& stream, const I_Serialisable& serialisable)
{
	C_ContextualStream ostream(stream);
	ostream.AddContext(new C_SomeDataSaver);
	serialisable.Serialise(ostream);
};

// so (3) has knowledge of (1) and (2), and (4) also has knowledge of (1) and 
// (2), but (1) and (2) know only about themselves and (3) and (4) have no 
// knowledge of each other.
@endcode
*/
class C_ContextualStream
{
public:
	C_ContextualStream(Sys::I_BinaryStream& binaryStream, const U32 hintNumContexts = 4);
	~C_ContextualStream();

	inline bool HasErrors() const {return GetFlag(E_Flag_HasErrors);} ///< @returns true if errors were encountered

	inline void Read(void* pDest, const U32 numBytes); ///< reads a number of bytes from the stream
	inline void Write(const void* pSrc, const U32 numBytes); ///< writes a number of bytes to the stream

	bool AddContextItem(const Sys::C_AnythingUid interfaceId, Sys::I_Anything* pContextItem); ///< add a context item by interface
	bool RemoveContextItem(const Sys::C_AnythingUid interfaceId); ///< remove a context item by interface
	Sys::I_Anything* GetContextItem(const Sys::C_AnythingUid interfaceId) const; ///< retrieve a context item by interface
	template <class T_ContextInterface> inline T_ContextInterface* GetContextItem() const; ///< retrieve a context item interface

	C_ContextualStreamPos GetStreamPos() const; ///< gets the current position.

	template <typename T> inline C_ContextualStream& operator << (const T& obj); ///< ostream operator
	template <typename T> inline C_ContextualStream& operator >> (T& obj); ///< istream operator

	template <typename T_ProxyType> inline T_ProxyType ReadAs(); ///< read the value as a specified data type
	template <typename T_ProxyType, typename T_DataType> inline void ReadAs(T_DataType& value); ///< read the value as a specified data type
	template <typename T_ProxyType, typename T_DataType> inline void WriteAs(const T_DataType& value); ///< write the value as a specified data type

private:
	typedef Btl::Pair<Sys::C_AnythingUid, Sys::I_Anything*> C_ContextItem; ///< interface id and implementation object
	typedef Btl::Vector<C_ContextItem> C_ContextItems; ///< set of interfaces implemented

	/// bitmasks for flags
	enum E_Flag
	{
		E_Flag_HasErrors =	1 << 0,		///< set if errors were encountered while reading/writing
	};

	U32						m_Flags;		///< E_Flag values
	Sys::I_BinaryStream&	m_BinaryStream;	///< stream to use
	C_ContextItems			m_ContextItems;	///< context interfaces implemented

	C_ContextualStream(const C_ContextualStream&);				///< can not be copied
	C_ContextualStream& operator = (const C_ContextualStream&);	///< can not be copied

	inline bool GetFlag(const E_Flag flag) const; ///< test if a flag is set
	inline void SetFlag(const E_Flag flag, const bool value = true); ///< set or clear a flag

};

} // namespace Sys


#include <misc_shared/Context/ContextualStream.inl>

#endif // CONTEXTUALSTREAM_5F4CCD39_6838_408F_AFE1_CC64228767A3_H
