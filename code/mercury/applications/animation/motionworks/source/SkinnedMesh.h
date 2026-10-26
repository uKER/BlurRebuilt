#ifndef SKINNEDMESH_H
#define SKINNEDMESH_H

#include <Display/Shader/PixelShader.h>
#include <btl/Containers/vector.h>
#include <btl/Containers/map.h>
#include <btl/utility/string.h>
#include <core/maths/hvec/new/IncludeGeneral.h>
#include <core/maths/hvec/new/IncludeGeneral.inl>

//#define ENABLE_NORMAL_MAPS
namespace Progress
{
	class I_ProgressBar;
}

namespace Aftk
{
	class C_Scene;
}

namespace Animation
{
	class C_JointTransform;
	class C_Skeleton;
}

namespace AnimationTools
{
	class C_Scene;
}

namespace DbUtil
{
	class I_DebugRender;
}

class C_Material
{
public:
	void Destroy()
	{
		if ( diffuseTexture.IsCreated() )
		{
			Display::Device()->DestroyTexture2D( &diffuseTexture );
		}
		if ( normalMapTexture.IsCreated() )
		{
			Display::Device()->DestroyTexture2D( &normalMapTexture );
		}

	}


	Btl::String				name;
	Display::C_Texture2D	diffuseTexture;
	Display::C_Texture2D	normalMapTexture;
	bool					hasDiffuseTexture;
	bool					hasNormalMapTexture;
	bool					hasAlpha;
};

struct C_TriangleList
{
	C_TriangleList( U32 inFirstIndex, U32 inSize, S32 inMaterialIndex ):
		firstIndex(inFirstIndex), size(inSize), materialIndex(inMaterialIndex)
	{
	}
	
	C_TriangleList() {}
	
	U32		firstIndex;
	U32		size;
	S32		materialIndex;
};

//
// C_SkinnedMesh
//
class C_SkinnedMesh
{
public:
	C_SkinnedMesh();
	~C_SkinnedMesh();

	struct SkinnedVertex
	{
		F32 m_position[3];
		F32 m_normal[3];
		#ifdef ENABLE_NORMAL_MAPS
		F32 m_tangent[3];
		F32 m_binormal[3];
		#endif
		F32 m_texCoord[2];
		
		void SetPosition( const Maths::Point3& pos ) { Maths::Store( pos, (Maths::StoragePoint3*) m_position ); }
		const Maths::Point3 GetPosition() const { return Maths::MakePoint3( m_position[0], m_position[1], m_position[2] ); }
		const Maths::Vector3 GetNormal() const { return Maths::Load( (const Maths::StorageVector3*) m_normal); }
		void SetNormal( const Maths::Vector3& norm ) { Maths::Store( norm, (Maths::StorageVector3*) m_normal ); }
		#ifdef ENABLE_NORMAL_MAPS
		const Maths::Vector3 GetTangent() const { return Maths::Load( (const Maths::StorageVector3*) m_tangent); }
		void SetTangent( const Maths::Vector3& norm ) { Maths::Store( norm, (Maths::StorageVector3*) m_tangent ); }
		const Maths::Vector3 GetBinormal() const { return Maths::Load( (const Maths::StorageVector3*) m_binormal); }
		void SetBinormal( const Maths::Vector3& norm ) { Maths::Store( norm, (Maths::StorageVector3*) m_binormal ); }
		#endif

	};

	struct Vertex
	{
		F32 m_position[3];
		F32 m_normal[3];
		U8 m_indices[4];
		U8 m_weights[4];
	};

	bool Create(
		Aftk::C_Scene& aftkScene,
		const AnimationTools::C_Scene& scene,
		const Animation::C_Skeleton& skeleton,
		Progress::I_ProgressBar* progressBar = NULL
	);
	
	void Destroy();
	
	bool IsValid() const;
	
	
	/// Calculate skinned vertex positions. Must be called before Render().
	void CalculateSkinnedVertices( const Maths::Mat43* jointWorldMatrices );
	
	/// Get number of vertices.
	U32 GetNumVertices() const		{ return m_vertices.Size(); }
	
	/// Get number of triangle indices.
	U32 GetNumTriangleIndices() const	{ return m_numTriangleIndices; }
	
	/// Get triangle indices.
	Display::C_IndexBuffer& GetTriangleIndices()	{ return m_triangleIndices; }
	
	Display::C_VertexBuffer& GetSkinnedVertices() { return m_skinnedVertices[ m_skinnedBufferIndex & 1 ]; }
		
	
private:

	bool CreateVertexBuffers( U32 numVertices );
	void DestroyVertexBuffers();
	
	bool CreateIndexBuffer( U32 numIndices, const U32* indices );
	void DestroyIndexBuffer();

	void DestroyMaterials();
		
	struct VertexWeight
	{
		F32 m_weight;
		U32 m_vertexIndex;
	};
	
	U32 m_numTriangleIndices;	
	U32 m_numVertices;
	Btl::Vector< SkinnedVertex >	m_vertices;
	Btl::Vector< Btl::Vector< VertexWeight > > m_jointVertexWeights;
public:
	enum E_ViewDirection
	{
		E_ViewDirection_NegX	= 0,
		E_ViewDirection_PosX,
		E_ViewDirection_NegY,
		E_ViewDirection_PosY,
		E_ViewDirection_NegZ,
		E_ViewDirection_PosZ,
		E_ViewDirection_Count
	};
	
