#ifndef RD_LOD_MESH_INL
#define RD_LOD_MESH_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace RenderingData {
namespace Build
{

//
// -----------------------------------------
// |-- LodVertex Inline Member Functions --|
// -----------------------------------------
//

NOALIAS __forceinline LodVertex::LodVertex()
    : m_Index( -1 )
    , m_Position( Maths::Const<Maths::Point3>::Zero() )
    , m_Flags( E_VertexFlag_None )
{
}

NOALIAS __forceinline void LodVertex::Initialise( const S32 index, const Maths::Point3 VECREF position )
{
    m_Index    = index;
    m_Position = position;
    //m_RootVertexIndices.PushBack( index );
}

NOALIAS __forceinline void LodVertex::AddEdgeIndex( const S32 edgeIndex )
{
    #if defined( _DEBUG )
    for ( S32 i=0; i<m_EdgeIndices.Size(); i++ )
    {
        BIZ_ASSERT( edgeIndex != m_EdgeIndices[i] );
    }
    #endif
    
    m_EdgeIndices.PushBack( edgeIndex );
}

NOALIAS __forceinline void LodVertex::AddTriangleIndex( const S32 triangleIndex )
{
    #if defined( _DEBUG )
    for ( S32 i=0; i<m_TriangleIndices.Size(); i++ )
    {
        BIZ_ASSERT( triangleIndex != m_TriangleIndices[i] );
    }
    #endif
    
    m_TriangleIndices.PushBack( triangleIndex );
}


//
// ---------------------------------------
// |-- LodEdge Inline Member Functions --|
// ---------------------------------------
//

NOALIAS __forceinline LodEdge::LodEdge()
    : m_Index( -1 )
    , m_Length( 0.0f )
    , m_Sharpness( 0.0f )
    , m_EdgeCollapseScoreVertex0ToVertex1( -1.0f )
    , m_EdgeCollapseScoreVertex1ToVertex0( -1.0f )
    , m_Flags( E_EdgeFlag_None )
{
    m_VertexIndex[0] = -1;
    m_VertexIndex[1] = -1;
}

NOALIAS __forceinline void LodEdge::Initialise( const S32 index, const S32 vertexIndex0, const S32 vertexIndex1 )
{
    using namespace Maths;
    m_Index = index;
    m_VertexIndex[0] = vertexIndex0;
    m_VertexIndex[1] = vertexIndex1;
}

NOALIAS __forceinline void LodEdge::AddTriangleIndex( const S32 triangleIndex )
{
    //#if defined( _DEBUG )
    //for ( S32 i=0; i<m_TriangleIndices.Size(); i++ )
    //{
    //    BIZ_ASSERT( triangleIndex != m_TriangleIndices[i] );
    //}
    //#endif
    
    m_TriangleIndices.PushBack( triangleIndex );
}

NOALIAS __forceinline void LodEdge::SetEdgeLength( const Maths::VF32 VECREF edgeLength )
{
    Store( edgeLength, &m_Length );
}

NOALIAS __forceinline S32 LodEdge::OtherVertexIndex( const S32 vertexIndex ) const
{
    BIZ_ASSERT( vertexIndex == m_VertexIndex[0] ||
                vertexIndex == m_VertexIndex[1] );
    
    if ( vertexIndex == m_VertexIndex[0] )
        return m_VertexIndex[1];
    else
        return m_VertexIndex[0];
}


//
// -------------------------------------------
// |-- LodTriangle Inline Member Functions --|
// -------------------------------------------
//

NOALIAS __forceinline LodTriangle::LodTriangle()
    : m_Index( -1 )
    , m_pPrimitive( NULL )
    //, m_VBSize( 0 )
    , m_TexCoordCount( 0 )
    , m_SurfaceNormal( Maths::Const< Maths::Normal3 >::Zero() )
    //, m_SurfaceArea( Maths::Const< Maths::VF32 >::Zero() )
    //, m_TextureDensity( Maths::Const< Maths::VF32 >::Zero() )
{
    for ( S32 i=0; i<3; i++ )
    {
        m_VBIndex[i] = 0;
        m_VertexIndex[i] = -1;
        m_EdgeIndex[i] = -1;
		Store(Maths::Const< Maths::Vec4F32 >::Zero(), &m_Colour[i]);
        Store(Maths::Const< Maths::Vec4F32 >::Zero(), &m_Normal[i]);
        Store(Maths::Const< Maths::Vec4F32 >::Zero(), &m_Tangent[i]);
        Store(Maths::Const< Maths::Vec4F32 >::Zero(), &m_BiNormal[i]);
    }
    for ( S32 i=0; i<m_MaxTexCoordCount; i++ )
    {
        for ( S32 j=0; j<3; j++ )
        {
             Store(Maths::Const< Maths::Vec4F32 >::Zero(), &m_TexCoord[i][j]);
        }
    }
}

NOALIAS __forceinline void LodTriangle::Initialise(
    const S32         index,
    void *            pPrimitive,
    const U32         vbSize,
    const U32         vbIndex0,
    const U32         vbIndex1,
    const U32         vbIndex2 )
{
    m_Index = index;
    m_pPrimitive = pPrimitive;
    //m_VBSize = vbSize;
    m_VBIndex[0] = vbIndex0;
    m_VBIndex[1] = vbIndex1;
    m_VBIndex[2] = vbIndex2;
}

NOALIAS __forceinline void* LodTriangle::Primitive() const
{
    return m_pPrimitive;
}

NOALIAS __forceinline S32 LodTriangle::VBIndex( const S32 i ) const
{
    return m_VBIndex[i];
}

NOALIAS __forceinline S32 LodTriangle::VertexIndex( const S32 i ) const
{
    return m_VertexIndex[i];
}

NOALIAS __forceinline void LodTriangle::SetVertexIndices( const S32 vertexIndex0, const S32 vertexIndex1, const S32 vertexIndex2 )
{
    m_VertexIndex[0] = vertexIndex0;
    m_VertexIndex[1] = vertexIndex1;
    m_VertexIndex[2] = vertexIndex2;
}

NOALIAS __forceinline void LodTriangle::SetEdgeIndices( const S32 edgeIndex0, const S32 edgeIndex1, const S32 edgeIndex2 )
{
    m_EdgeIndex[0] = edgeIndex0;
    m_EdgeIndex[1] = edgeIndex1;
    m_EdgeIndex[2] = edgeIndex2;
}

NOALIAS __forceinline void LodTriangle::SetNormals( const Maths::Vector3 VECREF normal0, const Maths::Vector3 VECREF normal1, const Maths::Vector3 VECREF normal2 )
{
    Store(MakeVec4F32(normal0), &m_Normal[0]);
    Store(MakeVec4F32(normal1), &m_Normal[1]);
    Store(MakeVec4F32(normal2), &m_Normal[2]);
}

NOALIAS __forceinline void LodTriangle::SetTangents( const Maths::Vector3 VECREF tangent0, const Maths::Vector3 VECREF tangent1, const Maths::Vector3 VECREF tangent2 )
{
	Store(MakeVec4F32(tangent0), &m_Tangent[0]);
	Store(MakeVec4F32(tangent1), &m_Tangent[1]);
	Store(MakeVec4F32(tangent2), &m_Tangent[2]);
}

NOALIAS __forceinline void LodTriangle::SetBiNormals( const Maths::Vector3 VECREF biNormal0, const Maths::Vector3 VECREF biNormal1, const Maths::Vector3 VECREF biNormal2 )
{
	Store(MakeVec4F32(biNormal0), &m_BiNormal[0]);
	Store(MakeVec4F32(biNormal1), &m_BiNormal[1]);
	Store(MakeVec4F32(biNormal2), &m_BiNormal[2]);
}

NOALIAS __forceinline void LodTriangle::SetColours( const Maths::Vector4 VECREF colour0, const Maths::Vector4 VECREF colour1, const Maths::Vector4 VECREF colour2 )
{
    Store(MakeVec4F32(colour0), &m_Colour[0]);
    Store(MakeVec4F32(colour1), &m_Colour[1]);
    Store(MakeVec4F32(colour2), &m_Colour[2]);
}

NOALIAS __forceinline void LodTriangle::AddTexCoords( const Maths::Vec4F32 VECREF texCoord0, const Maths::Vec4F32 VECREF texCoord1, const Maths::Vec4F32 VECREF texCoord2 )
{
    if ( m_TexCoordCount < m_MaxTexCoordCount )
    {
        Store(texCoord0, &m_TexCoord[ m_TexCoordCount ][0]);
        Store(texCoord1, &m_TexCoord[ m_TexCoordCount ][1]);
        Store(texCoord2, &m_TexCoord[ m_TexCoordCount ][2]);
        m_TexCoordCount++;
    }
}


//
// ---------------------------------------
// |-- LodMesh Inline Member Functions --|
// ---------------------------------------
//

NOALIAS __forceinline LodMesh::LodMesh()
    : m_pRoot( NULL )
    , m_VertexIterators( m_Vertices )
    , m_EdgeIterators( m_Edges )
    , m_TriangleIterators( m_Triangles )
{
    SetInvalidExtent( &m_BoundingBox );
    //m_BoundingBoxScale = Maths::Const<Maths::Vector3>::One();
}

NOALIAS __forceinline const LodMesh& LodMesh::operator = ( const LodMesh& copy )
{
    BIZ_ASSERT( copy.m_VertexIterators.m_IteratorStack.size() == 0 );
    BIZ_ASSERT( copy.m_EdgeIterators.m_IteratorStack.size() == 0 );
    BIZ_ASSERT( copy.m_TriangleIterators.m_IteratorStack.size() == 0 );
    
    m_Options   = copy.m_Options;
    m_pRoot     = copy.m_pRoot;
    m_Vertices  = copy.m_Vertices;
    m_Edges     = copy.m_Edges;
    m_Triangles = copy.m_Triangles;
    
    m_BoundingBox = copy.m_BoundingBox;
    //m_BoundingBoxScale = copy.m_BoundingBoxScale;
    
    //for ( S32 x=0; x<c_VertexIndexCubeSize; ++x )
    //    for ( S32 y=0; y<c_VertexIndexCubeSize; ++y )
    //        for ( S32 z=0; z<c_VertexIndexCubeSize; ++z )
    //            m_VertexIndexCube[x][y][z] = copy.m_VertexIndexCube[x][y][z];

	m_vertexMap = copy.m_vertexMap;
 
    return copy;
}

NOALIAS __forceinline LodVertex& LodMesh::Vertex( const S32 index )
{
    return m_Vertices[index];
}

NOALIAS __forceinline LodEdge& LodMesh::Edge( const S32 index )
{
    return m_Edges[index];
}

NOALIAS __forceinline LodTriangle& LodMesh::Triangle( const S32 index )
{
    return m_Triangles[index];
}

NOALIAS __forceinline const S32 LodMesh::VertexCount() const
{
    return m_Vertices.size();
}

NOALIAS __forceinline const S32 LodMesh::EdgeCount() const
{
    return m_Edges.size();
}

NOALIAS __forceinline const S32 LodMesh::TriangleCount() const
{
    return m_Triangles.size();
}

NOALIAS __forceinline void LodMesh::SetOptions( LodMeshOptions& options )
{
    m_Options = options;
}

NOALIAS __forceinline void LodMesh::SetBoundingBox( const Maths::AxisBox& boundingBox )
{
    using namespace Maths;
    m_BoundingBox = boundingBox;
    //m_BoundingBoxScale = MakeVF32( c_VertexIndexCubeSize ) / ( Max( m_BoundingBox ) - Min( m_BoundingBox ) );
}


} // namespace Build
} // namespace RenderingData

#endif // #ifndef RD_LOD_MESH_INL
