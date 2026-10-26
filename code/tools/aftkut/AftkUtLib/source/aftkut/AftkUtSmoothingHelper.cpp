#include "stdafx.h"

namespace AftkUt
{
	bool C_SmoothingHelper::CanSmooth(
		const Aftk::Vector3F32& v0,
		const Aftk::Vector3F32& v1, 
		F32 magnitudeScale,
		F32 angularTolerance,
		bool normalise)
	{
		F32 dot;
		if (normalise)
		{
			Aftk::Vector3F32 v0n = v0;
			Aftk::Vector3F32 v1n = v1;
			UtMaths::SafeNormalise(&v0n);
			UtMaths::SafeNormalise(&v1n);
			dot = (v0n| v1n);
		}
		else
			dot = (v0 | v1);

		if (dot > angularTolerance)
		{
			F32 m1 = Aftk::MagnitudeSquared(v0);
			F32 m2 = Aftk::MagnitudeSquared(v1);
			if ((m1) < AFTK_EPSILON && (m2 < AFTK_EPSILON))
			{
				return true;
			}
			if (fabsf((1/m1) - (1/m2)) < magnitudeScale)
			{
				return true;
			}
			return false;
		}
		else if( 
			(Aftk::EqualityTestFVector(v0, Aftk::Vector3F32(0,0,0))) && 
			(Aftk::EqualityTestFVector(v1, Aftk::Vector3F32(0,0,0))))
		{
			return true;
		}
		return false;
	}

	bool C_SmoothingHelper::ShouldSmooth(
		const Aftk::C_SceneMeshNode* pMeshNode)
	{
		C_NodeFlagParser actorParser(*pMeshNode, NULL, false);

		if (actorParser.GetFlagGroupCategory() == "System"
			&& actorParser.GetFlagGroupName() == "DoNotSmoothNormals")
			return false;

		return true;
	}

	bool C_SmoothingHelper::ShouldSmooth(
		const Aftk::C_Polygon& polygon)
	{
		const Aftk::C_SceneMeshNode *pMeshNode = polygon.GetMeshNodePtr();
		const Aftk::C_Scene *pScene = pMeshNode->GetScenePtr();

		S32 blindDataIndex = polygon.GetBlindDataMappingIndex();
		if (blindDataIndex != -1)
		{
			const Aftk::C_SharedResource<Aftk::C_BlindDataNode>* pResource =
				pMeshNode->GetPolygonBlindDataMappingPtr(blindDataIndex);

			if (pResource)
			{
				const Aftk::C_BlindDataNode* pData = &pResource->GetContent();
				for (U32 i=0; i<pData->GetNumAttributes(); i++)
				{
					const Aftk::C_GenericAttribute *pAttr = &pData->GetAttribute(i);
					if (pAttr->GetName() == "polySet_DoNotSmoothNormals")
						return false;
				}
			}
		}

		return true;
	}

	bool C_SmoothingHelper::GetEdgeVectors(
		const Aftk::C_Polygon& polygon,
		U32 polyVertIndex,
		Aftk::Vector3F32& v0,
		Aftk::Vector3F32& v1)
	{
		if (polygon.GetNumPolygonVertices() != 3)
			return false;

		const Aftk::C_SceneMeshNode* pAftkSceneMeshNode = polygon.GetMeshNodePtr();
		S32 i0, i1, i2;
		i0 = i1 = i2 = 0;
		for (U32 i=0; i<polygon.GetNumPolygonVertices(); ++i)
		{					
			if (polygon.GetPolygonVertexIndex(i) == polyVertIndex)
			{
				i0 = i;
				break;
			}
		}

		if (i0 == -1)
			return false;

		i1 = i0 - 1;
		i2 = i0 + 1;

		if (i1 < 0)
			i1 += polygon.GetNumPolygonVertices();
		if (i2 >= (S32)polygon.GetNumPolygonVertices())
			i2 -= polygon.GetNumPolygonVertices();

		Aftk::Position3 p0 = pAftkSceneMeshNode->GetVertexPosition(polygon.GetPolygonVertex(i0).GetPositionIndex());
		Aftk::Position3 p1 = pAftkSceneMeshNode->GetVertexPosition(polygon.GetPolygonVertex(i1).GetPositionIndex());
		Aftk::Position3 p2 = pAftkSceneMeshNode->GetVertexPosition(polygon.GetPolygonVertex(i2).GetPositionIndex());

		v0 = p1-p0;
		v1 = p2-p0;

		return true;
	}

	F32 C_SmoothingHelper::GetVertexWeight(
		const Aftk::Vector3F32& v0,
		const Aftk::Vector3F32& v1)
	{
		//weight 
		//@see GDC07/ShaderX 4 - "tangent space calculation for arbitrary triangles meshes"
		//may need to rethink this the non-linear behaviour of this...
		F32 q = UtMaths::Magnitude(v0)*UtMaths::Magnitude(v1);
		q = Maths::Max(0.0001f, q);	//avoid div 0 errors

		F32 f = (v0 | v1)/q;
		f = Maths::Clamp(f, -1.f, 1.f);

		return acos(f);
	}
}