	class C_SortAlongViewDirection
	{
	public:
		C_SortAlongViewDirection(const Btl::Vector<U32>& triangleIndices, const Btl::Vector<SkinnedVertex>& vertices, const Maths::HVec3& viewDir):
			m_viewDir(viewDir),
			m_triangleIndices(triangleIndices),
			m_vertices(vertices)
		{
		}
		
		bool operator ()(const C_TriangleList& a, const C_TriangleList& b) const
		{
			BIZ_ASSERT(a.size == 1 && b.size == 1);
			
			Maths::Vector3 viewDir = -m_viewDir;
			
			Maths::Point3 a_p1 = m_vertices[m_triangleIndices[a.firstIndex]].GetPosition();
			Maths::Point3 a_p2 = m_vertices[m_triangleIndices[a.firstIndex + 1]].GetPosition();
			Maths::Point3 a_p3 = m_vertices[m_triangleIndices[a.firstIndex + 2]].GetPosition();

			Maths::Point3 b_p1 = m_vertices[m_triangleIndices[b.firstIndex]].GetPosition();
			Maths::Point3 b_p2 = m_vertices[m_triangleIndices[b.firstIndex + 1]].GetPosition();
			Maths::Point3 b_p3 = m_vertices[m_triangleIndices[b.firstIndex + 2]].GetPosition();
			
			#if 1
			// Minimum
			Maths::VF32 a_dist = Maths::MakeVector3(a_p1) | viewDir;
			a_dist = Maths::Min(a_dist, Maths::MakeVector3(a_p2) | viewDir);
			a_dist = Maths::Min(a_dist, Maths::MakeVector3(a_p3) | viewDir);
			
			
			Maths::VF32 b_dist = Maths::MakeVector3(b_p1) | viewDir;
			b_dist = Maths::Min(b_dist, Maths::MakeVector3(b_p2) | viewDir);
			b_dist = Maths::Min(b_dist, Maths::MakeVector3(b_p3) | viewDir);
			#else
			// Average
			Maths::VF32 a_dist = Maths::MakeVector3(((a_p1 + a_p2 + a_p3) / Maths::MakeVF32(3.f))) | viewDir;
			Maths::VF32 b_dist = Maths::MakeVector3(((b_p1 + b_p2 + b_p3) / Maths::MakeVF32(3.f))) | viewDir;
			#endif
			
			return a_dist < b_dist;
		}
			
		
	private:
		Maths::Vector3						m_viewDir;
		const Btl::Vector<U32>&				m_triangleIndices;
		const Btl::Vector<SkinnedVertex>&	m_vertices;
	};
	

	Btl::Vector< C_TriangleList > m_opaqueTriangleLists;
	Btl::Vector< C_TriangleList > m_alphaTriangleLists;
	Btl::Vector< C_TriangleList > m_sortedAlphaTriangleLists[E_ViewDirection_Count];
	Btl::Vector< C_Material > m_materials;
private:	
	Display::C_IndexBuffer m_triangleIndices;
	
	Display::C_VertexBuffer m_skinnedVertices[ 2 ];
	U32 m_skinnedBufferIndex;
		
	const Animation::C_Skeleton*	m_skeleton;
};


//
// C_SkinnedMeshRenderer
//
class C_SkinnedMeshRenderer: public Display::I_Renderer
{
public:
	C_SkinnedMeshRenderer();
	~C_SkinnedMeshRenderer();

	
	void RenderSkinnedMesh(
		C_SkinnedMesh& skinnedMesh,
		const Maths::HMat43& modelWorldMatrix,
		const Maths::HMat43& cameraWorldMatrix,
		bool renderSolid,
		bool renderWireframe
	);
	
	void RenderTriangleLists(
		const Btl::Vector<C_TriangleList>& triangleLists,
		const C_SkinnedMesh& skinnedMesh
	) const;

	
protected:

	//
	// Display::I_Renderer interface
	//
	virtual bool OnCreate();
	virtual bool OnCreateVolatile();
	virtual void OnDestroyVolatile();
	virtual void OnDestroy();

	virtual bool OnInitialise();
	virtual void OnShutdown();
	virtual void OnReset();

private:
	bool CreateShaders();
	void DestroyShaders();
	bool CreateVertexDeclarations();

	enum E_VertexShader
	{
		E_VertexShader_Default	= 0,
		E_VertexShader_Count
	};
	
	enum E_PixelShader
	{
		E_PixelShader_Default = 0,
		E_PixelShader_Flat,
		E_PixelShader_Count
	};
	
	enum E_VertexFormat
	{
		E_VertexFormat_Default	 = 0,
		E_VertexFormat_Skinned,
		E_VertexFormat_Count
	};
	
	const Display::C_VertexDeclaration*	m_vertexDecls[ E_VertexFormat_Count ];
	Display::C_VertexShader m_vertexShaders[ E_VertexShader_Count ];
	Display::C_PixelShader	m_pixelShaders[ E_PixelShader_Count ];
};


#endif
