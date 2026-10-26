#ifndef AFTK_TRIANGLE_ITERATOR_H
#define AFTK_TRIANGLE_ITERATOR_H

#ifdef _MSC_VER_
#pragma once
#endif


#define AFTK_TRIANGLE_ITERATOR_STATIC_ARRAY_SIZE 256
#define AFTK_TRIANGLE_ITERATOR_VCLASSIFICATION_ARRAY_SIZE AFTK_TRIANGLE_ITERATOR_TRI_ARRAY_SIZE-2

namespace Aftk
{
	class C_TriangleIterator
	{
	public:
		C_TriangleIterator(	
			const C_Polygon& poly,
			const Matrix44* pMatrix = 0, 
			bool* pIsDegenerate = 0);
		
		~C_TriangleIterator();

		///\brief Returns the current node.
		///
		///@return The current node the iterator is positioned at.
		operator const U32*();

		///\brief Increments the iterator (prefix form).
		///
		///@return The current triangle the iterator is positioned at.
		U32* operator++();

		///\brief Increments the iterator (postfix form).
		///
		///@param dummy Dummy argument required for postfix form of increment operator.
		///@return The current triangle the iterator is positioned at.
		U32* operator++(int dummy);

		FORCE_INLINE U32 GetNumTriangles() const;

	private:
		struct TriangleIndices
		{
			U32 indices[3];
		};

		bool TriangleIsAnEar(
			const Matrix44* pMatrix,
			const Vector3F32& n0,
			S32 pvi1, S32 pvi2, S32 pvi3,
			const Vector3F32* v);

		void ClassifyVertices(
			bool* vertexClassification,
			const Vector3F32& n0);

		bool IsConvex(
			S32 pvi1, S32 pvi2, S32 pvi3,
			Vector3F32 n0);

		Vector3F32 GetTriangleNormal(
			Position3 v0,
			Position3 v1,
			Position3 v2,
			bool normalise,
			const Matrix44* pMatrix,
			bool* pIsDegenerate = 0) const;

		S32 GetNextActive(
			S32 pvi, 
			U32 vertexCount,
			bool* activeVertexArray);

		S32 GetPrevActive(
			S32 pvi, 
			U32 vertexCount,
			bool* activeVertexArray);

		void ClipEar(
			bool* activeVertexArray,
			bool* vertexClassification,
			const Vector3F32& n0,
			S32 prevActive, S32 i, S32 nextActive);

		U32 Triangulate(
			TriangleIndices* pTriangleIndices,
			F32& smallestArea,
			const bool* pOriginalActiveVertexArray,
			const bool* pOriginalVertexClassification,
			U32 vertexCount,
			U32 startingVertex,
			const Matrix44* pMatrix,
			const Vector3F32& n0);

		F32 GetArea(
			const Vector3F32* v,
			const Vector3F32& n0) const;

		U32 GetPolygonVertexIndex(U32 index) const;

		const C_PolygonVertex& GetPolygonVertexFromPoly(U32 index) const;

		union
		{
			TriangleIndices		m_triangleIndices[AFTK_TRIANGLE_ITERATOR_STATIC_ARRAY_SIZE];
			TriangleIndices*	m_pTriangleIndices;
		};
		union
		{
			U32					m_polyVertIndices[AFTK_TRIANGLE_ITERATOR_STATIC_ARRAY_SIZE];
			U32*				m_pPolyVertIndices;
		};

		S32						m_nOriginalVertices;
		S32						m_nVertices;
		U32						m_nTriangles;

		U32						m_currentTriangle;
		const C_Polygon&			m_poly;
		bool					m_usingDynamicArrays;
	};

	U32 C_TriangleIterator::GetNumTriangles() const
	{
		return m_nTriangles;
	}
}

#endif
