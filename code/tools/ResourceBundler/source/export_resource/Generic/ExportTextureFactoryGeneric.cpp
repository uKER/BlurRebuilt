#include "stdafx.h"

#include "../IExportTextureFactory.h"
#include "ExportTextureFactoryGeneric.h"
#include "../ExportTextureOptions.h"
#include "../IExportTexture.h"

#include "ExportTextureGeneric.h"
#include "ExportArrayOfTextures2DGeneric.h"
#include "ExportTextureArray2DGeneric.h"
#include "ExportTexture2DGeneric.h"
#include "ExportTexture3DGeneric.h"
#include "ExportTextureCubeMapGeneric.h"

using namespace ResourceBundler;

C_ExportTextureFactoryGeneric C_ExportTextureFactoryGeneric::m_instance;

I_ExportTexture* C_ExportTextureFactoryGeneric::CreateTexture2D() const
{
	return new C_ExportTexture2DGeneric;
}

I_ExportTexture* C_ExportTextureFactoryGeneric::CreateTextureArray2D() const
{
	return new C_ExportTextureArray2DGeneric;
}

I_ExportTexture* C_ExportTextureFactoryGeneric::CreateTexture3D() const
{
	return new C_ExportTexture3DGeneric;
}

I_ExportTexture* C_ExportTextureFactoryGeneric::CreateTextureCubeMap() const
{
	return new C_ExportTextureCubeMapGeneric;
}

const C_ExportTextureFactoryGeneric& C_ExportTextureFactoryGeneric::Instance()
{
	return m_instance;
}

C_ExportTextureFactoryGeneric::C_ExportTextureFactoryGeneric()
{
}
