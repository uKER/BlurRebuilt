
#include <system/Serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_FileBlock.h>


	template <typename S>
	void SpeedtreeBillboard::C_TreeBillboard::Serialise(S& s) const
	{
		s << m_Name;
		s << m_Seed;

		s << (U32)m_Variants.Size();
		for (U32 i=0; i<(U32)m_Variants.Size(); ++i)
		{
			s << m_Variants[i].m_Azimuth;
			s << m_Variants[i].m_Elevation;
			s << m_Variants[i].m_Condition;
			s << m_Variants[i].m_TextureIndex;
			s << m_Variants[i].m_MinX << m_Variants[i].m_MinY << m_Variants[i].m_MaxX << m_Variants[i].m_MaxY;
		}
	};

	template <typename S>
	void SpeedtreeBillboard::C_TreeBillboard::Deserialise(S& s)
	{
		s >> m_Name;
		s >> m_Seed;

		U32 numVariants;
		s >> numVariants;

		m_Variants.Resize(numVariants);
		for (U32 i=0; i<numVariants; ++i)
		{
			s >> m_Variants[i].m_Azimuth;
			s >> m_Variants[i].m_Elevation;
			s >> m_Variants[i].m_Condition;
			s >> m_Variants[i].m_TextureIndex;
			s >> m_Variants[i].m_MinX;
			s >> m_Variants[i].m_MinY;
			s >> m_Variants[i].m_MaxX;
			s >> m_Variants[i].m_MaxY;
		}
	};


	bool SpeedtreeBillboard::C_TreeBillboardSet::Serialise(Sys::I_File& file) const
	{
		
		Sys::C_FileBlock headerBlock(&file, m_TreeBillboardSet_HeaderBlock);
		Sys::BinarySerialiserAutoEndian headerSer(headerBlock);

		headerSer << m_Version;
		headerBlock.Overflow();

		Sys::C_FileBlock textureBlock(&file, m_TreeBillboardSet_TextureBlock);
		Sys::BinarySerialiserAutoEndian textureSer(textureBlock);
	
		textureSer << (U32) m_BillboardTextures.Size();
		for (U32 i=0; i<(U32)m_BillboardTextures.Size(); ++i)
		{
			textureSer << m_BillboardTextures[i].m_Diffuse;
			textureSer << m_BillboardTextures[i].m_Normal;
		}
		textureBlock.Overflow();
	
		Sys::C_FileBlock treeBlock(&file, m_TreeBillboardSet_TreeBlock);
		Sys::BinarySerialiserAutoEndian treeSer(treeBlock);
	
		treeSer << (U32) m_TreeBillboards.Size();
		for (U32 i=0; i<(U32)m_TreeBillboards.Size(); ++i)
			treeSer << m_TreeBillboards[i];

		for (U32 i=0; i<(U32)m_TreeBillboards.Size(); ++i)
		{
			LOGDEBUG(g_Log, "%s %d\n", m_TreeBillboards[i].GetName().Chars(), m_TreeBillboards[i].GetSeed());
			
			for (U32 j=0; j<m_TreeBillboards[i].GetNumVariants(); ++j)
			{
				const C_TreeBillboard::TreeBillboardVariation& var = m_TreeBillboards[i].GetVariant(j);
				LOGDEBUG(g_Log, "%d   %f %f %f %f", var.m_TextureIndex, var.m_MinX, var.m_MinY, var.m_MaxX, var.m_MaxY);
			}
		}

		treeBlock.Overflow();
		return true;
	};


	bool SpeedtreeBillboard::C_TreeBillboardSet::Deserialise(Sys::I_File& file)
	{
		Sys::C_FileBlock headerBlock(&file);
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> headerDes(headerBlock);

		if (strcmp(headerBlock.GetBlockHeader().m_Id, m_TreeBillboardSet_HeaderBlock))
			return false;

		U32 currentVersion;
		headerDes >> currentVersion;

		if (currentVersion > m_Version)
			return false;


		Sys::C_FileBlock textureBlock(&headerBlock);
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> textureDes(textureBlock);

		if (strcmp(textureBlock.GetBlockHeader().m_Id, m_TreeBillboardSet_TextureBlock))
			return false;

		U32 numTextureSets;
		textureDes >> numTextureSets;
		m_BillboardTextures.Resize(numTextureSets);

		for (U32 i=0; i<numTextureSets; ++i)
		{
			textureDes >> m_BillboardTextures[i].m_Diffuse;
			textureDes >> m_BillboardTextures[i].m_Normal;
		}

		Sys::C_FileBlock treeBlock(&textureBlock);
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> treeDes(treeBlock);

		if (strcmp(treeBlock.GetBlockHeader().m_Id, m_TreeBillboardSet_TreeBlock))
			return false;

		U32 numTrees;
		treeDes >> numTrees;
		m_TreeBillboards.Resize(numTrees);

		for (U32 i=0; i<numTrees; ++i)
			treeDes >> m_TreeBillboards[i];

		return true;
	};
