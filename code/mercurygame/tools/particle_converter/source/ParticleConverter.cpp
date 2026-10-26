#include "stdafx.h"

#include <dynamic/particle/tools_shared/UserShape3DData.h>
#include <dynamic/particle/tools_shared/UserShape3DVariants.h>
#include <dynamic/particle/tools_shared/UserParticleContainer.h>

#include "Options.h"
#include "ParticleConverter.h"

bool C_ParticleConverter::Convert(
	DynamicEffect::Particle::C_UserParticleContainer& output,
	const Aftk::C_Scene& aftkScene,
	const C_Options& options)
{
	for (Aftk::C_SceneNodeConstIterator it(&aftkScene); it; ++it)
	{
		const Aftk::C_SceneNode* p = it;
		if (IsVariantParent(*p))
		{
			DynamicEffect::Particle::C_UserParticleContainer::Shape3DVariantsArray& variantsArray = 
				output.GetShape3DVariantsArray();
			variantsArray.PushBack();
			DynamicEffect::Particle::C_UserShape3DVariants& variants = variantsArray[variantsArray.Size()-1];

			if (!ConvertVariants(variants, *p, options))
				return false;
		}
	}

	return true;
}

bool C_ParticleConverter::ConvertVariants(
	DynamicEffect::Particle::C_UserShape3DVariants& variants,
	const Aftk::C_SceneNode& aftkVariantParent,
	const C_Options& options)
{
	EXECUTER_INFO("Converting variants group %s.\n", aftkVariantParent.GetName().c_str());
	variants.SetName(aftkVariantParent.GetName().c_str());

	F32 extent = 0;
	bool success = FindVariantsExtent(
		extent,
		aftkVariantParent,
		options);
	if (!success) return false;
	if (extent == 0)
	{
		EXECUTER_ERROR("Variants group %s is degenerate.\n",
			aftkVariantParent.GetName().c_str());
		return false;
	}
	variants.SetScale(extent);
	EXECUTER_INFO("Variants group %s has a scale of %3.2f.\n",
		aftkVariantParent.GetName().c_str(), (F32)variants.GetScale());


	success = TraverseVariants3DParticleMesh(
		variants,
		aftkVariantParent,
		options);
	if (success)
		EXECUTER_INFO("Successfully converted %i meshes for group %s.\n", 
			variants.GetShape3DArray().Size(),
			aftkVariantParent.GetName().c_str());

	return success;
}


bool C_ParticleConverter::FindVariantsExtent(
	F32& largestExtent,
	const Aftk::C_SceneNode& aftkVariantParent,
	const C_Options& options)
{
	//Recurse over all meshes attached to our variant
	if (aftkVariantParent.GetType() == Aftk::E_ObjType_SceneMesh)
	{
		//Calculate scale-factor to reduce our mesh to the range -1..1. 
		Aftk::C_Bounds worldBounds = aftkVariantParent.GetWorldBounds();
		//Find the absolute value of our greatest extent.
		for (U32 i = 0; i < 3; i++)
			if (fabsf(worldBounds.GetMin()[i]) > largestExtent)
				largestExtent = fabsf(worldBounds.GetMin()[i]);
		for (U32 i = 0; i < 3; i++)
			if (fabsf(worldBounds.GetMax()[i]) > largestExtent)
				largestExtent = fabsf(worldBounds.GetMax()[i]);
	}

	//Recurse children.
	for (U32 iChild = 0; iChild < aftkVariantParent.GetNumChildren(); iChild++)
	{
		if (!FindVariantsExtent(
			largestExtent,
			*aftkVariantParent.GetChildPtr(iChild),
			options))
			return false;
	}

	return true;
}

bool C_ParticleConverter::TraverseVariants3DParticleMesh(
	DynamicEffect::Particle::C_UserShape3DVariants& variants,
	const Aftk::C_SceneNode& aftkVariantParent,
	const C_Options& options)
{
	//Recurse over all meshes attached to our variant
	if (aftkVariantParent.GetType() == Aftk::E_ObjType_SceneMesh)
	{
		//At the minute this converter
		DynamicEffect::Particle::C_UserShape3DVariants::Shape3DArray& shapeArray = 
			variants.GetShape3DArray();

		shapeArray.PushBack();
		DynamicEffect::Particle::C_UserShape3DData& convertedShape = shapeArray[shapeArray.Size()-1];

		U32 numVertices;

		if (!Convert3DParticle(
			convertedShape,
			numVertices,
			1.f / (F32)variants.GetScale(),
			(Aftk::C_SceneMeshNode&)aftkVariantParent,
			options))
			return false;

		if (shapeArray.Size() == 1)
		{
			if (numVertices/3 > options.GetTriangleLimit())
			{
				EXECUTER_WARN("Mesh %s has %i triangles and exceeds the triangle count limit. Mesh will be truncated to %i triangles!\n",
					variants.GetName().Chars(),
					numVertices/3,
					options.GetTriangleLimit());

				numVertices = options.GetTriangleLimit()*3;
				//Resize shape vertex array
				convertedShape.GetVertexArray().Resize(numVertices);
			}
			variants.SetNumVertices(numVertices);
		}

		//All variants must have the same vertex count.
		if (variants.GetNumVertices() != numVertices)
		{
			EXECUTER_WARN("Vertex count mismatch within variants group for mesh %s. First vertex count encountered was %i and we've just encountered %i!\n",
				aftkVariantParent.GetName().c_str(),
				variants.GetNumVertices(),
				numVertices);

			//Resize shape vertex array
			convertedShape.GetVertexArray().Resize(variants.GetNumVertices());
			if (numVertices < variants.GetNumVertices())
			{
				//Pad data with degenerates as required.
				U32 numToPad = variants.GetNumVertices()-numVertices;
				memset(
					&convertedShape.GetVertexArray()[numVertices], 
					0, 
					numToPad*sizeof(DynamicEffect::Particle::C_UserShape3DData::C_Vertex));
			}
		}
	}

	//Recurse children.
	for (U32 iChild = 0; iChild < aftkVariantParent.GetNumChildren(); iChild++)
	{
		if (!TraverseVariants3DParticleMesh(
			variants,
			*aftkVariantParent.GetChildPtr(iChild),
			options))
			return false;
	}

	return true;
}

