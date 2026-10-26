#pragma warning("WARNING: Including stream.h from 'common'. This should never happen.")
#pragma once

#include "exportscene\debugstream.h"
#include "utility\vector.h"
#include "utility\endian.h"
#include "utility\autopointer.h"

#ifdef GOTHAM3
#include "utility\stackallocator.h"
#include "resource\bizfile.h"
#else
#ifdef _CONVERTER
#include "filestuff.h"
#define BizFile FileStuff
#endif
#endif

class BizFile;

namespace Ut
{

///////////////////////////////////////////////////////////////////////

template <typename EndianSwap> class BinaryOutputStreamBuffer;
template <typename EndianSwap> class BinaryInputStreamBuffer;

// memory buffer
// used for input and output
class BinaryStreamBuffer
{
public:
	friend BinaryOutputStreamBuffer<Ut::DoEndianSwap>;
	friend BinaryOutputStreamBuffer<Ut::NoEndianSwap>;
	friend BinaryInputStreamBuffer<Ut::DoEndianSwap>;
	friend BinaryInputStreamBuffer<Ut::NoEndianSwap>;

	BinaryStreamBuffer() : m_lock(false) {}

	void Lock()
	{
		assert(m_lock == false);
		m_lock = true;
	}

	void UnLock()
	{
		assert(m_lock == true);
		m_lock = false;
	}

	void* Data()
	{
		if (m_buffer.Capacity() > 0) return &m_buffer[0];
		else return 0;
	}

	// returns true for success
	bool Initialise(int size)
	{
		return m_buffer.Reserve(size, 0);
	}

	int Capacity()
	{
		return m_buffer.Capacity();
	}

	void CheckCapacity(int l, char** position)
	{
		if (m_buffer.Capacity())
		{
			int offset = *position - &m_buffer[0];

			if (offset + l > m_buffer.Capacity())
			{
				m_buffer.Reserve(m_buffer.Capacity() * 2 + l, offset);
				*position = offset + &m_buffer[0];
			}
		}
		else
		{
			m_buffer.Reserve(l * 2, 0);
			*position = &m_buffer[0];
		}
	}

	int Size(const char* position) const 
	{
		if (position) return position - &m_buffer[0];
		else return 0;
	}

private:
	//Ut::DynamicArrayData<char, Ut::AlignedAllocator<char, 4> > m_buffer;
	Ut::DynamicArrayData<char, Ut::TrackedAlignedAllocator<char, 4, MTK_GFX | MTK_GFX_CITY> > m_buffer;
	bool m_lock;
};

///////////////////////////////////////////////////////////////////

template <typename T, typename EndianSwap> class ZLibCompressStream;

// output to binary stream
template <typename EndianSwap>
class BinaryOutputStreamBuffer
{
public:
	typedef EndianSwap EndianType;

	template<typename T> struct CompressedType {typedef ZLibCompressStream<T, EndianSwap> Result;};

	friend BinaryStreamBuffer;

	BinaryOutputStreamBuffer(BinaryStreamBuffer& stream) : m_position(0), m_stream(stream) 
	{
		if (stream.Capacity() > 0)
			m_position = (char*) stream.Data();
		else
			m_position = 0;

		m_stream.Lock();
	}

	~BinaryOutputStreamBuffer() {m_stream.UnLock();}

	BinaryOutputStreamBuffer& operator << (const char* s)
	{
		int length = strlen(s);
		assert(length >= 0);
		*this << length;
		WriteData(s, length);
		return *this;
	}

	BinaryOutputStreamBuffer& operator << (U32 n)
	{
		int l = sizeof(U32);
		m_stream.CheckCapacity(l, &m_position);
		EndianSwap::EndianSwap(n);
		(U32&) (*m_position) = n;
		m_position += l;
		return *this;
	}

	BinaryOutputStreamBuffer& operator << (U16 n)
	{
		int l = sizeof(U16);
		m_stream.CheckCapacity(l, &m_position);
		EndianSwap::EndianSwap(n);
		(U16&) (*m_position) = n;
		m_position += l;
		return *this;
	}

	BinaryOutputStreamBuffer& operator << (S32 n)
	{
		int l = sizeof(S32);
		m_stream.CheckCapacity(l, &m_position);
		EndianSwap::EndianSwap(n);
		(S32&) (*m_position) = n;
		m_position += l;
		return *this;
	}

