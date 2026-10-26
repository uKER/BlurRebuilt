#include "stdafx.h"

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUt.h>
#include <texture_export/texturemanager.h>
#include <texture_export/textureprocessing.h>
#include <renderingdata/renderingdata.h>
#include "converter_shared\renderdataconversion\MaterialManager.h"
#include "converter_shared\renderdataconversion\ConvertAftkMaterial.h"
#include "converter_shared\renderdataconversion\ConvertAftkMesh.h"
#include "converter_shared\renderdataconversion\EffectManager.h"
#include "converter_shared\renderdataconversion\Effect.h"
#include "converter_shared\renderdataconversion\EffectPermutationSelector.h"
#include <converter_shared/helpers/AftkMaterialHelpers.h>
#include <system/misc/StringUtil.h>
#include <algorithm>

namespace RenderDataConversion
{

/// constructor
EffectPermutationSelector::EffectPermutationSelector(MaterialManager *pMgr, const Aftk::C_SceneMeshNode *pMesh, const Aftk::C_MaterialNode *pMtl)
{
	m_pMgr = pMgr;
	m_pMesh = pMesh;
	m_pMtl = pMtl;

	m_pMtlConverter = new AftkMaterialConverter(m_pMgr, m_pMtl, E_NodeType_Mesh);
	m_pMeshConverter = new AftkMeshConverter(m_pMesh);
}

/// destructor
EffectPermutationSelector::~EffectPermutationSelector()
{
	delete m_pMtlConverter;
	delete m_pMeshConverter;
}

/// loads an effect and determines which permutation should be used for this particular mesh/material
Util::C_String EffectPermutationSelector::GetPermutation(const Util::C_String &sName)
{
	if (g_szGlobalEffectVariation.Length() > 0)
		AddPermutation(g_szGlobalEffectVariation);

	// calculate permutations
	Effect *pEffect = m_pMgr->m_pEffectManager->GetEffect(sName);
	pEffect->GetPermutation(this);

	// sort permutations
	int n = m_Permutations.Size();
	if (n>0)
		std::sort(&m_Permutations[0], &m_Permutations[0]+n);

	// concatenate permutations
	Util::C_String sRtn = Util::RemoveFileExtension(sName);
	for (int i=0; i<m_Permutations.Size(); i++)
		sRtn = sRtn + "_" + m_Permutations[i];

	return sRtn + ".fx";
}

/// get the aftk material converter
AftkMaterialConverter *EffectPermutationSelector::GetMtlConverter()
{
	return m_pMtlConverter;
}

/// get the aftk node converter
AftkMeshConverter *EffectPermutationSelector::GetMeshConverter()
{
	return m_pMeshConverter;
}

/// add a new permutation string, checks for duplicates
void EffectPermutationSelector::AddPermutation(const Util::C_String &s)
{
	// check for duplicates
	for (int i=0; i<m_Permutations.Size(); i++)
		if (m_Permutations[i] == s)
			return;

	m_Permutations.PushBack(s);
}

}
