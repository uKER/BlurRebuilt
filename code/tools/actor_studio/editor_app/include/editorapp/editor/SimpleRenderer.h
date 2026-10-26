#ifndef EDITOR_ACTOR_RENDERER
#define EDITOR_ACTOR_RENDERER

#ifdef _MSC_VER
#pragma once
#endif

#include <common/ResourcePtr.h>

namespace Editor
{
	class C_ActorScene;

	//TODO: are 2D textures sufficient?
	typedef AftkRender::C_ResourcePtr<Display::C_RefTexture2D> TextureResourcePtr;

	class C_SimpleRenderer : public I_EditorRenderer
	{
	public:
		struct RenderFlags
		{
			U32 fillmode	: 3;
			U32 cullmode	: 3;
			bool textured	: 1;
			bool lit		: 1;
		};

	public:
		C_SimpleRenderer();
		~C_SimpleRenderer();

		virtual void Clear();
		virtual void Update(AftkRender::E_RendererPass pass = AftkRender::E_RendererPass_Normal);

		virtual void GetVisible(
			const RECT& region, 
			C_DgNodeSelection& selection);
		
		virtual void Render();

		void SetObjTypePrimitive(
			const char* pTypeName, 
			PrimitiveResourcePtr pPrimitive,
			Maths::RGBA colour,
			const Ut::Vector<Btl::String> *pTexturePaths=0);
		
		void RemoveObjTypePrimitive(
			const char* pTypeName);

		const Maths::C_AxisBox* GetPrimitiveBounds(const char* pTypeName) const;
		bool PrimitiveExists(const char* pTypeName) const;

		bool AddPrimitive(
			const char* pTypeName, 
			const Maths::HMat44* pMatrix, 
			const Maths::C_AxisBox& bounds,
			const EditorLib::C_NodeId& id, U32 component, 
			const Maths::RGBA *pColourOverride = 0);

		const RenderFlags& GetRenderFlags() const;
		void SetRenderFlags(const RenderFlags& flags);

		void LoadTextures();

	protected:
		bool OnCreate();
		void OnDestroy();
		bool OnCreateVolatile();
		void OnDestroyVolatile();

		void PopulateVertexBuffer();
		void PopulateIndexBuffer();
		void PopulateInstanceBuffer();

		struct Primitive
		{
			Btl::String				typeName;
			PrimitiveResourcePtr	meshData;
			Maths::RGBA				defaultColour;

			U32						vertexOffset;
			U32						indexOffset;
			Maths::C_AxisBox		bounds;
			Ut::Vector<TextureResourcePtr> textures;
		};

		struct PrimitiveInstance
		{
			S32						basePrimitive;
			const Maths::HMat44*	pXform;
			Maths::RGBA				colour;

			//will probably sort this out at some point... guerilla'd in for selection
			EditorLib::C_NodeId nodeId;
			U32 componentId; //0xffffffff => scene actor locator node
		};

		const Primitive* FindPrimitive(const char* pTypeName) const;

		Ut::UtVector< Ut::DynamicVector<Primitive> >			m_PrimitiveData;
		Ut::UtVector< Ut::DynamicVector<PrimitiveInstance> >	m_PrimitiveInstances;
		Ut::UtVector< Ut::DynamicVector<TextureResourcePtr> >	m_Textures;


		Display::C_VertexBuffer		*m_pVertexBuffer;
		Display::C_VertexBuffer		*m_pInstanceBuffer;
		Display::C_IndexBuffer		*m_pIndexBuffer;

		//size of the dummy index data required...
		//required by non-indexed primitives for gpu instancing
		U32			m_NonIndexedVertexCount;
		U32			m_CurrentNumInstances;

		RenderFlags	m_RenderFlags;

		struct Flags
		{
			U32 vertexbufferdirty	: 1;
			U32 instancebufferdirty : 1;
			U32 indexbufferdirty	: 1;
		};
		mutable Flags m_Flags;

		static bool Compare(const PrimitiveInstance& l, const PrimitiveInstance& r);
	};
}

#endif
