#include "stdafx.h"
#include "TreeDefinition.h"

using namespace SpeedtreeBillboard;


const U32 C_TreeBillboardSet::m_Version = 100;


C_TreeBillboard::C_TreeBillboard(
	Util::C_String sptName,
	U32 seed,
	F32 size,
	F32 sizevariance,
	F32 azimuth,
	F32 elevation)
:
	m_Name(sptName),
	m_Seed(seed),
	m_Size(size),
	m_SizeVariance(sizevariance)
{
}

C_TreeBillboard::~C_TreeBillboard()
{
}

const Util::C_String& C_TreeBillboard::GetName() const
{
	return m_Name;
}

void C_TreeBillboard::SetName(Util::C_String& treeName)
{
	m_Name = treeName;
}

S32 C_TreeBillboard::GetSeed() const
{
	return m_Seed;
}

void C_TreeBillboard::SetSeed(S32 seed)
{
	m_Seed = seed;
}

F32 C_TreeBillboard::GetSize() const
{
	return m_Size;
}

void C_TreeBillboard::SetSize(F32 size)
{
	m_Size = size;
}

F32 C_TreeBillboard::GetSizeVariance() const
{
	return m_SizeVariance;
}

void C_TreeBillboard::SetSizeVariance(
	F32 sizevariance)
{
	m_SizeVariance = sizevariance;
}

U32 C_TreeBillboard::GetNumVariants() const
{
	return m_Variants.Size();
}

U32 C_TreeBillboard::AddVariant(const C_TreeBillboard::TreeBillboardVariation& variant)
{
	m_Variants.PushBack(variant);
	return m_Variants.Size()-1;
}

const C_TreeBillboard::TreeBillboardVariation& C_TreeBillboard::GetVariant(U32 index) const
{
	assert(index<(U32)m_Variants.Size());
	return m_Variants[index];
}


C_TreeBillboardSet::C_TreeBillboardSet()
{
	m_TreeBillboardSet_HeaderBlock = "treeset";
	m_TreeBillboardSet_TextureBlock = "treetex";
	m_TreeBillboardSet_TreeBlock = "treebrd";
}

C_TreeBillboardSet::~C_TreeBillboardSet()
{
}

U32 C_TreeBillboardSet::CreateNewTextureSet(
	const Util::C_String& diffuse,
	const Util::C_String& normal)
{
	m_BillboardTextures.PushBack();
	U32 lastIndex = m_BillboardTextures.Size()-1;
	m_BillboardTextures[lastIndex].m_Diffuse = diffuse;
	m_BillboardTextures[lastIndex].m_Normal = normal;

	return lastIndex;
}

const C_TreeBillboardSet::TextureNameSet& C_TreeBillboardSet::GetTextureNameSet(U32 set)
{
	assert(set<(U32)m_BillboardTextures.Size());
	return m_BillboardTextures[set];
}

U32 C_TreeBillboardSet::GetNumTextureNameSets() const
{
	return m_BillboardTextures.Size();
}

void C_TreeBillboardSet::AddTreeBillboardRecord(const C_TreeBillboard& billboard)
{
#ifdef _DEBUG
	for (U32 i=0; i<(U32)billboard.GetNumVariants(); ++i)
		assert(billboard.GetVariant(i).m_TextureIndex < (U32)m_BillboardTextures.Size());
#endif

	m_TreeBillboards.PushBack(billboard);
}

const C_TreeBillboard& C_TreeBillboardSet::GetTreeBillboardRecord(U32 index) const
{
	assert(index<(U32)m_TreeBillboards.Size());
	return m_TreeBillboards[index];
}

U32 C_TreeBillboardSet::GetNumTreeBillboardRecords() const
{
	return m_TreeBillboards.Size();
}



