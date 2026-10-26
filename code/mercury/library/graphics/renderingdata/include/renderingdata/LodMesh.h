#ifndef RD_LOD_MESH_H
#define RD_LOD_MESH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/Vector.h"
#include "btl/containers/map.h"
#include "btl/containers/set.h"
#include "btl/containers/openhashmap.h"
#include <deque>

namespace RenderingData {
namespace Build
{

class LodVertex;
class LodEdge;
class LodTriangle;
class LodMesh;

enum E_VertexFlags
{
    E_VertexFlag_None                  = 0x00000000,
    E_VertexFlag_MaterialBoundary      = 0x00000004,
    E_VertexFlag_ColourDiscontinuity   = 0x00000008,
    E_VertexFlag_TextureDiscontinuity  = 0x00000010,
    E_VertexFlag_NormalDiscontinuity   = 0x00000020,
    E_VertexFlag_TangentDiscontinuity  = 0x00000040,
    E_VertexFlag_BinormalDiscontinuity = 0x00000080,
    E_VertexFlag_Outline               = 0x00000100,
    E_VertexFlag_Reanalyse             = 0x00010000
};

enum E_EdgeFlags
{
    E_EdgeFlag_None                          = 0x00000000,
    E_EdgeFlag_CanCollapseVertex0ToVertex1   = 0x00000001,
    E_EdgeFlag_CanCollapseVertex1ToVertex0   = 0x00000002,
    E_EdgeFlag_MaterialBoundary              = 0x00000004,
    E_EdgeFlag_ColourDiscontinuity           = 0x00000008,
    E_EdgeFlag_TextureDiscontinuity          = 0x00000010,
    E_EdgeFlag_NormalDiscontinuity           = 0x00000020,
    E_EdgeFlag_TangentDiscontinuity          = 0x00000040,
    E_EdgeFlag_BinormalDiscontinuity         = 0x00000080,
    E_EdgeFlag_Outline                       = 0x00000100,
    E_EdgeFlag_RecalculateEdgeCollapseScores = 0x00001000,
    E_EdgeFlag_Reanalyse                     = 0x00010000
};

enum E_TriangleFlags
{
    E_TriangleFlag_None      = 0x00000000,
    E_TriangleFlag_Reanalyse = 0x00010000
};

class LodVertex
{
friend class LodMesh;
public:
    
    NOALIAS __forceinline       LodVertex();
    NOALIAS __forceinline void  Initialise( const S32 index, const Maths::Point3 VECREF position );
    NOALIAS __forceinline void  AddEdgeIndex( const S32 edgeIndex );
    NOALIAS __forceinline void  AddTriangleIndex( const S32 triangleIndex );
	NOALIAS __forceinline Maths::Point3 GetVertex()
	{
		return m_Position;
	}
    
private:
    S32                 m_Index;               ///< Index of this vertex in the LodMesh vertex array
    //Btl::Vector<S32>    m_RootVertexIndices;   ///< Indices of the vertices in the root LodMesh that this vertex originated from
    Btl::Vector<S32>    m_EdgeIndices;         ///< Indices of the edges in the LodMesh that contain this vertex
    Btl::Vector<S32>    m_TriangleIndices;     ///< Indices of the triangles in the LodMesh that contain this vertex
    U32                 m_Flags;               ///< Flags
    Maths::Point3       m_Position;            ///< Position of this vertex
    //Maths::VF32         m_MaxTextureDensity;   ///< Max texture density of adjacent triangles
    //Maths::VF32         m_TextureDensityRatio; ///< Texture density ratio of adjacent triangles
};

class LodEdge
{
friend class LodMesh;
public:
    