	BinaryOutputStreamBuffer& operator << (S16 n)
	{
		int l = sizeof(S16);
		m_stream.CheckCapacity(l, &m_position);
		EndianSwap::EndianSwap(n);
		(S16&) (*m_position) = n;
		m_position += l;
		return *this;
	}

	BinaryOutputStreamBuffer& operator << (U8 n)
	{
		int l = sizeof(U8);
		m_stream.CheckCapacity(l, &m_position);
		(U8&) (*m_position) = n;
		m_position += l;
		return *this;
	}

	BinaryOutputStreamBuffer& operator << (S8 n)
	{
		int l = sizeof(S8);
		m_stream.CheckCapacity(l, &m_position);
		(S8&) (*m_position) = n;
		m_position += l;
		return *this;
	}

	BinaryOutputStreamBuffer& operator << (float f)
	{
		int l = sizeof(float);
		m_stream.CheckCapacity(l, &m_position);
		EndianSwap::EndianSwap(f);
		(float&) (*m_position) = f;
		m_position += l;
		return *this;
	}

	BinaryOutputStreamBuffer& operator << (bool n) {return *this << (U8) n;}

	// raw data
	void WriteData(const void* p, int n)
	{
		m_stream.CheckCapacity(n, &m_position);

		const char* p1 = (char*) p;

		for (int i = 0; i < n; i++)
			*(m_position++) = *(p1++);
	}

	void BeginVList(int n) {*this << n;}
	void EndVList() {}

	// debug formatting - ignore
	void Write(Name& name) {}
	void BeginBlock() {}
	void EndBlock() {}
	void BeginElement(int n) {}
	void EndElement() {}
	void BeginHList() {}
	void BeginHList(int n) {*this << n;}
	void EndHList() {}

	int Size() const {return m_stream.Size(m_position);}

	void* Data()
	{
		return m_stream.Data();
	}

	void Align(int align) 
	{
		int s1 = m_position - (char*) m_stream.Data();
		int s2 = (s1 / align + ((s1 % align == 0) ? 0 : 1)) * align;
		m_stream.CheckCapacity(s2 - s1, &m_position);
		m_position = (char*) m_stream.Data() + s2;
	}

private:
	BinaryOutputStreamBuffer(const BinaryOutputStreamBuffer& other); // no copy

	BinaryStreamBuffer& m_stream;
	char* m_position;
};

// get size of stream data
class BinaryOutputStreamCounter
{
public:
	typedef Ut::NoEndianSwap EndianType;
	template<typename T> struct CompressedType {typedef T& Result;};
	friend BinaryStreamBuffer;

	BinaryOutputStreamCounter() : m_size(0) {}

	~BinaryOutputStreamCounter() {}

	BinaryOutputStreamCounter& operator << (const char* s)
	{
		int length = strlen(s);
		assert(length >= 0);
		*this << length;
		WriteData(s, length);
		return *this;
	}

	BinaryOutputStreamCounter& operator << (U32 n)
	{
		m_size += sizeof(U32);
		return *this;
	}

	BinaryOutputStreamCounter& operator << (U16 n)
	{
		m_size += sizeof(U16);
		return *this;
	}

	BinaryOutputStreamCounter& operator << (S32 n)
	{
		m_size += sizeof(S32);
		return *this;
	}

	BinaryOutputStreamCounter& operator << (S16 n)
	{
		m_size += sizeof(S16);
		return *this;
	}

	BinaryOutputStreamCounter& operator << (U8 n)
	{
		m_size += sizeof(U8);
		return *this;
	}

	BinaryOutputStreamCounter& operator << (S8 n)
	{
		m_size += sizeof(S8);
		return *this;
	}

	BinaryOutputStreamCounter& operator << (float f)
	{
		m_size += sizeof(float);
		return *this;
	}

	BinaryOutputStreamCounter& operator << (bool n) {return *this << (U8) n;}

	// raw data
	void WriteData(const void* p, int n)
	{
		m_size += n;
	}

	void BeginVList(int n) {*this << n;}
	void EndVList() {}

	// debug formatting - ignore
	void Write(Name& name) {}
	void BeginBlock() {}
	void EndBlock() {}
	void BeginElement(int n) {}
	void EndElement() {}
	void BeginHList() {}
	void BeginHList(int n) {*this << n;}
	void EndHList() {}

	int Size() const {return m_size;}

