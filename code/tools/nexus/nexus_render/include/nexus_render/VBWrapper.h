#ifndef NEXUS_RENDER_VBWRAPPER_H
#define NEXUS_RENDER_VBWRAPPER_H

namespace NexusRender
{
	class VBWrapper
	{
	public:
		VBWrapper();

		~VBWrapper();

		Tu::Status Create(
			U32 vertexSize,
			U32 nVerts,
			U32 nVertsPerPrim);

		void Destroy();

		void SetVertex(U32 index, void* pVertex);

		void* GetVertexPtrByVertexIndex(U32 index);

		void* GetVertexPtrByPrimitiveIndex(U32 index);

		U32 GetNumVertices() const;

		U32 GetVertexSize() const;

		void Lock();

		void Unlock();

		void Draw(D3DPRIMITIVETYPE primitiveType);

	protected:
		U32 m_vertexSize;
		U32 m_nVerts;

	private:
		void LockVertexBuffer(U32 index, void*& pVertexBufferContents);

		void UnlockVertexBuffer(U32 index);

		Ut::UtVector<Ut::DynamicVector<VertexBuffer> > m_vertexBuffers;
		U32 m_maxVBSize;
		U32 m_nVertsPerPrim;
		bool m_locked;
	};
}

#endif