    NOALIAS __forceinline       LodEdge();
    NOALIAS __forceinline void  Initialise( const S32 index, const S32 vertexIndex0, const S32 vertexIndex1 );
    NOALIAS __forceinline void  AddTriangleIndex( const S32 triangleIndex );
    NOALIAS __forceinline void  SetEdgeLength( const Maths::VF32 VECREF edgeLength );
    NOALIAS __forceinline S32   OtherVertexIndex( const S32 vertexIndex ) const;
    
private:
    S32                 m_Index;                                ///< Index of this edge in the LodMesh edge array
    S32                 m_VertexIndex[2];                       ///< Indices of the vertices contained in this edge
    Btl::Vector<S32>    m_TriangleIndices;                      ///< Indices of the triangles in the LodMesh that contain this edge
    F32                 m_Length;                               ///< Distance between the vertices at each end of this edge
    Maths::Normal3      m_Normal;                               ///< Normal in direction from vertex 0 to vertex 1
    F32                 m_Sharpness;                            ///< Measure of how 'sharp' the edge is in relation to neighbouring faces (i.e. angle between faces)
    F32                 m_EdgeCollapseScoreVertex0ToVertex1;    ///< Score for collapsing vertex0 to vertex1
    F32                 m_EdgeCollapseScoreVertex1ToVertex0;    ///< Score for collapsing vertex1 to vertex0
    U32                 m_Flags;                                ///< Flags
};

#if 0
class LodTriangleVertex
{
friend class LodTriangle;
friend class LodMesh;
public:
    
    
private:
    
    static const S32    m_MaxTexCoordCount = 8;
    
    Maths::Vector4      m_Colour;   ///< Vertex colour
    Maths::Vector3      m_Normal;   ///< Vertex normal
    Maths::Vector3      m_Tangent;  ///< Vertex tangent
    Maths::Vector3      m_BiNormal; ///< Vertex binormal
    Maths::Vec4F32      m_TexCoord[ m_MaxTexCoordCount ]; ///< Vertex texture coords (up to 8 sets)
};
#endif

class LodTriangle
{
friend class LodMesh;
friend class LodMeshOptions;
friend class LodTriangleVertexGradients;
public:
    
    NOALIAS __forceinline       LodTriangle();
    NOALIAS __forceinline void  Initialise( const S32         index,
                                            void *            pPrimitive,
                                            const U32         vbSize,
                                            const U32         vbIndex0,
                                            const U32         vbIndex1,
                                            const U32         vbIndex2 );
    
    NOALIAS __forceinline void* Primitive() const;
    NOALIAS __forceinline S32   VBIndex( const S32 i ) const;
    NOALIAS __forceinline S32   VertexIndex( const S32 i ) const;
    
    NOALIAS __forceinline void  SetVertexIndices( const S32 vertexIndex0, const S32 vertexIndex1, const S32 vertexIndex2 );
    NOALIAS __forceinline void  SetEdgeIndices( const S32 edgeIndex0, const S32 edgeIndex1, const S32 edgeIndex2 );
    NOALIAS __forceinline void  SetNormals( const Maths::Vector3 VECREF normal0, const Maths::Vector3 VECREF normal1, const Maths::Vector3 VECREF normal2 );
	NOALIAS __forceinline void  SetTangents( const Maths::Vector3 VECREF tangent0, const Maths::Vector3 VECREF tangent1, const Maths::Vector3 VECREF tangent2 );
	NOALIAS __forceinline void  SetBiNormals( const Maths::Vector3 VECREF biNormal0, const Maths::Vector3 VECREF biNormal1, const Maths::Vector3 VECREF biNormal2 );
    NOALIAS __forceinline void  SetColours( const Maths::Vector4 VECREF colour0, const Maths::Vector4 VECREF colour1, const Maths::Vector4 VECREF colour2 );
    NOALIAS __forceinline void  AddTexCoords( const Maths::Vec4F32 VECREF texCoord0, const Maths::Vec4F32 VECREF texCoord1, const Maths::Vec4F32 VECREF texCoord2 );

    static const S32    m_MaxTexCoordCount = 4;

private:
    
    S32                 m_Index;            ///< Index of this triangle in the LodMesh triangle array
    void *              m_pPrimitive;       ///< Pointer to the primitive that contains this triangle
    //U32                 m_VBSize;           ///< Size of the primitive vertex buffers (only for error checking)
    U16                 m_VBIndex[3];       ///< Primitive vertex buffer indices
	S16                 m_TexCoordCount;    ///< Number of texture coords
    S32                 m_VertexIndex[3];   ///< Indices of the vertices contained in this triangle
    S32                 m_EdgeIndex[3];     ///< Indices of the edges contained in this triangle
    Maths::Normal3      m_SurfaceNormal;    ///< Triangle surface normal
    //F32					m_SurfaceArea;      ///< Triangle surface area
    //F32					m_TextureDensity;   ///< Triangle texture density
    