	void Align(int align) 
	{
		int s1 = m_size;
		int s2 = (s1 / align + ((s1 % align == 0) ? 0 : 1)) * align;
		m_size = s2;
	}

private:
	template <typename EndianSwap> BinaryOutputStreamCounter(const BinaryOutputStreamBuffer<EndianSwap>& other); // no copy

	int m_size;
};

#ifdef _CONVERTER

// output to file
template <typename EndianSwap>
class BinaryFileOutputStream
{
public:
	typedef EndianSwap EndianType;
	template<typename T> struct CompressedType {typedef ZLibCompressStream<T, EndianSwap> Result;};

	BinaryFileOutputStream(CFile& file) : m_file(file) {}
	~BinaryFileOutputStream() {}

	BinaryFileOutputStream& operator << (const std::string& string)
	{
		*this << string.c_str();
		return *this;
	}

	BinaryFileOutputStream& operator << (const char* s)
	{
		int length = strlen(s);
		assert(length >= 0);
		*this << length;
		Write(s, length);
		return *this;
	}

	BinaryFileOutputStream& operator << (U32 n)
	{
		EndianSwap::EndianSwap(n);
		Write(&n, sizeof(U32));
		return *this;
	}

	BinaryFileOutputStream& operator << (U16 n)
	{
		EndianSwap::EndianSwap(n);
		Write(&n, sizeof(U16));
		return *this;
	}

	BinaryFileOutputStream& operator << (S32 n)
	{
		EndianSwap::EndianSwap(n);
		Write(&n, sizeof(S32));
		return *this;
	}

	BinaryFileOutputStream& operator << (S16 n)
	{
		EndianSwap::EndianSwap(n);
		Write(&n, sizeof(S16));
		return *this;
	}

	BinaryFileOutputStream& operator << (U8 n)
	{
		Write(&n, sizeof(U8));
		return *this;
	}

	BinaryFileOutputStream& operator << (S8 n)
	{
		Write(&n, sizeof(S8));
		return *this;
	}

	BinaryFileOutputStream& operator << (float f)
	{
		EndianSwap::EndianSwap(f);
		Write(&f, sizeof(float));
		return *this;
	}

	BinaryFileOutputStream& operator << (bool n) {return *this << (U8) n;}

	// raw data
	void WriteData(const U8* p, int n)
	{
		Write(p, n);
	}

	void BeginVList(int n) {*this << n;}
	void EndVList() {}

	// debug formating - ignore
	void Write(Name& name) {}
	void BeginBlock() {}
	void EndBlock() {}
	void BeginElement(int n) {}
	void EndElement() {}
	void BeginHList() {}
	void BeginHList(int n) {*this << n;}
	void EndHList() {}

	int Size() const {return (int) m_file.GetPosition();}

	void Align(int align) 
	{
		int s1 = Size();
		int s2 = (s1 / align + ((s1 % align == 0) ? 0 : 1)) * align;
		int offset = s2 - s1;

		m_file.SetLength(m_file.GetPosition() + offset);
		m_file.Seek(0, CFile::end);
	}

private:
	BinaryFileOutputStream(const BinaryFileOutputStream& other); // no copy

	void Write(const void* p, int n)
	{
		m_file.Write(p, n);
	}

	CFile& m_file;
};

#endif

/////////////////////////////////////////////////////////////

template<typename Base>
class OutputStream : public Base
{
public:
	OutputStream() {PushVersion(0);}

	template<typename T>
	explicit OutputStream(T& t) : Base(t) {PushVersion(0);}

	using Base::operator <<;

	OutputStream& operator << (Name name)
	{
		Write(name);
		return *this;
	}

	// call function using stream operator
	//OutputStream& operator << (void (OutputStream::*f)()) {(this->*f)(); return *this;}

	// write vector as vlist
	template<typename T>
	struct VListReference
	{
		VListReference(const T& vector) : m_vector(vector) {}
		const T& m_vector;
	};

	// write vector as hList
	template<typename T>
	struct HListReference
	{
		HListReference(const T& vector) : m_vector(vector) {}
		const T& m_vector;
	};

	template<typename T>
	struct ArrayVectorReference
	{
		ArrayVectorReference(const T& vector) : m_vector(vector) {}
		const T& m_vector;
	};

	// raw data
	template<typename T> // T is not used, but this causes a compiler error if it is not a template
	struct RawDataReference
	{
		RawDataReference(const U8* const& p, const int& n) : m_p(p), m_n(n) {}
		const U8* const& m_p;
		const int& m_n;
	};

	template<typename T>
	static HListReference<T> HList(const T& vector) {return HListReference<T>(vector);}

