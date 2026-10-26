#include <stdafx.h>
#include <Aftk/AftkTriangleIterator.h>

namespace Aftk
{
	const F32 earEpsilon = 1e-10f;
}

using namespace Aftk;


C_TriangleIterator::C_TriangleIterator(
	const Aftk::C_Polygon& poly,
	const Matrix44* pMatrix, 
	bool* pIsDegenerate) : m_poly(poly)
{
	m_usingDynamicArrays = false;
	m_currentTriangle = 0;
	m_nTriangles = 0;
	
	bool vertexClassification[AFTK_TRIANGLE_ITERATOR_STATIC_ARRAY_SIZE];
	bool activeVertexArray[AFTK_TRIANGLE_ITERATOR_STATIC_ARRAY_SIZE];
	F32 vertexAreaWeightArray[AFTK_TRIANGLE_ITERATOR_STATIC_ARRAY_SIZE];
	TriangleIndices* pTriangleIndices;
	bool* pVertexClassification;
	bool* pActiveVertexArray;
	F32* pVertexAreaWeightArray;
	U32* pPolyVertIndices;

	//No triangulation required.
	if (m_poly.GetNumPolygonVertices() == 3)
	{
		m_nOriginalVertices = 3;
		m_nVertices = 3;
		m_nTriangles = 1;
		m_polyVertIndices[0] = m_poly.GetPolygonVertexIndex(0);
		m_polyVertIndices[1] = m_poly.GetPolygonVertexIndex(1);
		m_polyVertIndices[2] = m_poly.GetPolygonVertexIndex(2);
		m_triangleIndices[0].indices[0] = m_polyVertIndices[0];
		m_triangleIndices[0].indices[1] = m_polyVertIndices[1];
		m_triangleIndices[0].indices[2] = m_polyVertIndices[2];
		return;
	}

	bool isDegenerate;
	Vector3F32 n0 = (Vector3F32&)m_poly.GetNormal(pMatrix, &isDegenerate);
	if (isDegenerate)
	{
		//If we're degenerate, then exit without adding ourselves to the new polygon array.
		if (pIsDegenerate)
			*pIsDegenerate = true;
		return;
	}

	m_nOriginalVertices = poly.GetNumPolygonVertices();
	if (m_nOriginalVertices < AFTK_TRIANGLE_ITERATOR_STATIC_ARRAY_SIZE)
	{
		pTriangleIndices = m_triangleIndices;
		pVertexClassification = vertexClassification;
		pActiveVertexArray = activeVertexArray;
		pVertexAreaWeightArray = vertexAreaWeightArray;
		pPolyVertIndices = m_polyVertIndices;

		//Reset the vertex area weight array.
		memset(vertexAreaWeightArray, 0, sizeof(bool)*m_nOriginalVertices);

		//Reset our array of triangle indices.
		memset(m_triangleIndices, 0, sizeof(TriangleIndices)*AFTK_TRIANGLE_ITERATOR_STATIC_ARRAY_SIZE);
		m_usingDynamicArrays = false;
	}
	else
	{
		m_pTriangleIndices = new TriangleIndices[m_nOriginalVertices-2];
		pTriangleIndices = m_pTriangleIndices;

		pVertexClassification = new bool[m_nOriginalVertices];
		pActiveVertexArray = new bool[m_nOriginalVertices];
		pVertexAreaWeightArray = new F32[m_nOriginalVertices];
		m_pPolyVertIndices = new U32[m_nOriginalVertices];
		pPolyVertIndices = m_pPolyVertIndices;
		m_usingDynamicArrays = true;
	}

	//Initialise array of polygon vertex indices;
	{
		U32 currentPVArraySize = 0;
		for (U32 iPolyVert = 0; iPolyVert < (U32)m_nOriginalVertices; iPolyVert++)
		{
			S32 pvIdx = m_poly.GetPolygonVertexIndex(iPolyVert);

			//Ignore invalid pvs
			if (pvIdx == -1)
				continue;

			S32 nextPvIdx;
			if ((S32)iPolyVert < m_nOriginalVertices-1)
				nextPvIdx = m_poly.GetPolygonVertexIndex(iPolyVert+1);
			else
				nextPvIdx = m_poly.GetPolygonVertexIndex(0);
			
			//Ignore adjacent pv duplicates.
			if (pvIdx == nextPvIdx)
				continue;

			pPolyVertIndices[currentPVArraySize] = pvIdx;
			currentPVArraySize++;
		}

		//Set our new vertex count.
		m_nVertices = currentPVArraySize;
	}
	//Set vertex classifcation array
	ClassifyVertices(pVertexClassification, n0);
	//Set active vertex array.
	for (U32 i = 0; i < (U32)m_nVertices; i++)
		pActiveVertexArray[i] = true;

	{
		S32 greatestTriCount = -1;
		F32 greatestSmallestArea = -1;
		S32 nTriangles;
		TriangleIndices	tempTriangleIndices[AFTK_TRIANGLE_ITERATOR_STATIC_ARRAY_SIZE];
		TriangleIndices* pTempTriangleIndices;
		if (!m_usingDynamicArrays)
			pTempTriangleIndices = tempTriangleIndices;
		else
			pTempTriangleIndices = new TriangleIndices[m_nVertices-2];

		for (S32 i = 0; i < m_nVertices-1; i++)
		{
			F32 smallestArea;
			nTriangles = Triangulate(
				pTempTriangleIndices,
				smallestArea,
				activeVertexArray,
				vertexClassification,
				m_nVertices,
				i, // starting vertex
				pMatrix,
				n0);

			if (smallestArea > greatestSmallestArea)
			{
				if (greatestTriCount == -1)
					greatestTriCount = nTriangles;
				else if (nTriangles < greatestTriCount)
				{
					//Invalid triangulation.
					continue;
				}

				//This is the best triangulation we've found so far.
				greatestSmallestArea = smallestArea;
				memcpy(pTriangleIndices, pTempTriangleIndices, sizeof(TriangleIndices)*nTriangles);
				m_nTriangles = nTriangles;
				greatestTriCount = nTriangles;
			}

			if (greatestTriCount != nTriangles)
			{
				//Would have been an invalid triangulation
				continue;
			}
		}

		if (m_usingDynamicArrays)
			delete[] pTempTriangleIndices;
	}

	//Delete dynamic arrays, if allocated.
	if (m_nOriginalVertices > AFTK_TRIANGLE_ITERATOR_STATIC_ARRAY_SIZE)
	{
		delete[] pVertexClassification;
		delete[] pActiveVertexArray;
		delete[] pVertexAreaWeightArray;
	}

	if (pIsDegenerate)
		pIsDegenerate = false;
}

