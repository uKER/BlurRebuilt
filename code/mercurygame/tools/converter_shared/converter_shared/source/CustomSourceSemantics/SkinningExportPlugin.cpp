#include "stdafx.h"

#include <renderingdata/renderingdata.h>
#include <aftk/Aftk.h>
#include <fx/Effect.h>

#include "converter_shared/renderdataconversion/ConvertAftkMaterial.h"
#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"
#include "converter_shared\renderdataconversion\ConstantAnnotationReader.h"
#include "converter_shared\renderdataconversion\VtxInputSourceSemantics.h"
#include "converter_shared/CustomSourceSemantics/SkinningProcessor.h"
#include "converter_shared/CustomSourceSemantics/SkinningExportPlugin.h"
#include "converter_shared/renderdataconversion/ConvertAftkMesh.h"

//#define PALETTE_INDEX_AS_ELEMENT

namespace SkinningProcessor
{

Debug::Logger &g_Log = Debug::GetLogger("converter_shared.skinning_plugin");

I_SkinningProcessor *g_pSkinningProcessor;

/// exporter plugin for exporting skinning data
class Handler : public RenderDataConversion::ConstantAnnotationReader
{
public:
	/// constructor
	Handler(FX::Parameter *pParam, const char *pszFXName) :
		RenderDataConversion::ConstantAnnotationReader(pParam, pszFXName)
	{
	}

	/// parse the given source semantic and return true if we recognise it
	bool IdentifySemantic(const char *pszSource, RenderDataConversion::E_CustomSemanticType type)
	{
		if (type != RenderDataConversion::E_CustomSemanticType_Constant)
			return false;

		return !stricmp(pszSource,"<MatrixPalette>");
	}

	/// read out data + MaterialElement for this parameter, given an Aftk Material
	/// automatically advances the pData pointer
	/// returns false if this parameter is not supposed to be stored in the renderingdata material
	bool GetMaterialData(RenderDataConversion::AftkMaterialConverter *pMtl, RenderingData::MaterialElement *pMtlElement, U8 **ppData)
	{
		RenderingData::Build::MaterialList *pMaterialList = &pMtl->GetSceneExport()->GetMaterialList();
		
		int iPalette = pMtl->GetCustomKey();

		pMtlElement->type = RenderingData::MATERIAL_ELEMENT_TYPE_F32;
		pMtlElement->count = 0;

		// on the first pass, the skinning won't have been calculated yet, so we ignore this part of the material
		// for clustering pass, ignore skinning code
		if (g_pSkinningProcessor->GetNumMatrixPalette() == 0 || iPalette == -1)
			return true;

		bool hardMappedBones = true;
		int nBones = g_pSkinningProcessor->GetNumHardMappedBones();
		assert(nBones < static_cast<S32>(g_pSkinningProcessor->GetMaxMatrices()));
		
		if (!nBones)
		{
			nBones = g_pSkinningProcessor->GetMatrixPaletteNumBones(iPalette);
			assert(nBones<=g_pSkinningProcessor->GetNumBones());
			hardMappedBones = false;
		}

		if (nBones == 0)
		{
			LOGERROR( g_Log, "An object has influence clusters but 0 bones. This means the skinning is going to be broken. Try running optimize on the scene in Maya.\n");
			return false;
		}

		// set the static material element
		// this contains an index into a dynamic matrix-material for each matrix in the palette
		pMtlElement->type = RenderingData::MATERIAL_ELEMENT_TYPE_DATA_INDEX;
		pMtlElement->count = nBones;
		pMtlElement->name = m_pParam->Name.Chars();

		if (!ppData)
			return true;

		// create the dynamic material and store it's index in the static material data
		for (int iBone=0; iBone<nBones; iBone++)
		{
			// get the name of the matrix that is supposed to go in this matrix palette entry
			const char *pszName = g_pSkinningProcessor->GetBoneName(iPalette, iBone);

			if (!pszName)
			{
				int iMtxIndex = g_pSkinningProcessor->GetMatrixPaletteBoneIndex(iPalette, iBone);
				pszName = g_pSkinningProcessor->GetBone(iMtxIndex)->GetName().c_str();
			}
				
			// create a new dynamic material type with the name of the matrix that needs to be mapped into this palette entry
			RenderingData::MaterialElement dynElement;			
			
			dynElement.count = 2; // the element stores the required type and count of the output material
			dynElement.name = pszName;
			dynElement.type = RenderingData::MATERIAL_ELEMENT_TYPE_U32;

			// create the dynamic material type
			RenderingData::MaterialType materialType;
			materialType.SetName("AnimatedParam");
			materialType.SetDynamicListIndex(hardMappedBones ? 1 : 0);
			materialType.AddElement(dynElement);
			int iDynamicMtlType = pMaterialList->InsertMaterialType(materialType);

			// create a matrix-material using the new material type
			RenderingData::Material material;
			pMaterialList->CreateMaterial(material, iDynamicMtlType);
			material.SetData(0, RenderingData::MATERIAL_ELEMENT_TYPE_F32);
			material.SetData(4, 12);

			// store the index of this new material in the static material output
			*((int*)*ppData) =  pMaterialList->InsertMaterial(material);
			(*ppData) += 4;
		}

		return true;
	}

};

/// exporter plugin for exporting skinning data
class VtxHandler : public RenderDataConversion::DefaultVtxInputSourceSemantics
{
public:
	/// constructor
	VtxHandler(RenderDataConversion::VtxInputAnnotationReader *pParent) :
		RenderDataConversion::DefaultVtxInputSourceSemantics(pParent)
	{
	}