	template<typename T>
	static VListReference<T> VList(const T& vector) {return VListReference<T>(vector);}

	template<typename T>
	static ArrayVectorReference<T> ArrayVector(const T& vector) {return ArrayVectorReference<T>(vector);}

	static RawDataReference<U8> RawData(const U8* const& p, const int& n) {return RawDataReference<U8>(p, n);}

	// can't access base typedef directly as return type for some reason
	/*template<typename T> struct CompressedType {typedef typename Base::CompressedType::Result Result;};

	template<typename T>
	static typename CompressedType<T>::Result Compress(const T& object) 
	{
		return typename CompressedType<T>::Result(object);
	}*/

	// default vector output as hlist
	template <typename S>
	OutputStream& operator << (const Ut::UtVector<S>& vector)
	{
		*this << HList(vector);
		return *this;
	}

	// vector as a vlist
	template<typename T>
	OutputStream& operator << (const VListReference<T>& b)
	{
		BeginVList(b.m_vector.Size());
		BeginBlock();

		for (int i = 0; i < b.m_vector.Size(); i++)
		{
			BeginElement(i); 
			*this << b.m_vector[i];
			EndElement();
		}

		EndVList();
		EndBlock();

		return *this;
	}

	// vector as an hlist
	template<typename T>
	OutputStream& operator << (const HListReference<T>& hList) 
	{
		BeginHList(hList.m_vector.Size());

		for (int i = 0; i < hList.m_vector.Size(); i++)
			*this << hList.m_vector[i];

		EndHList();

		return *this;
	}

	// array vector
	template<typename T>
	OutputStream& operator << (const ArrayVectorReference<T>& vector) 
	{
		BeginHList();

		for (int i = 0; i < vector.m_vector.Size(); i++)
			*this << vector.m_vector[i];

		EndHList();

		return *this;
	}

	// raw data
	template<typename T>
	OutputStream& operator << (const RawDataReference<T>& data) 
	{
		Base::WriteData(data.m_p, data.m_n);

		return *this;
	}

	void PushVersion(int version) {m_versionList.PushBack(version);}
	void PopVersion() {m_versionList.PopBack();}
	int Version() const {return m_versionList.Back();}

	UtVector<Ut::DynamicVector<int> > m_versionList;
};

///////////////////////////////////////////////////////////////////////
// input

// input from binary stream
template <typename EndianSwap>
class BinaryInputStreamBuffer
{
public:
	friend BinaryStreamBuffer;

	typedef EndianSwap EndianType;
	template<typename T> struct CompressedType {typedef ZLibCompressStream<T, EndianSwap> Result;};

	/*BinaryInputStreamBuffer& operator >> (int& n)
	{
		n = *(int*) m_position;
		m_position += sizeof(int);
		return *this;
	}*/

	BinaryInputStreamBuffer& operator >> (U32& n)
	{
		EndianSwap::EndianSwapD((char*) &n, m_position);
		m_position += sizeof(U32);
		return *this;
	}

	BinaryInputStreamBuffer& operator >> (U16& n)
	{
		EndianSwap::EndianSwapW((char*) &n, m_position);
		m_position += sizeof(U16);
		return *this;
	}

	BinaryInputStreamBuffer& operator >> (S32& n)
	{
		EndianSwap::EndianSwapD((char*) &n, m_position);
		m_position += sizeof(S32);
		return *this;
	}

	BinaryInputStreamBuffer& operator >> (S16& n)
	{
		EndianSwap::EndianSwapW((char*) &n, m_position);
		m_position += sizeof(S16);
		return *this;
	}

	BinaryInputStreamBuffer& operator >> (U8& n)
	{
		n = *(U8*) m_position;
		m_position += sizeof(U8);
		return *this;
	}

	BinaryInputStreamBuffer& operator >> (S8& n)
	{
		n = *(S8*) m_position;
		m_position += sizeof(S8);
		return *this;
	}

	BinaryInputStreamBuffer& operator >> (float& f)
	{
		EndianSwap::EndianSwapD((char*) &f, m_position);
		m_position += sizeof(float);
		return *this;
	}

	BinaryInputStreamBuffer& operator >> (bool& n) {U8 temp; *this >> temp; n = temp ? true : false; return *this;}

	typedef U8* PointerType;

