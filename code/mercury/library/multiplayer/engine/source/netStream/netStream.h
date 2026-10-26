#ifndef NETSTREAM_H
#define NETSTREAM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>

BD_REFERENCE(bdCommonAddr);
BD_REFERENCE(netStream);
class netStream: public bdReferencable
{
public:
	enum
	{
		STATUS_OK,
		STATUS_INVALID,
		STATUS_FAILED_READ,
		STATUS_FAILED_WRITE,
		STATUS_FAILED_READ_OVERFLOW,
		STATUS_FAILED_WRITE_OVERFLOW,
		STATUS_FAILED_READ_PROTECTED,
		STATUS_FAILED_WRITE_PROTECTED,
		STATUS_FAILED_READ_VERSION,
	};

	enum
	{
		MODE_READ  = (1 << 0),
		MODE_WRITE = (1 << 1),
	};

	netStream(int limit, int write=0, int read=0)
	: m_limit(limit)
	, m_write(write)
	, m_read(read)
	, m_status(STATUS_OK)
	, m_mode(0)
	{
		setMode(MODE_READ | MODE_WRITE);
	}

	virtual ~netStream()
	{}

	// write a Variable Bit Length Encoded integer.
	template<typename T> bool readVBLE(T& val);
	template<typename T> bool writeVBLE(const T& val);

	template<typename T> bool writeType(const T& val) { return writeBytes(&val, sizeof(val)); }
	template<typename T> bool readType(T& val)		  { return  readBytes(&val, sizeof(val)); }

	virtual netStream& operator = (const netStream& stream);
	
	virtual bool  read(netStream* stream, int len);
	virtual bool write(netStream* stream, int len);
	virtual bool  copy(netStream* stream);
	virtual bool different(netStream* stream);
	virtual bool different(netStream* stream, int len);

	virtual bool readMarker(const char* magic);
	virtual bool writeMarker(const char* magic);

	virtual bool readBytes(void* ptr, int len);
	virtual bool writeBytes(const void* ptr, int len);

	virtual void clear() { m_write = m_read = 0; m_status = STATUS_OK; }
	
	bool byteWriteOverflow(int bytes);
	bool  byteReadOverflow(int bytes);
	
	int  readRemaining() const { return (m_read >= m_write)? 0 : (m_write - m_read); }
	int  writeRemaining() const { return (m_write >= m_limit)? 0 : (m_limit - m_write); }

	bool       eof() const { return (readRemaining() == 0); }
	bool     error() const { return (m_status != STATUS_OK); }
	bool        ok() const { return (m_status == STATUS_OK); }

	int status() const { return m_status; }
	void setStatus(int status) { m_status = status; }
	
	int modeValid(int mode) const { return (m_mode & mode) == mode; }
	void changeMode(int mode, bool onOff) { if(onOff) m_mode |= mode; else m_mode &= ~mode; }
	void setMode(int mode) { m_mode = mode; }

	int  getReadByteCount() const { return m_read; }
	int getWriteByteCount() const { return m_write; }
	int getWriteByteLimit() const { return m_limit; }
	
	virtual bool  setReadByteCount(int ptr)	{ assert(ptr <= m_write); m_read = ptr; return (m_read <= m_write); }
	virtual bool setWriteByteCount(int ptr) { assert(ptr <= m_limit); m_write = ptr; return (m_write <= m_limit); }
	virtual bool setWriteByteLimit(int ptr) { m_limit = ptr; return true; }
	
	virtual const char* buffer	 () const { return NULL; }
	virtual char*		buffer	 ()		  { return NULL; }

	template<typename T> T clamp(T val, T min, T max) { return (val < min)? min : (val > max)? max : val; }

	template<typename T> U8  clampU8 (T val) { assert(val >= 0 && val <=  UCHAR_MAX); return (U8)  clamp<T>(val, 0,  UCHAR_MAX); }
	template<typename T> U16 clampU16(T val) { assert(val >= 0 && val <=  USHRT_MAX); return (U16) clamp<T>(val, 0,  USHRT_MAX); }
	template<typename T> U32 clampU32(T val) { assert(val >= 0 && val <=   UINT_MAX); return (U32) clamp<T>(val, 0,   UINT_MAX); }
	template<typename T> U64 clampU64(T val) { assert(val >= 0 && val <= ULLONG_MAX); return (U64) clamp<T>(val, 0, ULLONG_MAX); }

	template<typename T> S8  clampS8 (T val) { assert(val >=  CHAR_MIN && val <=  CHAR_MAX); return (S8)  clamp<T>(val,  CHAR_MIN,  CHAR_MAX); }
	template<typename T> S16 clampS16(T val) { assert(val >=  SHRT_MIN && val <=  SHRT_MAX); return (S16) clamp<T>(val,  SHRT_MIN,  SHRT_MAX); }
	template<typename T> S32 clampS32(T val) { assert(val >=   INT_MIN && val <=   INT_MAX); return (S32) clamp<T>(val,   INT_MIN,   INT_MAX); }
	template<typename T> S64 clampS64(T val) { assert(val >= LLONG_MIN && val <= LLONG_MAX); return (S64) clamp<T>(val, LLONG_MIN, LLONG_MAX); }

