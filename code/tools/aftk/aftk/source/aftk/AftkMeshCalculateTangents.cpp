#include "stdafx.h"

namespace Aftk
{
	bool CheckBuggeredFloat(float f)
	{
		int i = *(int*)&f;
		return ((i&0xff000000) == 0xff000000);
	}
}

using namespace Aftk;

C_Status C_MeshCalculateTangents::CalculateTangents(
	C_SceneMeshNode* pMesh,
	bool normalise,
	bool useVertexNormal)
{
	C_Status status;

	status = CalculateTangents(
		pMesh,
		-1, 
		normalise,
		useVertexNormal);
	if (!status) return status;

	return C_Status::E_Status_Success;
}

C_Status C_MeshCalculateTangents::CalculateTangents(
	C_SceneMeshNode* pMesh,
	S32 set, 
	bool normaliseTangentSpace,
	bool useVertexNormal)
{
	pMesh->ClearAnimationHelpers();

	C_Status status;
	if (set == -1)
	{
		for (U32 iSet = 0; iSet < pMesh->GetNumTextureCoordinateSets(); iSet++)
		{
			status = CalculateTangents(
				pMesh,
				iSet, 
				normaliseTangentSpace,
				useVertexNormal);
			if (!status) return status;
		}
		return C_Status::E_Status_Success;
	}

	Vector3F32 tan;
	Vector3F32 bin;

	const Position3Array& vertexPositions = pMesh->GetVertexPositionArray();
	const Normal3Array& vertexNormals = pMesh->GetVertexNormalArray();
	
	C_TextureCoordinateSet& uvSet = pMesh->GetTextureCoordinateSet(set);
	TextureCoordinate4Array& uvs = uvSet.GetTextureCoordinateArray();
	Vector3F32Array& tangents = uvSet.GetTangentArray();
	Vector3F32Array& binormals = uvSet.GetBinormalArray();

	//Clear existing tangent space information for this set.
	{
		tangents.Clear();
		binormals.Clear();

		for (U32 iPv = 0; iPv < pMesh->GetNumPolygonVertices(); iPv++)
		{
			C_PolygonVertex& pPolyVert = pMesh->GetPolygonVertex(iPv);
			pPolyVert.SetTangentIndex(set, -1);
			pPolyVert.SetBinormalIndex(set, -1);
		}
	}

	for (U32 iLodLevel = 0; iLodLevel < (U32)pMesh->GetNumLodLevels(); iLodLevel++)
	{
		for (U32 jPoly = 0; jPoly < (U32)pMesh->GetNumPolygons(iLodLevel); jPoly++)
		{
			C_Polygon* pPoly = &pMesh->GetPolygon(iLodLevel, jPoly);

			if (pPoly->GetNumPolygonVertices() < 3)
				continue;

			for (U32 iPVert = 0; iPVert < pPoly->GetNumPolygonVertices(); iPVert++)
			{
				// get next and previous vertices
				S32 i0 = iPVert;
				C_PolygonVertex* pPolyVert0 = &pPoly->GetPolygonVertex(i0);
				S32 i1 = iPVert + 1; 
				S32 i2 = iPVert - 1;

				if (i1 >= (S32) pPoly->GetNumPolygonVertices()) 
					i1 -= pPoly->GetNumPolygonVertices();
				if (i2 < 0) 
					i2 += pPoly->GetNumPolygonVertices();

				C_PolygonVertex* pPolyVert1 = &pPoly->GetPolygonVertex(i1);
				C_PolygonVertex* pPolyVert2 = &pPoly->GetPolygonVertex(i2);

				Position3 p0 = vertexPositions[pPolyVert0->GetPositionIndex()];
				Position3 p1 = vertexPositions[pPolyVert1->GetPositionIndex()];
				Position3 p2 = vertexPositions[pPolyVert2->GetPositionIndex()];

				Vector4F32 t0;
				if (pPolyVert0->GetTextureCoordinateIndex(set) != -1)
				{
					t0 = uvs[pPolyVert0->GetTextureCoordinateIndex(set)];
				}
				else
				{
					t0[0] = 0;
					t0[1] = 0;
					t0[2] = 0;
				}
				Vector4F32 t1;
				if (pPolyVert1->GetTextureCoordinateIndex(set) != -1)
				{
					t1 = uvs[pPolyVert1->GetTextureCoordinateIndex(set)];
				}
				else
				{
					t1[0] = 0;
					t1[1] = 0;
					t1[2] = 0;
				}
				Vector4F32 t2;
				if (pPolyVert2->GetTextureCoordinateIndex(set) != -1)
				{
					t2 = uvs[pPolyVert2->GetTextureCoordinateIndex(set)];
				}
				else
				{
					t2[0] = 0;
					t2[1] = 0;
					t2[2] = 0;
				}

				//Calculate edge vectors.
				Vector3F32 q0 = p1 - p0;
				Vector3F32 q1 = p2 - p0;

				//Calculate UV deltas across edges.
				Vector2F32 uvq0 = t1 - t0;
				Vector2F32 uvq1 = t2 - t0;

				//Calculate determinant of coefficient matrix.
				float f = uvq0[0] * uvq1[1] - uvq1[0] * uvq0[1];
				F32 det = 1 / f;
				if (f != 0 && !CheckBuggeredFloat(det))
				{
					//Calclate tangent and bitangent vectors. The negation here inverts our vectors to be
					//compliant with Ed's old tangent calculation.
					tan = -det * (uvq1[1]*q0 - uvq0[1] * q1);

					if (useVertexNormal)
					{
						//Calculate the normal given by our two edges.
						Vector3F32 n;
						if (pPolyVert0->GetNormalIndex() == -1)
						{
							n = q1 ^ q0;	
						}
						else
						{
							n = vertexNormals[pPolyVert0->GetNormalIndex()];
						}
						UtMaths::SafeNormalise(&n);
						// Orthnonormalise to normal
						tan -= n * (tan | n);
						bin = tan ^ n;
					}
					else
					{
						bin = -det * (-uvq1[0]*q0 + uvq0[0] * q1);
					}
				}
				else
				{
					// invalid tangent space. set tangent and binormal to 0
					tan[0] = tan[1] = tan[2] =
					bin[0] = bin[1] = bin[2] = 0;
				}

				//Normalise tangent vectors if
				UtMaths::SafeNormalise(&tan);
				UtMaths::SafeNormalise(&bin);

				if (!normaliseTangentSpace)
				{
					//weight by uv area to produce better smoothing results
					float w = fabsf(f);
					tan *= w;
					bin *= w;
				}

				//We may have already processed this polygon vertex. So check to see if we must split this
				//vertex.
				bool splitVertex = false;
				if ((pPolyVert0->GetTangentIndex(set) != -1) || (pPolyVert0->GetBinormalIndex(set) != -1))
				{
					if (pPolyVert0->GetTangentIndex(set) == -1)
					{
						//Dodgy input data - pad
						tangents.PushBack(Vector3F32(0,0,0));
						S32 tIndex = tangents.Size()-1;
						pPolyVert0->SetTangentIndex(set, tIndex);
					}
					if (pPolyVert0->GetBinormalIndex(set) == -1)
					{
						//Dodgy input data - pad
						binormals.PushBack(Vector3F32(0,0,0));
						S32 bIndex = (U32)binormals.Size()-1;
						pPolyVert0->SetBinormalIndex(set, bIndex);
					}
					const Vector3F32 testt1 = pMesh->GetVertexTangent(
						set, pPolyVert0->GetTangentIndex(set));
					const Vector3F32 testb1 = pMesh->GetVertexBinormal(
						set, pPolyVert0->GetBinormalIndex(set));
					if (EqualityTestFVector(tan, testt1) && EqualityTestFVector(bin, testb1))
					{
						//Don't bother doing anything, the recalculated tangent space is the same for this point.
						continue;
					}
					else
					{
						//We've come across the same PV for a second time, only this time we have different
						//tangent vectors! We need to split this vertex.
						splitVertex = true;
					}
				}

				//Regardless of whether we're splitting or not, we need to add new tangents and binormals to 
				//the mesh.
				tangents.PushBack(tan);
				S32 tIndex = tangents.Size()-1;
				binormals.PushBack(bin);
				S32 bIndex = binormals.Size()-1;

				if (splitVertex)
				{
					//Create new PV.
					C_PolygonVertex pv = *pPolyVert0;
					pv.SetTangentIndex(set, tIndex);
					pv.SetBinormalIndex(set, bIndex);
					U32 newPVIndex = pMesh->AddPolygonVertex(pv);
					pPoly->SetPolygonVertexIndex(iPVert, newPVIndex);
				}
				else
				{
					//Modify existing PV.
					pPolyVert0->SetTangentIndex(set, tIndex);
					pPolyVert0->SetBinormalIndex(set, bIndex);
				}
			}
		}
	}

	return C_Status::E_Status_Success;
}