C_TriangleIterator::~C_TriangleIterator()
{
	if (m_usingDynamicArrays)
	{
		delete[] m_pTriangleIndices;
		delete[] m_pPolyVertIndices;
	}
}

U32 C_TriangleIterator::GetPolygonVertexIndex(U32 index) const
{
	if (!m_usingDynamicArrays)
		return m_polyVertIndices[index];

	return m_pPolyVertIndices[index];
}

const C_PolygonVertex& C_TriangleIterator::GetPolygonVertexFromPoly(U32 index) const
{
	return m_poly.GetMeshNodePtr()->GetPolygonVertex(GetPolygonVertexIndex(index));
}

bool C_TriangleIterator::TriangleIsAnEar(
	const Matrix44* pMatrix,
	const Vector3F32& n0,
	S32 pvi1, S32 pvi2, S32 pvi3,
	const Vector3F32* v)
{
	//Condition 3 check.
	//Condition 3: Triangle does not contain any of the polygons active vertices.
	//Active vertices are those that have not been used in triangles yet.
	Vector3F32 delta1(v[1] - v[0]);
	Vector3F32 delta2(v[2] - v[1]);
	Vector3F32 delta3(v[0] - v[2]);

	Vector3F32 n1 = n0 ^ delta1;
	Vector3F32 n2 = n0 ^ delta2;
	Vector3F32 n3 = n0 ^ delta3;
	UtMaths::SafeNormalise(&n1);
	UtMaths::SafeNormalise(&n2);
	UtMaths::SafeNormalise(&n3);

	//We need to know the orientation of the triangle.
	bool counterClockwise;

	{
		Vector3F32 e0 = v[1] - v[0];
		Vector3F32 e1 = v[1] - v[0];
		F32 dot = e0 | e1;
		if (dot > 0)
			counterClockwise = true;
		else
			counterClockwise = false;
	}
	for (S32 iPV = 0; iPV < m_nVertices; iPV++)
	{
		//If this vertex is active, and is not one of the vertices currently under
		//consideration, then...
		if (
			(iPV != pvi1) &&
			(iPV != pvi2) &&
			(iPV != pvi3))
		{
			//Get vertex position q.
			const C_PolygonVertex& pvq = GetPolygonVertexFromPoly(iPV);
			S32 vPosq = pvq.GetPositionIndex();
			Vector3F32 q = (const Vector3F32&)m_poly.m_pMeshNode->GetVertexPosition(vPosq);
			if (pMatrix)
				q = ApplyMatrix(*pMatrix, q);

			Vector3F32 delta1 = (q - v[0]);
			Vector3F32 delta2 = (q - v[1]);
			Vector3F32 delta3 = (q - v[2]);
			UtMaths::SafeNormalise(&delta1);
			UtMaths::SafeNormalise(&delta2);
			UtMaths::SafeNormalise(&delta3);

			F32 dot1 = n1 | delta1;
			F32 dot2 = n2 | delta2;
			F32 dot3 = n3 | delta3;

			if (counterClockwise)
			{
				if (
					(dot1 > -earEpsilon) &&
					(dot2 > -earEpsilon) &&
					(dot3 > -earEpsilon)
				)
				{
					//Triangle is not an ear.
					return false;
				}
			}
			else
			{
				if (
					(!(dot1 > -earEpsilon)) &&
					(!(dot2 > -earEpsilon)) &&
					(!(dot3 > -earEpsilon))
				)
				{
					return false;
				}
			}
		}
	}

	return true;
}


