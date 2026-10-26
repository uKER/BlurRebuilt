#ifndef EDITOR_COMPLEX_ACTOR_RENDERER
#define EDITOR_COMPLEX_ACTOR_RENDERER

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_ActorScene;

	class ComplexObject;

	class I_ComplexObjectCallback
	{
	public:
		virtual void ComplexRendererCallback(ComplexObject* object) = 0;
	};

	//TODO: support multiple primitive types (tri-strip/list, line-strip/list, points...)
	//TODO: support renderable components (temporarily removed out by OT due to point list assumption)
	class ComplexObject
	{
	private:
		Display::C_VertexBuffer* m_VertexBuffer;
		int m_VertexBufferSize;
		Display::E_PrimitiveType m_PrimType;

	public:
		I_ComplexObjectCallback* m_Callback;

		ComplexObject(Display::C_VertexBuffer* vertexBuffer, int vertexBufferSize, Display::E_PrimitiveType primType, I_ComplexObjectCallback* callback)
		{
			m_VertexBuffer = vertexBuffer;
			m_VertexBufferSize = vertexBufferSize;
			m_Callback = callback;
			m_PrimType = primType;
		}

		Display::E_PrimitiveType GetPrimitiveType()
		{
			return m_PrimType;
		}

		void SetPrimitiveType(Display::E_PrimitiveType primType)
		{
			m_PrimType = primType;
		}

		void DestroyVertexBuffer()
		{
			SAFE_DELETE_PTR(m_VertexBuffer);
		}

		Display::C_VertexBuffer* GetVertexBuffer()
		{
			return m_VertexBuffer;
		}

		void SetVertexBuffer(Display::C_VertexBuffer* vertexBuffer)
		{
			m_VertexBuffer = vertexBuffer;
		}

		int GetVertexBufferSize() const
		{
			return m_VertexBufferSize;
		}

		void SetVertexBufferSize(int vertexBufferSize)
		{
			m_VertexBufferSize = vertexBufferSize;
		}
	};

	class ComplexObjectInstance
	{
	public:
		const Maths::HMat44* m_pXform;
		Maths::RGBA	m_Colour;
		ComplexObject* m_pObject;
		EditorLib::C_NodeId m_DgNodeId;
		int m_VertexOffset;
		int m_VertexCount;
	};

	class C_ComplexRenderer : public I_EditorRenderer
	{
	public:
		C_ComplexRenderer();
		~C_ComplexRenderer();

		virtual void Clear();
		virtual void Update(AftkRender::E_RendererPass pass = AftkRender::E_RendererPass_Normal);

		virtual void GetVisible(
			const RECT& region, 
			C_DgNodeSelection& selection);

		virtual void Render();

		int CreateComplexObject(int vertexCount, ComplexObject** complexObject, I_ComplexObjectCallback* callback=0);
		bool ResizeComplexObject(int vertexCount, ComplexObject* complexObject);
		void DeleteComplexObject(ComplexObject** complexObject);

		void DrawInstance(ComplexObject* complexObject,
			int vertexOffset,
			int vertexCount,
			const Maths::HMat44* pMatrix,
			const Maths::C_AxisBox& bounds,
			const EditorLib::C_NodeId& id,
			Maths::RGBA* pColour);

	protected:

		bool OnCreate();
		void OnDestroy();
		bool OnCreateVolatile();
		void OnDestroyVolatile();

		// New stuff
		Ut::UtVector< Ut::DynamicVector<ComplexObjectInstance> > m_ComplexObjectInstances;
		Ut::UtVector< Ut::DynamicVector<ComplexObject*> > m_OwnedComplexObjects;
	};
}

#endif