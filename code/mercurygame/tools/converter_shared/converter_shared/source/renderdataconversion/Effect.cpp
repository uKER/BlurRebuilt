#include "stdafx.h"

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUt.h>
#include <Display/Device/DeviceTypesEnums.h>
#include <renderingdata/renderingdata.h>
#include <converter_shared/helpers/AftkMaterialHelpers.h>
#include "converter_shared\renderdataconversion\EffectManager.h"
#include "converter_shared\renderdataconversion\Effect.h"
#include "converter_shared\renderdataconversion\ConvertAftkMaterial.h"
#include <fx/fxb.h>

#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"
#include "converter_shared\renderdataconversion\VtxInputAnnotationReader.h"
#include "converter_shared\renderdataconversion\EffectPermutationSelector.h"

#include "converter_shared\renderdataconversion\CustomSemanticManager.h"

#include <fx/effect.h>

namespace RenderDataConversion
{

/// constructor
Effect::Effect()
{
	m_pFX = NULL;
	m_iEffectIndex = -1;
	m_pSceneExport = NULL;
	for (int i=0; i<E_NodeType_Max; i++)
		m_nUsedVertexStreams[i] = 0;
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
bool Effect::Initialise(RenderingData::Build::Scene *pSceneExport, const Util::C_String &sFXBPath, const Util::C_String &sName)
{
	m_sName = sName;

	Load(sFXBPath);

	// if we can't load the required .fxb file, use default.fxb instead
	if (m_pFX == 0)
		return false;

	SetupRenderingDataEffect(pSceneExport);

	SetupAnnotationReaders();

	SetupVertexFormats();

	return true;
}

/// return true if the parameter with the given index is referenced in the given bindings vector
bool Effect::IsParameterUsed(Btl::Vector<FX::FunctionBinding> &bindings, int iParam)
{
	for (int i=0; i<bindings.Size(); i++)
	{
		if (bindings[i].ParameterIdx == iParam)
			return true;
	}

	return false;
}

/// returns false if the parameter with the given index is not used in any of the techniques
bool Effect::IsParameterUsed(int iParam)
{
	for (int iTech=0; iTech<m_pFX->Techniques.Size(); iTech++)
	{
		FX::Technique *pTech = &m_pFX->Techniques[iTech];
		for (int iPass=0; iPass<pTech->Passes.Size(); iPass++)
		{
			FX::Pass *pPass = &pTech->Passes[iPass];
			if (IsParameterUsed(pPass->PixelShader.Bindings, iParam) ||
				IsParameterUsed(pPass->VertexShader.Bindings, iParam))
				return true;
		}
	}

	return false; // not used
}

/// initialise all of the annotation readers
void Effect::SetupAnnotationReaders()
{
	for (int i=0; i<m_pFX->Parameters.Size(); i++)
	{
		ParamAnnotationReader *p = g_CustomSemanticManager.FindAnnotationReader(&m_pFX->Parameters[i], m_pFX->Name);
		if (p)
			p->SetUsed(IsParameterUsed(i));
		m_ParamAnnotationReaders.PushBack(p);
	}

	for (int i=0; i<m_pFX->VtxInputDeclaration.Size(); i++)
		m_VtxAnnotationReaders.PushBack(new VtxInputAnnotationReader(this, &m_pFX->VtxInputDeclaration[i]));
}

/// construct the rendering data effect
void Effect::SetupRenderingDataEffect(RenderingData::Build::Scene *pSceneExport)
{
	m_pSceneExport = pSceneExport;
}

/// return the index of this effect in the rendering data
int Effect::GetEffectIndex()
{
	if (m_iEffectIndex != -1)
		return m_iEffectIndex;
	
	RenderingData::Build::Effect effect;
	effect.SetName(Btl::String(m_sName.Chars()));
	m_iEffectIndex = m_pSceneExport->AddEffect(effect);
	return m_iEffectIndex;
}

/// read out data for this effect, given an Aftk Material
void Effect::GetMaterialData(AftkMaterialConverter *pMtl, RenderingData::MaterialType *pMaterialType, int *piSize, U8 **ppData, E_NodeType type)
{
	if (type != E_NodeType_Instance)
	{
		pMaterialType->SetName(Btl::String(GetName().Chars()));
		pMaterialType->SetEffect(GetEffectIndex());
	}

	AnnotationReaderList::GetMaterialData(pMtl, pMaterialType, piSize, ppData, type);
}

/// load the Effect from the .fx file
void Effect::Load(const Util::C_String &sFXBPath)
{
	Util::C_String sFXBName = sFXBPath + "\\" + m_sName + "b";
	Sys::I_File *pFile = Sys::C_FileSystem::Instance().Open(sFXBName);

	if (!pFile)
	{
		LOGDEBUG( g_Log, "Failed to open shader '%s'.\n", sFXBName.Chars());
		m_pFX = NULL;
		return;
	}

	m_pFX = (FX::Effect*)FX::ImportFXB(pFile);
	delete pFile;
}

/// dump a vertex declaration element to a string
Util::C_String Effect::DumpVertexDeclElement(const RenderingData::VertexFormat::VertexElement e)
{
	Util::C_String name = "???", type = "???";
	switch (e.usage)
	{
	case RenderingData::VERTEX_ELEMENT_USAGE_POSITION: name = "POSITION"; break;
	case RenderingData::VERTEX_ELEMENT_USAGE_BLENDWEIGHT: name = "BLENDWEIGHT"; break;
	case RenderingData::VERTEX_ELEMENT_USAGE_BLENDINDICES: name = "BLENDINDICES"; break;
	case RenderingData::VERTEX_ELEMENT_USAGE_NORMAL: name = "NORMAL"; break;
	case RenderingData::VERTEX_ELEMENT_USAGE_PSIZE: name = "PSIZE"; break;
	case RenderingData::VERTEX_ELEMENT_USAGE_TEXCOORD: name = "TEXCOORD"; break;
	case RenderingData::VERTEX_ELEMENT_USAGE_TANGENT: name = "TANGENT"; break;
	case RenderingData::VERTEX_ELEMENT_USAGE_BINORMAL: name = "BINORMAL"; break;
	case RenderingData::VERTEX_ELEMENT_USAGE_TESSFACTOR: name = "TESSFACTOR"; break;
	case RenderingData::VERTEX_ELEMENT_USAGE_COLOR: name = "COLOR"; break;
	case RenderingData::VERTEX_ELEMENT_USAGE_SAMPLE: name = "SAMPLE"; break;
	}
	switch(e.type)
	{
	case RenderingData::VERTEX_ELEMENT_TYPE_S16_2: type = "S16_2"; break;
	case RenderingData::VERTEX_ELEMENT_TYPE_S16_4: type = "S16_4"; break;
	case RenderingData::VERTEX_ELEMENT_TYPE_S16_2N: type = "S16_2N"; break;
	case RenderingData::VERTEX_ELEMENT_TYPE_S16_4N: type = "S16_4N"; break;
	case RenderingData::VERTEX_ELEMENT_TYPE_F32_1: type = "F32_1"; break;
	case RenderingData::VERTEX_ELEMENT_TYPE_F32_2: type = "F32_2"; break;
	case RenderingData::VERTEX_ELEMENT_TYPE_F32_3: type = "F32_3"; break;
	case RenderingData::VERTEX_ELEMENT_TYPE_F32_4: type = "F32_4"; break;
	case RenderingData::VERTEX_ELEMENT_TYPE_F16_2: type = "F16_2"; break;
	case RenderingData::VERTEX_ELEMENT_TYPE_F16_4: type = "F16_4"; break;
	case RenderingData::VERTEX_ELEMENT_TYPE_U8_4: type = "U8_4"; break;
	case RenderingData::VERTEX_ELEMENT_TYPE_U8_4N: type = "U8_4N"; break;
	case RenderingData::VERTEX_ELEMENT_TYPE_SHHD_3N: type = "SHHD_3N"; break;
	case RenderingData::VERTEX_ELEMENT_TYPE_SDDD_3N: type = "SDDD_3N"; break;
	}

	Util::C_String sRtn;
	sRtn.Printf("%s%d %s", name.Chars(), e.usageIndex, type.Chars());
	return sRtn;
}

/// construct the vertex format
void Effect::SetupVertexFormats()
{
	int nTotalElements = 0;

	for (int t=0; t<E_NodeType_Max; t++)
	{
		if (t == E_NodeType_InstanceReset)
			continue;	// instance reset material does not have any associated vertex data

		LOGDEBUG( g_Log, "--- Vertex declaration for %s (%d)\n", GetName().Chars(), (int)t);

		// query which effect parameters should be coming through from Maya
		for (int i=0; i<m_VtxAnnotationReaders.Size(); i++)
		{
			VtxInputAnnotationReader *pAr = m_VtxAnnotationReaders[i];

			if (pAr->NeedsExport((E_NodeType)t))
			{
				RenderingData::VertexFormat::VertexElement element;
				S32 iStream;
				if (pAr->GetVertexDescElement(&element, &iStream))
				{
					RenderingData::VertexFormat *pFormat = &m_VertexFormat[(E_NodeType)t][iStream];
					pFormat->AddVertexElement(element);
					nTotalElements++;
					LOGDEBUG( g_Log, "%s (%s) (stream %d)\n", DumpVertexDeclElement(element).Chars(), pAr->DebugDesc().Chars(), iStream);

					if (m_nUsedVertexStreams[t]<=iStream)
						m_nUsedVertexStreams[t] = iStream+1;
				}
			}
		}

		LOGDEBUG( g_Log, "\n");

		if (t != E_NodeType_Instance && nTotalElements == 0)
		{
			LOGERROR( g_Log, "Effect %s generated empty vertex buffer. Check your shader code.\n", m_sName.Chars());
			assert(false);
		}
	}
}

/// get the renderingdata vertex format for this effect
const RenderingData::VertexFormat *Effect::GetVertexFormat(E_NodeType type, int iStream)
{
	assert(iStream>=0 && iStream<MAX_VERTEX_STREAMS);
	return &m_VertexFormat[type][iStream];
}

/// returns the number of used vertex streams
int Effect::GetNumVertexStreams(E_NodeType t)
{
	return m_nUsedVertexStreams[t];
}

/// get the vertex buffer data for a single Aftk vertex
void Effect::GetVertexData(const AftkMeshConverter *pMesh, int iPoly, int iVertex, E_NodeType nodeType, int iStream, U8 **ppData)
{
	// for each vertex input semantic, get the data
	for (int i=0; i<m_VtxAnnotationReaders.Size(); i++)
	{
		VtxInputAnnotationReader *pAr = m_VtxAnnotationReaders[i];
		if (pAr->NeedsExport(nodeType) && pAr->NeedsExportingForStream(iStream))
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

	RenderingData::MaterialElement dummy;

	// for each input parameter, check whether it exists
	for (int i=0; i<m_ParamAnnotationReaders.Size(); i++)
	{
		ParamAnnotationReader *pA = m_ParamAnnotationReaders[i];

		if (pA)
		{
			if (pA->NeedsExport(E_NodeType_Mesh) && !pA->GetMaterialData(pEPS->GetMtlConverter(), &dummy, NULL))
				pA->SetPermutation(pEPS);
		}
	}
}

/// see if we have a sorttype annotation
void Effect::GetSortTypeAnnotation(RenderingData::SortType *pSortType)
{
	if (m_pFX->Techniques.Size() < 1)
		return;

	// see if we have a 'sorttype' annotation
	Btl::Vector<FX::Annotation> &annotations = m_pFX->Techniques[0].Annotations;
	for (int i=0; i<annotations.Size(); i++)
	{
		if (!stricmp(annotations[i].Name.Chars(), "SortType") && annotations[i].Type == FX::NGLFX_ANNOTATION_STRING)
		{
			if (!stricmp(annotations[i].String, "backfront"))
			{
				*pSortType = RenderingData::SORT_TYPE_BACK_FRONT;
			}				
			else if (!stricmp(annotations[i].String, "key"))
			{
				*pSortType = RenderingData::SORT_TYPE_KEY;
			}
			else if (!stricmp(annotations[i].String, "postresolve"))
			{
				*pSortType = RenderingData::SORT_TYPE_POST_RESOLVE;
			}
			else if (!stricmp(annotations[i].String, "racification"))
			{
				*pSortType = RenderingData::SORT_TYPE_RACIFICATION;
			}
		}
	}
}

/// returns the value of the annotation with the given name, if found
const char *Effect::GetAnnotation(const char *psz)
{
	if (m_pFX->Techniques.Size() < 1)
		return NULL;

	// see if we have a 'sorttype' annotation
	Btl::Vector<FX::Annotation> &annotations = m_pFX->Techniques[0].Annotations;
	for (int i=0; i<annotations.Size(); i++)
	{
		if (!stricmp(annotations[i].Name.Chars(), psz))
		{
			if (annotations[i].Type != FX::NGLFX_ANNOTATION_STRING)
			{
				LOGWARN( g_Log, "Annotation '%s' on shader '%s' unexpected type (expected string).\n",
					psz, m_sName.Chars());
			}
			return annotations[i].String;
		}
	}

	return NULL;
}

/// returns the value of the annotation with the given name, if found
bool Effect::GetAnnotation(const char *psz, bool *pbValue)
{
	for (int iTech=0; iTech<m_pFX->Techniques.Size(); iTech++)
	{
		// see if we have the annotation with the given name
		Btl::Vector<FX::Annotation> &annotations = m_pFX->Techniques[iTech].Annotations;
		for (int i=0; i<annotations.Size(); i++)
		{
			if (!stricmp(annotations[i].Name.Chars(), psz))
			{
				if (annotations[i].Type != FX::NGLFX_ANNOTATION_BOOL)
				{
					LOGWARN( g_Log, "Annotation '%s' on shader '%s' unexpected type (expected bool).\n",
						psz, m_sName.Chars());
				}
				*pbValue = annotations[i].Bool?true:false;
				return true;
			}
		}
	}

	return false;
}

/// see if we have a decal order override annotation
bool Effect::GetAnnotation(const char *psz, int *piValue)
{
	for (int iTech=0; iTech<m_pFX->Techniques.Size(); iTech++)
	{
		// see if we have the annotation with the given name
		Btl::Vector<FX::Annotation> &annotations = m_pFX->Techniques[iTech].Annotations;
		for (int i=0; i<annotations.Size(); i++)
		{
			if (!stricmp(annotations[i].Name.Chars(), psz))
			{
				if (annotations[i].Type != FX::NGLFX_ANNOTATION_INT)
				{
					LOGWARN( g_Log, "Annotation '%s' on shader '%s' unexpected type (expected int).\n",
						psz, m_sName.Chars());
				}
				*piValue = annotations[i].Int;
				return true;
			}
		}
	}

	return false;
}

/// get the default blend state from the .fx file
void Effect::GetDefaultBlendState(RenderingData::BlendState *pBlendState)
{
	if (m_pFX->Techniques.Size() < 1 || m_pFX->Techniques[0].Passes.Size() < 1)
		return;

	FX::nglFXRenderStates *pRS = &m_pFX->Techniques[0].Passes[0].RenderStates;
	if (pRS->SrcBlend != FX::E_Blend_Unknown)
		pBlendState->SetSrcBlend((RenderingData::BlendFactor)((U32)pRS->SrcBlend-1));
	if (pRS->DestBlend != FX::E_Blend_Unknown)
		pBlendState->SetDestBlend((RenderingData::BlendFactor)((U32)pRS->DestBlend-1));
	switch (pRS->BlendOp)
	{
	case FX::E_BlendOp_Unknown: break;
	case FX::E_BlendOp_Add: pBlendState->SetBlendOp(RenderingData::BLEND_OP_ADD); break;
	case FX::E_BlendOp_Subtract: pBlendState->SetBlendOp(RenderingData::BLEND_OP_SUBTRACT); break;
	case FX::E_BlendOp_RevSubtract: pBlendState->SetBlendOp(RenderingData::BLEND_OP_REVSUBTRACT); break;
	case FX::E_BlendOp_Min: pBlendState->SetBlendOp(RenderingData::BLEND_OP_MIN); break;
	case FX::E_BlendOp_Max: pBlendState->SetBlendOp(RenderingData::BLEND_OP_MAX); break;
	default: BIZ_ASSERT(0); break; // shouldn't happen
	}
	if (pRS->AlphaBlendEnable != FX::E_Bool_Unknown)
		pBlendState->SetAlphaBlendEnable(pRS->AlphaBlendEnable?true:false);
	if (pRS->AlphaTestEnable != FX::E_Bool_Unknown)
		pBlendState->SetAlphaTestEnable(pRS->AlphaTestEnable?true:false);
//	if (pRS->AlphaRef > 0)
//		pBlendState->SetAlphaRef(pRS->AlphaRef);
	if (pRS->AlphaToMaskEnable != FX::E_Bool_Unknown)
		pBlendState->SetAlphaToMaskEnable(pRS->AlphaToMaskEnable?true:false);
	switch (pRS->CullMode)
	{
	case FX::E_CullMode_Ccw:	pBlendState->SetCullMode(RenderingData::CULL_MODE_REVERSED);	break;
	case FX::E_CullMode_Cw:	pBlendState->SetCullMode(RenderingData::CULL_MODE_NORMAL);		break;
	case FX::E_CullMode_None:	pBlendState->SetCullMode(RenderingData::CULL_MODE_NONE);		break;
	default: break;
	}
	pBlendState->SetZBias((S32)pRS->DepthBias);
}

};
