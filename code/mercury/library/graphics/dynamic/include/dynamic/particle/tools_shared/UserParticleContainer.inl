C_UserParticleContainer::C_UserParticleContainer()
{
	m_particlePackageBlock = "PtlData";
	m_shape3DVariantsBlock = "UsrVar3";
}

C_UserParticleContainer::Shape3DVariantsArray& C_UserParticleContainer::GetShape3DVariantsArray()
{
	return m_userShape3DVariants;
}

const C_UserParticleContainer::Shape3DVariantsArray& C_UserParticleContainer::GetShape3DVariantsArray() const
{
	return m_userShape3DVariants;
}

bool C_UserParticleContainer::Serialise(Sys::I_File& file) const
{
	//Block header for file.
	Sys::C_FileBlock packageBlock(&file, m_particlePackageBlock);
	Sys::BinarySerialiserAutoEndian ar(packageBlock);

	ar << m_currentFileVersion;
	{
		//3D variants block.
		Sys::C_FileBlock shapeVariantsBlock(&packageBlock, m_shape3DVariantsBlock);
		Sys::BinarySerialiserAutoEndian ar(shapeVariantsBlock);

		ar << (U32)m_userShape3DVariants.Size();
		for (U32 i = 0; i < (U32)m_userShape3DVariants.Size(); i++)
			ar << ((C_UserParticleContainer*)this)->m_userShape3DVariants[i];
	}

	return true;
}

bool C_UserParticleContainer::Deserialise(Sys::I_File& file)
{
	Reset();

	//Block header for file.
	Sys::C_FileBlock packageBlock(&file);
	{
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(packageBlock);
		U32 currentFileVersion;
		ar >> currentFileVersion;
		file.SetVersionNumber(currentFileVersion);

		if (currentFileVersion > m_currentFileVersion)
			return false;
	}

	if (strcmp(packageBlock.GetBlockHeader().m_Id, m_particlePackageBlock))
		return true;
	{
		Sys::C_FileBlock block(&packageBlock);
		
		//Read 3D variants block.
		if (!strcmp(block.GetBlockHeader().m_Id, m_shape3DVariantsBlock))
		{
			if (block.GetVersionNumber() != m_currentFileVersion)
				return false;
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(block);

			U32 numRecords;
			ar >> numRecords;
			m_userShape3DVariants.Resize(numRecords);
			for (U32 i = 0; i < (U32)m_userShape3DVariants.Size(); i++)
				ar >> m_userShape3DVariants[i];
		}
	}

	return true;
}

void C_UserParticleContainer::Reset()
{
	m_userShape3DVariants.Clear();
}
