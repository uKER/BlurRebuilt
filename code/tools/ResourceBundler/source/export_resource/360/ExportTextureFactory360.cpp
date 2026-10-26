#include "stdafx.h"

#include "../IExportTextureFactory.h"
#include "ExportTextureFactory360.h"
#include "../ExportTextureOptions.h"
#include "../IExportTexture.h"
#include "ExportTexture360.h"
#include "ExportArrayOfTextures2D360.h"
#include "ExportTexture2D360.h"
#include "ExportTextureArray2D360.h"
#include "ExportTexture3D360.h"
#include "ExportTextureCubeMap360.h"

using namespace ResourceBundler;

C_ExportTextureFactory360 C_ExportTextureFactory360::m_instance;

I_ExportTexture* C_ExportTextureFactory360::CreateTexture2D() const
{
	return new C_ExportTexture2D360;
}

I_ExportTexture* C_ExportTextureFactory360::CreateTextureArray2D() const
{
	return new C_ExportTextureArray2D360;
}

I_ExportTexture* C_ExportTextureFactory360::CreateTexture3D() const
{
	return new C_ExportTexture3D360;
}

I_ExportTexture* C_ExportTextureFactory360::CreateTextureCubeMap() const
{
	return new C_ExportTextureCubeMap360;
}

const C_ExportTextureFactory360& C_ExportTextureFactory360::Instance()
{
	return m_instance;
}

C_ExportTextureFactory360::C_ExportTextureFactory360()
{
}
