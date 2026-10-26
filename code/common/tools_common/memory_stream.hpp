#ifndef __MEMORY_STREAM_H__
#define __MEMORY_STREAM_H__

namespace std
{

template<
	class Elem, 
	class Tr = char_traits<Elem> 
>	
class memory_streambuf : 
	public basic_streambuf<Elem, Tr>
{
public:
	typedef basic_streambuf<Elem, Tr>  basic_streambuf_type;

	typedef unsigned char byte_type;
	typedef byte_type* byte_buffer_type;

	memory_streambuf()
	{
		m_pData = NULL;
		m_iDataSize = m_iBufferSize = m_iReadPosition = 0;
		m_iGrowSize = 64 * 1024;
	}

	~memory_streambuf()
	{
		if (m_pData)
			delete m_pData;
	}

	int sync()
	{
		// ???
		return 0;
	}

	int overflow (int c)
	{
		m_iDataSize++;
		// increase buffer size
		if (m_iDataSize >= m_iBufferSize)
		{
			m_iBufferSize += m_iGrowSize;
			char *pNewData = new char[m_iBufferSize];
			if (m_pData)
				memcpy(pNewData, m_pData, sizeof(char) * m_iDataSize);
			delete [] m_pData;
			m_pData = pNewData;
		}

		// append data
		m_pData[m_iDataSize-1] = c;

		return 1;
	}

	int underflow ()
	{
		if (m_iReadPosition >= m_iDataSize)
			return 0;
		Elem c = m_pData[ m_iReadPosition ];
		m_iReadPosition++;
		return c;
	}

#if defined(COMPILER_VC_7) || defined(COMPILER_GNU)
	virtual streamsize xsgetn(Elem * _Ptr, streamsize _Count)
	{
		int iEnd = (m_iReadPosition + _Count);
		if (iEnd > m_iDataSize)
			iEnd = m_iDataSize;
		int n = iEnd - m_iReadPosition;
		memcpy(_Ptr, m_pData + m_iReadPosition, sizeof(char) * n);
		m_iReadPosition += n;

		return n;
	}

	virtual streamsize xsputn(const char *_Ptr, streamsize _Count)
	{
		int iNewDataSize = m_iDataSize + _Count;

		// increase buffer size
		if (iNewDataSize >= m_iBufferSize)
		{
			m_iBufferSize = iNewDataSize + m_iGrowSize;
			char *pNewData = new char[m_iBufferSize];
			if (m_pData)
				memcpy(pNewData, m_pData, sizeof(char) * m_iDataSize);
			delete [] m_pData;
			m_pData = pNewData;
		}

		// append data
		memcpy(m_pData+m_iDataSize, _Ptr, _Count);
		m_iDataSize = iNewDataSize;

		return _Count;
	}
#endif

	virtual streamsize _Xsgetn_s(Elem * _Ptr, size_t _Ptr_size, streamsize _Count)
	{
		int iEnd = (m_iReadPosition + _Count);
		if (iEnd > m_iDataSize)
			iEnd = m_iDataSize;
		int n = iEnd - m_iReadPosition;
		memcpy(_Ptr, m_pData + m_iReadPosition, sizeof(char) * n);
		m_iReadPosition += n;

		return n;
	}

	void setgrowsize(int i)
	{
		m_iGrowSize = i;
	}

	int getlength()
	{
		return m_iDataSize;
	}

	char *detach()
	{
		char *pOut = m_pData;
		m_iBufferSize = m_iDataSize = 0;
		m_pData = NULL;
		return pOut;
	}

	void attach(char *pData, int c)
	{
		m_pData = pData;
		m_iDataSize = m_iBufferSize = c;
	}

	streamsize flush()
	{
		// ???
	}

	char		*m_pData;
	int			m_iBufferSize;
	int			m_iDataSize;
	int			m_iGrowSize;
	int			m_iReadPosition;

	char *getbuffer()
	{
		return m_pData; 
	}

private:
};



template<class _Elem,
	class _Traits>
	class memory_stream
	: public basic_iostream<_Elem, _Traits>
	{	// control insertions and extractions from a stream buffer
public:
	typedef typename _Traits::int_type int_type;
	typedef typename _Traits::pos_type pos_type;
	typedef typename _Traits::off_type off_type;

	memory_stream()
		: basic_iostream<_Elem, _Traits>(&_Strbuf)
		{	// construct from stream buffer pointer
		}

	virtual ~memory_stream()
		{	// destroy the object
		}

	memory_streambuf<_Elem, _Traits> *getbuf()
	{
		return &_Strbuf;
	};
	

	memory_streambuf<_Elem, _Traits> _Strbuf;
	};

typedef memory_stream<char, char_traits<char> > memorystream;


};

#endif
