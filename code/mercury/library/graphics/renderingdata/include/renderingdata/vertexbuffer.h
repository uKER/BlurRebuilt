#ifndef RD_VERTEX_BUFFER_H
#define RD_VERTEX_BUFFER_H
#ifdef _MSC_VER
#pragma once
#endif

#include "resourceindex.h"
#include "datatypes.h"

namespace RenderingData
{
	class VertexData;

	// Tesselation.
	enum VertexElementMethod
	{
		VERTEX_ELEMENT_METHOD_DEFAULT
	};

	// Does not always make any difference - PS3 ignores vertex usage.
	// May be used for vertex processing during export, or tesselation while rendering.
	enum VertexElementUsage
	{
		VERTEX_ELEMENT_USAGE_POSITION,
		VERTEX_ELEMENT_USAGE_BLENDWEIGHT,
		VERTEX_ELEMENT_USAGE_BLENDINDICES,
		VERTEX_ELEMENT_USAGE_NORMAL,
		VERTEX_ELEMENT_USAGE_PSIZE,
		VERTEX_ELEMENT_USAGE_TEXCOORD,
		VERTEX_ELEMENT_USAGE_TANGENT,
		VERTEX_ELEMENT_USAGE_BINORMAL,
		VERTEX_ELEMENT_USAGE_TESSFACTOR,
		VERTEX_ELEMENT_USAGE_COLOR,
		VERTEX_ELEMENT_USAGE_SAMPLE,

		VERTEX_ELEMENT_USAGE_MAX
	};

	// Format for a single vertex stream.
	class VertexFormat
	{
	public:
		VertexFormat();

		struct VertexElement
		{
			VertexElementType type;
			VertexElementMethod method;
			VertexElementUsage usage;
			S32 usageIndex;

			S32 GetNumF32s() const;
			S32 GetF32Offset(S32 floatIndex) const;
		};

		S32 GetNumVertexElements() const;
		const VertexElement& GetVertexElement(S32 i) const;
		S32 AddVertexElement(const VertexElement& e);

		U32 GetDataSize() const;
		S32 GetOffset(S32 element) const;

		friend class VertexData;
		FRIEND_SERIALISE(VertexFormat)

	private:
		S32 m_dataSize;
		Btl::Vector<VertexElement> m_elements;
		Btl::Vector<U16> m_offsets;
	};

	bool operator == (const VertexFormat& v1, const VertexFormat& v2);
	bool operator != (const VertexFormat& v1, const VertexFormat& v2);
	bool operator < (const VertexFormat& v1, const VertexFormat& v2);
	
	// Format for multiple vertex streams.
	class VertexDeclaration
	{
	public:
		struct VertexElement
		{
			S16 stream;
			S16 offset;
			VertexElementType type;
			VertexElementMethod method;
			VertexElementUsage usage;
			S8 usageIndex;
		};

		S32 GetNumVertexElements() const;
		S32 AddVertexElement();
		const VertexElement& GetVertexElement(S32 i) const;
		VertexElement& GetVertexElement(S32 i);

		FRIEND_SERIALISE(VertexDeclaration)

	private:
		Btl::Vector<VertexElement> m_elements;
	};

	bool operator == (const VertexDeclaration& v1, const VertexDeclaration& v2);
	bool operator != (const VertexDeclaration& v1, const VertexDeclaration& v2);
	bool operator < (const VertexDeclaration& v1, const VertexDeclaration& v2);
	
	// Holds the data for a vertex buffer.
	class VertexData
	{
	public:
		VertexData();
		~VertexData();

		VertexData(const VertexData& vertexData);
		VertexData& operator = (const VertexData& vertexData);

		void Create(const VertexFormat& format);

		S32 Size() const;
		S32 Capacity() const;

		class ConstVertexReference
		{
		public:
			ConstVertexReference(const VertexData& vb, S32 index);

			const U8* operator & () const;
			const VertexFormat& GetVertexFormat() const;

			// T should be the appropriate type in DataTypes.
			template<typename T> const T& GetData(S32 element) const;

		protected:
			VertexData& m_vb;
			S32 m_index;
		};

		class VertexReference : public ConstVertexReference
		{
		public:
			VertexReference(VertexData& vb, S32 index);

			void SetData(void* p, U32 dataSize);

			// T should be the appropriate type in DataTypes.
			template<typename T> void SetData(S32 element, const T& t);

			VertexReference& operator = (const VertexReference& v);
			VertexReference& operator = (const ConstVertexReference& v);

		private:
			void SetData(S32 element, void* p, U32 dataSize);
		};

		ConstVertexReference operator [] (S32 i) const;
		VertexReference operator [] (S32 i);

		const VertexFormat& GetVertexFormat() const;

		const U8* GetData() const;
		void SetData(void* p, U32 size);

		void Resize(S32 n);
		void Reserve(S32 n);
		S32 PushBack();
		S32 PushBack(const ConstVertexReference& vertex);

		void ChangeVertexElementType(VertexElementType originalType, VertexElementType finalType);
		void ReverseU8_4();

		friend class ConstVertexReference;
		friend class VertexReference;

	private:
		VertexFormat m_vertexFormat;
		S32 m_nElements;
		S32 m_capacity;
		U32* m_pData;
	};

	S32 Compare(const VertexData::ConstVertexReference& p1, const VertexData::ConstVertexReference& p2);

	void Swap(VertexData& a, VertexData& b);

	#include "noninterface/vertexbuffer.inl"
}

#endif