	template<typename T> bool writeU8 (const T& val) { U8  temp = clampU8 (val); return writeBytes(&temp, sizeof(temp)); }
	template<typename T> bool writeU16(const T& val) { U16 temp = clampU16(val); return writeBytes(&temp, sizeof(temp)); }
	template<typename T> bool writeU32(const T& val) { U32 temp = clampU32(val); return writeBytes(&temp, sizeof(temp)); }
	template<typename T> bool writeU64(const T& val) { U64 temp = clampU64(val); return writeBytes(&temp, sizeof(temp)); }
	
	template<typename T> bool writeS8 (const T& val) { S8  temp = clampS8 (val); return writeBytes(&temp, sizeof(temp)); }
	template<typename T> bool writeS16(const T& val) { S16 temp = clampS16(val); return writeBytes(&temp, sizeof(temp)); }
	template<typename T> bool writeS32(const T& val) { S32 temp = clampS32(val); return writeBytes(&temp, sizeof(temp)); }
	template<typename T> bool writeS64(const T& val) { S64 temp = clampS64(val); return writeBytes(&temp, sizeof(temp)); }
	
	template<typename T> bool writeF32(const T& val) { F32 temp = (F32) val; return writeBytes(&temp, sizeof(temp)); }
	template<typename T> bool writeF64(const T& val) { F64 temp = (F64) val; return writeBytes(&temp, sizeof(temp)); }

	template<typename T> bool readU8 (T& val) { U8  temp; bool ret = readBytes(&temp, sizeof(temp)); if(ok()) val = (T)temp; return ret; }
	template<typename T> bool readU16(T& val) { U16 temp; bool ret = readBytes(&temp, sizeof(temp)); if(ok()) val = (T)temp; return ret; }
	template<typename T> bool readU32(T& val) { U32 temp; bool ret = readBytes(&temp, sizeof(temp)); if(ok()) val = (T)temp; return ret; }
	template<typename T> bool readU64(T& val) { U64 temp; bool ret = readBytes(&temp, sizeof(temp)); if(ok()) val = (T)temp; return ret; }
	
	template<typename T> bool readS8 (T& val) { S8  temp; bool ret = readBytes(&temp, sizeof(temp)); if(ok()) val = (T)temp; return ret; }
	template<typename T> bool readS16(T& val) { S16 temp; bool ret = readBytes(&temp, sizeof(temp)); if(ok()) val = (T)temp; return ret; }
	template<typename T> bool readS32(T& val) { S32 temp; bool ret = readBytes(&temp, sizeof(temp)); if(ok()) val = (T)temp; return ret; }
	template<typename T> bool readS64(T& val) { S64 temp; bool ret = readBytes(&temp, sizeof(temp)); if(ok()) val = (T)temp; return ret; }

	template<typename T> bool readF32(T& val) { F32 temp; bool ret = readBytes(&temp, sizeof(temp)); if(ok()) val = (T)temp; return ret; }
	template<typename T> bool readF64(T& val) { F64 temp; bool ret = readBytes(&temp, sizeof(temp)); if(ok()) val = (T)temp; return ret; }

public:
	bool readSafe(void* ptr, int len);
	bool writeSafe(const void* ptr, int len);

	virtual bool readBytes_internal(void* ptr, int len)=0;
	virtual bool writeBytes_internal(const void* ptr, int len)=0;
	
	int		m_read;
	int		m_write;
	int		m_limit;
	int		m_status;
	int		m_mode;
};


// write a Variable Bit Length Encoded integer.
template<typename TYPE> 
bool netStream::readVBLE(TYPE& val)
{
	val = 0;

	unsigned char next;
	TYPE		  exponent = 1;
	unsigned char encoded;
	unsigned char byte;
	
	do
	{
		if(!readType(encoded)) return false;
		next = (encoded & 128);
		byte = (encoded & 127);
		val  = val + (byte * exponent);
		exponent *= 128;
	}
	while(next != 0);

	assert(val > 0);

	return true;
}

template<typename TYPE> 
bool netStream::writeVBLE(const TYPE& val)
{
	assert(val > 0);

	TYPE		  rest = val;
	unsigned char next;
	unsigned char encoded;
	unsigned char byte;
	
	do
	{
		byte = (rest & 127);
		rest /= 128;
		next = (rest > 0)? 128 : 0;
		encoded = byte | next;
		if(!writeType(encoded)) return false;
	}
	while(next != 0);

	return true;
}
#endif //NETSTREAM_H