S32 C_TriangleIterator::GetNextActive(
	S32 pvi, 
	U32 m_nVertices,
	bool* activeVertexArray)
{
	while(true)
	{
		if (++pvi == m_nVertices) 
			pvi = 0;
		if (activeVertexArray[pvi])
			return pvi;
	}
}

S32 C_TriangleIterator::GetPrevActive(
	S32 pvi, 
	U32 m_nVertices,
	bool* activeVertexArray)
{
	while(true)
	{
		if (--pvi == -1) 
			pvi = m_nVertices-1;
		if (activeVertexArray[pvi])
			return pvi;
	}
}

void C_TriangleIterator::ClipEar(
	bool* activeVertexArray,
	bool* vertexClassification,
	const Vector3F32& n0,
	S32 prevActive, S32 i, S32 nextActive)
{
	activeVertexArray[i] = false;

	//Re-classify vertices adjacent to our ear.
	S32 earPrevIMinus1 = GetPrevActive(prevActive, m_nVertices, activeVertexArray);
	S32 earPrevIPlus1 = GetNextActive(prevActive, m_nVertices, activeVertexArray);

	S32 earNextIMinus1 = GetPrevActive(nextActive, m_nVertices, activeVertexArray);
	S32 earNextIPlus1 = GetNextActive(nextActive, m_nVertices, activeVertexArray);
	vertexClassification[prevActive] = IsConvex(
		earPrevIMinus1,
		prevActive,
		earPrevIPlus1,
		n0);

	vertexClassification[nextActive] = IsConvex(
		earNextIMinus1,
		nextActive,
		earNextIPlus1,
		n0);
}

C_TriangleIterator::operator const U32*()
{
	if (m_currentTriangle >= m_nTriangles)
		return 0;

	if (m_usingDynamicArrays)
		return m_pTriangleIndices[m_currentTriangle].indices;

	return m_triangleIndices[m_currentTriangle].indices;
}

//Pre increment
U32* C_TriangleIterator::operator++()
{
	m_currentTriangle++;

	if (m_currentTriangle > m_nTriangles)
	{
		//We're at the end of our triangle list.
		m_currentTriangle = m_nTriangles;
		return 0;
	}

	if (m_usingDynamicArrays)
		return m_pTriangleIndices[m_currentTriangle].indices;

	return m_triangleIndices[m_currentTriangle].indices;
}

U32* C_TriangleIterator::operator++(int dummy)
{
	U32* pTmp = m_triangleIndices[m_currentTriangle].indices;
	++(*this);
	return pTmp;
}

