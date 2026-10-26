#include "stdafx.h"

#include <system/serialise.h>

// .FXB exporting code

#define NGL_GPU_D3D_PC 1
#include "ngl/mercury_to_ngl.h"
#include "ngl/defs.h"
#include "ngl/system.h"

#include "fx/targets/pcfx.h"
#include "fx/targets/xefx.h"
#include "fx/targets/ps3fx.h"
#include "fx/fxb.h"

#include <system/serialise/C_FileBlock.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

bool ExportFxbSection(Sys::C_FileBlock *pRoot, const char *pszInFile, const char *pszBlockName, NglBaseFx *pFX, const char **ppszDefines, const char **ppszIncludes, const char *pszOutputFilename)
{
	Sys::C_FileBlock block(pRoot, pszBlockName);
	return pFX->SerialOutFx(&block, pszInFile, ppszDefines, ppszIncludes, pszOutputFilename) != ACT_ERROR;
}

bool ExportFXB(Sys::I_File *pFile, const char *pszInFile, int iFlags, const char **ppszDefines, const char **ppszIncludes, const char *pszOutputFilename)
{
	// write header
	Sys::BinarySerialiserAutoEndian ar(*pFile);
	ar << 'FXB2';

	Sys::C_FileBlock root(pFile, "FXB");

	// write platform specific data
	if (iFlags & FXB_EXPORT_PC)
	{
		NglPcFx fx; if (!ExportFxbSection(&root, pszInFile, "PCFX", &fx, ppszDefines, ppszIncludes, pszOutputFilename)) return false;
	}
	if (iFlags & FXB_EXPORT_XENON)
	{
		NglXeFx fx; if (!ExportFxbSection(&root, pszInFile, "XEFX", &fx, ppszDefines, ppszIncludes, pszOutputFilename)) return false;
	}
	if (iFlags & FXB_EXPORT_PS3)
	{
		NglPs3Fx fx; if (!ExportFxbSection(&root, pszInFile, "PS3FX", &fx, ppszDefines, ppszIncludes, pszOutputFilename)) return false;
	}
	return true;
}