    //LodTriVertex        m_Vertex[3];        ///< Triangle vertex data
    
	Maths::StorageVec4F32_U8N	m_Colour[3];        ///< Vertex colour
	Maths::StorageVec4F32_S16N	m_Normal[3];        ///< Vertex normal
    Maths::StorageVec4F32_S16N	m_Tangent[3];       ///< Vertex tangent
    Maths::StorageVec4F32_S16N	m_BiNormal[3];      ///< Vertex binormal
	Maths::StorageVec4F32_F16	m_TexCoord[m_MaxTexCoordCount][3]; ///< Vertex texture coords (up to 8 sets)
};

template < typename T_Data >
class VectorIterator
{
public:
    
	VectorIterator( std::deque< T_Data >& vector, const U32 index = 0 )
        : m_Vector( vector )
        , m_Index( index )
        , m_Modified( false )
    {
    }
    
    T_Data* operator -> ()  { return &m_Vector[ m_Index ]; }
	T_Data& operator * ()	{ return m_Vector[ m_Index ]; }
    operator bool () const  { return m_Index < (U32)m_Vector.size(); }
    bool Valid() const      { return m_Index < (U32)m_Vector.size(); }
    bool Modified() const   { return m_Modified; }
    U32 Index() const       { return m_Index; }
    
    S32 operator = ( const S32 index ) { m_Index = index; return index; }
    U32 operator = ( const U32 index ) { m_Index = index; return index; }
    
    VectorIterator< T_Data >& operator ++ () { ++m_Index; m_Modified = false; return *this; }
    VectorIterator< T_Data >& operator -- () { --m_Index; m_Modified = false; return *this; }
    
    void operator ++ ( int ) { m_Index++; m_Modified = false; }
    void operator -- ( int ) { m_Index--; m_Modified = false; }
    
    void Swapped( const S32 index1, const S32 index2 )
    {
        if ( m_Index == index1 )
        {
            m_Index = index2;
            m_Modified = true;
            #if defined( _DEBUG )
            //LOGDEBUG( g_Log, "Swapped the vector element that this iterator is referencing!" );
            #endif
        }
        else if ( m_Index == index2 )
        {
            m_Index = index1;
            m_Modified = true;
            #if defined( _DEBUG )
            //LOGDEBUG( g_Log, "Swapped the vector element that this iterator is referencing!" );
            #endif
        }
    }
    
    void Deleted( const S32 index )
    {
        if ( m_Index == index )
        {
            m_Index = 0x80000000;
            m_Modified = true;
            #if defined( _DEBUG )
            //LOGDEBUG( g_Log, "Deleted the vector element that this iterator is referencing!" );
            #endif
        }
    }
    
private:
    
    std::deque< T_Data >&  m_Vector;   ///< Vector that this iterator belongs to
    U32                     m_Index;    ///< Index of the current element in the vector
    bool                    m_Modified; ///< Set to true if the vector has any elements swapped or deleted
};

template < typename T_Data >
class VectorIteratorStack
{
friend class LodMesh;
public:
    
    VectorIteratorStack( std::deque< T_Data >& vector )
        : m_Vector( vector )
    {
    }
    
    Btl::Vector< T_Data >& Vector()
    {
        return m_Vector;
    }
    
    U32 PushBack( const U32 vectorIndex = 0 )
    {
        const U32 stackIndex( m_IteratorStack.size() );
        m_IteratorStack.push_back( VectorIterator< T_Data >( m_Vector, vectorIndex ) );
        //return m_IteratorStack.Back();
        return stackIndex;
    }
    
    void PopBack()
    {
        m_IteratorStack.pop_back();
    }
    
    void Swapped( const S32 index1, const S32 index2 )
    {
        for ( S32 i=0; i<(S32)m_IteratorStack.size(); i++ )
        {
            m_IteratorStack[i].Swapped( index1, index2 );
        }
    }
    
