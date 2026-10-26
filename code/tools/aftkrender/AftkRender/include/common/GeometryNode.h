#ifndef AFTK_RENDER_GEOMETRY_NODE_H
#define AFTK_RENDER_GEOMETRY_NODE_H

#ifdef _MSC_VER
#pragma once
#endif


namespace AftkRender
{
	class C_Renderer;
	class C_GeometryNode;

	struct C_VertexStream
	{
		enum E_VertexStreamUsage
		{
			E_VertexStreamUsage_Position_Normals,
			E_VertexStreamUsage_Position_Normals_Colour,
			E_VertexStreamUsage_UVs,
			E_VertexStreamUsage_Weights_PaletteIndices,
			E_VertexStreamUsage_Tangents_Binormals
		};

		C_VertexStream(const U8 s, E_VertexStreamUsage u)
		: streamIndex(s)
		, usage(u)
		{ }

		Display::C_VertexBuffer		vertexBuffer;
		const U8					streamIndex;
		const E_VertexStreamUsage	usage;
	};
	typedef Btl::Vector<C_VertexStream> VertexStreamArray;

	struct C_PrimitiveBatch
	{
		U32		vertexOffset;
		U32		indexOffset;
		U32		primitiveCount;
		S16		material;
		Display::E_PrimitiveType primitiveType;
	};
	typedef Btl::Vector<C_PrimitiveBatch> PrimitiveBatchArray;


	class C_GeometryCluster
	{
	public:
		C_GeometryCluster(C_GeometryNode* pNode);
		virtual  ~C_GeometryCluster();

		virtual void Clear();
		virtual bool Build()=0;

		virtual void RenderGeometry(U32 batch) const;

		U32 GetNumVertexStreams() const;
		const C_VertexStream& GetVertexStream(U32 i) const;

		U32 GetNumPrimitiveBatches() const;
		const C_PrimitiveBatch& GetPrimitiveBatch(U32 i) const;

		const C_GeometryNode* GetGeometryNode() const;
		const Display::C_VertexDeclaration* GetVertexDeclaration() const;

	protected:
		void DestroyVertexBuffers();
		void DestroyIndexBuffers();

		VertexStreamArray				m_VertexStreams;
		Display::C_IndexBuffer			m_IndexBuffer;

		PrimitiveBatchArray				m_PrimitiveBatches;
		U8								m_NumTexCoordSets;
		Display::C_VertexFormat			m_VertexFormat;
		Display::C_VertexDeclaration	*m_pVertexDeclaration;

		C_GeometryNode*					m_pGeometryNode;
	};
	typedef Btl::Vector<C_GeometryCluster>	GeometryClusterArray;
	typedef Btl::Vector<C_GeometryCluster*>	GeometryClusterPtrArray;


	class C_GeometryNode
	{
	public:
		C_GeometryNode();
		virtual ~C_GeometryNode();

		virtual void Clear();
		virtual bool Build()=0;

		U32 GetNumClusters() const;
		const C_GeometryCluster* GetClusterPtr(U32 index) const;

	protected:
		void DestroyGeometryClusters();
		GeometryClusterPtrArray		m_GeometryClusters;
	};

}
#endif

