#include "stdafx.h"

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUt.h>
#include <renderingdata/renderingdata.h>
#include <converter_shared/helpers/AftkMaterialHelpers.h>
#include "converter_shared\renderdataconversion\EffectManager.h"
#include "converter_shared\renderdataconversion\Effect.h"
#include <fx/fxb.h>

#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"
#include "converter_shared\renderdataconversion\TextureAnnotationReader.h"
#include "converter_shared\renderdataconversion\ConstantAnnotationReader.h"
#include "converter_shared\renderdataconversion\VtxInputAnnotationReader.h"
#include "converter_shared\renderdataconversion\EffectPermutationSelector.h"

#include <fx/effect.h>

namespace RenderDataConversion
{

/// constructor
Effect::Effect()
{
	m_pFX = NULL;
}

/// destructor
Effect::~Effect()
{
	delete m_pFX;

	for (int i=0; i<m_ParamAnnotationReaders.Size(); i++)
		delete m_ParamAnnotationReaders[i];

	for (int i=0; i<m_VtxAnnotationReaders.Size(); i++)
		delete m_VtxAnnotationReaders[i];
}

/// get name
Util::C_String Effect::GetName()
{
	return m_sName;
}

/// initialises an effect by loading the associated .fx file
void Effect::Initialise(RenderingData::Build::Scene *pSceneExport, const Util::C_String &sFXBPath, const Util::C_String &sName)
{
	m_sName = sName;

	Load(sFXBPath);

	// if we can't load the required .fxb file, use default.fxb instead
	if (m_pFX == 0)
	{
		m_sName = g_szDefaultFXName;
		Load(sFXBPath);
		if (m_pFX == 0)
		{
			LOGFATAL( g_Log, "Failed to load '%s'. Mesh export cannot continue.\n", m_sName.Chars());
			exit(-1);
		}
	}

	SetupRenderingDataEffect(pSceneExport);

	SetupAnnotationReaders();

	SetupMaterialType(pSceneExport);

	SetupVertexFormats();
}

/// initialise all of the annotation readers
void Effect::SetupAnnotationReaders()
{
	for (int i=0; i<m_pFX->Parameters.Size(); i++)
		m_ParamAnnotationReaders.PushBack(GetAnnotationReaderForParameter(&m_pFX->Parameters[i]));

	for (int i=0; i<m_pFX->VtxInputDeclaration.Size(); i++)
		m_VtxAnnotationReaders.PushBack(new VtxInputAnnotationReader(this, &m_pFX->VtxInputDeclaration[i]));
}

/// construct the rendering data effect
void Effect::SetupRenderingDataEffect(RenderingData::Build::Scene *pSceneExport)
{
	RenderingData::Build::Effect effect;
	effect.SetName(Btl::String(m_sName.Chars()));
	m_iEffectIndex = pSceneExport->AddEffect(effect);
}

/// returns the annotation reader for a particular parameter type
ParamAnnotationReader *Effect::GetAnnotationReaderForParameter(FX::Parameter *pParam)
{
	switch (pParam->Type)
	{
	case FX::NGLFX_PARAM_SAMPLER:
		return new TextureAnnotationReader(pParam, m_pFX->Name);
	case FX::NGLFX_PARAM_VECTOR:
	case FX::NGLFX_PARAM_MATRIX:
		return new ConstantAnnotationReader(pParam, m_pFX->Name);
	default:
		return NULL;
	}
}

/// construct the rendering data material type
void Effect::SetupMaterialType(RenderingData::Build::Scene *pSceneExport)
{
	// query which effect parameters should be coming through from Maya (for each nodetype)
	for (int iType=0; iType<E_NodeType_Max; iType++)
	{
		RenderingData::MaterialType materialType;
		if (iType != E_NodeType_Instance)
			materialType.SetName(Btl::String(m_sName.Chars()));
		int iSize = 0;

		for (int i=0; i<m_ParamAnnotationReaders.Size(); i++)
		{
			ParamAnnotationReader *pA = m_ParamAnnotationReaders[i];

			RenderingData::MaterialElement materialElement;
			if (pA && pA->GetMaterialElement(&materialElement) && pA->NeedsExport((E_NodeType)iType))
			{
				materialType.AddElement(materialElement);
				iSize += RenderingData::materialDataTypeSizes[materialElement.type] * materialElement.count;
			}
		}

		if (iType != E_NodeType_Instance)
			materialType.SetEffect(m_iEffectIndex);
		m_iMaterialTypeIndex[iType] = pSceneExport->GetMaterialList().InsertMaterialType(materialType);
		m_iMaterialDataSize[iType] = iSize;
	}
}

/// returns the index of the material type associated with this effect
int Effect::GetMaterialTypeIndex(E_NodeType type)
{
	return m_iMaterialTypeIndex[type];
}

/// returns the size (in bytes) of the material data
int Effect::GetMaterialDataSize(E_NodeType type)
{
	return m_iMaterialDataSize[type];
}

/// read out data for this effect, given an Aftk Material
void Effect::GetMaterialData(AftkMaterialConverter *pMtl, U8 **ppData, E_NodeType type)
{
	// query which effect parameters should be coming through from Maya
	for (int i=0; i<m_ParamAnnotationReaders.Size(); i++)
	{
		ParamAnnotationReader *pA = m_ParamAnnotationReaders[i];

		if (pA)
		{
			if (pA->NeedsExport(type))
				pA->GetMaterialData(pMtl, ppData);
		}
	}
}

/// load the Effect from the .fx file
void Effect::Load(const Util::C_String &sFXBPath)
{
	Util::C_String sFXBName = sFXBPath + "\\" + m_sName + "b";
	Sys::I_File *pFile = Sys::C_FileSystem::Instance().Open(sFXBName);

	if (!pFile)
	{
		LOGWARN( g_Log, "Failed to open shader '%s'.\n", sFXBName.Chars());
	}

	// load
	if (!pFile)
	{
		m_pFX = NULL;
		return;
	}

	m_pFX = (FX::Effect*)FX::ImportFXB(pFile);
	delete pFile;
}


/// construct the vertex format
void Effect::SetupVertexFormats()
{
	for (int t=0; t<E_NodeType_Max; t++)
	{
		RenderingData::VertexFormat *pFormat = &m_VertexFormat[(E_NodeType)t];
		assert(pFormat->GetNumVertexElements() == 0);

		// query which effect parameters should be coming through from Maya
		for (int i=0; i<m_VtxAnnotationReaders.Size(); i++)
		{
			VtxInputAnnotationReader *pAr = m_VtxAnnotationReaders[i];

			if (pAr->NeedsExport((E_NodeType)t))
			{
				RenderingData::VertexFormat::VertexElement element;
				if (pAr->GetVertexDescElement(&element))
					pFormat->AddVertexElement(element);
			}
		}
	}
}

/// get the renderingdata vertex format for this effect
const RenderingData::VertexFormat *Effect::GetVertexFormat(E_NodeType type)
{
	return &m_VertexFormat[type];
}

/// get the vertex buffer data for a single Aftk vertex
void Effect::GetVertexData(const AftkMeshConverter *pMesh, int iPoly, int iVertex, E_NodeType nodeType, U8 **ppData)
{
	// for each vertex input semantic, get the data
	for (int i=0; i<m_VtxAnnotationReaders.Size(); i++)
	{
		VtxInputAnnotationReader *pAr = m_VtxAnnotationReaders[i];
		if (pAr->NeedsExport(nodeType))
			pAr->GetVertexData(pMesh, iPoly, iVertex, ppData);
	}
}

/// get the .fx #define permutation for the given node + material
void Effect::GetPermutation(EffectPermutationSelector *pEPS)
{
	// for each vertex input semantic, check whether it exists
	for (int i=0; i<m_VtxAnnotationReaders.Size(); i++)
	{
		VtxInputAnnotationReader *pAr = m_VtxAnnotationReaders[i];
		if (!pAr->GetVertexData(pEPS->GetMeshConverter(), 0, 0, NULL))
			pAr->SetPermutation(pEPS);
	}

	// for each input parameter, check whether it exists
	for (int i=0; i<m_ParamAnnotationReaders.Size(); i++)
	{
		ParamAnnotationReader *pA = m_ParamAnnotationReaders[i];

		if (pA)
		{
			if (pA->NeedsExport(E_NodeType_Mesh) && !pA->GetMaterialData(pEPS->GetMtlConverter(), NULL))
				pA->SetPermutation(pEPS);
		}
	}
}

};
