#include "stdafx.h"

#include "fx/effect.h"
#include <Core/Containers/containerserialisation.h>
#include <system/Serialise.h>
#include <system/Serialise/C_FileBlock.h>
#include <system/serialise/BinarySerialise.h>
#if !CFG_CONVERTER
#include <Display.h>
#endif

namespace FX
{

// .FXB importing - pass in the pointer to the Effect to load into
bool ImportFXB(Sys::I_File *pFile, I_Effect *_pFX)
{
#if !CFG_CONVERTER
	RESOURCE_MEMORY_ID("FX Shader");
#endif

	// read header
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pFile);
	int x;
	ar >> x;
	if (x == 'FXB2')
		pFile->SetEndianSwapFlag(false);
	else if (x == '2BXF')
		pFile->SetEndianSwapFlag(true);
	else
		assert(0); // invalid file format - re-export shaders

	Sys::C_FileBlock root(pFile);
	while (root.GetPosition() < root.Length())
	{
		Sys::C_FileBlock block(&root);
		if (!strcmp(block.GetBlockHeader().m_Id,
#ifdef PLATFORM_PC
			"PCFX"
#elif PLATFORM_PS3
			"PS3FX"
#endif
			))
		{
			Effect *pFX = (Effect*)_pFX;
			if (block.GetEndianSwapFlag())
			{
				Sys::BinaryDeserialiser<Ut::DoEndianSwap> ar(block);
				ar >> *pFX;
				return true;
			}
			else
			{
				Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(block);
				ar >> *pFX;
				return true;
			}
		}
		else
		{
			block.Read(NULL, block.GetBlockHeader().m_Size - (U32)block.GetPosition());
		}
	}
	return false;	// the .fxb file didn't contain the correct target platform shader data
}

/// load a .fxb file, returns a pointer to the loaded Effect
I_Effect *ImportFXB(Sys::I_File *pFile)
{
	Effect *p = new Effect;
	if (ImportFXB(pFile, p))
		return p;
	delete p;
	return NULL;
}

}