bool C_ParticleConverter::Convert3DParticle(
	DynamicEffect::Particle::C_UserShape3DData& output,
	U32& numVertices,
	const F32 inverseScale,
	const Aftk::C_SceneMeshNode& aftkParticleMesh,
	const C_Options& options)
{
	//Currently exporting simple triangle list.
	DynamicEffect::Particle::C_UserShape3DData::VertexArray& outputVertexArray = output.GetVertexArray();
	U32 nTriangles = aftkParticleMesh.GetNumPolygons(0);
	numVertices = nTriangles * 3;
	outputVertexArray.Resize(numVertices);

	//Initialise a pointer to the start of our output vertex buffer.
	DynamicEffect::Particle::C_UserShape3DData::C_Vertex* pOutputVertex = &outputVertexArray[0];
	S16 materialIndex;
	for (U32 iTri = 0; iTri < nTriangles; iTri++)
	{
		const Aftk::C_Polygon& tri = aftkParticleMesh.GetPolygon(0, iTri);
		if (tri.GetNumPolygonVertices() != 3)
			return false;

		//Grab material from first poly.
		if (iTri == 0)
		{
			materialIndex = tri.GetMaterialMappingIndex();
		}
		else if (materialIndex != tri.GetMaterialMappingIndex())
		{
			//TODO: warning here. > 1 material in mesh.
		}

		for (U32 j = 0; j < 3; j++)
		{
			const Aftk::C_PolygonVertex& pv = tri.GetPolygonVertex(j);

			//Copy vertex data.
			S32 vertexPositionIndex = pv.GetPositionIndex();

			S32 vertexNormalIndex = pv.GetNormalIndex();

			S32 vertexUVIndex = -1;
			S32 vertexTangentIndex = -1;
			S32 vertexBinormalIndex = -1;
			//Inspect UV set 0.
			if (aftkParticleMesh.GetNumTextureCoordinateSets() > 0)
			{
				vertexUVIndex = pv.GetTextureCoordinateIndex(0);
				vertexTangentIndex = pv.GetTangentIndex(0);
				vertexBinormalIndex = pv.GetBinormalIndex(0);
			}

			pOutputVertex->position = 
				(DynamicEffect::Particle::C_UserShape3DData::Vector3F32&)aftkParticleMesh.GetVertexPosition(
					vertexPositionIndex);

			pOutputVertex->position.v[0] *= inverseScale;
			pOutputVertex->position.v[1] *= inverseScale;
			pOutputVertex->position.v[2] *= inverseScale;

			if (vertexNormalIndex != -1)
			{
				Aftk::Normal3 n = aftkParticleMesh.GetVertexNormal(
					vertexNormalIndex);
				UtMaths::Normalise((Aftk::Vector3F32*)&n);
				pOutputVertex->normal = 
					(DynamicEffect::Particle::C_UserShape3DData::Vector3F32&)n;
			}
			else
				memset(&pOutputVertex->normal, 0, sizeof(pOutputVertex->normal));

			if (aftkParticleMesh.GetNumTextureCoordinateSets() > 0)
			{
				if (vertexUVIndex != -1)
					pOutputVertex->uv0 = 
						(DynamicEffect::Particle::C_UserShape3DData::Vector2F32&)aftkParticleMesh.GetTextureCoordinate(
							0,
							vertexUVIndex);
				else
					memset(&pOutputVertex->uv0, 0, sizeof(pOutputVertex->uv0));

				if (vertexTangentIndex != -1)
					pOutputVertex->tan0 = 
						(DynamicEffect::Particle::C_UserShape3DData::Vector3F32&)aftkParticleMesh.GetVertexTangent(
							0,
							vertexTangentIndex);
				else
					memset(&pOutputVertex->tan0, 0, sizeof(pOutputVertex->tan0));

				if (vertexBinormalIndex != -1)
					pOutputVertex->bin0 = 
						(DynamicEffect::Particle::C_UserShape3DData::Vector3F32&)aftkParticleMesh.GetVertexBinormal(
							0,
							vertexBinormalIndex);
				else
					memset(&pOutputVertex->bin0, 0, sizeof(pOutputVertex->bin0));
			}
		
			//Move to next vertex.
			pOutputVertex++;
		}
	}

	return true;
}

bool C_ParticleConverter::IsVariantParent(const Aftk::C_SceneNode& aftkSceneNode)
{
	if (aftkSceneNode.GetType() == Aftk::E_ObjType_SceneNode &&
		aftkSceneNode.GetName().length() > 0 &&
		aftkSceneNode.GetName()[0] == '_')
		return true;

	return false;
}