    void Deleted( const S32 index )
    {
        for ( S32 i=0; i<(S32)m_IteratorStack.size(); i++ )
        {
            m_IteratorStack[i].Deleted( index );
        }
    }
    
    VectorIterator< T_Data >& operator [] ( const U32 index )
    {
        return m_IteratorStack[ index ];
    }
    
private:
    
    std::deque< T_Data >&                  m_Vector;
    std::deque< VectorIterator< T_Data > > m_IteratorStack;
};

template < typename T_Data >
class VectorIteratorRef
{
public:
    
    VectorIteratorRef( VectorIteratorStack< T_Data >& stack, const U32 vectorIndex = 0 )
        : m_IteratorStack( stack )
        , m_Index( stack.PushBack( vectorIndex ) )
    {
    }
    
    ~VectorIteratorRef()
    {
        m_IteratorStack.PopBack();
    }
    
    T_Data* operator -> ()  { return m_IteratorStack[ m_Index ].operator -> (); }
	T_Data& operator * ()	{ return m_IteratorStack[ m_Index ].operator * (); }
    operator bool () const  { return m_IteratorStack[ m_Index ].operator bool (); }
    bool Valid() const      { return m_IteratorStack[ m_Index ].Valid(); }
    bool Modified() const   { return m_IteratorStack[ m_Index ].Modified(); }
    U32 Index() const       { return m_IteratorStack[ m_Index ].Index(); }
    
    S32 operator = ( const S32 index ) { m_IteratorStack[ m_Index ] = index; return index; }
    U32 operator = ( const U32 index ) { m_IteratorStack[ m_Index ] = index; return index; }
    
    VectorIterator< T_Data >& operator ++ () { return ++m_IteratorStack[ m_Index ]; }
    VectorIterator< T_Data >& operator -- () { return --m_IteratorStack[ m_Index ]; }
    
    void operator ++ ( int ) { m_IteratorStack[ m_Index ]++; }
    void operator -- ( int ) { m_IteratorStack[ m_Index ]--; }
    
private:
    
    VectorIteratorStack< T_Data >&  m_IteratorStack;
    U32                             m_Index;
};

class LodMeshOptions
{
public:
    
    LodMeshOptions()
        : weldThreshold( 0.0001f )
        , materialBoundaryPenalty( 1.0f )
        , colourDiscontinuityThreshold( 0.01f ) // distance test (4d)
        , colourDiscontinuityPenalty( 1.0f )
        , textureDiscontinuityThreshold( 0.01f ) // distance test (3d)
        , textureDiscontinuityPenalty( 1.0f )
        , normalDiscontinuityThreshold( 0.995f ) // dot product test: cosine of 5.732 degrees
        , normalDiscontinuityPenalty( 1.0f )
        , tangentDiscontinuityThreshold( 0.995f ) // dot product test: cosine of 5.732 degrees
        , tangentDiscontinuityPenalty( 1.0f )
        , binormalDiscontinuityThreshold( 0.995f ) // dot product test: cosine of 5.732 degrees
        , binormalDiscontinuityPenalty( 1.0f )
		, applyGradientPenalties(false)
		, applyTriangleAreaPenalty(true)
		, scalePenaltyByEdgeLength(true)
        
        , outlineEdgePenalty( 1000.0f )
        , sharpnessPenalty( 1.0f )
        
        , reduceVertices( true )   // Set to true to reduce number of vertices  <<< one of these options must be set to true
        , vertexReductionRatio( 0.5f )
        , minimumVertices( 8 )
        
        , reduceTriangles( false ) // set to true to reduce number of triangles <<< and the rest set to false!
        , triangleReductionRatio( 0.5f )
        , minimumTriangles( 8 )

		, minEdgeScore(0.f)
        
        , preserveConvexHullIntegrity( false )
        , convexThreshold( 0.1f )
        
        , useViewDirection( false )
        , viewDirectionPenalty( 1.0f )
        , viewDirection( Maths::Const<Maths::Normal3>::AxisZ() )
        
