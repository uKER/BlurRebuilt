//-----------------------------------------------------------------------------
// File: ApplyMaterials.cpp
//
// Desc: Apply new materials (including lightmaps) to the mesh
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Lux/TextureTypes/RGBTexture.h"
#include "Lux/Parameterization/TextureAtlas.h"
#include "Lux/LightmapPacking/ApplyMaterials.h"
#include "Lux/Parameterization/LMPolyAttributes.h"


// --- Attribute table

// generate a table of unique combinations of material no. and chart no.
void CAttributeTable::Initialize(int nMat, int nTex)
{
	// if no charts were created, don't generate a conversion table either
	m_nTextures	 = nTex;
	m_nMaterials = (nMat+1);

	m_piConversionTable = new const Aftk::C_MaterialResource* [m_nTextures * m_nMaterials];

	// initialize table
	for (int i=0; i<m_nTextures*m_nMaterials; i++)
		m_piConversionTable[i] = NULL;

	m_AttributeList.Clear();
}

// convert a material + chart number into an attribute number
const Aftk::C_MaterialResource *CAttributeTable::GetAttribute(int iMaterial, int iAtlas, bool *pbNew)
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
void CAttributeTable::SetAttribute(int iMaterial, int iAtlas, const Aftk::C_MaterialResource *pRef)
{
	iMaterial++;

	assert(iMaterial>=0 && iMaterial<m_nMaterials);
	assert(iAtlas>=0 && iAtlas<m_nTextures);

	// set this combination in our table
	assert(iMaterial + iAtlas * m_nMaterials < m_nTextures * m_nMaterials);
	m_piConversionTable[iMaterial + iAtlas * m_nMaterials] = pRef;
}

// get number of entries in the table
int CAttributeTable::GetNumAttributes()
{
	return m_AttributeList.Size();
}

// free the tables
void CAttributeTable::Purge()
{
	m_AttributeList.Clear();
	delete [] m_piConversionTable;
}



// ---
