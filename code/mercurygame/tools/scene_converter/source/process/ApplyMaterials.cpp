#include "stdafx.h"

#include <aftk/Aftk.h>
#include "converter_shared/helpers/AftkMaterialHelpers.h"
#include "process/ApplyMaterials.h"

/// constructor
C_ApplyMaterials::C_ApplyMaterials()
 : m_piConversionTable(0), m_nTextures(0), m_nMaterials(0)
{
}

/// destructor
C_ApplyMaterials::~C_ApplyMaterials()
{
	Purge();
}

// generate a table of unique combinations of material no. and additional data no.
void C_ApplyMaterials::Initialize(Aftk::C_Scene *pScene, int nTex)
{
	m_pAftkScene = pScene;

	// if no charts were created, don't generate a conversion table either
	m_nTextures	 = nTex;
	m_nMaterials = m_pAftkScene->GetNumMaterialNodes()+1;
	m_nNewMaterials = 0;

	m_piConversionTable = new const Aftk::C_MaterialResource* [m_nTextures * m_nMaterials];

	// initialize table
	for (int i=0; i<m_nTextures*m_nMaterials; i++)
		m_piConversionTable[i] = NULL;

	m_AttributeList.Clear();

	GetMaterialsFromAftkManager();
}

// convert a material + chart number into an attribute number
const Aftk::C_MaterialResource *C_ApplyMaterials::GetAttribute(int iMaterial, int iAtlas, bool *pbNew)
{
	*pbNew = false;

	iMaterial++;

//	assert(iMaterial>=0 && iMaterial<m_nMaterials);
	assert(iAtlas>=0 && iAtlas<m_nTextures);

	// lookup this combination in our table
	assert(iMaterial + iAtlas * m_nMaterials < m_nTextures * m_nMaterials);
	const Aftk::C_MaterialResource **ppRef = &m_piConversionTable[iMaterial + iAtlas * m_nMaterials];
	if (*ppRef == NULL)
	{
		// this combination hasn't come up yet - make a new entry in the attribute table
//		*ppRef = m_AttributeList.Size();

		SAttribute sa;
		sa.iMaterial = iMaterial;
		sa.iAtlas = iAtlas;
		m_AttributeList.PushBack(sa);
		*pbNew = true;
	}

	return *ppRef;
}

// set an attribute
void C_ApplyMaterials::SetAttribute(int iMaterial, int iAtlas, const Aftk::C_MaterialResource *pRef)
{
	iMaterial++;

	assert(iMaterial>=0 && iMaterial<m_nMaterials);
	assert(iAtlas>=0 && iAtlas<m_nTextures);

	// set this combination in our table
	assert(iMaterial + iAtlas * m_nMaterials < m_nTextures * m_nMaterials);
	m_piConversionTable[iMaterial + iAtlas * m_nMaterials] = pRef;
}

// get number of entries in the table
int C_ApplyMaterials::GetNumAttributes()
{
	return m_AttributeList.Size();
}

// free the tables
void C_ApplyMaterials::Purge()
{
	m_AttributeList.Clear();
	delete [] m_piConversionTable;
}

// copy materials from manager and delete the manager's copy
void C_ApplyMaterials::GetMaterialsFromAftkManager()
{
	int n = m_pAftkScene->GetNumMaterialNodes();
	for (int i=0; i<n; i++)
	{
		const Aftk::C_MaterialResource* pRes = m_pAftkScene->GetMaterialResourcePtr(i);
		Aftk::C_MaterialNode *pMtl = const_cast<Aftk::C_MaterialNode*>(&pRes->GetContent());

		m_MaterialList.PushBack( *pMtl );

		m_MtlMap[ pMtl ] = pRes->GetIndex();
	}
}