	/// parse the given source semantic and return true if we recognise it
	bool IdentifySourceSemantic(const char *pszSource)
	{
		if (!stricmp(pszSource,"blendweights"))
		{
			m_Type = E_Type_BlendWeight;
			return true;
		}
		if (!stricmp(pszSource,"blendindices"))
		{
			m_Type = E_Type_BlendIndex;
			return true;
		}
		return false;
	}

	/// get the data for the given vertex
	bool GetData(const RenderDataConversion::AftkMeshConverter *pMeshConv, int iPolygon, int iVtx, float *ppData)
	{
		const Aftk::C_SceneMeshNode *pMesh = pMeshConv->GetMeshNode();
		const Aftk::C_Polygon *pPoly = &pMesh->GetPolygon(0, iPolygon);

		U32 boneIndexArray[32];
		F32	boneWeightArray[32];

		// The boneIndexArray is relative to the global bone list. It needs to remapped to index into the
		// appropriate matrix palette for this vertex.
		g_pSkinningProcessor->GetVertexInfo(Polygon(pMesh, iPolygon), iVtx, boneIndexArray, boneWeightArray);
		S32 paletteIndex = g_pSkinningProcessor->GetMatrixPaletteIndex(Polygon(pMesh, iPolygon));

		if (!g_pSkinningProcessor->GetNumHardMappedBones())
		{
			if (paletteIndex >= 0)
			{
				S32 numPaletteBones = g_pSkinningProcessor->GetMatrixPaletteNumBones(paletteIndex);
						
				for (U32 vertexBoneIndex = 0; vertexBoneIndex < g_pSkinningProcessor->GetMaxVertexBlend(); ++vertexBoneIndex)
				{
					for (S32 paletteBoneIndex = 0; paletteBoneIndex < numPaletteBones; ++paletteBoneIndex)
					{
						if (g_pSkinningProcessor->GetMatrixPaletteBoneIndex(paletteIndex, paletteBoneIndex) == boneIndexArray[vertexBoneIndex])
						{
							boneIndexArray[vertexBoneIndex] = paletteBoneIndex;
							break;
						}
					}
				}
			}
		}
		
		if (m_Type == E_Type_BlendWeight)
		{
			U32 iTotalWeight = 255;

			S32 iKeyWeight = 0;
			F32 fKeyWeight = boneWeightArray[m_iIndex];

			for (S32 i = 1, count = 4; i < count; ++i)
			{
				if (boneWeightArray[m_iIndex + i] > fKeyWeight)
				{
					fKeyWeight = boneWeightArray[m_iIndex + i];
					iKeyWeight = i;
				}
			}

			if (iKeyWeight != 0)
				volatile S32 breakhere = 1;

			for (S32 i = 0, count = 4; i < count; ++i)
			{
				if (i == iKeyWeight)
					continue;

				F32 fWeight = boneWeightArray[m_iIndex + i];

				U32 iWeight = static_cast<U32>(::floorf(fWeight * 255.f));
				iTotalWeight -= iWeight;

				ppData[i] = iWeight / 255.f;
			}

			ppData[iKeyWeight] = iTotalWeight / 255.f;
		}
		else // E_Type_BlendIndex
		{
			ppData[0] = (float)boneIndexArray[m_iIndex+0];
			ppData[1] = (float)boneIndexArray[m_iIndex+1];
			ppData[2] = (float)boneIndexArray[m_iIndex+2];
			ppData[3] = (float)boneIndexArray[m_iIndex+3];
		}

		ppData++;

		return true;
	}

private:
	enum E_Type
	{
		E_Type_BlendWeight,
		E_Type_BlendIndex,
	};

	E_Type m_Type;
};

void Register()
{
	RenderDataConversion::RegisterAnnotationReader<Handler>();
	RenderDataConversion::RegisterVtxInputAnnotationReader<VtxHandler>();
}

void SetSkinningProcessor(I_SkinningProcessor *p)
{
	g_pSkinningProcessor = p;
}


}