        , colourPriority( 1.0f / 255.0f )
        , normalPriority( 1.0f )
        , tangentPriority( 1.0f )
        , binormalPriority( 1.0f )
    {
        texCoordPriority[0] = 1.0f;
        for ( S32 i=1; i<LodTriangle::m_MaxTexCoordCount; i++ )
        {
            texCoordPriority[i] = texCoordPriority[i-1] * 0.5f;
        }
    }
    
    F32             weldThreshold;
    F32             materialBoundaryPenalty;
    F32             colourDiscontinuityThreshold;
    F32             colourDiscontinuityPenalty;
    F32             textureDiscontinuityThreshold;
    F32             textureDiscontinuityPenalty;
    F32             normalDiscontinuityThreshold;
    F32             normalDiscontinuityPenalty;
    F32             tangentDiscontinuityThreshold;
    F32             tangentDiscontinuityPenalty;
    F32             binormalDiscontinuityThreshold;
    F32             binormalDiscontinuityPenalty;
	bool			applyGradientPenalties;
	bool			applyTriangleAreaPenalty;
	bool			scalePenaltyByEdgeLength;
    
    // Outline edge penalty
    F32             outlineEdgePenalty;
    
    // Edge sharpness penalty
    F32             sharpnessPenalty;
    
    // Vertex reduction options
    bool            reduceVertices;
    S32             minimumVertices;
    F32             vertexReductionRatio;
    
    // Triangle reduction options
    bool            reduceTriangles;
    S32             minimumTriangles;
    F32             triangleReductionRatio;

	F32				minEdgeScore;
    
    // Convex hull options
    bool            preserveConvexHullIntegrity;
    F32             convexThreshold;
    
    // View direction dependent options [ NOT IMPLEMENTED YET! ]
    bool            useViewDirection;
    F32             viewDirectionPenalty;
    Maths::Normal3  viewDirection;
    
    // Texture coord distance & normal/tangent/binormal priorities
    F32             texCoordPriority[ LodTriangle::m_MaxTexCoordCount ];
    F32             colourPriority;
    F32             normalPriority;
    F32             tangentPriority;
    F32             binormalPriority;
};

class LodTriangleVertexGradients
{
public:
	Maths::Vector3 colour[4];
	Maths::Vector3 texCoord[ LodTriangle::m_MaxTexCoordCount ][4];
};

class LodMesh
{
public:

    NOALIAS __forceinline                   LodMesh();
    NOALIAS __forceinline const LodMesh&    operator = ( const LodMesh& copy );
    NOALIAS __forceinline LodVertex&        Vertex( const S32 index );
    NOALIAS __forceinline LodEdge&          Edge( const S32 index );
    NOALIAS __forceinline LodTriangle&      Triangle( const S32 index );
    NOALIAS __forceinline const S32         VertexCount() const;
    NOALIAS __forceinline const S32         EdgeCount() const;
    NOALIAS __forceinline const S32         TriangleCount() const;
    NOALIAS __forceinline void              SetOptions( LodMeshOptions& options );
    NOALIAS __forceinline void              SetBoundingBox( const Maths::AxisBox& boundingBox );
    
    S32                 AddVertex( const Maths::Point3 VECREF point );
    S32                 AddEdge( const S32 vertexIndex0, const S32 vertexIndex1 );
    S32                 AddTriangle( void * pPrimitive, const S32 vertexBufferSize, const U32 vbIndex0, const U32 vbIndex1, const U32 vbIndex2 );
    
    void                DeleteVertex( const S32 vertexIndex );
    void                DeleteEdge( const S32 edgeIndex );
    void                DeleteTriangle( const S32 triangleIndex );
    
    void                RemapVertexBufferIndices( const S32 sourceVertexIndex, const S32 targetVertexIndex );
    void                RemapVertex( const S32 sourceVertexIndex, const S32 targetVertexIndex );
    void                RemapEdge( const S32 sourceEdgeIndex, const S32 targetEdgeIndex );
    void                RemapTriangle( const S32 sourceTriangleIndex, const S32 targetTriangleIndex );
    
