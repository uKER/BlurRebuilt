#ifndef OBJECT_RENDERING_FRAGMENT_H
#define OBJECT_RENDERING_FRAGMENT_H

namespace SRD
{
	class ObjectRenderingBatch;

	class ObjectRenderingFragment
	{
	public:
		ObjectRenderingFragment();
		ObjectRenderingFragment(const ObjectRenderingFragment& rhs);
		~ObjectRenderingFragment();

		const	ObjectRenderingFragment& operator=(const ObjectRenderingFragment& rhs);

		void	SetD3dWindowPtr(D3dWindow* pD3dWindow);
		
		void	SetMaterialResourcePtr(const Aftk::C_MaterialResource* pMaterialResource);
		void	SetObjectRenderingBatchPtr(ObjectRenderingBatch* pRenderingBatch);
		U32		AddVertexBufferIndex(U32 index);

		const Aftk::C_MaterialResource*	GetMaterialResourcePtr() const;
		const ObjectRenderingBatch*						GetObjectRenderingBatchPtr() const;
		U32												GetNumVertexBufferIndices() const;
		U32												GetVertexBufferIndex(U32 instance) const;

		void											OnUpdateDevice();

		void	Clear();

	private:
		void	Reset();

		///Store the window that we are attached to.
		D3dWindow*										m_pD3dWindow;

		const Aftk::C_MaterialResource*	m_pMaterialResource;
		ObjectRenderingBatch*							m_pObjectRenderingBatch;
		Ut::UtVector< Ut::DynamicVector<U32> >			m_vertexBufferIndices;
	};
}

#endif