	// raw data
	PointerType Read(U8* p, int n)
	{
		if (p)
		{
			assert(n >= 0);
			memcpy(p, m_position, n);
			//XMemCpy(p, m_position, n);
			m_position += n;

			return 0;
		}
		else
		{
			PointerType pData = (U8*) m_position;
			m_position += n;
			return pData;
		}
	}

	void SetOffset(int n)
	{
		m_position = m_begin + n;
	}

	bool StreamValid() const {return m_begin != 0;}

protected:
	BinaryInputStreamBuffer(U8* begin) : m_begin(begin), m_position(begin) {}
	~BinaryInputStreamBuffer() {}

private:
	BinaryInputStreamBuffer(const BinaryOutputStreamBuffer<EndianSwap>& other);

	U8* m_begin;
	U8* m_position;
};

// input from binary stream
template <typename EndianSwap>
class BinaryFileInputStream
{
public:
	friend BinaryStreamBuffer;

	typedef EndianSwap EndianType;
	template<typename T> struct CompressedType {typedef ZLibCompressStream<T, EndianSwap> Result;};

#ifdef _CONVERTER
	UT_FORCE_INLINE BinaryFileInputStream& operator >> (std::string& s)
	{
		int length;
		*this >> length;
		s.resize(length);
		m_file.Read(&(s[0]), length);
		return *this;
	}
#endif

	UT_FORCE_INLINE BinaryFileInputStream& operator >> (U32& data)
	{
#ifndef _CONVERTER
		U32 bytesRead;
		m_file.ReadFile(&data, sizeof(data), bytesRead);
#else
		m_file.Read(&data, sizeof(data));
#endif
		EndianSwap::EndianSwap(data);
		return *this;
	}

	UT_FORCE_INLINE BinaryFileInputStream& operator >> (U16& data)
	{
#ifndef _CONVERTER
		U32 bytesRead;
		m_file.ReadFile(&data, sizeof(data), bytesRead);
#else
		m_file.Read(&data, sizeof(data));
#endif
		EndianSwap::EndianSwap(data);
		return *this;
	}

	UT_FORCE_INLINE BinaryFileInputStream& operator >> (S32& data)
	{
#if !defined(_CONVERTER)
		U32 bytesRead;
		m_file.ReadFile(&data, sizeof(data), bytesRead);
#else
		m_file.Read(&data, sizeof(data));
#endif
		EndianSwap::EndianSwap(data);
		return *this;
	}

	UT_FORCE_INLINE BinaryFileInputStream& operator >> (S16& data)
	{
#if !defined(_CONVERTER)
		U32 bytesRead;
		m_file.ReadFile(&data, sizeof(data), bytesRead);
#else
		m_file.Read(&data, sizeof(data));
#endif
		EndianSwap::EndianSwap(data);
		return *this;
	}

	UT_FORCE_INLINE BinaryFileInputStream& operator >> (U8& data)
	{
#if !defined(_CONVERTER)
		U32 bytesRead;
		m_file.ReadFile(&data, sizeof(data), bytesRead);
#else
		m_file.Read(&data, sizeof(data));
#endif
		return *this;
	}

	UT_FORCE_INLINE BinaryFileInputStream& operator >> (S8& data)
	{
#if !defined(_CONVERTER)
		U32 bytesRead;
		m_file.ReadFile(&data, sizeof(data), bytesRead);
#else
		m_file.Read(&data, sizeof(data));
#endif
		return *this;
	}

	UT_FORCE_INLINE BinaryFileInputStream& operator >> (float& data)
	{
#if !defined(_CONVERTER)
		U32 bytesRead;
		m_file.ReadFile(&data, sizeof(data), bytesRead);
#else
		m_file.Read(&data, sizeof(data));
#endif
		EndianSwap::EndianSwap(data);
		return *this;
	}

	UT_FORCE_INLINE BinaryFileInputStream& operator >> (bool& data) {U8 temp; *this >> temp; data = temp ? true : false; return *this;}

	typedef Ut::AutoPointer<U8, Ut::ArrayAllocator<U8> > PointerType;

	// raw data
	PointerType Read(U8* p, int n)
	{
		if (p)
		{
			assert(p);
#if !defined(_CONVERTER)
			U32 bytesRead;
			m_file.ReadFile(p, n, bytesRead);
#else
			m_file.Read(p, n);
#endif
			return PointerType();
		}
		else
		{
			PointerType pData;
			Allocate(pData, n);
#if !defined(_CONVERTER)
			U32 bytesRead;
			m_file.ReadFile(&*pData, n, bytesRead);
#else
			m_file.Read(&*pData, n);
#endif
			return pData;
		}
	}

