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

		void	SetD3DWindowPtr(D3DWindow* pD3DWindow);
		
		void	SetMaterialResourcePtr(const AftkSharedResource< AftkMaterialNode >* pMaterialResource);
		void	SetObjectRenderingBatchPtr(ObjectRenderingBatch* pRenderingBatch);
		U32		AddVertexBufferIndex(U32 index);

		const AftkSharedResource< AftkMaterialNode >*	GetMaterialResourcePtr() const;
		const ObjectRenderingBatch*						GetObjectRenderingBatchPtr() const;
		U32												GetNumVertexBufferIndices() const;
		U32												GetVertexBufferIndex(U32 instance) const;

		void											OnUpdateDevice();

		void	Clear();

	private:
		void	Reset();

		///Store the window that we are attached to.
		D3DWindow*										m_pD3DWindow;

		const AftkSharedResource< AftkMaterialNode >*	m_pMaterialResource;
		ObjectRenderingBatch*							m_pObjectRenderingBatch;
		Ut::UtVector< Ut::DynamicVector<U32> >			m_vertexBufferIndices;
	};
}

#endif
