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

/// read out data for this effect, given an Aftk Material
void AnnotationReaderList::GetMaterialData(AftkMaterialConverter *pMtl, RenderingData::MaterialType *pMaterialType, int *piSize, U8 **ppData, E_NodeType type)
{
	RenderingData::MaterialElement materialElement;
	int iSize = 0;
	U8 *pStart = *ppData;

	// query which effect parameters should be coming through from Maya
	for (int i=0; i<m_ParamAnnotationReaders.Size(); i++)
	{
		ParamAnnotationReader *pA = m_ParamAnnotationReaders[i];

		if (pA && pA->NeedsExport(type))
		{
			// if this parameter is not used on this particular effect,
			// and it's not a shared parameter that is being set on an instance, then skip it
			if (!pA->IsUsed() && type != E_NodeType_Instance)
				continue;

			// if this parameter is on a different stream than the one we're exporting then skip it
			if (!pA->NeedsExportingForStream(pMtl->GetStream()))
				continue;

			U8 **ppDataOld = ppData;
			if (pA->GetMaterialData(pMtl, &materialElement, ppData) || !pA->m_bAllowOmit)
			{
				pMaterialType->AddElement(materialElement);
				iSize += RenderingData::materialDataTypeSizes[materialElement.type] * materialElement.count;
			}
			else
				ppData = ppDataOld;
			assert(*ppData - pStart == iSize);
		}
	}

	*piSize = iSize;
}

/// add an annotation reader (checks for duplicates by name)
void AnnotationReaderList::AddAnnotationReader(ParamAnnotationReader *pA)
{
	for (int i=0; i<m_ParamAnnotationReaders.Size(); i++)
	{
		if (!strcmp(m_ParamAnnotationReaders[i]->GetFXParameter()->Name.Chars(), pA->GetFXParameter()->Name.Chars()))
			return;
	}

	m_ParamAnnotationReaders.PushBack(pA);
}

/// add all annotation readers from a given effect to the current effect, checks for duplicates by name
void AnnotationReaderList::AddAnnotationReadersFromEffect(Effect *pEffect, E_NodeType type)
{
	for (int i=0; i<pEffect->m_ParamAnnotationReaders.Size(); i++)
	{
		ParamAnnotationReader *pA = pEffect->m_ParamAnnotationReaders[i];

		if (pA && pA->NeedsExport(type) && pA->IsUsed())
			AddAnnotationReader(pA);
	}
}

};