	UT_FORCE_INLINE bool StreamValid() const {return m_file.IsOpen();}

protected:
	BinaryFileInputStream(BizFile& file) : m_file(file) 
	{
#ifndef _CONVERTER
		file.SetEndianSwap(false);
#endif
	}

	~BinaryFileInputStream() {}

private:
	BizFile& m_file;
};

template <typename Base>
class InputStream : public Base
{
public:
#if !defined(_CONVERTER) && !defined(RELEASE_VERSION)
	InputStream(U8* begin, DWORD systemID) : Base(begin), dwSystemID(systemID) { PushVersion(0); }
	InputStream(BizFile& file, DWORD systemID) : Base(file), dwSystemID(systemID) { PushVersion(0); }
#else
	InputStream(U8* begin, DWORD systemID=0) : Base(begin), dwSystemID(systemID)  { PushVersion(0); }
	InputStream(BizFile& file, DWORD systemID=0) : Base(file), dwSystemID(systemID)  { PushVersion(0); }
#endif

	using Base::operator >>;

	// read vector
	template <typename S>
	UT_FORCE_INLINE InputStream& operator >> (Ut::UtVector<S>& vector)
	{
		*this >> DynamicVector(vector);
		return *this;
	}

	template<typename T>
	struct ArrayVectorType
	{
		ArrayVectorType(T& vector) : m_vector(vector) {}
		T& m_vector;
	};

	template<typename T>
	struct StackVectorType
	{
		StackVectorType(T& vector) : m_vector(vector) {}
		T& m_vector;
	};

	template<typename T>
	struct DynamicVectorType
	{
		DynamicVectorType(T& vector) : m_vector(vector) {}
		T& m_vector;
	};

	template<typename T>
	struct ManagedVectorType
	{
		ManagedVectorType(T& vector) : m_vector(vector) {}
		T& m_vector;
	};

	template<typename T>
	static ArrayVectorType<T> ArrayVector(T& vector) {return ArrayVectorType<T>(vector);}

	template<typename T>
	static StackVectorType<T> StackVector(T& vector) {return StackVectorType<T>(vector);}

	template<typename T>
	static DynamicVectorType<T> DynamicVector(T& vector) {return DynamicVectorType<T>(vector);}

	//template<typename T>
	//static ManagedVectorType<T> ManagedVector(T& vector) {return ManagedVectorType<T>(vector);}

	// can't access base typedef directly as return type for some reason
	template<typename T> struct CompressedType {typedef typename Base::CompressedType::Result Result;};

	template<typename T>
	static typename CompressedType<T>::Result Decompress(T& object) 
	{
		return typename CompressedType<T>::Result(object);
	}

	// fixed size vector
	template<typename T>
	UT_FORCE_INLINE InputStream& operator >> (ArrayVectorType<T>& vector) 
	{
		for (int i = 0; i < vector.m_vector.Size(); i++)
			*this >> vector.m_vector[i];

		return *this;
	}

	// vector without reserve function
	template<typename T>
	UT_FORCE_INLINE InputStream& operator >> (StackVectorType<T>& vector) 
	{
		int n;
		*this >> n;

		for (int i = 0; i < n; i++)
		{
			vector.m_vector.PushBack();
			*this >> vector.m_vector[i];
		}

		return *this;
	}

	// resizeable vector
	template<typename T>
	UT_FORCE_INLINE InputStream& operator >> (DynamicVectorType<T>& vector) 
	{
		int n;
		*this >> n;

		// only stream into an empty vector
		assert(vector.m_vector.Capacity() == 0);
		vector.m_vector.Reserve(n);

		for (int i = 0; i < n; i++)
		{
			vector.m_vector.PushBack();
			*this >> vector.m_vector.Back();
		}

		return *this;
	}

	UT_FORCE_INLINE typename Base::PointerType Read(U8* p, int n)
	{
		return Base::Read(p, n);
	}

	UT_FORCE_INLINE void PushVersion(int version) {m_versionList.PushBack(version);}
	UT_FORCE_INLINE void PopVersion() {m_versionList.PopBack();}
	UT_FORCE_INLINE int Version() const {return m_versionList.Back();}

	UtVector<Ut::DynamicVector<int> > m_versionList;
	DWORD dwSystemID;	// Which system we're currently processing data for, i.e. City, Cars
	//StackAllocator* m_pStackAllocator;
};

}