// 
void C_ApplyMaterials::SetMaterialResourcePtr(Aftk::C_Polygon* pPoly, const Aftk::C_MaterialResource* pRef)
{
	Aftk::C_SceneMeshNode* pMesh = pPoly->GetMeshNodePtr();

	if (pRef)
	{
		S16 materialLocalIndex = pMesh->AddPolygonMaterialMapping(pRef);
		assert(materialLocalIndex != -1);
		pPoly->SetMaterialMappingIndex(materialLocalIndex);
	}
	else
	{
		pPoly->SetMaterialMappingIndex(-1);
	}
}

// assign a lightmap to this polygon's material
void C_ApplyMaterials::SetMaterialToPolygon(Aftk::C_Polygon *pPoly, int iAtlas)
{
	// get this polygon's material ID
	int iID = -1;
	const Aftk::C_MaterialResource *pRes = AftkHelpers::GetMaterialResourcePtr(pPoly);
	if (pRes)
		iID = pRes->GetIndex();

	pPoly->DeleteMaterial();

	const Aftk::C_MaterialResource *pRef = GetMaterialCombo(iID, iAtlas);

	// set it to the current polygon
	SetMaterialResourcePtr(pPoly, pRef);
}

// get a material from an old mtl + atlas combo
const Aftk::C_MaterialResource *C_ApplyMaterials::GetMaterialCombo(int iMat, int iAtlas)
{
	bool	bNew;
	const Aftk::C_MaterialResource *pRef = GetAttribute(iMat, iAtlas, &bNew);

	// if this is a new combination - create a new material
	if (bNew)
	{
		Aftk::C_MaterialNode newMaterial;

		// make copy of original material (if it exists)
		if (iMat != -1)
			newMaterial = m_MaterialList[iMat];

		pRef = GenerateNewMaterial(newMaterial, iAtlas);
		SetAttribute(iMat, iAtlas, pRef);

		m_nNewMaterials++;
	}

	return pRef;
}

// generate a new material from an old one + an added lightmap
/*
const Aftk::C_MaterialResource *C_ApplyMaterials::GenerateNewMaterial(Aftk::C_MaterialNode &newMaterial, int iAtlas )
{
	const Aftk::C_MaterialNode *pOriginalMat;

	// check whether this material already has a lightmap applied to it
	for (U32 i=0; i<newMaterial.GetNumShaderImageNodes(); i++)
	{
		const std::string &name = newMaterial.GetShaderImageNodePtr(i)->GetName();
		if (name == "lightmap0")
			return m_pAftkScene->AddMaterialNode(newMaterial);
	}

	// generate the filename for the atlas that this chart is in
	char *pszName = GenerateLightmapFilename( iAtlas );

	//2) Create image node
	Aftk::C_ShaderImageNode* pShaderImageNode = new Aftk::C_ShaderImageNode();
	pShaderImageNode->SetName("lightmap0");
	pShaderImageNode->GetFilePlug().SetValue(pszName);
	pShaderImageNode->GetImageTypePlug().SetValue("lightmap");
	//Add shader image node to material
	newMaterial.AddShaderNodePtr(pShaderImageNode);

	//3) Create main shader node
	Aftk::C_ShaderNode* pShaderNode = new Aftk::C_ShaderNode();
	pShaderNode->SetName("luxProperties");
	{
		Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
		pShaderNode->AddPlugPtr(pPlug);
		pPlug->SetName("lightmap0");

		//Connect lightmap0 plug to lightmap0 file node
		pPlug->SetSourceConnectionPtr(pShaderImageNode);
	}
	//Add shader node to material
	newMaterial.AddShaderNodePtr(pShaderNode);

	//4) Setup material properties
	{
		Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
		newMaterial.AddPlugPtr(pPlug);
		pPlug->SetName("lux");

		//Connect lux plug to luxProperties node
		pPlug->SetSourceConnectionPtr(pShaderNode);
	}

	newMaterial.SetName( newMaterial.GetName() + ".c" );

	// return ref
	return m_pAftkScene->AddMaterialNode(newMaterial);
}
*/