    void                ReplaceVertexInAdjacentEdges( const S32 sourceVertexIndex, const S32 targetVertexIndex );
    void                ReplaceVertexInAdjacentTriangles( const S32 sourceVertexIndex, const S32 targetVertexIndex );
    void                ReplaceEdgeInAdjacentTriangles( const S32 sourceEdgeIndex, const S32 targetEdgeIndex );
    
    void                DeleteEdgeFromAdjacentVertices( const S32 edgeIndex );
    void                DeleteTriangleFromAdjacentVertices( const S32 triangleIndex );
    void                DeleteTriangleFromAdjacentEdges( const S32 triangleIndex );
    
    void                DeleteDuplicateEdgesAdjacentToVertex( const S32 vertexIndex );
    void                DeleteDegenerateTrianglesAdjacentToEdge( const S32 edgeIndex );
    
    void                AnalyseVertexPhase1( const S32 vertexIndex );
    void                AnalyseVertexPhase2( const S32 vertexIndex );
    void                AnalyseEdge( const S32 edgeIndex );
    void                AnalyseTriangle( const S32 triangleIndex );
    void                AnalyseMesh();
    void                AnalyseMeshAdjacentToVertex( const S32 vertexIndex );
    
    void                Clone( LodMesh* pClone );
    
    Maths::VF32         CalculateEdgeCollapseScore( S32 sourceVertexIndex, S32 targetVertexIndex );
    S32                 SelectEdgeToCollapse( S32* pSourceVertexIndex, S32* pTargetVertexIndex, F32* pScore, F32 minEdgeScore, S32 startIndex );
    void                CollapseEdge( S32 edgeIndex, S32 sourceVertexIndex, S32 targetVertexIndex );
    void                Reduce();
    
    //void                CollapseVertexPair( S32 sourceVertexIndex, S32 targetVertexIndex );
    
    NOALIAS void        ValidateVertex( const S32 vertexIndex ) const;
    NOALIAS void        ValidateEdge( const S32 edgeIndex ) const;
    NOALIAS void        ValidateTriangle( const S32 triangleIndex ) const;
    NOALIAS void        Validate() const;
    
private:
	typedef Btl::Set<S32> TriangleIndexSet;

	void LodMesh::CalculateGradients(const LodTriangle& triangle, int vertex, LodTriangleVertexGradients* gradients);
	S32 FindRemapTriangle(S32 triangleIndex, S32 sourceVertexIndex, S32 targetVertexIndex, TriangleIndexSet& triangleIndexSet);
	void RemapVertexBufferIndices2( const S32 sourceVertexIndex, const S32 targetVertexIndex );
	S32 FindVertexInTriangle(S32 triangleIndex, S32 vertexIndex);
    
    typedef VectorIteratorRef< LodVertex >   VertexIterator;
    typedef VectorIteratorRef< LodEdge >     EdgeIterator;
    typedef VectorIteratorRef< LodTriangle > TriangleIterator;
    
    static const S32 c_VertexIndexCubeSize = 8;
    
    LodMeshOptions                      m_Options;
    LodMesh*                            m_pRoot;
    std::deque< LodVertex >				m_Vertices;
    std::deque< LodEdge >				m_Edges;
	std::deque< LodTriangle >			m_Triangles;
    VectorIteratorStack< LodVertex >    m_VertexIterators;
    VectorIteratorStack< LodEdge >      m_EdgeIterators;
    VectorIteratorStack< LodTriangle >  m_TriangleIterators;
    Maths::AxisBox                      m_BoundingBox;
    Maths::Vector3                      m_BoundingBoxScale;

    Btl::Vector< S32 >                  m_VertexIndexCube[c_VertexIndexCubeSize][c_VertexIndexCubeSize][c_VertexIndexCubeSize];
	typedef Btl::OpenHashMap<Maths::Point3, S32> VertexMap;
	VertexMap							m_vertexMap;
};

} // namespace Build
} // namespace RenderingData


namespace Btl
{
inline U32 Hash(const Maths::Point3& p)
{
	return Btl::HashRaw((U8*)&p, 12);
}
}

#endif // #ifndef RD_LOD_MESH_H
