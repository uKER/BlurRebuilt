#include "stdafx.h"

#include <Core/Maths/Hvec/New/IncludeGeneral.h>
#include <Core/Maths/Hvec/New/IncludeGeneral.inl>

#include <system/DebugLog.h>

#include "renderingdata/LodMesh.h"
#include "renderingdata/noninterface/LodMesh.inl"

static Debug::Logger &g_Log = Debug::GetLogger( "graphics.renderingdata" );

namespace RenderingData {
namespace Build
{

static int g_nMaxMessages = 5;

#if defined( USER_JDENTON ) && defined( CFG_ALLOW_OPTIMIZE_OFF )
#pragma optimize( "", off )
#endif

S32 LodMesh::AddVertex( const Maths::Point3 VECREF point )
{
    using namespace Maths;
    
    const VF32 weldThreshold( Load<VF32>( &m_Options.weldThreshold ) );
    const S32 vertexCount( m_Vertices.size() );
    
    //for ( S32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++ )
    //{
    //    LodVertex& vertex = m_Vertices[ vertexIndex ];
    //    const VF32 d( Distance( point, vertex.m_Position ) );
    //    if ( d < weldThreshold )
    //    {
    //        return vertexIndex;
    //    }
    //}

	if (m_Options.weldThreshold > 0.f)
	{
		const Vector3 v( ( MakeVector3( point ) - Min( m_BoundingBox ) ) * m_BoundingBoxScale );
		const S32 x( Clamp( (S32)GetF32( X(v) ), 0, c_VertexIndexCubeSize - 1 ) );
		const S32 y( Clamp( (S32)GetF32( Y(v) ), 0, c_VertexIndexCubeSize - 1 ) );
		const S32 z( Clamp( (S32)GetF32( Z(v) ), 0, c_VertexIndexCubeSize - 1 ) );
	    
		BIZ_ASSERT( ( x >= 0 ) && ( x <= c_VertexIndexCubeSize ) );
		BIZ_ASSERT( ( y >= 0 ) && ( y <= c_VertexIndexCubeSize ) );
		BIZ_ASSERT( ( z >= 0 ) && ( z <= c_VertexIndexCubeSize ) );
	    
		const S32 vertexIndexCubeCount( m_VertexIndexCube[x][y][z].Size() );
		for ( S32 i = 0; i < vertexIndexCubeCount; ++i )
		{
			const S32 vertexIndex( m_VertexIndexCube[x][y][z][i] );
			LodVertex& vertex = m_Vertices[ vertexIndex ];
			const VF32 d( Distance( point, vertex.m_Position ) );
			if ( d < weldThreshold ) // TODO: Replace with align to grid!!!
			{
				return vertexIndex;
			}
		}

		const S32 vertexIndex( vertexCount );
		m_Vertices.push_back(LodVertex());
		LodVertex& vertex = m_Vertices.back();
		vertex.Initialise( vertexIndex, point );
		m_VertexIndexCube[x][y][z].PushBack( vertexIndex );
		return vertexIndex;
	}
	else
	{
		VertexMap::InsertionReturnType result = m_vertexMap.Insert(point);

		if (!result.inserted)
		{
			return result.iterator->GetValue();
		}
		else
		{
			const S32 vertexIndex( vertexCount );
			m_Vertices.push_back(LodVertex());
			LodVertex& vertex = m_Vertices.back();
			vertex.Initialise( vertexIndex, point );
			result.iterator->GetValue() = vertexIndex;
			return vertexIndex;
		}
	}
}

S32 LodMesh::AddEdge( const S32 vertexIndex0, const S32 vertexIndex1 )
{
    using namespace Maths;
    
    LodVertex& vertex0( m_Vertices[ vertexIndex0 ] );
    LodVertex& vertex1( m_Vertices[ vertexIndex1 ] );
    
    #if 0
    const S32 edgeCount( m_Edges.Size() );
    for ( S32 edgeIndex = 0; edgeIndex < edgeCount; edgeIndex++ )
    {
        const LodEdge& edge = m_Edges[ edgeIndex ];
        if ( ( ( edge.m_VertexIndex[0] == vertexIndex0 ) && ( edge.m_VertexIndex[1] == vertexIndex1 ) ) ||
             ( ( edge.m_VertexIndex[0] == vertexIndex1 ) && ( edge.m_VertexIndex[1] == vertexIndex0 ) ) )
        {
            return edgeIndex;
        }
    }
    #else
    const S32 vertexEdgeCount( vertex0.m_EdgeIndices.Size() );
    for ( S32 i=0; i<vertexEdgeCount; ++i )
    {
        const S32 edgeIndex( vertex0.m_EdgeIndices[i] );
        const LodEdge& edge( m_Edges[ edgeIndex ] );
        BIZ_ASSERT( ( edge.m_VertexIndex[1] == vertexIndex0 ) ||
                    ( edge.m_VertexIndex[0] == vertexIndex0 ) );
        if ( ( edge.m_VertexIndex[1] == vertexIndex1 ) ||
             ( edge.m_VertexIndex[0] == vertexIndex1 ) )
        {
            return edgeIndex;
        }
    }
    #endif
    
    const S32 edgeIndex( m_Edges.size() );
    m_Edges.push_back(LodEdge());
    LodEdge& edge = m_Edges.back();
    edge.Initialise( edgeIndex, vertexIndex0, vertexIndex1 );
    vertex0.AddEdgeIndex( edgeIndex );
    vertex1.AddEdgeIndex( edgeIndex );
    return edgeIndex;
}

S32 LodMesh::AddTriangle( void * pPrimitive, const S32 vertexBufferSize, const U32 vbIndex0, const U32 vbIndex1, const U32 vbIndex2 )
{
    const S32 triangleCount( m_Triangles.size() );
    m_Triangles.push_back(LodTriangle());
    LodTriangle& triangle( m_Triangles.back() );
    triangle.Initialise( triangleCount, pPrimitive, vertexBufferSize, vbIndex0, vbIndex1, vbIndex2 );
    return triangleCount;
}

void LodMesh::DeleteVertex( const S32 vertexIndex )
{
    #if defined( _DEBUG )
    
    //LOGDEBUG( g_Log, "LodMesh::DeleteVertex(): vertexIndex = %d\n", vertexIndex );
    
    // Ensure that no edges in the mesh are still using this vertex
    for ( S32 i=0; i<(S32)m_Edges.size(); i++ )
    {
        LodEdge& edge( m_Edges[i] );
        for ( S32 j=0; j<2; j++ )
        {
            BIZ_ASSERT( edge.m_VertexIndex[j] != vertexIndex );
        }
    }
    
    // Ensure that no triangles in the mesh are still using this vertex
    for ( S32 i=0; i<(S32)m_Triangles.size(); i++ )
    {
        LodTriangle& triangle( m_Triangles[i] );
        for ( S32 j=0; j<3; j++ )
        {
            BIZ_ASSERT( triangle.m_VertexIndex[j] != vertexIndex );
        }
    }
    
    #endif
    
    // Delete the vertex by swapping it with the last vertex and then decrementing the size of the array
    const S32 vertexCount( m_Vertices.size() );
    const S32 lastVertexIndex( vertexCount - 1 );
    if ( vertexIndex != lastVertexIndex )
    {
        // Remap any use of lastVertexIndex to vertexIndex
        RemapVertex( lastVertexIndex, vertexIndex );

		std::swap(m_Vertices[vertexIndex], m_Vertices[lastVertexIndex]);
        
        // Swap vertex with lastVertex
		//std::deque<LodVertex>::iterator vertex( m_Vertices.begin() + vertexIndex  );
        //std::deque<LodVertex>::iterator lastVertex( m_Vertices.begin() + lastVertexIndex );
        //m_Vertices.Swap<LodVertex>( vertex, lastVertex );
        
        // Update swapped vertex's index
        m_Vertices[ vertexIndex ].m_Index = vertexIndex;
        
        m_VertexIterators.Swapped( vertexIndex, lastVertexIndex );
    }
    m_Vertices.pop_back();
    
    m_VertexIterators.Deleted( lastVertexIndex );
}

void LodMesh::DeleteEdge( const S32 edgeIndex )
{
    #if defined( _DEBUG )
    
    //LOGDEBUG( g_Log, "LodMesh::DeleteEdge(): edgeIndex = %d\n", edgeIndex );
    
    // Ensure that no vertices in the mesh are still using this edge
    for ( S32 i=0; i<(S32)m_Vertices.size(); i++ )
    {
        LodVertex& vertex( m_Vertices[i] );
        for ( S32 j=0; j<vertex.m_EdgeIndices.Size(); j++ )
        {
            BIZ_ASSERT( vertex.m_EdgeIndices[j] != edgeIndex );
        }
    }
    
    // Ensure that no triangles in the mesh are still using this edge
    for ( S32 i=0; i<(S32)m_Triangles.size(); i++ )
    {
        LodTriangle& triangle( m_Triangles[i] );
        for ( S32 j=0; j<3; j++ )
        {
            BIZ_ASSERT( triangle.m_EdgeIndex[j] != edgeIndex );
        }
    }
    
    #endif
    
    // Delete the edge by swapping it with the last edge and then decrementing the size of the array
    const S32 edgeCount( m_Edges.size() );
    const S32 lastEdgeIndex( edgeCount - 1 );
    if ( edgeIndex != lastEdgeIndex )
    {
        // Remap any use of lastEdgeIndex to edgeIndex
        RemapEdge( lastEdgeIndex, edgeIndex );

		std::swap(m_Edges[edgeIndex], m_Edges[lastEdgeIndex]);
        
        // Swap edge with lastEdge
        //std::deque<LodEdge>::iterator edge( m_Edges.begin() + edgeIndex  );
        //std::deque<LodEdge>::iterator lastEdge( m_Edges.begin() + lastEdgeIndex );
        //m_Edges.Swap<LodEdge>( edge, lastEdge );
        
        // Update swapped edge's index
        m_Edges[ edgeIndex ].m_Index = edgeIndex;
        
        m_EdgeIterators.Swapped( edgeIndex, lastEdgeIndex );
    }
    m_Edges.pop_back();
    
    m_EdgeIterators.Deleted( lastEdgeIndex );
}

void LodMesh::DeleteTriangle( const S32 triangleIndex )
{
    #if defined( _DEBUG )
    
    //LOGDEBUG( g_Log, "LodMesh::DeleteTriangle(): triangleIndex = %d\n", triangleIndex );
    
    // Ensure that no vertices in the mesh are still using this triangle
    for ( S32 i=0; i<(S32)m_Vertices.size(); i++ )
    {
        LodVertex& vertex( m_Vertices[i] );
        for ( S32 j=0; j<vertex.m_TriangleIndices.Size(); j++ )
        {
            BIZ_ASSERT( vertex.m_TriangleIndices[j] != triangleIndex );
        }
    }
    
    // Ensure that no edges in the mesh are still using this triangle
    for ( S32 i=0; i<(S32)m_Edges.size(); i++ )
    {
        LodEdge& edge( m_Edges[i] );
        for ( S32 j=0; j<edge.m_TriangleIndices.Size(); j++ )
        {
            BIZ_ASSERT( edge.m_TriangleIndices[j] != triangleIndex );
        }
    }
    
    #endif
    
    // Delete the triangle by swapping it with the last triangle and then decrementing the size of the array
    const S32 triangleCount( m_Triangles.size() );
    const S32 lastTriangleIndex( triangleCount - 1 );
    if ( triangleIndex != lastTriangleIndex )
    {
        // Remap any use of lastTriangleIndex to triangleIndex
        RemapTriangle( lastTriangleIndex, triangleIndex );

		std::swap(m_Triangles[triangleIndex], m_Triangles[lastTriangleIndex]);
        
        // Swap triangle with lastTriangle
        //std::deque<LodTriangle>::iterator triangle( m_Triangles.begin() + triangleIndex );
        //std::deque<LodTriangle>::iterator lastTriangle( m_Triangles.begin() + lastTriangleIndex );
        //m_Triangles.Swap<LodTriangle>( triangle, lastTriangle );
        
        // Update swapped triangle's index
        m_Triangles[ triangleIndex ].m_Index = triangleIndex;
        
        m_TriangleIterators.Swapped( triangleIndex, lastTriangleIndex );
    }
    m_Triangles.pop_back();
    
    m_TriangleIterators.Deleted( lastTriangleIndex );
}

//void LodMesh::AddTriangleVertexData( const S32 triangleIndex, const VertexData& vertexData )
//{
//    using namespace DataTypes;
//    using namespace Maths;
//    
//    LodTriangle& triangle( m_Triangles[ triangleIndex ] );
//    
//    const VertexFormat& vertexFormat = vertexData.GetVertexFormat();
//    
//    const S32 vertexCount = vertexData.Size();
//    const S32 vertexSize = vertexFormat.GetDataSize();
//    
//    const S32 elementCount = vertexFormat.GetNumVertexElements();
//    for ( S32 elementIndex = 0; elementIndex < elementCount; elementIndex++ )
//    {
//        const VertexFormat::VertexElement& element = vertexFormat.GetVertexElement( elementIndex );
//        
//        const F32 zero( 0.0f );
//        Maths::Vec4F32 v0, v1, v2;
//        
//        switch ( element.type )
//        {
//		    case VERTEX_ELEMENT_TYPE_S16_2 :
//		    {
//		        const S16_2 data0( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[0] ).GetData< S16_2 >( elementIndex ) );
//		        const S16_2 data1( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[1] ).GetData< S16_2 >( elementIndex ) );
//		        const S16_2 data2( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[2] ).GetData< S16_2 >( elementIndex ) );
//		        v0 = MakeVec4F32( (F32)data0.X(), (F32)data0.Y(), zero, zero );
//		        v1 = MakeVec4F32( (F32)data1.X(), (F32)data1.Y(), zero, zero );
//		        v2 = MakeVec4F32( (F32)data2.X(), (F32)data2.Y(), zero, zero );
//		    }
//		    break;
//		    
//		    case VERTEX_ELEMENT_TYPE_S16_4 :
//		    {
//		        const S16_4 data0( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[0] ).GetData< S16_4 >( elementIndex ) );
//		        const S16_4 data1( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[1] ).GetData< S16_4 >( elementIndex ) );
//		        const S16_4 data2( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[2] ).GetData< S16_4 >( elementIndex ) );
//		        v0 = MakeVec4F32( (F32)data0.X(), (F32)data0.Y(), (F32)data0.Z(), (F32)data0.W() );
//		        v1 = MakeVec4F32( (F32)data1.X(), (F32)data1.Y(), (F32)data1.Z(), (F32)data1.W() );
//		        v2 = MakeVec4F32( (F32)data2.X(), (F32)data2.Y(), (F32)data2.Z(), (F32)data2.W() );
//		    }
//		    break;
//		    
//		    case VERTEX_ELEMENT_TYPE_S16_2N :
//		    {
//		        const S16_2N data0( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[0] ).GetData< S16_2N >( elementIndex ) );
//		        const S16_2N data1( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[1] ).GetData< S16_2N >( elementIndex ) );
//		        const S16_2N data2( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[2] ).GetData< S16_2N >( elementIndex ) );
//		        v0 = MakeVec4F32( (F32)data0.X(), (F32)data0.Y(), zero, zero );
//		        v1 = MakeVec4F32( (F32)data1.X(), (F32)data1.Y(), zero, zero );
//		        v2 = MakeVec4F32( (F32)data2.X(), (F32)data2.Y(), zero, zero );
//		    }
//		    break;
//		    
//		    case VERTEX_ELEMENT_TYPE_S16_4N :
//		    {
//		        const S16_4N data0( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[0] ).GetData< S16_4N >( elementIndex ) );
//		        const S16_4N data1( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[1] ).GetData< S16_4N >( elementIndex ) );
//		        const S16_4N data2( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[2] ).GetData< S16_4N >( elementIndex ) );
//		        v0 = MakeVec4F32( (F32)data0.X(), (F32)data0.Y(), (F32)data0.Z(), (F32)data0.W() );
//		        v1 = MakeVec4F32( (F32)data1.X(), (F32)data1.Y(), (F32)data1.Z(), (F32)data1.W() );
//		        v2 = MakeVec4F32( (F32)data2.X(), (F32)data2.Y(), (F32)data2.Z(), (F32)data2.W() );
//		    }
//		    break;
//		    
//		    case VERTEX_ELEMENT_TYPE_F32_1 :
//		    {
//		        const F32_1 data0( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[0] ).GetData< F32_1 >( elementIndex ) );
//		        const F32_1 data1( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[1] ).GetData< F32_1 >( elementIndex ) );
//		        const F32_1 data2( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[2] ).GetData< F32_1 >( elementIndex ) );
//		        v0 = MakeVec4F32( (F32)data0.X(), zero, zero, zero );
//		        v1 = MakeVec4F32( (F32)data1.X(), zero, zero, zero );
//		        v2 = MakeVec4F32( (F32)data2.X(), zero, zero, zero );
//		    }
//		    break;
//		    
//		    case VERTEX_ELEMENT_TYPE_F32_2 :
//		    {
//		        const F32_2 data0( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[0] ).GetData< F32_2 >( elementIndex ) );
//		        const F32_2 data1( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[1] ).GetData< F32_2 >( elementIndex ) );
//		        const F32_2 data2( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[2] ).GetData< F32_2 >( elementIndex ) );
//		        v0 = MakeVec4F32( (F32)data0.X(), (F32)data0.Y(), zero, zero );
//		        v1 = MakeVec4F32( (F32)data1.X(), (F32)data1.Y(), zero, zero );
//		        v2 = MakeVec4F32( (F32)data2.X(), (F32)data2.Y(), zero, zero );
//		    }
//		    break;
//		    
//		    case VERTEX_ELEMENT_TYPE_F32_3 :
//		    {
//		        const F32_3 data0( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[0] ).GetData< F32_3 >( elementIndex ) );
//		        const F32_3 data1( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[1] ).GetData< F32_3 >( elementIndex ) );
//		        const F32_3 data2( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[2] ).GetData< F32_3 >( elementIndex ) );
//		        v0 = MakeVec4F32( (F32)data0.X(), (F32)data0.Y(), (F32)data0.Z(), zero );
//		        v1 = MakeVec4F32( (F32)data1.X(), (F32)data1.Y(), (F32)data1.Z(), zero );
//		        v2 = MakeVec4F32( (F32)data2.X(), (F32)data2.Y(), (F32)data2.Z(), zero );
//		    }
//		    break;
//		    
//		    case VERTEX_ELEMENT_TYPE_F32_4 :
//		    {
//		        const F32_4 data0( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[0] ).GetData< F32_4 >( elementIndex ) );
//		        const F32_4 data1( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[1] ).GetData< F32_4 >( elementIndex ) );
//		        const F32_4 data2( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[2] ).GetData< F32_4 >( elementIndex ) );
//		        v0 = MakeVec4F32( (F32)data0.X(), (F32)data0.Y(), (F32)data0.Z(), (F32)data0.W() );
//		        v1 = MakeVec4F32( (F32)data1.X(), (F32)data1.Y(), (F32)data1.Z(), (F32)data1.W() );
//		        v2 = MakeVec4F32( (F32)data2.X(), (F32)data2.Y(), (F32)data2.Z(), (F32)data2.W() );
//		    }
//		    break;
//		    
//		    case VERTEX_ELEMENT_TYPE_F16_2 :
//		    {
//		        const F16_2 data0( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[0] ).GetData< F16_2 >( elementIndex ) );
//		        const F16_2 data1( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[1] ).GetData< F16_2 >( elementIndex ) );
//		        const F16_2 data2( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[2] ).GetData< F16_2 >( elementIndex ) );
//		        v0 = MakeVec4F32( (F32)data0.X(), (F32)data0.Y(), zero, zero );
//		        v1 = MakeVec4F32( (F32)data1.X(), (F32)data1.Y(), zero, zero );
//		        v2 = MakeVec4F32( (F32)data2.X(), (F32)data2.Y(), zero, zero );
//		    }
//		    break;
//		    
//		    case VERTEX_ELEMENT_TYPE_F16_4 :
//		    {
//		        const F16_4 data0( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[0] ).GetData< F16_4 >( elementIndex ) );
//		        const F16_4 data1( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[1] ).GetData< F16_4 >( elementIndex ) );
//		        const F16_4 data2( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[2] ).GetData< F16_4 >( elementIndex ) );
//		        v0 = MakeVec4F32( (F32)data0.X(), (F32)data0.Y(), (F32)data0.Z(), (F32)data0.W() );
//		        v1 = MakeVec4F32( (F32)data1.X(), (F32)data1.Y(), (F32)data1.Z(), (F32)data1.W() );
//		        v2 = MakeVec4F32( (F32)data2.X(), (F32)data2.Y(), (F32)data2.Z(), (F32)data2.W() );
//		    }
//		    break;
//		    
//		    case VERTEX_ELEMENT_TYPE_U8_4 :
//		    {
//		        const U8_4 data0( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[0] ).GetData< U8_4 >( elementIndex ) );
//		        const U8_4 data1( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[1] ).GetData< U8_4 >( elementIndex ) );
//		        const U8_4 data2( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[2] ).GetData< U8_4 >( elementIndex ) );
//		        v0 = MakeVec4F32( (F32)data0.X(), (F32)data0.Y(), (F32)data0.Z(), (F32)data0.W() );
//		        v1 = MakeVec4F32( (F32)data1.X(), (F32)data1.Y(), (F32)data1.Z(), (F32)data1.W() );
//		        v2 = MakeVec4F32( (F32)data2.X(), (F32)data2.Y(), (F32)data2.Z(), (F32)data2.W() );
//		    }
//		    break;
//		    
//		    case VERTEX_ELEMENT_TYPE_U8_4N :
//		    {
//		        const U8_4N data0( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[0] ).GetData< U8_4N >( elementIndex ) );
//		        const U8_4N data1( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[1] ).GetData< U8_4N >( elementIndex ) );
//		        const U8_4N data2( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[2] ).GetData< U8_4N >( elementIndex ) );
//		        v0 = MakeVec4F32( (F32)data0.X(), (F32)data0.Y(), (F32)data0.Z(), (F32)data0.W() );
//		        v1 = MakeVec4F32( (F32)data1.X(), (F32)data1.Y(), (F32)data1.Z(), (F32)data1.W() );
//		        v2 = MakeVec4F32( (F32)data2.X(), (F32)data2.Y(), (F32)data2.Z(), (F32)data2.W() );
//		    }
//		    break;
//		    
//		    case VERTEX_ELEMENT_TYPE_SHHD_3N :
//		    {
//		        const SHHD_3N data0( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[0] ).GetData< SHHD_3N >( elementIndex ) );
//		        const SHHD_3N data1( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[1] ).GetData< SHHD_3N >( elementIndex ) );
//		        const SHHD_3N data2( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[2] ).GetData< SHHD_3N >( elementIndex ) );
//		        v0 = MakeVec4F32( (F32)data0.X(), (F32)data0.Y(), (F32)data0.Z(), zero );
//		        v1 = MakeVec4F32( (F32)data1.X(), (F32)data1.Y(), (F32)data1.Z(), zero );
//		        v2 = MakeVec4F32( (F32)data2.X(), (F32)data2.Y(), (F32)data2.Z(), zero );
//		    }
//		    break;
//		    
//		    case VERTEX_ELEMENT_TYPE_SDDD_3N :
//		    {
//		        const SDDD_3N data0( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[0] ).GetData< SDDD_3N >( elementIndex ) );
//		        const SDDD_3N data1( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[1] ).GetData< SDDD_3N >( elementIndex ) );
//		        const SDDD_3N data2( VertexData::ConstVertexReference( vertexData, triangle.m_VBIndex[2] ).GetData< SDDD_3N >( elementIndex ) );
//		        v0 = MakeVec4F32( (F32)data0.X(), (F32)data0.Y(), (F32)data0.Z(), zero );
//		        v1 = MakeVec4F32( (F32)data1.X(), (F32)data1.Y(), (F32)data1.Z(), zero );
//		        v2 = MakeVec4F32( (F32)data2.X(), (F32)data2.Y(), (F32)data2.Z(), zero );
//		    }
//		    break;
//		    
//		    default: break;
//        }
//        
//        switch ( element.usage )
//        {
//		    case VERTEX_ELEMENT_USAGE_POSITION :
//		    {
//	            const Maths::Point3 point0( MakePoint3( v0 ) );
//	            const Maths::Point3 point1( MakePoint3( v1 ) );
//	            const Maths::Point3 point2( MakePoint3( v2 ) );
//	            
//	            const S32 vertexIndex0 = AddVertex( point0 );
//	            const S32 vertexIndex1 = AddVertex( point1 );
//	            const S32 vertexIndex2 = AddVertex( point2 );
//	            
//	            BIZ_ASSERT( vertexIndex0 != vertexIndex1 );
//	            BIZ_ASSERT( vertexIndex0 != vertexIndex2 );
//	            BIZ_ASSERT( vertexIndex1 != vertexIndex2 );
//	            
//	            const S32 edgeIndex0 = AddEdge( vertexIndex0, vertexIndex1 );
//	            const S32 edgeIndex1 = AddEdge( vertexIndex1, vertexIndex2 );
//	            const S32 edgeIndex2 = AddEdge( vertexIndex2, vertexIndex0 );
//	            
//	            BIZ_ASSERT( edgeIndex0 != edgeIndex1 );
//	            BIZ_ASSERT( edgeIndex0 != edgeIndex2 );
//	            BIZ_ASSERT( edgeIndex1 != edgeIndex2 );
//	            
//	            triangle.m_VertexIndex[0] = vertexIndex0;
//	            triangle.m_VertexIndex[1] = vertexIndex1;
//	            triangle.m_VertexIndex[2] = vertexIndex2;
//	            
//	            triangle.m_EdgeIndex[0] = edgeIndex0;
//	            triangle.m_EdgeIndex[1] = edgeIndex1;
//	            triangle.m_EdgeIndex[2] = edgeIndex2;
//	            
//	            LodVertex& vertex0( m_Vertices[ vertexIndex0 ] );
//	            LodVertex& vertex1( m_Vertices[ vertexIndex1 ] );
//	            LodVertex& vertex2( m_Vertices[ vertexIndex2 ] );
//	            
//	            LodEdge& edge0( m_Edges[ edgeIndex0 ] );
//	            LodEdge& edge1( m_Edges[ edgeIndex1 ] );
//	            LodEdge& edge2( m_Edges[ edgeIndex2 ] );
//	            
//                vertex0.AddTriangleIndex( triangleIndex );
//                vertex1.AddTriangleIndex( triangleIndex );
//                vertex2.AddTriangleIndex( triangleIndex );
//	            
//	            edge0.AddTriangleIndex( triangleIndex );
//	            edge1.AddTriangleIndex( triangleIndex );
//	            edge2.AddTriangleIndex( triangleIndex );
//		    }
//		    break;
//		    
//		    case VERTEX_ELEMENT_USAGE_BLENDWEIGHT :
//    	    //DebugBreak();
//		    break;
//		    
//		    case VERTEX_ELEMENT_USAGE_BLENDINDICES :
//		    //DebugBreak();
//		    break;
//		    
//		    case VERTEX_ELEMENT_USAGE_NORMAL :
//		    triangle.m_Normal[0] = MakeVector3( v0 );
//		    triangle.m_Normal[1] = MakeVector3( v1 );
//		    triangle.m_Normal[2] = MakeVector3( v2 );
//		    //DebugBreak();
//		    break;
//		    
//		    case VERTEX_ELEMENT_USAGE_PSIZE :
//		    //DebugBreak();
//		    break;
//		    
//		    case VERTEX_ELEMENT_USAGE_TEXCOORD :
//		    if ( triangle.m_TexCoordCount < triangle.m_MaxTexCoordCount )
//		    {
//		        triangle.m_TexCoord[ triangle.m_TexCoordCount ][0] = v0;
//		        triangle.m_TexCoord[ triangle.m_TexCoordCount ][1] = v1;
//		        triangle.m_TexCoord[ triangle.m_TexCoordCount ][2] = v2;
//		        triangle.m_TexCoordCount++;
//		    }
//		    //DebugBreak();
//		    break;
//		    
//		    case VERTEX_ELEMENT_USAGE_TANGENT :
//		    triangle.m_Tangent[0] = MakeVector3( v0 );
//		    triangle.m_Tangent[1] = MakeVector3( v1 );
//		    triangle.m_Tangent[2] = MakeVector3( v2 );
//		    //DebugBreak();
//		    break;
//		    
//		    case VERTEX_ELEMENT_USAGE_BINORMAL :
//		    triangle.m_BiNormal[0] = MakeVector3( v0 );
//		    triangle.m_BiNormal[1] = MakeVector3( v1 );
//		    triangle.m_BiNormal[2] = MakeVector3( v2 );
//		    //DebugBreak();
//		    break;
//		    
//		    case VERTEX_ELEMENT_USAGE_TESSFACTOR :
//		    //DebugBreak();
//		    break;
//		    
//		    case VERTEX_ELEMENT_USAGE_COLOR :
//		    triangle.m_Colour[0] = RGBA( (U8)GetF32(X(v0)), (U8)GetF32(Y(v0)), (U8)GetF32(Z(v0)), (U8)GetF32(W(v0)) );
//		    triangle.m_Colour[1] = RGBA( (U8)GetF32(X(v1)), (U8)GetF32(Y(v1)), (U8)GetF32(Z(v1)), (U8)GetF32(W(v1)) );
//		    triangle.m_Colour[2] = RGBA( (U8)GetF32(X(v2)), (U8)GetF32(Y(v2)), (U8)GetF32(Z(v2)), (U8)GetF32(W(v2)) );
//		    //DebugBreak();
//		    break;
//		    
//		    case VERTEX_ELEMENT_USAGE_SAMPLE :
//		    //DebugBreak();
//		    break;
//		    
//		    default: break;
//        }
//    }
//}

S32 LodMesh::FindVertexInTriangle(S32 triangleIndex, S32 vertexIndex)
{
	const LodTriangle& triangle( m_Triangles[ triangleIndex ] );

	if (triangle.m_VertexIndex[0] == vertexIndex) return 0;
	if (triangle.m_VertexIndex[1] == vertexIndex) return 1;
	if (triangle.m_VertexIndex[2] == vertexIndex) return 2;
	return -1;
}

S32 LodMesh::FindRemapTriangle(S32 triangleIndex, S32 sourceVertexIndex, S32 targetVertexIndex, TriangleIndexSet& triangleIndexSet)
{
	TriangleIndexSet::InsertionReturnType result = triangleIndexSet.Insert(triangleIndex);
	if (!result.inserted)
		return -1;

	const LodTriangle& sourceTriangle( m_Triangles[ triangleIndex ] );

	for (int te = 0; te < 3; te++)
	{
		const LodEdge& edge(m_Edges[sourceTriangle.m_EdgeIndex[te]]);

		if (edge.m_VertexIndex[0] == sourceVertexIndex || edge.m_VertexIndex[1] == sourceVertexIndex)
		{
			if (edge.m_VertexIndex[0] == targetVertexIndex || edge.m_VertexIndex[1] == targetVertexIndex)
				return triangleIndex;

			if ( (edge.m_Flags & ( E_EdgeFlag_MaterialBoundary |
                                      E_EdgeFlag_TextureDiscontinuity |
                                      E_EdgeFlag_Outline |
									  E_EdgeFlag_ColourDiscontinuity) ) == 0
				&& edge.m_Sharpness * m_Options.sharpnessPenalty < m_Options.minEdgeScore)
			{
				for (int et = 0; et < edge.m_TriangleIndices.Size(); et++)
				{
					S32 remapTriangle = FindRemapTriangle(edge.m_TriangleIndices[et], sourceVertexIndex, targetVertexIndex, triangleIndexSet);

					if (remapTriangle != -1)
						return remapTriangle;
				}
			}
		}
	}

	return -1;
}

void LodMesh::RemapVertexBufferIndices2( const S32 sourceVertexIndex, const S32 targetVertexIndex )
{
	LodVertex& sourceVertex( m_Vertices[ sourceVertexIndex ] );
    LodVertex& targetVertex( m_Vertices[ targetVertexIndex ] );

	//bool remapped = true;

	RemapVertexBufferIndices(sourceVertexIndex, targetVertexIndex);

    // Loop through all triangles connected to sourceVertex
    /*const S32 sourceTriangleIndicesCount( sourceVertex.m_TriangleIndices.Size() );
    for ( S32 sourceTriangleIndicesIndex=0; sourceTriangleIndicesIndex<sourceTriangleIndicesCount; sourceTriangleIndicesIndex++ )
    {
		const S32 sourceTriangleIndex( sourceVertex.m_TriangleIndices[ sourceTriangleIndicesIndex ] );
		LodTriangle& sourceTriangle( m_Triangles[ sourceTriangleIndex ] );

		TriangleIndexSet set;
		S32 remapTriangleIndex = FindRemapTriangle(sourceTriangleIndex, sourceVertexIndex, targetVertexIndex, set);
		if (remapTriangleIndex != -1)
		{
			int breakhere = 0;
		}
		else
		{
			int breakhere = 0;
			//remapped = false;
		}

		if ( remapTriangleIndex >= 0 )
		{
			LodTriangle& targetTriangle( m_Triangles[ remapTriangleIndex ] );

			S32 sourceTriangleVertexIndex = FindVertexInTriangle(sourceTriangleIndex, sourceVertexIndex);
			S32 targetTriangleVertexIndex = FindVertexInTriangle(remapTriangleIndex, targetVertexIndex);

			sourceTriangle.m_VBIndex[ sourceTriangleVertexIndex ]  = targetTriangle.m_VBIndex[ targetTriangleVertexIndex ];
			sourceTriangle.m_Colour[ sourceTriangleVertexIndex ]   = targetTriangle.m_Colour[ targetTriangleVertexIndex ];
			sourceTriangle.m_Normal[ sourceTriangleVertexIndex ]   = targetTriangle.m_Normal[ targetTriangleVertexIndex ];
			sourceTriangle.m_Tangent[ sourceTriangleVertexIndex ]  = targetTriangle.m_Tangent[ targetTriangleVertexIndex ];
			sourceTriangle.m_BiNormal[ sourceTriangleVertexIndex ] = targetTriangle.m_BiNormal[ targetTriangleVertexIndex ];
             
			BIZ_ASSERT( sourceTriangle.m_TexCoordCount == targetTriangle.m_TexCoordCount );
			for ( S32 texCoordIndex = 0; texCoordIndex < sourceTriangle.m_TexCoordCount; texCoordIndex++ )
			{
				sourceTriangle.m_TexCoord[ texCoordIndex ][ sourceTriangleVertexIndex ] = targetTriangle.m_TexCoord[ texCoordIndex ][ targetTriangleVertexIndex ];
			}
		}
	}*/
}

// For each triangle containing sourceVertex find another triangle
// with the same primitive that contains targetVertex and copy
// the vertex buffer index from that
// In addition, copy all the texture coords, normals, colour etc.
void LodMesh::RemapVertexBufferIndices( const S32 sourceVertexIndex, const S32 targetVertexIndex )
{
    using namespace Maths;
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    const VF32 nearlyZero( Const<VF32>::Frac<1,1000>() );
    const VF32 fltMax( Const<VF32>::Max() );
    const Vec4F32 v4_zero( Const<Vec4F32>::Zero() );
    const S32 next[3] = { 1, 2, 0 };
    
    LodVertex& sourceVertex( m_Vertices[ sourceVertexIndex ] );
    LodVertex& targetVertex( m_Vertices[ targetVertexIndex ] );
    
    // Loop through all triangles connected to sourceVertex
    const S32 sourceTriangleIndicesCount( sourceVertex.m_TriangleIndices.Size() );
    for ( S32 sourceTriangleIndicesIndex=0; sourceTriangleIndicesIndex<sourceTriangleIndicesCount; sourceTriangleIndicesIndex++ )
    {
        // Find the index of this vertex in the sourceTriangle
        const S32 sourceTriangleIndex( sourceVertex.m_TriangleIndices[ sourceTriangleIndicesIndex ] );
        LodTriangle& sourceTriangle( m_Triangles[ sourceTriangleIndex ] );
        bool foundSource = false;
        for ( S32 sourceTriangleVertexIndex=0; sourceTriangleVertexIndex<3; sourceTriangleVertexIndex++ )
        {
            if ( sourceTriangle.m_VertexIndex[ sourceTriangleVertexIndex ] == sourceVertexIndex )
            {
                const Normal3 sourceTriNormal( sourceTriangle.m_SurfaceNormal );
                const Point3  S( sourceVertex.m_Position );
                const Point3  T( targetVertex.m_Position );
                const Plane   sourceTriPlane( MakePlane( sourceTriNormal, S ) );
                const Point3  projectedT( Project( T, sourceTriPlane ) );
                
                //#if defined( USER_JDENTON )
                //BIZ_ASSERT( Length( projectedT - S ) > nearlyZero );
                //#endif
                
                Vec4F32 targetTexCoord[ LodTriangle::m_MaxTexCoordCount ];
                
                if ( Length( projectedT - S ) <= nearlyZero )
                {
                    for ( S32 k=0; k<sourceTriangle.m_TexCoordCount; k++ )
                    {
                        targetTexCoord[k] = Load(&sourceTriangle.m_TexCoord[k][ sourceTriangleVertexIndex ]);
                        #if defined( USER_JDENTON )
                        BIZ_ASSERT( Finite( targetTexCoord[k] ) );
                        #endif
                    }
                }
                else
                {
                    const Plane plane( MakePlane( Normalise( ( projectedT - S ) ^ sourceTriNormal ), S ) );
                    const LodVertex& v0( m_Vertices[ sourceTriangle.m_VertexIndex[0] ] );
                    const LodVertex& v1( m_Vertices[ sourceTriangle.m_VertexIndex[1] ] );
                    const LodVertex& v2( m_Vertices[ sourceTriangle.m_VertexIndex[2] ] );
                    const VF32 d0( Distance( plane, v0.m_Position ) );
                    const VF32 d1( Distance( plane, v1.m_Position ) );
                    const VF32 d2( Distance( plane, v2.m_Position ) );
                    
                    S32 index0, index1, index2;
                    if ( ( ( d1 <= d0 ) && ( d0 <= d2 ) ) ||
                         ( ( d1 >= d0 ) && ( d0 >= d2 ) ) )
                    {
                        index0 = 0; index1 = 1; index2 = 2;
                    }
                    else if ( ( ( d0 <= d1 ) && ( d1 <= d2 ) ) ||
                              ( ( d0 >= d1 ) && ( d1 >= d2 ) ) )
                    {
                        index0 = 1; index1 = 2; index2 = 0;
                    }
                    else if ( ( ( d1 <= d2 ) && ( d2 <= d0 ) ) ||
                              ( ( d1 >= d2 ) && ( d2 >= d0 ) ) )
                    {
                        index0 = 2; index1 = 0; index2 = 1;
                    }
                    else BIZ_ASSERT_FAIL( "Failed to calculate indices!" );
                    
                    const LodVertex& vertex0( m_Vertices[ sourceTriangle.m_VertexIndex[ index0 ] ] );
                    const LodVertex& vertex1( m_Vertices[ sourceTriangle.m_VertexIndex[ index1 ] ] );
                    const LodVertex& vertex2( m_Vertices[ sourceTriangle.m_VertexIndex[ index2 ] ] );
                    
                    const Vector3 L1_direction( projectedT - S );
                    const Line L1( MakeLine( vertex0.m_Position, vertex0.m_Position + L1_direction ) );
                    const Line L2( MakeLine( vertex1.m_Position, vertex2.m_Position ) );
                    
                    #if defined( USER_JDENTON )
                    BIZ_ASSERT( CoPlanar( L1, L2 ) );
                    //BIZ_ASSERT( !Parallel( L1, L2 ) );
                    #endif
                    
                    const VF32 parallelLineThreshold( Const<VF32>::Frac<9999,10000>() );
                    if ( !Parallel( L1, L2, parallelLineThreshold ) )
                    {
                        VF32 t1, t2;
                        const VBool intersected = IntersectInfiniteLines24( L1, L2, &t1, &t2 );
                        
                        #if defined( USER_JDENTON )
                        BIZ_ASSERT( intersected );
                        if ( !intersected )
                        {
                            BIZ_ASSERT( CoPlanar( L1, L2 ) );
                            BIZ_ASSERT( !Parallel( L1, L2 ) );
                        }
                        #endif
                        
                        const Point3 P0( vertex0.m_Position );
                        const Point3 P12( Lerp( vertex1.m_Position, vertex2.m_Position, t2 ) );
                        
                        //#if defined( USER_JDENTON )
                        //BIZ_ASSERT( Length( P0 - P12 ) >= nearlyZero );
                        //#endif
                        
                        const VF32 ratio( Length( L1 ) / Length( P0 - P12 ) );
                        
                        if ( ( ( projectedT - S ) | ( P0 - P12 ) ) >= zero )
                        {
                            for ( S32 k=0; k<sourceTriangle.m_TexCoordCount; k++ )
                            {
                                const Vec4F32 interpolatedTexCoord( Lerp( Load(&sourceTriangle.m_TexCoord[k][index1]), Load(&sourceTriangle.m_TexCoord[k][index2]), t2 ) );
                                const Vec4F32 deltaTexCoord( Load(&sourceTriangle.m_TexCoord[k][index0]) - interpolatedTexCoord );
                                targetTexCoord[k] = MulAdd( deltaTexCoord, ratio, Load(&sourceTriangle.m_TexCoord[k][ sourceTriangleVertexIndex ]) );
                                #if defined( USER_JDENTON )
                                BIZ_ASSERT( Finite( targetTexCoord[k] ) );
                                #endif
                            }
                        }
                        else
                        {
                            for ( S32 k=0; k<sourceTriangle.m_TexCoordCount; k++ )
                            {
                                const Vec4F32 interpolatedTexCoord( Lerp( Load(&sourceTriangle.m_TexCoord[k][index1]), Load(&sourceTriangle.m_TexCoord[k][index2]), t2 ) );
                                const Vec4F32 deltaTexCoord( interpolatedTexCoord - Load(&sourceTriangle.m_TexCoord[k][index0]) );
                                targetTexCoord[k] = MulAdd( deltaTexCoord, ratio, Load(&sourceTriangle.m_TexCoord[k][ sourceTriangleVertexIndex ]) );
                                #if defined( USER_JDENTON )
                                BIZ_ASSERT( Finite( targetTexCoord[k] ) );
                                #endif
                            }
                        }
                    }
                    else
                    {
                        const S32 index0( sourceTriangleVertexIndex );
                        const S32 index1( next[index0] );
                        const S32 index2( next[index1] );
                        
                        const LodVertex& vertex0( m_Vertices[ sourceTriangle.m_VertexIndex[ index0 ] ] );
                        const LodVertex& vertex1( m_Vertices[ sourceTriangle.m_VertexIndex[ index1 ] ] );
                        const LodVertex& vertex2( m_Vertices[ sourceTriangle.m_VertexIndex[ index2 ] ] );
                        
                        const Line L1( MakeLine( S, projectedT ) );
                        const Line L2( MakeLine( vertex1.m_Position, vertex2.m_Position ) );
                        
                        const VF32 ratio( Length( L1 ) * RcpLength( L2 ) );
                        
                        //#if defined( USER_JDENTON )
                        //BIZ_ASSERT( Length( L2 ) >= nearlyZero );
                        //#endif
                        
                        if ( ( Vector( L1 ) | Vector( L2 ) ) >= zero )
                        //if ( ( Vector( L1 ) | Vector( L2 ) ) <= zero )
                        {
                            for ( S32 k=0; k<sourceTriangle.m_TexCoordCount; k++ )
                            {
                                const Vec4F32 texCoord0( Load(&sourceTriangle.m_TexCoord[k][index0]) );
                                const Vec4F32 texCoord1( Load(&sourceTriangle.m_TexCoord[k][index0]) );
                                const Vec4F32 texCoord2( Load(&sourceTriangle.m_TexCoord[k][index0]) );
                                targetTexCoord[k] = MulAdd( texCoord2 - texCoord1, ratio, texCoord0 );
                                #if defined( USER_JDENTON )
                                BIZ_ASSERT( Finite( targetTexCoord[k] ) );
                                #endif
                            }
                        }
                        else
                        {
                            for ( S32 k=0; k<sourceTriangle.m_TexCoordCount; k++ )
                            {
                                const Vec4F32 texCoord0( Load(&sourceTriangle.m_TexCoord[k][index0]) );
                                const Vec4F32 texCoord1( Load(&sourceTriangle.m_TexCoord[k][index0]) );
                                const Vec4F32 texCoord2( Load(&sourceTriangle.m_TexCoord[k][index0]) );
                                targetTexCoord[k] = MulAdd( texCoord1 - texCoord2, ratio, texCoord0 );
                                #if defined( USER_JDENTON )
                                BIZ_ASSERT( Finite( targetTexCoord[k] ) );
                                #endif
                            }
                        }
                    }
                }
                
                VF32 leastError( fltMax );
                S32  bestTargetTriangleIndex = -1;
                S32  bestTargetTriangleVertexIndex = -1;
                
                const S32 targetTriangleIndexCount( targetVertex.m_TriangleIndices.Size() );
                for ( S32 k=0; k<targetTriangleIndexCount; k++ )
                {
                    const S32 targetTriangleIndex( targetVertex.m_TriangleIndices[k] );
                    LodTriangle& targetTriangle( m_Triangles[ targetTriangleIndex ] );
                    if ( sourceTriangle.m_pPrimitive == targetTriangle.m_pPrimitive )
                    {
                        const VF32 rcpTexCoordCount( Rcp( MakeVF32( sourceTriangle.m_TexCoordCount ) ) );
                        for ( S32 targetTriangleVertexIndex=0; targetTriangleVertexIndex<3; targetTriangleVertexIndex++ )
                        {
                            if ( targetTriangle.m_VertexIndex[ targetTriangleVertexIndex ] == targetVertexIndex )
                            {
                                VF32 rmsError( zero );
                                
                                BIZ_ASSERT( sourceTriangle.m_TexCoordCount == targetTriangle.m_TexCoordCount );
                                for ( S32 texCoordIndex = 0; texCoordIndex < sourceTriangle.m_TexCoordCount; texCoordIndex++ )
                                {
                                    const Vec4F32 t1( targetTexCoord[ texCoordIndex ] );
                                    const Vec4F32 t2( Load(&targetTriangle.m_TexCoord[ texCoordIndex ][ targetTriangleVertexIndex ]) );
                                    rmsError += DistanceSquared4( t1, t2 ) * m_Options.texCoordPriority[ texCoordIndex ];
                                    
                                    #if defined( USER_JDENTON )
                                    BIZ_ASSERT( Finite( rmsError ) );
                                    BIZ_ASSERT( rmsError < fltMax );
                                    BIZ_ASSERT( rmsError >= zero );
                                    #endif
                                }
                                
                                // NOTE: No projection for normal/tangent/binormal yet...
                                
                                rmsError += DistanceSquared4( Load(&sourceTriangle.m_Colour[ sourceTriangleVertexIndex ]),
                                                              Load(&targetTriangle.m_Colour[ targetTriangleVertexIndex ]) ) * m_Options.colourPriority;
                                
                                #if defined( USER_JDENTON )
                                if ( !Finite( rmsError ) )
                                {
                                    LOGDEBUG( g_Log, "sourceTriangle.m_Colour[%d] = ( %f, %f, %f, %f )", sourceTriangleVertexIndex,
                                        GetF32( X( sourceTriangle.m_Colour[ sourceTriangleVertexIndex ] ) ),
                                        GetF32( Y( sourceTriangle.m_Colour[ sourceTriangleVertexIndex ] ) ),
                                        GetF32( Z( sourceTriangle.m_Colour[ sourceTriangleVertexIndex ] ) ),
                                        GetF32( W( sourceTriangle.m_Colour[ sourceTriangleVertexIndex ] ) ) );
                                    LOGDEBUG( g_Log, "targetTriangle.m_Colour[%d] = ( %f, %f, %f, %f )", targetTriangleVertexIndex,
                                        GetF32( X( targetTriangle.m_Colour[ targetTriangleVertexIndex ] ) ),
                                        GetF32( Y( targetTriangle.m_Colour[ targetTriangleVertexIndex ] ) ),
                                        GetF32( Z( targetTriangle.m_Colour[ targetTriangleVertexIndex ] ) ),
                                        GetF32( W( targetTriangle.m_Colour[ targetTriangleVertexIndex ] ) ) );
                                }
                                #endif
                                
                                #if defined( USER_JDENTON )
                                BIZ_ASSERT( Finite( rmsError ) );
                                BIZ_ASSERT( rmsError < fltMax );
                                BIZ_ASSERT( rmsError >= zero );
                                #endif

                                rmsError += Square( one - Clamp( Dot3( Load(&sourceTriangle.m_Normal[ sourceTriangleVertexIndex ]),
                                                                       Load(&targetTriangle.m_Normal[ targetTriangleVertexIndex ]) ), zero, one ) ) * m_Options.normalPriority;
                                
                                #if defined( USER_JDENTON )
                                BIZ_ASSERT( Finite( rmsError ) );
                                BIZ_ASSERT( rmsError < fltMax );
                                BIZ_ASSERT( rmsError >= zero );
                                #endif
                                
                                rmsError += Square( one - Clamp( Dot3( Load(&sourceTriangle.m_Tangent[ sourceTriangleVertexIndex ]),
                                                                      Load(&targetTriangle.m_Tangent[ targetTriangleVertexIndex ]) ), zero, one ) ) * m_Options.tangentPriority;
                                
                                #if defined( USER_JDENTON )
                                BIZ_ASSERT( Finite( rmsError ) );
                                BIZ_ASSERT( rmsError < fltMax );
                                BIZ_ASSERT( rmsError >= zero );
                                #endif
                                
                                rmsError += Square( one - Clamp( Dot3( Load(&sourceTriangle.m_BiNormal[ sourceTriangleVertexIndex ]),
                                                                      Load(&targetTriangle.m_BiNormal[ targetTriangleVertexIndex ]) ), zero, one ) ) * m_Options.binormalPriority;

                                #if defined( USER_JDENTON )
                                BIZ_ASSERT( Finite( rmsError ) );
                                BIZ_ASSERT( rmsError < fltMax );
                                BIZ_ASSERT( rmsError >= zero );
                                #endif
                                
                                //rmsError = Sqrt( rmsError * rcpTexCoordCount ); // not required as it's just a scaling factor in a comparison...
                                
                                if ( leastError > rmsError )
                                {
                                    leastError = rmsError;
                                    bestTargetTriangleIndex = targetTriangleIndex;
                                    bestTargetTriangleVertexIndex = targetTriangleVertexIndex;
                                }
                            }
                        }
                    }
                }
                
                if ( bestTargetTriangleIndex >= 0 )
                {
                    LodTriangle& targetTriangle( m_Triangles[ bestTargetTriangleIndex ] );
                    
                    sourceTriangle.m_VBIndex[ sourceTriangleVertexIndex ]  = targetTriangle.m_VBIndex[ bestTargetTriangleVertexIndex ];
                    sourceTriangle.m_Colour[ sourceTriangleVertexIndex ]   = targetTriangle.m_Colour[ bestTargetTriangleVertexIndex ];
                    sourceTriangle.m_Normal[ sourceTriangleVertexIndex ]   = targetTriangle.m_Normal[ bestTargetTriangleVertexIndex ];
                    sourceTriangle.m_Tangent[ sourceTriangleVertexIndex ]  = targetTriangle.m_Tangent[ bestTargetTriangleVertexIndex ];
                    sourceTriangle.m_BiNormal[ sourceTriangleVertexIndex ] = targetTriangle.m_BiNormal[ bestTargetTriangleVertexIndex ];
                    
                    BIZ_ASSERT( sourceTriangle.m_TexCoordCount == targetTriangle.m_TexCoordCount );
                    for ( S32 texCoordIndex = 0; texCoordIndex < sourceTriangle.m_TexCoordCount; texCoordIndex++ )
                    {
                        sourceTriangle.m_TexCoord[ texCoordIndex ][ sourceTriangleVertexIndex ] = targetTriangle.m_TexCoord[ texCoordIndex ][ bestTargetTriangleVertexIndex ];
                    }
                }
                //#if defined( USER_JDENTON )
                //else BIZ_ASSERT_FAIL( "Failed to find targetTriangle!" );
                //#else
                else
				{
					if ( --g_nMaxMessages >= 0 )
						LOGWARN( g_Log, "Failed to find targetTriangle! [bestTargetTriangleIndex = %d]", bestTargetTriangleIndex );
				}
                //#endif
                
                foundSource = true;
                break;
            }
        }
        #if defined( USER_JDENTON )
        BIZ_ASSERT( foundSource );
        #endif
    }
}

void LodMesh::RemapVertex( const S32 sourceVertexIndex, const S32 targetVertexIndex )
{
    LodVertex& sourceVertex( m_Vertices[ sourceVertexIndex ] );
    
    // Remap any use of sourceVertexIndex in edges with targetVertexIndex
    const S32 edgeIndexCount( sourceVertex.m_EdgeIndices.Size() );
    for ( S32 i=0; i<edgeIndexCount; i++ )
    {
        LodEdge& edge( m_Edges[ sourceVertex.m_EdgeIndices[i] ] );
        for ( S32 j=0; j<2; j++ )
        {
            if ( edge.m_VertexIndex[j] == sourceVertexIndex )
                edge.m_VertexIndex[j] = targetVertexIndex;
        }
    }
    
    // Remap any use of sourceVertexIndex in triangles with targetVertexIndex
    const S32 triangleIndexCount( sourceVertex.m_TriangleIndices.Size() );
    for ( S32 i=0; i<triangleIndexCount; i++ )
    {
        LodTriangle& triangle( m_Triangles[ sourceVertex.m_TriangleIndices[i] ] );
        for ( S32 j=0; j<3; j++ )
        {
            if ( triangle.m_VertexIndex[j] == sourceVertexIndex )
                triangle.m_VertexIndex[j] = targetVertexIndex;
        }
    }
}

void LodMesh::RemapEdge( const S32 sourceEdgeIndex, const S32 targetEdgeIndex )
{
    LodEdge& sourceEdge( m_Edges[ sourceEdgeIndex ] );
    
    // Remap any use of sourceEdgeIndex in vertices with targetEdgeIndex
    for ( S32 i=0; i<2; i++ )
    {
        LodVertex& vertex( m_Vertices[ sourceEdge.m_VertexIndex[i] ] );
        const S32 edgeIndexCount( vertex.m_EdgeIndices.Size() );
        for ( S32 j=0; j<edgeIndexCount; j++ )
        {
            if ( vertex.m_EdgeIndices[j] == sourceEdgeIndex )
                vertex.m_EdgeIndices[j] = targetEdgeIndex;
        }
    }
    
    // Remap any use of sourceEdgeIndex in triangles with targetEdgeIndex
    const S32 triangleIndexCount( sourceEdge.m_TriangleIndices.Size() );
    for ( S32 i=0; i<triangleIndexCount; i++ )
    {
        LodTriangle& triangle( m_Triangles[ sourceEdge.m_TriangleIndices[i] ] );
        for ( S32 j=0; j<3; j++ )
        {
            if ( triangle.m_EdgeIndex[j] == sourceEdgeIndex )
                triangle.m_EdgeIndex[j] = targetEdgeIndex;
        }
    }
}

void LodMesh::RemapTriangle( const S32 sourceTriangleIndex, const S32 targetTriangleIndex )
{
    LodTriangle& sourceTriangle( m_Triangles[ sourceTriangleIndex ] );
    
    // Remap any use of sourceTriangleIndex in vertices with targetTriangleIndex
    for ( S32 i=0; i<3; i++ )
    {
        const S32 sourceTriangleVertexIndex( sourceTriangle.m_VertexIndex[i] );
        if ( sourceTriangleVertexIndex >= 0 ) // Ignore degenerate tris
        {
            LodVertex& vertex( m_Vertices[ sourceTriangleVertexIndex ] );
            const S32 triangleIndexCount( vertex.m_TriangleIndices.Size() );
            for ( S32 j=0; j<triangleIndexCount; j++ )
            {
                if ( vertex.m_TriangleIndices[j] == sourceTriangleIndex )
                    vertex.m_TriangleIndices[j] = targetTriangleIndex;
            }
        }
    }
    
    // Remap any use of sourceTriangleIndex in edges with targetTriangleIndex
    for ( S32 i=0; i<3; i++ )
    {
        const S32 sourceTriangleEdgeIndex( sourceTriangle.m_EdgeIndex[i] );
        if ( sourceTriangleEdgeIndex >= 0 ) // Ignore degenerate tris
        {
            LodEdge& edge( m_Edges[ sourceTriangleEdgeIndex ] );
            const S32 triangleIndexCount( edge.m_TriangleIndices.Size() );
            for ( S32 j=0; j<triangleIndexCount; j++ )
            {
                if ( edge.m_TriangleIndices[j] == sourceTriangleIndex )
                    edge.m_TriangleIndices[j] = targetTriangleIndex;
            }
        }
    }
}

void LodMesh::ReplaceVertexInAdjacentEdges( const S32 sourceVertexIndex, const S32 targetVertexIndex )
{
    #if defined( _DEBUG )
    //LOGDEBUG( g_Log, "LodMesh::ReplaceVertexInAdjacentEdges(): sourceVertexIndex = %d, targetVertexIndex = %d \n", sourceVertexIndex, targetVertexIndex );
    #endif
    
    LodVertex& sourceVertex( m_Vertices[ sourceVertexIndex ] );
    LodVertex& targetVertex( m_Vertices[ targetVertexIndex ] );
    const S32 edgeIndexCount( sourceVertex.m_EdgeIndices.Size() );
    for ( S32 i=0; i<edgeIndexCount; i++ )
    {
        const S32 edgeIndex( sourceVertex.m_EdgeIndices[i] );
        LodEdge& edge( m_Edges[ edgeIndex ] );
        for ( S32 j=0; j<2; j++ )
        {
            if ( edge.m_VertexIndex[j] == sourceVertexIndex )
            {
                // Replace this vertex's index with the target vertex index
                edge.m_VertexIndex[j] = targetVertexIndex;
                
                // Add edge's index to the target vertex
                if ( edge.m_VertexIndex[1-j] != targetVertexIndex )
                    targetVertex.AddEdgeIndex( edgeIndex );
                
                //break; // During edge collapse, some indices may be duplicated... 
            }
        }
    }
}

void LodMesh::ReplaceVertexInAdjacentTriangles( const S32 sourceVertexIndex, const S32 targetVertexIndex )
{
    #if defined( _DEBUG )
    //LOGDEBUG( g_Log, "LodMesh::ReplaceVertexInAdjacentTriangles(): sourceVertexIndex = %d, targetVertexIndex = %d \n", sourceVertexIndex, targetVertexIndex );
    #endif
    
    LodVertex& sourceVertex( m_Vertices[ sourceVertexIndex ] );
    LodVertex& targetVertex( m_Vertices[ targetVertexIndex ] );
    const S32 triangleIndexCount( sourceVertex.m_TriangleIndices.Size() );
    for ( S32 i=0; i<triangleIndexCount; i++ )
    {
        const S32 triangleIndex( sourceVertex.m_TriangleIndices[i] );
        LodTriangle& triangle( m_Triangles[ triangleIndex ] );
        for ( S32 j=0; j<3; j++ )
        {
            if ( triangle.m_VertexIndex[j] == sourceVertexIndex )
            {
                // Replace this vertex's index with the target vertex index
                triangle.m_VertexIndex[j] = targetVertexIndex;
                
                // Add triangle's index to the target vertex
                targetVertex.AddTriangleIndex( triangleIndex );
                
                //break; // During edge collapse, some indices may be duplicated... 
            }
        }
    }
}

void LodMesh::ReplaceEdgeInAdjacentTriangles( const S32 sourceEdgeIndex, const S32 targetEdgeIndex )
{
    #if defined( _DEBUG )
    //LOGDEBUG( g_Log, "LodMesh::ReplaceEdgeInAdjacentTriangles(): sourceEdgeIndex = %d, targetEdgeIndex = %d \n", sourceEdgeIndex, targetEdgeIndex );
    #endif
    
    LodEdge& sourceEdge( m_Edges[ sourceEdgeIndex ] );
    LodEdge& targetEdge( m_Edges[ targetEdgeIndex ] );
    const S32 triangleIndexCount( sourceEdge.m_TriangleIndices.Size() );
    for ( S32 i=0; i<triangleIndexCount; i++ )
    {
        const S32 triangleIndex( sourceEdge.m_TriangleIndices[i] );
        LodTriangle& triangle( m_Triangles[ triangleIndex ] );
        for ( S32 j=0; j<3; j++ )
        {
            if ( triangle.m_EdgeIndex[j] == sourceEdgeIndex )
            {
                // Replace this edge's index with the target edge index
                triangle.m_EdgeIndex[j] = targetEdgeIndex;
                
                // Add triangle's index to the target edge
                targetEdge.AddTriangleIndex( triangleIndex );
                
                //break; // During edge collapse, some indices may be duplicated... 
            }
        }
    }
}

void LodMesh::DeleteEdgeFromAdjacentVertices( const S32 edgeIndex )
{
    #if defined( _DEBUG )
    //LOGDEBUG( g_Log, "LodMesh::DeleteEdgeFromAdjacentVertices(): edgeIndex = %d\n", edgeIndex );
    #endif
    
    LodEdge& edge( m_Edges[ edgeIndex ] );
    for ( S32 i=0; i<2; i++ )
    {
        const S32 vertexIndex( edge.m_VertexIndex[i] );
        LodVertex& vertex( m_Vertices[ vertexIndex ] );
        for ( S32 j=0; j<vertex.m_EdgeIndices.Size(); j++ )
        {
            if ( vertex.m_EdgeIndices[j] == edgeIndex )
            {
                #if defined( _DEBUG )
                //LOGDEBUG( g_Log, "deleting edge %d from vertex %d\n", edgeIndex, edge.m_VertexIndex[i] );
                #endif
                vertex.m_EdgeIndices.EraseByIndexNonOrdered( j );
                break;
            }
        }
    }
}

void LodMesh::DeleteTriangleFromAdjacentVertices( const S32 triangleIndex )
{
    #if defined( _DEBUG )
    //LOGDEBUG( g_Log, "LodMesh::DeleteTriangleFromAdjacentVertices(): triangleIndex = %d\n", triangleIndex );
    #endif
    
    LodTriangle& triangle( m_Triangles[ triangleIndex ] );
    for ( S32 i=0; i<3; i++ )
    {
        const S32 vertexIndex( triangle.m_VertexIndex[i] );
        if ( vertexIndex >= 0 ) // degenerate triangles have verteIndexes of -1
        {
            LodVertex& vertex( m_Vertices[ vertexIndex ] );
            Btl::Vector<S32>& vertexTriangleIndices( vertex.m_TriangleIndices );
            for ( S32 j=0; j<vertexTriangleIndices.Size(); j++ )
            {
                if ( vertexTriangleIndices[j] == triangleIndex )
                {
                    #if defined( _DEBUG )
                    //LOGDEBUG( g_Log, "deleting triangle %d from vertex %d\n", triangleIndex, vertexIndex );
                    #endif
                    vertexTriangleIndices.EraseByIndexNonOrdered( j );
                    break;
                }
            }
        }
    }
}

void LodMesh::DeleteTriangleFromAdjacentEdges( const S32 triangleIndex )
{
    #if defined( _DEBUG )
    //LOGDEBUG( g_Log, "LodMesh::DeleteTriangleFromAdjacentEdges(): triangleIndex = %d\n", triangleIndex );
    #endif
    
    LodTriangle& triangle( m_Triangles[ triangleIndex ] );
    for ( S32 i=0; i<3; i++ )
    {
        //bool found = false;
        const S32 edgeIndex( triangle.m_EdgeIndex[i] );
        if ( edgeIndex >= 0 ) // degenerate triangles have edgeIndexes of -1
        {
            LodEdge& edge( m_Edges[ edgeIndex ] );
            Btl::Vector<S32>& edgeTriangleIndices( edge.m_TriangleIndices );
            for ( S32 j=0; j<edgeTriangleIndices.Size(); j++ )
            {
                if ( edgeTriangleIndices[j] == triangleIndex )
                {
                    #if defined( _DEBUG )
                    //LOGDEBUG( g_Log, "deleting triangle %d from edge %d\n", triangleIndex, edgeIndex );
                    #endif
                    edgeTriangleIndices.EraseByIndexNonOrdered( j );
                    //found = true;
                    break;
                }
            }
        }
        //BIZ_ASSERT( found );
    }
}

/// LodMesh: Delete duplicate edges connected to vertex
void LodMesh::DeleteDuplicateEdgesAdjacentToVertex( const S32 vertexIndex )
{
    #if defined( _DEBUG )
    //LOGDEBUG( g_Log, "LodMesh::DeleteDuplicateEdgesAdjacentToVertex(): vertexIndex = %d\n", vertexIndex );
    #endif
    
    LodVertex& vertex( m_Vertices[ vertexIndex ] );
    for ( S32 i=0; i<vertex.m_EdgeIndices.Size(); i++ )
    {
        const S32 edgeIndex1( vertex.m_EdgeIndices[i] );
        LodEdge edge1( m_Edges[ edgeIndex1 ] );
        
        for ( S32 j=i+1; j<vertex.m_EdgeIndices.Size(); j++ )
        {
            const S32 edgeIndex2( vertex.m_EdgeIndices[j] );
            LodEdge edge2( m_Edges[ edgeIndex2 ] );
            
            if ( ( ( edge1.m_VertexIndex[0] == edge2.m_VertexIndex[0] ) &&
                   ( edge1.m_VertexIndex[1] == edge2.m_VertexIndex[1] ) ) ||
                 ( ( edge1.m_VertexIndex[0] == edge2.m_VertexIndex[1] ) &&
                   ( edge1.m_VertexIndex[1] == edge2.m_VertexIndex[0] ) ) )
            {
                // *** TEMP: just in case a vertex contains the same edge twice ***
                //BIZ_ASSERT( edgeIndex1 != edgeIndex2 );
                if ( edgeIndex1 != edgeIndex2 )
                {
                    #if defined( _DEBUG )
                    //LOGDEBUG( g_Log, "LodMesh::DeleteDuplicateEdgesAdjacentToVertex(): deleting duplicate edge %d\n", edgeIndex2 );
                    #endif
                    ReplaceEdgeInAdjacentTriangles( edgeIndex2, edgeIndex1 );
                    DeleteEdgeFromAdjacentVertices( edgeIndex2 );
                    DeleteEdge( edgeIndex2 );
                    break;
                }
            }
        }
    }
}

void LodMesh::DeleteDegenerateTrianglesAdjacentToEdge( const S32 edgeIndex )
{
    #if defined( _DEBUG )
    //LOGDEBUG( g_Log, "LodMesh::DeleteDegenerateTrianglesAdjacentToEdge(): edgeIndex = %d\n", edgeIndex );
    #endif
    
    LodEdge& edge( m_Edges[ edgeIndex ] );
    
    S32 i = 0;
    while ( i < edge.m_TriangleIndices.Size() )
    {
        const S32 triangleIndex( edge.m_TriangleIndices[i] );
        LodTriangle& triangle( m_Triangles[ triangleIndex ] );
        if ( ( triangle.m_EdgeIndex[0] == triangle.m_EdgeIndex[1] ) ||
             ( triangle.m_EdgeIndex[0] == triangle.m_EdgeIndex[2] ) ||
             ( triangle.m_EdgeIndex[1] == triangle.m_EdgeIndex[2] ) )
        {
            // NOTE: We don't increment 'i' if we delete the triangle, because
            // the delete operation works by swapping the last element of the vector with
            // the element to be deleted and then decrementing the size of the vector!
            #if defined( _DEBUG )
            //LOGDEBUG( g_Log, "LodMesh::DeleteDegenerateTrianglesAdjacentToEdge(): deleting triangle %d\n", triangleIndex );
            #endif
            DeleteTriangleFromAdjacentVertices( triangleIndex );
            DeleteTriangleFromAdjacentEdges( triangleIndex );
            DeleteTriangle( triangleIndex );
        }
        else
        {
            i++;
        }
    }
}

void LodMesh::AnalyseVertexPhase1( const S32 vertexIndex )
{
    LodVertex& vertex( m_Vertices[ vertexIndex ] );
    vertex.m_Flags = E_VertexFlag_None;
}

void LodMesh::AnalyseVertexPhase2( const S32 vertexIndex )
{
    using namespace Maths;
    
    LodVertex& vertex( m_Vertices[ vertexIndex ] );
    
    for ( S32 i=0; i<vertex.m_EdgeIndices.Size(); i++ )
    {
        LodEdge& edge( m_Edges[ vertex.m_EdgeIndices[i] ] );
        if ( edge.m_Flags & E_EdgeFlag_MaterialBoundary )
        {
            vertex.m_Flags |= E_VertexFlag_MaterialBoundary;
        }
        if ( edge.m_Flags & E_EdgeFlag_TextureDiscontinuity )
        {
            vertex.m_Flags |= E_VertexFlag_TextureDiscontinuity;
        }
        if ( edge.m_Flags & E_EdgeFlag_Outline )
        {
            vertex.m_Flags |= E_VertexFlag_Outline;
        }
    }
    
    //LOGINFO( g_Log, "Vertex m_TextureDensityRatio = %f\n", GetF32( vertex.m_TextureDensityRatio ) );
}

void LodMesh::AnalyseEdge( const S32 edgeIndex )
{
    using namespace Maths;
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 half( Const<VF32>::Half() );
    const VF32 one( Const<VF32>::One() );
    const VF32 nearlyZero( Const<VF32>::Frac<1,10000>() );
    const VF32 colourDiscontinuityThresholdSquared( Square( Load<VF32>( &m_Options.colourDiscontinuityThreshold ) ) );
    const VF32 textureDiscontinuityThresholdSquared( Square( Load<VF32>( &m_Options.textureDiscontinuityThreshold ) ) );
    const VF32 normalDiscontinuityThreshold( Load<VF32>( &m_Options.normalDiscontinuityThreshold ) );
    const VF32 tangentDiscontinuityThreshold( Load<VF32>( &m_Options.tangentDiscontinuityThreshold ) );
    const VF32 binormalDiscontinuityThreshold( Load<VF32>( &m_Options.binormalDiscontinuityThreshold ) );
    const VF32 convexThreshold( Load<VF32>( &m_Options.convexThreshold ) );
    
    LodEdge& edge( m_Edges[ edgeIndex ] );
    edge.m_Flags = E_EdgeFlag_None;
    
    const S32 vertexIndex0( edge.m_VertexIndex[0] );
    const S32 vertexIndex1( edge.m_VertexIndex[1] );
    
    LodVertex& vertex0( m_Vertices[ vertexIndex0 ] );
    LodVertex& vertex1( m_Vertices[ vertexIndex1 ] );
    
    const S32 vertex0_TriangleIndices( vertex0.m_TriangleIndices.Size() );
    const S32 vertex1_TriangleIndices( vertex1.m_TriangleIndices.Size() );
    
    edge.m_Normal = Normalise( vertex1.m_Position - vertex0.m_Position );
    
    // Check if vertex0 can be collapsed to vertex1
    bool canCollapseVertex0ToVertex1 = true;
    
    // Every primitive that uses vertex0 must also use vertex1
    if ( canCollapseVertex0ToVertex1 )
    {
        for ( S32 i=0; i<vertex0_TriangleIndices; i++ )
        {
            const LodTriangle& triangle0( m_Triangles[ vertex0.m_TriangleIndices[i] ] );
            bool found = false;
            for ( S32 j=0; j<vertex1_TriangleIndices; j++ )
            {
                const LodTriangle& triangle1( m_Triangles[ vertex1.m_TriangleIndices[j] ] );
                if ( triangle0.m_pPrimitive == triangle1.m_pPrimitive )
                {
                    found = true;
                    break;
                }
            }
            if ( !found )
            {
                canCollapseVertex0ToVertex1 = false;
                break;
            }
        }
    }
    
    // Don't collapse vertex0 to vertex1 if it will flip any triangle normals
    // (except for triangles with a vertex0-vertex1 edge )
    if ( canCollapseVertex0ToVertex1 )
    {
        for ( S32 i=0; i<vertex0_TriangleIndices; i++ )
        {
            const LodTriangle& triangle( m_Triangles[ vertex0.m_TriangleIndices[i] ] );
            
            S32 triVertexIndex0( triangle.m_VertexIndex[0] );
            S32 triVertexIndex1( triangle.m_VertexIndex[1] );
            S32 triVertexIndex2( triangle.m_VertexIndex[2] );
            
            // Ignore triangles with a vertex0-vertex1 edge
            if ( ( triVertexIndex0 != vertexIndex1 ) &&
                 ( triVertexIndex1 != vertexIndex1 ) &&
                 ( triVertexIndex2 != vertexIndex1 ) )
            {
                const LodVertex& oldTriVertex0( m_Vertices[ triVertexIndex0 ] );
                const LodVertex& oldTriVertex1( m_Vertices[ triVertexIndex1 ] );
                const LodVertex& oldTriVertex2( m_Vertices[ triVertexIndex2 ] );
                
                if ( triVertexIndex0 == vertexIndex0 ) triVertexIndex0 = vertexIndex1;
                if ( triVertexIndex1 == vertexIndex0 ) triVertexIndex1 = vertexIndex1;
                if ( triVertexIndex2 == vertexIndex0 ) triVertexIndex2 = vertexIndex1;
                
                const LodVertex& newTriVertex0( m_Vertices[ triVertexIndex0 ] );
                const LodVertex& newTriVertex1( m_Vertices[ triVertexIndex1 ] );
                const LodVertex& newTriVertex2( m_Vertices[ triVertexIndex2 ] );
                
                const Maths::Point3  old_p0( oldTriVertex0.m_Position );
                const Maths::Point3  old_p1( oldTriVertex1.m_Position );
                const Maths::Point3  old_p2( oldTriVertex2.m_Position );
                
                const Maths::Point3  new_p0( newTriVertex0.m_Position );
                const Maths::Point3  new_p1( newTriVertex1.m_Position );
                const Maths::Point3  new_p2( newTriVertex2.m_Position );
                
                const Maths::Vector3 old_vector( ( old_p1 - old_p0 ) ^ ( old_p2 - old_p0 ) );
                const Maths::Vector3 new_vector( ( new_p1 - new_p0 ) ^ ( new_p2 - new_p0 ) );
                
                const Maths::Normal3 old_normal( Normalise( old_vector ) );
                const Maths::Normal3 new_normal( Normalise( new_vector ) );
                
                //if ( ( old_normal | new_normal ) < zero )
                //if ( ( old_normal | new_normal ) < -nearlyZero )
                if ( ( old_normal | new_normal ) < nearlyZero )
                {
                    canCollapseVertex0ToVertex1 = false;
                    break;
                }
            }
        }
    }
    
    if ( /* canCollapseVertex0ToVertex1 && */ m_Options.preserveConvexHullIntegrity )
    {
        for ( S32 i=0; canCollapseVertex0ToVertex1 && ( i < vertex0_TriangleIndices ); i++ )
        {
            const LodTriangle& triangle( m_Triangles[ vertex0.m_TriangleIndices[i] ] );
            
            S32 triVertexIndex0( triangle.m_VertexIndex[0] );
            S32 triVertexIndex1( triangle.m_VertexIndex[1] );
            S32 triVertexIndex2( triangle.m_VertexIndex[2] );
            
            // Ignore triangles with a vertex0-vertex1 edge
            if ( ( triVertexIndex0 != vertexIndex1 ) &&
                 ( triVertexIndex1 != vertexIndex1 ) &&
                 ( triVertexIndex2 != vertexIndex1 ) )
            {
                if ( triVertexIndex0 == vertexIndex0 ) triVertexIndex0 = vertexIndex1;
                if ( triVertexIndex1 == vertexIndex0 ) triVertexIndex1 = vertexIndex1;
                if ( triVertexIndex2 == vertexIndex0 ) triVertexIndex2 = vertexIndex1;
                
                const LodVertex& triVertex0( m_Vertices[ triVertexIndex0 ] );
                const LodVertex& triVertex1( m_Vertices[ triVertexIndex1 ] );
                const LodVertex& triVertex2( m_Vertices[ triVertexIndex2 ] );
                
                const Maths::Point3 p0( triVertex0.m_Position );
                const Maths::Point3 p1( triVertex1.m_Position );
                const Maths::Point3 p2( triVertex2.m_Position );
                
                const Plane plane( MakePlane( p0, p1, p2 ) );
                
                S32 triVertexIndex[3] = { triVertexIndex0, triVertexIndex1, triVertexIndex2 };
                
                for ( S32 j=0; canCollapseVertex0ToVertex1 && (j<3); j++ )
                {
                    const S32 index_v1( triVertexIndex[j] );
                    const LodVertex& v1( m_Vertices[ index_v1 ] );
                    
                    for ( S32 k=0; k<v1.m_EdgeIndices.Size(); k++ )
                    {
                        const LodEdge& edge( m_Edges[ v1.m_EdgeIndices[k] ] );
                        const S32 index_v2( edge.OtherVertexIndex( index_v1 ) );
                        const LodVertex& v2( m_Vertices[ index_v2 ] );
                        
                        //LOGINFO( g_Log, "distance01 = %+8.3f", GetF32( Distance( plane, v2.m_Position ) ) );
                        
                        if ( Distance( plane, v2.m_Position ) > convexThreshold )
                        {
                            canCollapseVertex0ToVertex1 = false;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    if ( canCollapseVertex0ToVertex1 )
        edge.m_Flags |= E_EdgeFlag_CanCollapseVertex0ToVertex1;
    
    // Check if vertex1 can be collapsed to vertex0
    bool canCollapseVertex1ToVertex0 = true;
    
    // Every primitive that uses vertex1 must also use vertex0
    if ( canCollapseVertex1ToVertex0 )
    {
        for ( S32 i=0; i<vertex1_TriangleIndices; i++ )
        {
            const LodTriangle& triangle1( m_Triangles[ vertex1.m_TriangleIndices[i] ] );
            bool found = false;
            for ( S32 j=0; j<vertex0_TriangleIndices; j++ )
            {
                const LodTriangle& triangle0( m_Triangles[ vertex0.m_TriangleIndices[j] ] );
                if ( triangle1.m_pPrimitive == triangle0.m_pPrimitive )
                {
                    found = true;
                    break;
                }
            }
            if ( !found )
            {
                canCollapseVertex1ToVertex0 = false;
                break;
            }
        }
    }
    
    // Don't collapse vertex1 to vertex0 if it will flip any triangle normals
    // (except for triangles with a vertex0-vertex1 edge )
    if ( canCollapseVertex1ToVertex0 )
    {
        for ( S32 i=0; i<vertex1_TriangleIndices; i++ )
        {
            const LodTriangle& triangle( m_Triangles[ vertex1.m_TriangleIndices[i] ] );
            
            S32 triVertexIndex0( triangle.m_VertexIndex[0] );
            S32 triVertexIndex1( triangle.m_VertexIndex[1] );
            S32 triVertexIndex2( triangle.m_VertexIndex[2] );
            
            // Ignore triangles with a vertex0-vertex1 edge
            if ( ( triVertexIndex0 != vertexIndex0 ) &&
                 ( triVertexIndex1 != vertexIndex0 ) &&
                 ( triVertexIndex2 != vertexIndex0 ) )
            {
                const LodVertex& oldTriVertex0( m_Vertices[ triVertexIndex0 ] );
                const LodVertex& oldTriVertex1( m_Vertices[ triVertexIndex1 ] );
                const LodVertex& oldTriVertex2( m_Vertices[ triVertexIndex2 ] );
                
                if ( triVertexIndex0 == vertexIndex1 ) triVertexIndex0 = vertexIndex0;
                if ( triVertexIndex1 == vertexIndex1 ) triVertexIndex1 = vertexIndex0;
                if ( triVertexIndex2 == vertexIndex1 ) triVertexIndex2 = vertexIndex0;
                
                const LodVertex& newTriVertex0( m_Vertices[ triVertexIndex0 ] );
                const LodVertex& newTriVertex1( m_Vertices[ triVertexIndex1 ] );
                const LodVertex& newTriVertex2( m_Vertices[ triVertexIndex2 ] );
                
                const Maths::Point3  old_p0( oldTriVertex0.m_Position );
                const Maths::Point3  old_p1( oldTriVertex1.m_Position );
                const Maths::Point3  old_p2( oldTriVertex2.m_Position );
                
                const Maths::Point3  new_p0( newTriVertex0.m_Position );
                const Maths::Point3  new_p1( newTriVertex1.m_Position );
                const Maths::Point3  new_p2( newTriVertex2.m_Position );
                
                const Maths::Vector3 old_vector( ( old_p1 - old_p0 ) ^ ( old_p2 - old_p0 ) );
                const Maths::Vector3 new_vector( ( new_p1 - new_p0 ) ^ ( new_p2 - new_p0 ) );
                
                const Maths::Normal3 old_normal( Normalise( old_vector ) );
                const Maths::Normal3 new_normal( Normalise( new_vector ) );
                
                //if ( ( old_normal | new_normal ) < zero )
                //if ( ( old_normal | new_normal ) < -nearlyZero )
                if ( ( old_normal | new_normal ) < nearlyZero )
                {
                    canCollapseVertex1ToVertex0 = false;
                    break;
                }
            }
        }
    }
    
    if ( /* canCollapseVertex1ToVertex0 && */ m_Options.preserveConvexHullIntegrity )
    {
        for ( S32 i=0; canCollapseVertex1ToVertex0 && ( i < vertex1_TriangleIndices ); i++ )
        {
            const LodTriangle& triangle( m_Triangles[ vertex1.m_TriangleIndices[i] ] );
            
            S32 triVertexIndex0( triangle.m_VertexIndex[0] );
            S32 triVertexIndex1( triangle.m_VertexIndex[1] );
            S32 triVertexIndex2( triangle.m_VertexIndex[2] );
            
            // Ignore triangles with a vertex0-vertex1 edge
            if ( ( triVertexIndex0 != vertexIndex0 ) &&
                 ( triVertexIndex1 != vertexIndex0 ) &&
                 ( triVertexIndex2 != vertexIndex0 ) )
            {
                if ( triVertexIndex0 == vertexIndex1 ) triVertexIndex0 = vertexIndex0;
                if ( triVertexIndex1 == vertexIndex1 ) triVertexIndex1 = vertexIndex0;
                if ( triVertexIndex2 == vertexIndex1 ) triVertexIndex2 = vertexIndex0;
                
                const LodVertex& triVertex0( m_Vertices[ triVertexIndex0 ] );
                const LodVertex& triVertex1( m_Vertices[ triVertexIndex1 ] );
                const LodVertex& triVertex2( m_Vertices[ triVertexIndex2 ] );
                
                const Maths::Point3 p0( triVertex0.m_Position );
                const Maths::Point3 p1( triVertex1.m_Position );
                const Maths::Point3 p2( triVertex2.m_Position );
                
                const Plane plane( MakePlane( p0, p1, p2 ) );
                
                S32 triVertexIndex[3] = { triVertexIndex0, triVertexIndex1, triVertexIndex2 };
                
                for ( S32 j=0; canCollapseVertex1ToVertex0 && (j<3); j++ )
                {
                    const S32 index_v1( triVertexIndex[j] );
                    const LodVertex& v1( m_Vertices[ index_v1 ] );
                    
                    for ( S32 k=0; k<v1.m_EdgeIndices.Size(); k++ )
                    {
                        const LodEdge& edge( m_Edges[ v1.m_EdgeIndices[k] ] );
                        const S32 index_v2( edge.OtherVertexIndex( index_v1 ) );
                        const LodVertex& v2( m_Vertices[ index_v2 ] );
                        
                        //LOGINFO( g_Log, "distance10 = %+8.3f", GetF32( Distance( plane, v2.m_Position ) ) );
                        
                        if ( Distance( plane, v2.m_Position ) > convexThreshold )
                        {
                            canCollapseVertex1ToVertex0 = false;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    if ( canCollapseVertex1ToVertex0 )
        edge.m_Flags |= E_EdgeFlag_CanCollapseVertex1ToVertex0;
    
    //edge.m_Sharpness = 0.0f;
    VF32 edgeSharpness( zero );
    
    for ( S32 i=0; i<edge.m_TriangleIndices.Size(); i++ )
    {
        const LodTriangle& triangle1( m_Triangles[ edge.m_TriangleIndices[i] ] );
        
        S32 tri1_vertex1( 0 );
        S32 tri1_vertex2( 0 );
        if      ( triangle1.m_EdgeIndex[0] == edge.m_Index ) { tri1_vertex1 = 0; tri1_vertex2 = 1; }
        else if ( triangle1.m_EdgeIndex[1] == edge.m_Index ) { tri1_vertex1 = 1; tri1_vertex2 = 2; }
        else if ( triangle1.m_EdgeIndex[2] == edge.m_Index ) { tri1_vertex1 = 2; tri1_vertex2 = 0; }
        else BIZ_ASSERT_FAIL( "triangle1 does not contain edge!\n" );
        
        for ( S32 j=i+1; j<edge.m_TriangleIndices.Size(); j++ )
        {
            const LodTriangle& triangle2( m_Triangles[ edge.m_TriangleIndices[j] ] );
            
            S32 tri2_vertex1( 0 );
            S32 tri2_vertex2( 0 );
            if      ( triangle2.m_EdgeIndex[0] == edge.m_Index ) { tri2_vertex1 = 1; tri2_vertex2 = 0; }
            else if ( triangle2.m_EdgeIndex[1] == edge.m_Index ) { tri2_vertex1 = 2; tri2_vertex2 = 1; }
            else if ( triangle2.m_EdgeIndex[2] == edge.m_Index ) { tri2_vertex1 = 0; tri2_vertex2 = 2; }
            else BIZ_ASSERT_FAIL( "triangle2 does not contain edge!\n" );

            if ( triangle1.m_pPrimitive == triangle2.m_pPrimitive )
            {
                if ( ( DistanceSquared4( Load(&triangle1.m_Colour[ tri1_vertex1 ]), Load(&triangle2.m_Colour[ tri2_vertex1 ]) ) >= colourDiscontinuityThresholdSquared ) ||
                     ( DistanceSquared4( Load(&triangle1.m_Colour[ tri1_vertex2 ]), Load(&triangle2.m_Colour[ tri2_vertex2 ]) ) >= colourDiscontinuityThresholdSquared ) )
                {
                    edge.m_Flags |= E_EdgeFlag_ColourDiscontinuity;
                }

				for ( S32 t=0; t<LodTriangle::m_MaxTexCoordCount; t++ )
                {
                    if ( ( DistanceSquared3( Load(&triangle1.m_TexCoord[t][ tri1_vertex1 ]), Load(&triangle2.m_TexCoord[t][ tri2_vertex1 ]) ) >= textureDiscontinuityThresholdSquared ) ||
                         ( DistanceSquared3( Load(&triangle1.m_TexCoord[t][ tri1_vertex2 ]), Load(&triangle2.m_TexCoord[t][ tri2_vertex2 ]) ) >= textureDiscontinuityThresholdSquared ) )
                    {
                        edge.m_Flags |= E_EdgeFlag_TextureDiscontinuity;
                        break;
                    }
				}

				if (m_Options.applyGradientPenalties)
				{
					LodTriangleVertexGradients gt1v1, gt2v1, gt1v2, gt2v2;
					CalculateGradients(triangle1, tri1_vertex1, &gt1v1);
					CalculateGradients(triangle1, tri1_vertex2, &gt1v2);
					CalculateGradients(triangle2, tri2_vertex1, &gt2v1);
					CalculateGradients(triangle2, tri2_vertex2, &gt2v2);

					for (int i = 0; i < 4; i++)
					{
						if ( ( DistanceSquared( gt1v1.colour[i], gt2v1.colour[i] ) >= colourDiscontinuityThresholdSquared ) ||
							( DistanceSquared( gt1v2.colour[i], gt2v2.colour[i] ) >= colourDiscontinuityThresholdSquared ) )
						{
							edge.m_Flags |= E_EdgeFlag_ColourDiscontinuity;
						}
					}
	                
					for ( S32 t=0; t<LodTriangle::m_MaxTexCoordCount; t++ )
					{
						for (int i = 0; i < 4; i++)
						{
							if ( ( DistanceSquared( gt1v1.texCoord[t][i], gt2v1.texCoord[t][i] ) >= textureDiscontinuityThresholdSquared ) ||
								( DistanceSquared( gt1v2.texCoord[t][i], gt2v2.texCoord[t][i] ) >= textureDiscontinuityThresholdSquared ) )
							{
								edge.m_Flags |= E_EdgeFlag_TextureDiscontinuity;
							}
						}
					}
				}
            }
            else
            {
                edge.m_Flags |= E_EdgeFlag_MaterialBoundary;
            }
            
            const VF32 sharpness( ( one - ( triangle1.m_SurfaceNormal | triangle2.m_SurfaceNormal ) ) * half ); // mapped into range [0,1]
            //edge.m_Sharpness = Max( edge.m_Sharpness, sharpness );
            edgeSharpness = Max( edgeSharpness, sharpness );
        }
    }
    
    Store( edgeSharpness, &edge.m_Sharpness );
    
    if ( edge.m_TriangleIndices.Size() == 1 )
    {
        edge.m_Flags |= E_EdgeFlag_Outline;
    }
    
    edge.SetEdgeLength( Distance( vertex0.m_Position, vertex1.m_Position ) );
}

void LodMesh::AnalyseTriangle( const S32 triangleIndex )
{
    using namespace Maths;
    
    LodTriangle& triangle( m_Triangles[ triangleIndex ] );
    
    LodVertex& vertex0( m_Vertices[ triangle.m_VertexIndex[0] ] );
    LodVertex& vertex1( m_Vertices[ triangle.m_VertexIndex[1] ] );
    LodVertex& vertex2( m_Vertices[ triangle.m_VertexIndex[2] ] );
    
    const Maths::Point3 p0( vertex0.m_Position );
    const Maths::Point3 p1( vertex1.m_Position );
    const Maths::Point3 p2( vertex2.m_Position );
    
    const Maths::Vector3 cp( ( p1 - p0 ) ^ ( p2 - p0 ) );
    const Maths::Normal3 normal( Normalise( cp ) );
    const VF32           area( Length( cp ) * Const<VF32>::Half() );
    const VF32           rcpArea( Rcp( area ) );
    const VF32           zero( Const<VF32>::Zero() );
    
    triangle.m_SurfaceNormal = normal;
    /*triangle.m_SurfaceArea = area;
    
    VF32 maxTextureDensity( zero );
    for ( S32 i=0; i<triangle.m_TexCoordCount; i++ )
    {
        const Maths::Vector3 v0( MakeVector3( triangle.m_TexCoord[i][0] ) );
        const Maths::Vector3 v1( MakeVector3( triangle.m_TexCoord[i][1] ) );
        const Maths::Vector3 v2( MakeVector3( triangle.m_TexCoord[i][2] ) );
        const Maths::Vector3 cp( ( v1 - v0 ) ^ ( v2 - v0 ) );
        const VF32           textureArea( Length( cp ) * Const<VF32>::Half() );
        const VF32           textureDensity( textureArea * rcpArea );
        maxTextureDensity = Max( maxTextureDensity, textureDensity );
    }
    
    triangle.m_TextureDensity = maxTextureDensity;*/
    
    //LOGINFO( g_Log, "Triangle m_TextureDensity = %f\n", GetF32( maxTextureDensity ) );
}

void LodMesh::AnalyseMesh()
{
    // Analyse vertices phase 1
    const S32 vertexCount( m_Vertices.size() );
    for ( S32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++ )
    {
        AnalyseVertexPhase1( vertexIndex );
    }
    
    // Analyse triangles
    const S32 triangleCount( m_Triangles.size() );
    for ( S32 triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++ )
    {
        AnalyseTriangle( triangleIndex );
    }
    
    // Analyse edges
    const S32 edgeCount( m_Edges.size() );
    for ( S32 edgeIndex = 0; edgeIndex < edgeCount; edgeIndex++ )
    {
        AnalyseEdge( edgeIndex );
    }
    
    // Analyse vertices phase 2
    for ( S32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++ )
    {
        AnalyseVertexPhase2( vertexIndex );
    }
    
    // Calculate edge collapse scores
    for ( S32 edgeIndex = 0; edgeIndex < edgeCount; edgeIndex++ )
    {
        LodEdge& edge( m_Edges[ edgeIndex ] );
        
        edge.m_EdgeCollapseScoreVertex0ToVertex1 = -1.0f;
        edge.m_EdgeCollapseScoreVertex1ToVertex0 = -1.0f;
        if ( edge.m_Flags & E_EdgeFlag_CanCollapseVertex0ToVertex1 )
        {
            Store( CalculateEdgeCollapseScore( edge.m_VertexIndex[0], edge.m_VertexIndex[1] ), &edge.m_EdgeCollapseScoreVertex0ToVertex1 );
        }
        if ( edge.m_Flags & E_EdgeFlag_CanCollapseVertex1ToVertex0 )
        {
            Store( CalculateEdgeCollapseScore( edge.m_VertexIndex[1], edge.m_VertexIndex[0] ), &edge.m_EdgeCollapseScoreVertex1ToVertex0 );
        }
    }
}

void LodMesh::AnalyseMeshAdjacentToVertex( const S32 vertexIndex )
{
    LodVertex& vertex( m_Vertices[ vertexIndex ] );
    
    AnalyseVertexPhase1( vertexIndex );
    
    // Analyse other vertices on edges connected to this vertex
    for ( S32 i=0; i<vertex.m_EdgeIndices.Size(); i++ )
    {
        const S32 edgeIndex( vertex.m_EdgeIndices[i] );
        LodEdge& edge( m_Edges[ edgeIndex ] );
        AnalyseVertexPhase1( edge.OtherVertexIndex( vertexIndex ) );
    }
    
    // Analyse triangles connected to this vertex
    for ( S32 i=0; i<vertex.m_TriangleIndices.Size(); i++ )
    {
        const S32 triangleIndex( vertex.m_TriangleIndices[i] );
        AnalyseTriangle( triangleIndex );
    }
    
    // Analyse edges connected to this vertex
    for ( S32 i=0; i<vertex.m_EdgeIndices.Size(); i++ )
    {
        const S32 edgeIndex1( vertex.m_EdgeIndices[i] );
        LodEdge& edge1( m_Edges[ edgeIndex1 ] );
        AnalyseEdge( edgeIndex1 );
        edge1.m_Flags |= E_EdgeFlag_RecalculateEdgeCollapseScores;
        
        LodVertex& vertex2( m_Vertices[ edge1.OtherVertexIndex( vertexIndex ) ] );
        for ( S32 i=0; i<vertex2.m_EdgeIndices.Size(); i++ )
        {
            const S32 edgeIndex2( vertex2.m_EdgeIndices[i] );
            LodEdge& edge2( m_Edges[ edgeIndex2 ] );
            edge2.m_Flags |= E_EdgeFlag_Reanalyse;
        }
        for ( S32 i=0; i<vertex2.m_EdgeIndices.Size(); i++ )
        {
            const S32 edgeIndex2( vertex2.m_EdgeIndices[i] );
            LodEdge& edge2( m_Edges[ edgeIndex2 ] );
            if ( edge2.m_Flags & E_EdgeFlag_Reanalyse )
            {
                edge2.m_Flags &= ~E_EdgeFlag_Reanalyse;
                AnalyseEdge( edgeIndex2 );
                edge2.m_Flags |= E_EdgeFlag_RecalculateEdgeCollapseScores;
            }
        }
    }
    
    AnalyseVertexPhase2( vertexIndex );
    
    // Analyse other vertices on edges connected to this vertex
    for ( S32 i=0; i<vertex.m_EdgeIndices.Size(); i++ )
    {
        const S32 edgeIndex( vertex.m_EdgeIndices[i] );
        LodEdge& edge( m_Edges[ edgeIndex ] );
        AnalyseVertexPhase2( edge.OtherVertexIndex( vertexIndex ) );
    }
    
    // Calculate edge collapse scores
    for ( S32 i=0; i<vertex.m_EdgeIndices.Size(); i++ )
    {
        const S32 edgeIndex1( vertex.m_EdgeIndices[i] );
        LodEdge& edge1( m_Edges[ edgeIndex1 ] );
        
        if ( edge1.m_Flags & E_EdgeFlag_RecalculateEdgeCollapseScores )
        {
            edge1.m_Flags &= ~E_EdgeFlag_RecalculateEdgeCollapseScores;
            edge1.m_EdgeCollapseScoreVertex0ToVertex1 = -1.0f;
            edge1.m_EdgeCollapseScoreVertex1ToVertex0 = -1.0f;
            if ( edge1.m_Flags & E_EdgeFlag_CanCollapseVertex0ToVertex1 )
            {
                Store( CalculateEdgeCollapseScore( edge1.m_VertexIndex[0], edge1.m_VertexIndex[1] ), &edge1.m_EdgeCollapseScoreVertex0ToVertex1 );
            }
            if ( edge1.m_Flags & E_EdgeFlag_CanCollapseVertex1ToVertex0 )
            {
                Store( CalculateEdgeCollapseScore( edge1.m_VertexIndex[1], edge1.m_VertexIndex[0] ), &edge1.m_EdgeCollapseScoreVertex1ToVertex0 );
            }
        }
        
        LodVertex& vertex2( m_Vertices[ edge1.OtherVertexIndex( vertexIndex ) ] );
        for ( S32 i=0; i<vertex2.m_EdgeIndices.Size(); i++ )
        {
            const S32 edgeIndex2( vertex2.m_EdgeIndices[i] );
            LodEdge& edge2( m_Edges[ edgeIndex1 ] );
            
            if ( edge2.m_Flags & E_EdgeFlag_RecalculateEdgeCollapseScores )
            {
                edge2.m_Flags &= ~E_EdgeFlag_RecalculateEdgeCollapseScores;
                edge2.m_EdgeCollapseScoreVertex0ToVertex1 = -1.0f;
                edge2.m_EdgeCollapseScoreVertex1ToVertex0 = -1.0f;
                if ( edge2.m_Flags & E_EdgeFlag_CanCollapseVertex0ToVertex1 )
                {
                    Store( CalculateEdgeCollapseScore( edge2.m_VertexIndex[0], edge2.m_VertexIndex[1] ), &edge2.m_EdgeCollapseScoreVertex0ToVertex1 );
                }
                if ( edge2.m_Flags & E_EdgeFlag_CanCollapseVertex1ToVertex0 )
                {
                    Store( CalculateEdgeCollapseScore( edge2.m_VertexIndex[1], edge2.m_VertexIndex[0] ), &edge2.m_EdgeCollapseScoreVertex1ToVertex0 );
                }
            }
        }
    }
}

void LodMesh::Clone( LodMesh* pClone )
{
    *pClone = *this;
    pClone->m_pRoot = m_pRoot ? m_pRoot : this;
    pClone->AnalyseMesh();
}

Maths::VF32 LodMesh::CalculateEdgeCollapseScore( /* TODO: Add pointer to edge here! */ S32 sourceVertexIndex, S32 targetVertexIndex )
{
    using namespace Maths;
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 half( Const<VF32>::Half() );
    const VF32 one( Const<VF32>::One() );
    const VF32 two( Const<VF32>::Two() );
    const VF32 nearlyZero( Const<VF32>::Epsilon() );
    const VF32 fltMax( Const<VF32>::Max() );
    
    LodVertex& sourceVertex( m_Vertices[ sourceVertexIndex ] );
    LodVertex& targetVertex( m_Vertices[ targetVertexIndex ] );
    
    const Maths::Vector3 sourceToTargetVector( targetVertex.m_Position - sourceVertex.m_Position );
    
    LodEdge* pEdge = NULL;
    for ( S32 i=0; i<sourceVertex.m_EdgeIndices.Size(); i++ )
    {
        LodEdge& edge( m_Edges[ sourceVertex.m_EdgeIndices[i] ] );
        if ( edge.OtherVertexIndex( sourceVertexIndex ) == targetVertexIndex )
        {
            pEdge = &edge;
            break;
        }
    }
    
    VF32 distance;
    Maths::Normal3 sourceToTargetNormal;
    if ( pEdge )
    {
        distance = Load<VF32>( &pEdge->m_Length );
        sourceToTargetNormal = pEdge->m_Normal;
    }
    else
    {
        distance = Distance( sourceVertex.m_Position, targetVertex.m_Position );
        sourceToTargetNormal = Normalise( sourceToTargetVector );
    }
    
    VF32 error( one );

	bool testVertex = false;
    
    for ( S32 i=0; i<sourceVertex.m_TriangleIndices.Size(); i++ )
    {
        // Compute error for modified triangles (one vertex has been moved)
        
        LodTriangle& triangle( m_Triangles[ sourceVertex.m_TriangleIndices[i] ] );
        S32 triVertexIndex0( triangle.m_VertexIndex[0] );
        S32 triVertexIndex1( triangle.m_VertexIndex[1] );
        S32 triVertexIndex2( triangle.m_VertexIndex[2] );
        
        if      ( triVertexIndex0 == sourceVertexIndex ) triVertexIndex0 = targetVertexIndex;
        else if ( triVertexIndex1 == sourceVertexIndex ) triVertexIndex1 = targetVertexIndex;
        else if ( triVertexIndex2 == sourceVertexIndex ) triVertexIndex2 = targetVertexIndex;
        else BIZ_ASSERT_FAIL( "Triangle doesn't contain sourceVertex!\n" );
        
        // Ignore degenerate triangles
        if ( ( triVertexIndex0 != triVertexIndex1 ) &&
             ( triVertexIndex0 != triVertexIndex2 ) &&
             ( triVertexIndex1 != triVertexIndex2 ) )
        {
            const Maths::Normal3 oldTriNormal( triangle.m_SurfaceNormal );
            //const VF32           oldTriArea( triangle.m_SurfaceArea );
            
            const Maths::Point3  newP0( m_Vertices[ triVertexIndex0 ].m_Position );
            const Maths::Point3  newP1( m_Vertices[ triVertexIndex1 ].m_Position );
            const Maths::Point3  newP2( m_Vertices[ triVertexIndex2 ].m_Position );
            const Maths::Vector3 newTriVector( ( newP1 - newP0 ) ^ ( newP2 - newP0 ) );
            const VF32           test( oldTriNormal | newTriVector );
            const Maths::Normal3 newTriNormal( Normalise( newTriVector ) );
            const VF32           newTriArea( Length( newTriVector ) * half );
            const VF32           dp( oldTriNormal | newTriNormal );

            const Maths::Point3  oldP0( m_Vertices[ triangle.m_VertexIndex[0] ].m_Position );
            const Maths::Point3  oldP1( m_Vertices[ triangle.m_VertexIndex[1] ].m_Position );
            const Maths::Point3  oldP2( m_Vertices[ triangle.m_VertexIndex[2] ].m_Position );
            const Maths::Vector3 oldTriVector( ( oldP1 - oldP0 ) ^ ( oldP2 - oldP0 ) );
            const VF32           oldTriArea( Length( oldTriVector ) * half );

			if (Abs(X(newP0) - 104.4) < MakeVF32(0.5f) && Abs(Z(newP0) - 133.1) < MakeVF32(0.5f))
				testVertex = true;
            
            if ( test >= zero )
            {
                //const Maths::Normal3 newTriNormal( Normalise( newTriVector ) );
                //const VF32           newTriArea( Length( newTriVector ) * half );
                //const VF32           dp( oldTriNormal | newTriNormal );
                
                ////error += Clamp( one - dp, nearlyZero, one ) * RMS( oldTriArea, newTriArea );
                ////error += Clamp( Square( one - dp ), nearlyZero, one ) * RMS( oldTriArea, newTriArea );
                ////error += Clamp( Square( one - dp ), nearlyZero, one ) * Max( oldTriArea, newTriArea );
                ////error = Max( error, Clamp( one - dp, nearlyZero, one ) * Max( oldTriArea, newTriArea ) );
                ////error += ( ( Clamp( ArcCos( dp ), nearlyZero, one ) * nine ) + one ) * RMS( oldTriArea, newTriArea );
                ////error += ( ( Clamp( one - dp, nearlyZero, one ) * Const<VF32>::Int<9>() ) + one ) * RMS( oldTriArea, newTriArea );
                ////error += ( ( Clamp( one - dp, nearlyZero, one ) * Const<VF32>::Int<9>() ) + one ) * Abs( newTriArea - oldTriArea );
                //error += ( ( Clamp( one - dp, nearlyZero, one ) * Const<VF32>::Int<9>() ) + one ) * RMS( oldTriArea, newTriArea );
                
				if (m_Options.applyTriangleAreaPenalty)
					error += ( ( Clamp( one - dp, nearlyZero, one ) * Const<VF32>::Int<9>() ) + one ) * Max( oldTriArea, newTriArea );
				else
					error += ( ( Clamp( one - dp, nearlyZero, one ) * Const<VF32>::Int<9>() ) ) * Max( oldTriArea, newTriArea );
                
                #if defined( USER_JDENTON )
                BIZ_ASSERT( error >= nearlyZero );
                BIZ_ASSERT( error <= fltMax );
                BIZ_ASSERT( Finite( error ) );
                #endif
            }
            else
            {
                return -one;
            }
        }
    }
    
    // Check which edges have material boundary and test if that is perpendicular to source-target direction
    //if ( sourceVertex.m_Flags & ( E_VertexFlag_MaterialBoundary |
    //                              E_VertexFlag_TextureDiscontinuity |
    //                              E_VertexFlag_Outline ) )
    {
        const VF32 materialBoundaryPenalty( Load<VF32>( &m_Options.materialBoundaryPenalty ) );
        const VF32 textureDiscontinuityPenalty( Load<VF32>( &m_Options.textureDiscontinuityPenalty ) );
        const VF32 outlineEdgePenalty( Load<VF32>( &m_Options.outlineEdgePenalty ) );
        const VF32 sharpnessPenalty( Load<VF32>( &m_Options.sharpnessPenalty ) );
		const VF32 colourDiscontinuityPenalty( Load<VF32>( &m_Options.colourDiscontinuityPenalty ) );
        
        for ( S32 i=0; i<sourceVertex.m_EdgeIndices.Size(); i++ )
        {
            const S32 edgeIndex( sourceVertex.m_EdgeIndices[i] );
            if ( !pEdge || ( edgeIndex != pEdge->m_Index ) )
            {
                LodEdge& edge( m_Edges[ edgeIndex ] );
                const Maths::Normal3 edgeNormal( edge.m_Normal );
                const VF32 edgeLength( Load<VF32>( &edge.m_Length ) );
                const VF32 dp( sourceToTargetNormal | edgeNormal );

                VF32 error2 = ( one - Clamp( Abs( dp ), zero, one ) ) * Load<VF32>( &edge.m_Sharpness ) * sharpnessPenalty;
                
                if ( edge.m_Flags & ( E_EdgeFlag_MaterialBoundary |
                                      E_EdgeFlag_TextureDiscontinuity |
                                      E_EdgeFlag_Outline |
									  E_EdgeFlag_ColourDiscontinuity) )
                {
                    VF32 penalty( zero );
                    if ( edge.m_Flags & E_EdgeFlag_MaterialBoundary )
                    {
                        penalty = Max( penalty, materialBoundaryPenalty );
                    }
                    if ( edge.m_Flags & E_EdgeFlag_Outline )
                    {
                        penalty = Max( penalty, outlineEdgePenalty );
                    }
                    if ( edge.m_Flags & E_EdgeFlag_TextureDiscontinuity )
                    {
                        penalty = Max( penalty, textureDiscontinuityPenalty );
                    }
					if ( edge.m_Flags & E_EdgeFlag_ColourDiscontinuity )
                    {
                        penalty = Max( penalty, colourDiscontinuityPenalty );
                    }

					error2 += ( one - Clamp( Abs( dp ), zero, one ) ) * penalty;
                    
                    #if defined( USER_JDENTON )
                    BIZ_ASSERT( error >= nearlyZero );
                    BIZ_ASSERT( error <= fltMax );
                    BIZ_ASSERT( Finite( error ) );
                    #endif
                }

				if (m_Options.scalePenaltyByEdgeLength)
					error += error2 * edgeLength * distance;
				else
					error += error2;
            }
        }
        
        //VF32 errorFactor( one );
        //for ( S32 i=0; i<sourceVertex.m_EdgeIndices.Size(); i++ )
        //{
        //    LodEdge& edge( m_Edges[ sourceVertex.m_EdgeIndices[i] ] );
        //    LodVertex& vertex0( m_Vertices[ edge.m_VertexIndex[0] ] );
        //    LodVertex& vertex1( m_Vertices[ edge.m_VertexIndex[1] ] );
        //    const Maths::Normal3 edgeNormal( edge.m_Normal );
        //    const VF32 dp( sourceToTargetNormal | edgeNormal );
        //    
        //    VF32 penalty( zero );
        //    
        //    if ( edge.m_Flags & E_EdgeFlag_MaterialBoundary )
        //    {
        //        penalty = Max( penalty, materialBoundaryPenalty );
        //    }
        //    if ( edge.m_Flags & E_EdgeFlag_TextureDiscontinuity )
        //    {
        //        penalty = Max( penalty, textureDiscontinuityPenalty );
        //    }
        //    if ( edge.m_Flags & E_EdgeFlag_Outline )
        //    {
        //        penalty = Max( penalty, outlineEdgePenalty );
        //    }
        //    
        //    errorFactor = Max( errorFactor, one + ( ( one - Abs( dp ) ) * penalty ) );
        //}
        //error *= errorFactor;
    }
    
    if ( m_Options.useViewDirection )
    {
        const VF32 viewDirectionPenalty( Load<VF32>( &m_Options.viewDirectionPenalty ) );
        const VF32 dp( sourceToTargetNormal | m_Options.viewDirection );
        error *= one + ( ( one - Clamp( Abs( dp ), zero, one ) ) * viewDirectionPenalty );
    }
    
    #if defined( USER_JDENTON )
    BIZ_ASSERT( error >= nearlyZero );
    BIZ_ASSERT( error <= fltMax );
    BIZ_ASSERT( Finite( error ) );
    #endif
    
    const VF32 score( Select( Abs( error ) >= nearlyZero, Rcp( error ), fltMax ) );
    
    //#if defined( USER_JDENTON )
    //BIZ_ASSERT( score >= zero );
    //#endif
    
    return score;
}

S32 LodMesh::SelectEdgeToCollapse( S32* pSourceVertexIndex, S32* pTargetVertexIndex, F32* pScore, F32 minEdgeScore, S32 startIndex )
{
    S32 selectedSourceVertexIndex = -1;
    S32 selectedTargetVertexIndex = -1;
    S32 selectedEdgeIndex = -1;
    
    F32 selectedEdgeScore( 0.0f );
    
    for ( EdgeIterator edgeIterator( m_EdgeIterators, startIndex ); edgeIterator; edgeIterator++ )
    {
		const LodEdge& edge = *edgeIterator;
        const F32 edgeScore01( edge.m_EdgeCollapseScoreVertex0ToVertex1 );
        const F32 edgeScore10( edge.m_EdgeCollapseScoreVertex1ToVertex0 );
        
        if ( ( edge.m_Flags & E_EdgeFlag_CanCollapseVertex0ToVertex1 ) && ( edgeScore01 > selectedEdgeScore ) )
        {
            selectedEdgeIndex = edge.m_Index;
            selectedEdgeScore = edgeScore01;
            selectedSourceVertexIndex = edge.m_VertexIndex[0];
            selectedTargetVertexIndex = edge.m_VertexIndex[1];
        }
        if ( ( edge.m_Flags & E_EdgeFlag_CanCollapseVertex1ToVertex0 ) && ( edgeScore10 > selectedEdgeScore ) )
        {
            selectedEdgeIndex = edge.m_Index;
            selectedEdgeScore = edgeScore10;
            selectedSourceVertexIndex = edge.m_VertexIndex[1];
            selectedTargetVertexIndex = edge.m_VertexIndex[0];
        }

		if (selectedEdgeIndex > -1 && selectedEdgeScore > minEdgeScore)
			break;

        BIZ_ASSERT( !edgeIterator.Modified() );
    }
    
    *pSourceVertexIndex = selectedSourceVertexIndex;
    *pTargetVertexIndex = selectedTargetVertexIndex;
	*pScore = selectedEdgeScore;

    return selectedEdgeIndex;
}

void LodMesh::CollapseEdge( S32 edgeIndex, S32 sourceVertexIndex, S32 targetVertexIndex )
{
    #if defined( _DEBUG )
    //LOGDEBUG( g_Log, "LodMesh::CollapseEdge( edgeIndex = %d, sourceVertexIndex = %d, targetVertexIndex = %d )\n", edgeIndex, sourceVertexIndex, targetVertexIndex );
    #endif
    
    EdgeIterator   edge( m_EdgeIterators, edgeIndex );
    VertexIterator sourceVertex( m_VertexIterators, sourceVertexIndex );
    VertexIterator targetVertex( m_VertexIterators, targetVertexIndex );
    
    BIZ_ASSERT( edge );
    BIZ_ASSERT( sourceVertex );
    BIZ_ASSERT( targetVertex );
    
    BIZ_ASSERT( ( ( edge->m_VertexIndex[0] == sourceVertex.Index() ) && ( edge->m_VertexIndex[1] == targetVertex.Index() ) ) ||
                ( ( edge->m_VertexIndex[0] == targetVertex.Index() ) && ( edge->m_VertexIndex[1] == sourceVertex.Index() ) ) );
    
    // For each triangle containing sourceVertex find another triangle
    // with the same primitive that contains targetVertex and copy
    // the vertex buffer indices from that
	if (m_Options.applyGradientPenalties)
	    RemapVertexBufferIndices2( sourceVertex.Index(), targetVertex.Index() );
	else
		RemapVertexBufferIndices( sourceVertex.Index(), targetVertex.Index() );
    
    // For each triangle attached to this edge, find the other edge
    // that contains sourceVertex and replace all uses of that edge in it's
    // adjacent triangles with the third edge (that contains targetVertex).
    // Delete the replaced edge from adjacent vertices, then delete the
    // replaced edge from the mesh. Finally, delete the triangle from
    // adjacent vertices & edges then delete the triangle from the mesh.
    
    while ( edge->m_TriangleIndices.Size() > 0 )
    {
        TriangleIterator triangle( m_TriangleIterators, edge->m_TriangleIndices.Back() );
        
        EdgeIterator sourceEdge( m_EdgeIterators, -1 );
        EdgeIterator targetEdge( m_EdgeIterators, -1 );
        
        // For each triangle attached to this edge, find the other
        // 2 edges that contain sourceVertex and targetVertex
        for ( S32 j=0; j<3; j++ )
        {
            const S32 triangleEdgeIndex( triangle->m_EdgeIndex[j] );
            if ( triangleEdgeIndex != edge.Index() )
            {
                LodEdge& triangleEdge( m_Edges[ triangleEdgeIndex ] );
                
                if ( ( triangleEdge.m_VertexIndex[0] == sourceVertex.Index() ) ||
                     ( triangleEdge.m_VertexIndex[1] == sourceVertex.Index() ) )
                {
                    sourceEdge = triangleEdgeIndex;
                }
                
                if ( ( triangleEdge.m_VertexIndex[0] == targetVertex.Index() ) ||
                     ( triangleEdge.m_VertexIndex[1] == targetVertex.Index() ) )
                {
                    targetEdge = triangleEdgeIndex;
                }
            }
        }
        
        if ( sourceEdge && targetEdge )
        {
            // Delete the triangle from adjacent vertices & edges and then delete it from the mesh
            // ( Could be done in DeleteDegenerateTrianglesAdjacentToEdge() below... )
            DeleteTriangleFromAdjacentVertices( triangle.Index() ); 
            DeleteTriangleFromAdjacentEdges( triangle.Index() );
            DeleteTriangle( triangle.Index() );
            
            // Replace all uses of sourceEdge in it's adjacent triangles with targetEdge (that contains targetVertex)
            // Delete the sourceEdge from adjacent vertices and then delete it from the mesh
            ReplaceEdgeInAdjacentTriangles( sourceEdge.Index(), targetEdge.Index() );
            DeleteEdgeFromAdjacentVertices( sourceEdge.Index() );
            DeleteEdge( sourceEdge.Index() );
            
            // Delete any degenerate triangles connected to targetEdge
            DeleteDegenerateTrianglesAdjacentToEdge( targetEdge.Index() );
        }
        else
        {
			if ( --g_nMaxMessages >= 0 )
	            LOGWARN( g_Log, "LodMesh::CollapseEdge(): Failed to find edge to replace!\n" );
            break;
        }
    }
    
    BIZ_ASSERT( edge->m_TriangleIndices.Empty() );
    
    // Delete edge from adjacent vertices
    // (adjacent triangles have already been deleted)
    DeleteEdgeFromAdjacentVertices( edge.Index() );
    
    // Replace sourceVertex in adjacent edges & triangles with targetVertex
    ReplaceVertexInAdjacentEdges( sourceVertex.Index(), targetVertex.Index() );
    ReplaceVertexInAdjacentTriangles( sourceVertex.Index(), targetVertex.Index() );
    
    // Add root vertex indices from sourceVertex to targetVertex
    //const S32 sourceRootVertexCount( sourceVertex->m_RootVertexIndices.Size() );
    //const S32 targetRootVertexCount( targetVertex->m_RootVertexIndices.Size() );
    //BIZ_ASSERT( sourceRootVertexCount > 0 );
    //targetVertex->m_RootVertexIndices.Reserve( sourceRootVertexCount + targetRootVertexCount );
    //for ( S32 i=0; i<sourceRootVertexCount; i++ )
    //{
    //    targetVertex->m_RootVertexIndices.PushBack( sourceVertex->m_RootVertexIndices[i] );
    //}
    
    // Delete edge from mesh
    DeleteEdge( edge.Index() );
    
    // Delete sourceVertex from mesh
    DeleteVertex( sourceVertex.Index() );
    
    // Delete duplicate edges connected to targetVertex
    DeleteDuplicateEdgesAdjacentToVertex( targetVertex.Index() );
    
    #if defined( _DEBUG )
    // Validate the mesh after each edge collapse
    //Validate();
    #endif
    
    // Delete targetVertex if all it's connected edges & triangles have been deleted
    if ( ( targetVertex->m_TriangleIndices.Size() == 0 ) &&
         ( targetVertex->m_EdgeIndices.Size() == 0 ) )
    {
        DeleteVertex( targetVertex.Index() );
    }
    else
    {
        // Re-Analyse parts of mesh connected to targetVertex because
        // topology adjacent to it has changed.
        //AnalyseMesh();
        AnalyseMeshAdjacentToVertex( targetVertex.Index() );
    }
    
    #if defined( _DEBUG )
    //LOGDEBUG( g_Log, "\n" );
    #endif
}

#if 0
void LodMesh::CollapseVertexPair( S32 sourceVertexIndex, S32 targetVertexIndex )
{
    #if defined( _DEBUG )
    //LOGDEBUG( g_Log, "LodMesh::CollapseVertexPair( sourceVertexIndex = %d, targetVertexIndex = %d )\n", sourceVertexIndex, targetVertexIndex );
    #endif
    
    VertexIterator sourceVertex( m_VertexIterators, sourceVertexIndex );
    VertexIterator targetVertex( m_VertexIterators, targetVertexIndex );
    
    BIZ_ASSERT( sourceVertex );
    BIZ_ASSERT( targetVertex );
    
    // For each triangle containing sourceVertex find another triangle
    // with the same primitive that contains targetVertex and copy
    // the vertex buffer indices from that
    RemapVertexBufferIndices( sourceVertex.Index(), targetVertex.Index() );
    
    // Search for any edges connected to both sourceVertex & targetVertex
    for ( S32 i=0; i<sourceVertex->m_EdgeIndices.Size(); i++ )
    {
        EdgeIterator edge( m_EdgeIterators, sourceVertex->m_EdgeIndices[i] );
        
        if ( ( ( edge->m_VertexIndex[0] == sourceVertexIndex ) && ( edge->m_VertexIndex[1] == targetVertexIndex ) ) ||
             ( ( edge->m_VertexIndex[1] == sourceVertexIndex ) && ( edge->m_VertexIndex[0] == targetVertexIndex ) ) )
        {
            // For each triangle attached to this edge, find the other edge
            // that contains sourceVertex and replace all uses of that edge in it's
            // adjacent triangles with the third edge (that contains targetVertex).
            // Delete the replaced edge from adjacent vertices, then delete the
            // replaced edge from the mesh. Finally, delete the triangle from
            // adjacent vertices & edges then delete the triangle from the mesh.
            
            while ( edge->m_TriangleIndices.Size() > 0 )
            {
                TriangleIterator triangle( m_TriangleIterators, edge->m_TriangleIndices.Back() );
                
                EdgeIterator sourceEdge( m_EdgeIterators, -1 );
                EdgeIterator targetEdge( m_EdgeIterators, -1 );
                
                // For each triangle attached to this edge, find the other
                // 2 edges that contain sourceVertex and targetVertex
                for ( S32 j=0; j<3; j++ )
                {
                    const S32 triangleEdgeIndex( triangle->m_EdgeIndex[j] );
                    if ( triangleEdgeIndex != edge.Index() )
                    {
                        LodEdge& triangleEdge( m_Edges[ triangleEdgeIndex ] );
                        
                        if ( ( triangleEdge.m_VertexIndex[0] == sourceVertex.Index() ) ||
                             ( triangleEdge.m_VertexIndex[1] == sourceVertex.Index() ) )
                        {
                            sourceEdge = triangleEdgeIndex;
                        }
                        
                        if ( ( triangleEdge.m_VertexIndex[0] == targetVertex.Index() ) ||
                             ( triangleEdge.m_VertexIndex[1] == targetVertex.Index() ) )
                        {
                            targetEdge = triangleEdgeIndex;
                        }
                    }
                }
                
                if ( sourceEdge && targetEdge )
                {
                    // Delete the triangle from adjacent vertices & edges and then delete it from the mesh
                    // ( Could be done in DeleteDegenerateTrianglesAdjacentToEdge() below... )
                    DeleteTriangleFromAdjacentVertices( triangle.Index() ); 
                    DeleteTriangleFromAdjacentEdges( triangle.Index() );
                    DeleteTriangle( triangle.Index() );
                    
                    // Replace all uses of sourceEdge in it's adjacent triangles with targetEdge (that contains targetVertex)
                    // Delete the sourceEdge from adjacent vertices and then delete it from the mesh
                    ReplaceEdgeInAdjacentTriangles( sourceEdge.Index(), targetEdge.Index() );
                    DeleteEdgeFromAdjacentVertices( sourceEdge.Index() );
                    DeleteEdge( sourceEdge.Index() );
                    
                    // Delete any degenerate triangles connected to targetEdge
                    // NOTE: This also deletes 'triangle'
                    DeleteDegenerateTrianglesAdjacentToEdge( targetEdge.Index() );
                }
                else
                {
					if ( --g_nMaxMessages >= 0 )
	                    LOGWARN( g_Log, "LodMesh::CollapseVertexPair(): Failed to find edge to replace!\n" );
                    break;
                }
            }
            
            BIZ_ASSERT( edge->m_TriangleIndices.Empty() );
            
            // Delete edge from adjacent vertices
            // (adjacent triangles have already been deleted)
            DeleteEdgeFromAdjacentVertices( edge.Index() );
            
            // Delete edge from mesh
            DeleteEdge( edge.Index() );
            
            // Found it, shouldn't be more than one...
            break;
        }
    }
    
    // Replace sourceVertex in adjacent edges & triangles with targetVertex
    ReplaceVertexInAdjacentEdges( sourceVertex.Index(), targetVertex.Index() );
    ReplaceVertexInAdjacentTriangles( sourceVertex.Index(), targetVertex.Index() );
    
    // Add root vertex indices from sourceVertex to targetVertex
    //const S32 sourceRootVertexCount( sourceVertex->m_RootVertexIndices.Size() );
    //const S32 targetRootVertexCount( targetVertex->m_RootVertexIndices.Size() );
    //BIZ_ASSERT( sourceRootVertexCount > 0 );
    //targetVertex->m_RootVertexIndices.Reserve( sourceRootVertexCount + targetRootVertexCount );
    //for ( S32 i=0; i<sourceRootVertexCount; i++ )
    //{
    //    targetVertex->m_RootVertexIndices.PushBack( sourceVertex->m_RootVertexIndices[i] );
    //}
    
    // Delete sourceVertex from mesh
    DeleteVertex( sourceVertex.Index() );
    
    // Delete duplicate edges connected to targetVertex
    DeleteDuplicateEdgesAdjacentToVertex( targetVertex.Index() );
    
    #if defined( _DEBUG )
    // Validate the mesh after each vertex pair collapse
    //Validate();
    #endif
    
    // Delete targetVertex if all it's connected edges & triangles have been deleted
    if ( ( targetVertex->m_TriangleIndices.Size() == 0 ) &&
         ( targetVertex->m_EdgeIndices.Size() == 0 ) )
    {
        DeleteVertex( targetVertex.Index() );
    }
    else
    {
        // Re-Analyse parts of mesh connected to targetVertex because
        // topology adjacent to it has changed.
        //AnalyseMesh();
        AnalyseMeshAdjacentToVertex( targetVertex.Index() );
    }
    
    #if defined( _DEBUG )
    //LOGDEBUG( g_Log, "\n" );
    #endif
}
#endif

void LodMesh::Reduce()
{
    using namespace Maths;
    
    const F32 vertexReductionRatio( m_Options.vertexReductionRatio );
    const F32 triangleReductionRatio( m_Options.triangleReductionRatio );
    const S32 minimumVertices( m_Options.minimumVertices );
    const S32 minimumTriangles( m_Options.minimumTriangles );
    const bool reduceVertices( m_Options.reduceVertices );
    const bool reduceTriangles( m_Options.reduceTriangles );
	const F32 minEdgeScore( m_Options.minEdgeScore );
    
    BIZ_ASSERT( reduceVertices || reduceTriangles );
    BIZ_ASSERT( !( reduceVertices && reduceTriangles ) );
    
    #if defined( USER_JDENTON )
    LOGINFO( g_Log, "LodMesh::Reduce(): Started\n" );
    Validate();
    #endif
    
    const S32 originalVertexCount( m_Vertices.size() );
    const S32 originalEdgeCount( m_Edges.size() );
    const S32 originalTriangleCount( m_Triangles.size() );
    const S32 reducedVertexCount( Max( (S32)( (F32)m_Vertices.size() * vertexReductionRatio ), minimumVertices ) );
    const S32 reducedTriangleCount( Max( (S32)( (F32)m_Triangles.size() * triangleReductionRatio ), minimumTriangles ) );
	S32 startIndex = 0;
    
    //S32 retry = 5;
    
    // TODO: Build up an array of pairs of vertices to collapse. Compute the error metric
    // for each and then sort the array by this score.
    do
    {
        S32 sourceVertexIndex = 0, targetVertexIndex = 0;
		F32 edgeScore;
        S32 edgeIndex;
		
		if (vertexReductionRatio > 0.f)
			edgeIndex= SelectEdgeToCollapse( &sourceVertexIndex, &targetVertexIndex, &edgeScore, FLT_MAX, 0);
		else
			edgeIndex= SelectEdgeToCollapse( &sourceVertexIndex, &targetVertexIndex, &edgeScore, minEdgeScore, startIndex);

        if ( edgeIndex >= 0 && edgeScore > minEdgeScore)
        {
            CollapseEdge( edgeIndex, sourceVertexIndex, targetVertexIndex );
			startIndex = edgeIndex;
        }
        else 
        {
            //if ( retry > 0 )
            //{
            //    LOGWARN( g_Log, "LodMesh::Reduce(): Failed to find an edge to collapse. Analysing mesh and retrying. [ vertices = %d, triangles = %d ]\n", m_Vertices.Size(), m_Triangles.Size() );
            //    AnalyseMesh();
            //    retry--;
            //}
            //else
            //{
				if (edgeIndex < 0 && --g_nMaxMessages >= 0 )
	                LOGWARN( g_Log, "LodMesh::Reduce(): Failed to find an edge to collapse. Terminating.\n" );
                break;
            //}
        }
    }
    while ( ( reduceVertices && ( (S32) m_Vertices.size() > reducedVertexCount ) ) ||
            ( reduceTriangles && ( (S32) m_Triangles.size() > reducedTriangleCount ) ) );
    
    #if defined( USER_JDENTON )
    LOGINFO( g_Log, "LodMesh::Reduce(): Finished\n" );
    Validate();
    #endif
    
    if ( reduceVertices )
    {
        LOGDEBUG( g_Log, "LodMesh::Reduce(): Reduced vertices from %d to %d, reduction ratio = %.02f%%\n",
            originalVertexCount, m_Vertices.size(), 100.0f * (F32)m_Vertices.size() / Max( (F32)originalVertexCount, 1.0f ) );
    }
    else if ( reduceTriangles )
    {
        LOGDEBUG( g_Log, "LodMesh::Reduce(): Reduced triangles from %d to %d, reduction ratio = %.02f%%\n",
            originalTriangleCount, m_Triangles.size(), 100.0f * (F32)m_Triangles.size() / Max( (F32)originalTriangleCount, 1.0f ) );
    }
    
    #if 0
    
    LOGINFO( g_Log, "LodMesh::Reduce(): Reduced edges from %d to %d, reduction ratio = %.02f%%\n",
        originalEdgeCount, m_Edges.Size(), 100.0f * (F32)m_Edges.size() / Max( (F32)originalEdgeCount, 1.0f ) );
    
    #endif
}

NOALIAS void LodMesh::ValidateVertex( const S32 vertexIndex ) const
{
    const LodVertex& vertex( m_Vertices[ vertexIndex ] );
    
    BIZ_ASSERT( vertex.m_Index == vertexIndex );
    
    for ( S32 i=0; i<vertex.m_EdgeIndices.Size(); i++ )
    {
        const S32 edgeIndex( vertex.m_EdgeIndices[i] );
        
        BIZ_ASSERT( ( edgeIndex >= 0 ) && ( edgeIndex < (S32) m_Edges.size() ) );
        
        for ( S32 j=i+1; j<vertex.m_EdgeIndices.Size(); j++ )
            BIZ_ASSERT( edgeIndex != vertex.m_EdgeIndices[j] );
        
        const LodEdge& edge( m_Edges[ edgeIndex ] );
        
        BIZ_ASSERT( ( edge.m_VertexIndex[0] == vertexIndex ) ||
                    ( edge.m_VertexIndex[1] == vertexIndex ) );
    }
    
    for ( S32 i=0; i<vertex.m_TriangleIndices.Size(); i++ )
    {
        const S32 triangleIndex( vertex.m_TriangleIndices[i] );
        
        BIZ_ASSERT( ( triangleIndex >= 0 ) && ( triangleIndex < (S32) m_Triangles.size() ) );
        
        for ( S32 j=i+1; j<vertex.m_TriangleIndices.Size(); j++ )
            BIZ_ASSERT( triangleIndex != vertex.m_TriangleIndices[j] );
        
        const LodTriangle& triangle( m_Triangles[ triangleIndex ] );
        
        BIZ_ASSERT( ( triangle.m_VertexIndex[0] == vertexIndex ) ||
                    ( triangle.m_VertexIndex[1] == vertexIndex ) ||
                    ( triangle.m_VertexIndex[2] == vertexIndex ) );
    }
}

NOALIAS void LodMesh::ValidateEdge( const S32 edgeIndex ) const
{
    const LodEdge& edge( m_Edges[ edgeIndex ] );
    
    BIZ_ASSERT( edge.m_Index == edgeIndex );
    
    BIZ_ASSERT( edge.m_VertexIndex[0] != edge.m_VertexIndex[1] );
    
    for ( S32 i=0; i<2; i++ )
    {
        const S32 vertexIndex( edge.m_VertexIndex[i] );
        
        BIZ_ASSERT( ( vertexIndex >= 0 ) && ( vertexIndex < (S32) m_Vertices.size() ) );
        
        const LodVertex& vertex( m_Vertices[ vertexIndex ] );
        
        bool found = false;
        for ( S32 j=0; j<vertex.m_EdgeIndices.Size(); j++ )
        {
            if ( vertex.m_EdgeIndices[j] == edgeIndex )
            {
                found = true;
                break;
            }
        }
        BIZ_ASSERT( found );
    }
    
    for ( S32 i=0; i<edge.m_TriangleIndices.Size(); i++ )
    {
        const S32 triangleIndex( edge.m_TriangleIndices[i] );
        
        BIZ_ASSERT( ( triangleIndex >= 0 ) && ( triangleIndex < (S32) m_Triangles.size() ) );
        
        for ( S32 j=i+1; j<edge.m_TriangleIndices.Size(); j++ )
            BIZ_ASSERT( triangleIndex != edge.m_TriangleIndices[j] );
        
        const LodTriangle& triangle( m_Triangles[ triangleIndex ] );
        
        BIZ_ASSERT( ( triangle.m_EdgeIndex[0] == edgeIndex ) ||
                ( triangle.m_EdgeIndex[1] == edgeIndex ) ||
                ( triangle.m_EdgeIndex[2] == edgeIndex ) );
    }
}

NOALIAS void LodMesh::ValidateTriangle( const S32 triangleIndex ) const
{
    const LodTriangle& triangle( m_Triangles[ triangleIndex ] );
    
    BIZ_ASSERT( triangle.m_Index == triangleIndex );
    
    BIZ_ASSERT( triangle.m_VBIndex[0] <= 65536);//triangle.m_VBSize );
    BIZ_ASSERT( triangle.m_VBIndex[1] <= 65536);//triangle.m_VBSize );
    BIZ_ASSERT( triangle.m_VBIndex[2] <= 65536);//triangle.m_VBSize );
    
    BIZ_ASSERT( ( triangle.m_VertexIndex[0] != triangle.m_VertexIndex[1] ) &&
                ( triangle.m_VertexIndex[0] != triangle.m_VertexIndex[2] ) &&
                ( triangle.m_VertexIndex[1] != triangle.m_VertexIndex[2] ) );
    
    for ( S32 i=0; i<3; i++ )
    {
        const S32 vertexIndex( triangle.m_VertexIndex[i] );
        
        BIZ_ASSERT( ( vertexIndex >= 0 ) && ( vertexIndex < (S32) m_Vertices.size() ) );
        
        const LodVertex& vertex( m_Vertices[ vertexIndex ] );
        
        bool found = false;
        for ( S32 j=0; j<vertex.m_TriangleIndices.Size(); j++ )
        {
            if ( vertex.m_TriangleIndices[j] == triangleIndex )
            {
                found = true;
                break;
            }
        }
        BIZ_ASSERT( found );
    }
    
    BIZ_ASSERT( ( triangle.m_EdgeIndex[0] != triangle.m_EdgeIndex[1] ) &&
                ( triangle.m_EdgeIndex[0] != triangle.m_EdgeIndex[2] ) &&
                ( triangle.m_EdgeIndex[1] != triangle.m_EdgeIndex[2] ) );
    
    for ( S32 i=0; i<3; i++ )
    {
        const S32 edgeIndex( triangle.m_EdgeIndex[i] );
        
        BIZ_ASSERT( ( edgeIndex >= 0 ) && ( edgeIndex < (S32) m_Edges.size() ) );
        
        const LodEdge& edge( m_Edges[ edgeIndex ] );
        
        bool found = false;
        for ( S32 j=0; j<edge.m_TriangleIndices.Size(); j++ )
        {
            if ( edge.m_TriangleIndices[j] == triangleIndex )
            {
                found = true;
                break;
            }
        }
        BIZ_ASSERT( found );
        
        BIZ_ASSERT( ( edge.m_VertexIndex[0] == triangle.m_VertexIndex[0] ) ||
                    ( edge.m_VertexIndex[0] == triangle.m_VertexIndex[1] ) ||
                    ( edge.m_VertexIndex[0] == triangle.m_VertexIndex[2] ) );
        
        BIZ_ASSERT( ( edge.m_VertexIndex[1] == triangle.m_VertexIndex[0] ) ||
                    ( edge.m_VertexIndex[1] == triangle.m_VertexIndex[1] ) ||
                    ( edge.m_VertexIndex[1] == triangle.m_VertexIndex[2] ) );
    }
}

NOALIAS void LodMesh::Validate() const
{
#if defined( USER_JDENTON )
    using namespace Maths;
    
    for ( S32 vertexIndex1 = 0; vertexIndex1 < (S32) m_Vertices.size(); vertexIndex1++ )
    {
        const LodVertex& vertex1( m_Vertices[ vertexIndex1 ] );
        for ( S32 vertexIndex2 = vertexIndex1 + 1; vertexIndex2 < (S32) m_Vertices.size(); vertexIndex2++ )
        {
            const LodVertex& vertex2( m_Vertices[ vertexIndex2 ] );
            BIZ_ASSERT( Distance( vertex1.m_Position, vertex2.m_Position ) >= Load<VF32>( &m_Options.weldThreshold ) );
        }
    }
    
    for ( S32 vertexIndex=0; vertexIndex < (S32) m_Vertices.size(); vertexIndex++ )
    {
        ValidateVertex( vertexIndex );
    }
    
    for ( S32 edgeIndex=0; edgeIndex < (S32) m_Edges.size(); edgeIndex++ )
    {
        ValidateEdge( edgeIndex );
    }
    
    for ( S32 triangleIndex=0; triangleIndex < (S32) m_Triangles.size(); triangleIndex++ )
    {
        ValidateTriangle( triangleIndex );
    }
#endif
}

#if defined( USER_JDENTON ) && defined( CFG_ALLOW_OPTIMIZE_OFF )
#pragma optimize( "", on )
#endif

} // namespace Build
} // namespace RenderingData
