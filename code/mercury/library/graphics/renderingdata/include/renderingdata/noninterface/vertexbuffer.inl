// VertexFormat
inline VertexFormat::VertexFormat() : m_dataSize(0) 
{
}

inline S32 VertexFormat::GetNumVertexElements() const
{
	return m_elements.Size();
}

inline const VertexFormat::VertexElement& VertexFormat::GetVertexElement(S32 i) const
{
	return m_elements[i];
}

inline S32 VertexFormat::GetOffset(S32 element) const
{
	return m_offsets[element];
}

inline U32 VertexFormat::GetDataSize() const
{
	return m_dataSize;
}

// VertexDeclaration
inline S32 VertexDeclaration::GetNumVertexElements() const
{
	return m_elements.Size();
}

inline S32 VertexDeclaration::AddVertexElement()
{
	m_elements.PushBack();
	return m_elements.Size() - 1;
}

inline const VertexDeclaration::VertexElement& VertexDeclaration::GetVertexElement(S32 i) const
{
	return m_elements[i];
}

inline VertexDeclaration::VertexElement& VertexDeclaration::GetVertexElement(S32 i)
{
	return m_elements[i];
}

// VertexData
inline VertexData::VertexData() : m_nElements(0), m_capacity(0), m_pData(0) 
{
}

inline VertexData::~VertexData()
{
	delete [] m_pData;
}

inline void VertexData::Create(const VertexFormat& format)
{
	assert(m_nElements == 0);
	assert(m_capacity == 0);
	assert(m_pData == 0);

	m_vertexFormat = format;
}

inline S32 VertexData::Size() const
{
	return m_nElements;
}

inline S32 VertexData::Capacity() const
{
	return m_capacity;
}

inline void VertexData::Resize(S32 n)
{
	assert(n >= 0);

	if (m_nElements < n) 
		Reserve(n);

	m_nElements = n;
}

// ConstVertexReference
inline VertexData::ConstVertexReference::ConstVertexReference(const VertexData& vb, S32 index) : m_vb((VertexData&)vb), m_index(index)
{
}

inline const U8* VertexData::ConstVertexReference::operator & () const
{
	return ((U8*) m_vb.m_pData) + m_vb.m_vertexFormat.GetDataSize() * m_index;
}

template<typename T> inline const T& VertexData::ConstVertexReference::GetData(S32 element) const
{
	U8* p = ((U8*)m_vb.m_pData) + m_vb.m_vertexFormat.GetDataSize() * m_index + m_vb.m_vertexFormat.GetOffset(element);
	return *(const T*) p;
}

inline const VertexFormat& VertexData::ConstVertexReference::GetVertexFormat() const
{
	return m_vb.GetVertexFormat();
}

// VertexReference
inline VertexData::VertexReference::VertexReference(VertexData& vb, S32 index) : ConstVertexReference(vb, index)
{
}

inline void VertexData::VertexReference::SetData(void* p, U32 dataSize)
{
	assert(dataSize == m_vb.m_vertexFormat.GetDataSize());
	memcpy((void*)&GetData<U8>(0), p, Maths::Min(dataSize, m_vb.m_vertexFormat.GetDataSize()));
}

template<typename T> inline void VertexData::VertexReference::SetData(S32 element, const T& t)
{
	SetData(element, &t, sizeof(t));
}

inline VertexData::VertexReference& VertexData::VertexReference::operator = (const ConstVertexReference& v)
{
	assert(m_vb.m_vertexFormat == v.GetVertexFormat());
	memcpy((void*) &GetData<U8>(0), &v.GetData<U8>(0), m_vb.m_vertexFormat.GetDataSize());
	return *this;
}

inline VertexData::VertexReference& VertexData::VertexReference::operator = (const VertexReference& v)
{
	assert(m_vb.m_vertexFormat == v.GetVertexFormat());
	memcpy((void*) &GetData<U8>(0), &v.GetData<U8>(0), m_vb.m_vertexFormat.GetDataSize());
	return *this;
}

inline VertexData::ConstVertexReference VertexData::operator [] (S32 i) const
{
	assert(i >= 0 && i < m_nElements);
	return ConstVertexReference(*this, i);
}

inline VertexData::VertexReference VertexData::operator [] (S32 i)
{
	assert(i >= 0 && i < m_nElements);
	return VertexReference(*this, i);
}

inline const VertexFormat& VertexData::GetVertexFormat() const
{
	return m_vertexFormat;
}

inline const U8* VertexData::GetData() const
{
	return (U8*) m_pData;
}

inline void VertexData::SetData(void* p, U32 size)
{
	assert(size == m_nElements * m_vertexFormat.GetDataSize());
	memcpy(m_pData, p, Maths::Min(size, m_nElements * m_vertexFormat.GetDataSize()));
}

inline void Swap(VertexData& a, VertexData& b)
{
	Btl::SwapByMemcopy(a, b);
}