void C_TriangleIterator::ClassifyVertices(
	bool* vertexClassification,
	const Vector3F32& n0)
{
	U32 concaveCount = 0;
	for (U32 i = 0; i < (U32)m_nVertices; i++)
	{
		if (i == 0)
		{
			if (IsConvex(
				m_nVertices-1,
				i,
				i+1,
				n0))
			{
				vertexClassification[i] = true;
			}
			else
			{
				vertexClassification[i] = false;
				concaveCount++;
			}
		}
		else if (i == m_nVertices -1)
		{
			if (IsConvex(
				i-1,
				i,
				0,
				n0))
			{
				vertexClassification[i] = true;
			}
			else
			{
				vertexClassification[i] = false;
				concaveCount++;
			}			
		}
		else
		{
			if (IsConvex(
				i-1,
				i,
				i+1,
				n0))
			{
				vertexClassification[i] = true;
			}
			else
			{
				vertexClassification[i] = false;
				concaveCount++;
			}
		}
	}
}

bool C_TriangleIterator::IsConvex(
	S32 pvi1, S32 pvi2, S32 pvi3,
	Vector3F32 n0)
{
	const C_PolygonVertex& pv1 = GetPolygonVertexFromPoly(pvi1);
	const C_PolygonVertex& pv2 = GetPolygonVertexFromPoly(pvi2);
	const C_PolygonVertex& pv3 = GetPolygonVertexFromPoly(pvi3);

	S32 vPos1 = pv1.GetPositionIndex();
	S32 vPos2 = pv2.GetPositionIndex();
	S32 vPos3 = pv3.GetPositionIndex();

	Position3 p1 = m_poly.m_pMeshNode->GetVertexPosition(vPos1);
	Position3 p2 = m_poly.m_pMeshNode->GetVertexPosition(vPos2);
	Position3 p3 = m_poly.m_pMeshNode->GetVertexPosition(vPos3);

	//Make a degenerate
	if (EqualityTestFVector(p2, p3))
	{
		if (!EqualityTestFVector(p1, p2))
			return true;
	}
	else if (EqualityTestFVector(p2, p1))
	{
		if (!EqualityTestFVector(p3, p2))
			return true;
	}

	Matrix44 worldMatrix = m_poly.m_pMeshNode->GetWorldMatrix();
	Vector3F32 triNormal = 
		GetTriangleNormal(p1, p2, p3, true, &worldMatrix);

	//Condition 1 and 2 validated by this check.
	//Condition 1: Vertices do not form a degenerate triangle.
	//Condition 2: Triangle orientation is consistent with polygon normal.
	if (!((n0 | triNormal) > earEpsilon))
		return false;

	return true;
}

Vector3F32 C_TriangleIterator::GetTriangleNormal(
	Position3 v0,
	Position3 v1,
	Position3 v2,
	bool normalise,
	const Matrix44* pMatrix,
	bool* pIsDegenerate) const
{
	Vector3F32 normal;
	float d = 0.f;

	if (pMatrix)
	{
		v0 = ApplyMatrix(*pMatrix, v0);
		v1 = ApplyMatrix(*pMatrix, v1);
		v2 = ApplyMatrix(*pMatrix, v2);
	}
	(Vector3F32&)v1 = (Vector3F32&)v1 - (Vector3F32&)v0;
	(Vector3F32&)v2 = (Vector3F32&)v2 - (Vector3F32&)v0;

	//Sum of difference between our two points
	d += fabs(v2.x - v1.x) + fabs(v2.y - v1.y) + fabs(v2.z - v1.z);

	//Sum the cross products of the triangle normals.
	normal = (Vector3F32&)v2 ^ (Vector3F32&)v1;

	//float s = fabs(nx) + fabs(ny) + fabs(nz);
	F32 s = fabsf(normal[E_Axis_X]) + fabsf(normal[E_Axis_Y]) + fabsf(normal[E_Axis_Z]);
	if (pIsDegenerate)
	{
		if (s < 0.000001f || s < d * 0.0001f)
		{
			*pIsDegenerate = true;
		}
		else
		{
			*pIsDegenerate = false;
		}
	}
	if (normalise)
		UtMaths::SafeNormalise(&normal);
	return normal;
}


U32 C_TriangleIterator::Triangulate(
	TriangleIndices* pTriangleIndices,
	F32& smallestArea,
	const bool* pOriginalActiveVertexArray,
	const bool* pOriginalVertexClassification,
	U32 m_nVertices,
	U32 startingVertex,
	const Matrix44* pMatrix,
	const Vector3F32& n0)
{
	bool vertexClassification[AFTK_TRIANGLE_ITERATOR_STATIC_ARRAY_SIZE];
	bool activeVertexArray[AFTK_TRIANGLE_ITERATOR_STATIC_ARRAY_SIZE];
	bool* pVertexClassification;
	bool* pActiveVertexArray;

	if (m_nVertices < AFTK_TRIANGLE_ITERATOR_STATIC_ARRAY_SIZE)
	{
		pVertexClassification = vertexClassification;
		pActiveVertexArray = activeVertexArray;
	}
	else
	{
		pVertexClassification = new bool[m_nVertices];
		pActiveVertexArray = new bool[m_nVertices];
	}

	memcpy(pActiveVertexArray, pOriginalActiveVertexArray, sizeof(bool)*m_nVertices);
	memcpy(pVertexClassification, pOriginalVertexClassification, sizeof(bool)*m_nVertices);

	U32 nEmittedTris = 0;
	smallestArea = FLT_MAX;
	bool done = false;
	while (!done)
	{
		bool earFound = false;

		S32 i = startingVertex;
		for (U32 numVerticesProcessed = 0; numVerticesProcessed < m_nVertices; numVerticesProcessed++)
		{
			if (activeVertexArray[i] && vertexClassification[i])
			{
				S32 prevActive = GetPrevActive(i, m_nVertices, activeVertexArray);
				S32 nextActive = GetNextActive(i, m_nVertices, activeVertexArray);

				const C_PolygonVertex& pv1 = GetPolygonVertexFromPoly(prevActive);
				const C_PolygonVertex& pv2 = GetPolygonVertexFromPoly(i);
				const C_PolygonVertex& pv3 = GetPolygonVertexFromPoly(nextActive);

				S32 vPos1 = pv1.GetPositionIndex();
				S32 vPos2 = pv2.GetPositionIndex();
				S32 vPos3 = pv3.GetPositionIndex();

				Vector3F32 v[3];
				v[0] = (Vector3F32&)m_poly.m_pMeshNode->GetVertexPosition(vPos1);
				v[1] = (Vector3F32&)m_poly.m_pMeshNode->GetVertexPosition(vPos2);
				v[2] = (Vector3F32&)m_poly.m_pMeshNode->GetVertexPosition(vPos3);

				//Transform points into the same space as n0.
				if (pMatrix)
				{
					v[0] = ApplyMatrix(*pMatrix, v[0]);
					v[1] = ApplyMatrix(*pMatrix, v[1]);
					v[2] = ApplyMatrix(*pMatrix, v[2]);
				}

				F32 triArea;
				if (TriangleIsAnEar(
					pMatrix,
					n0, 
					prevActive,
					i,
					nextActive,
					v))
				{
					triArea = GetArea(v, n0);
//					AFTK_ASSERT(triArea > 0);
					//Create a triangle.
					S32 pvIdx1 = GetPolygonVertexIndex(prevActive);
					S32 pvIdx2 = GetPolygonVertexIndex(i);
					S32 pvIdx3 = GetPolygonVertexIndex(nextActive);
					pTriangleIndices[nEmittedTris].indices[0] = pvIdx1;
					pTriangleIndices[nEmittedTris].indices[1] = pvIdx2;
					pTriangleIndices[nEmittedTris].indices[2] = pvIdx3;
					
					//Clip this triangle from the remainder of our poly.
					ClipEar(
						activeVertexArray,
						vertexClassification,
						n0, prevActive, i, nextActive); 
					
					earFound = true;
					if (triArea < smallestArea)
						smallestArea = triArea;
					nEmittedTris++;
					break;
				}
			}

			if (i >= S32(m_nVertices)-1)
				i -= S32(m_nVertices)-1;
			else
				i++;
		}

		if (!earFound)
		{
			//Couldn't find any more ears.
			done = true;
		}
	}

	if (m_nOriginalVertices >= AFTK_TRIANGLE_ITERATOR_STATIC_ARRAY_SIZE)
	{
		delete[] pVertexClassification;
		delete[] pActiveVertexArray;
	}
	return nEmittedTris;
}

F32 C_TriangleIterator::GetArea(
	const Vector3F32* v,
	const Vector3F32& n0) const
{
	//The area of the parallelogram is equal to the magnitude of the cross product 
	//of vectors representing two adjacent sides:
	Vector3F32 edge0 = v[1] - v[0];
	Vector3F32 edge1 = v[1] - v[2];
    Vector3F32 c = edge0 ^ edge1;

	//Quick exit for degenerates.
	if (Aftk::MagnitudeSquared(c) == 0)
		return 0.f;

	//As we're dealing with a triangle, halve the area.
	F32 area = Aftk::Magnitude(c) * 0.5f;
    return area;
}


