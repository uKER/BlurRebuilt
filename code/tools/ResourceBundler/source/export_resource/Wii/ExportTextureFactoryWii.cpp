#include "stdafx.h"

#include "../IExportTextureFactory.h"
#include "ExportTextureFactoryWii.h"
#include "../ExportTextureOptions.h"
#include "../IExportTexture.h"
#include "ExportTextureWii.h"
#include "ExportArrayOfTextures2DWii.h"
#include "ExportTexture2DWii.h"
#include "ExportTextureArray2DWii.h"
#include "../Generic/ExportArrayOfTextures2DGeneric.h"
#include "../Generic/ExportTexture3DGeneric.h"
#include "../Generic/ExportTextureCubeMapGeneric.h"

using namespace ResourceBundler;

C_ExportTextureFactoryWii C_ExportTextureFactoryWii::m_instance;

I_ExportTexture* C_ExportTextureFactoryWii::CreateTexture2D() const
{
	return new C_ExportTexture2DWii;
}

I_ExportTexture* C_ExportTextureFactoryWii::CreateTextureArray2D() const
{
	return new C_ExportTextureArray2DWii;
}

I_ExportTexture* C_ExportTextureFactoryWii::CreateTexture3D() const
{
	return new C_ExportTexture3DGeneric;
}

I_ExportTexture* C_ExportTextureFactoryWii::CreateTextureCubeMap() const
{
	return new C_ExportTextureCubeMapGeneric;
}

const C_ExportTextureFactoryWii& C_ExportTextureFactoryWii::Instance()
{
	return m_instance;
}

C_ExportTextureFactoryWii::C_ExportTextureFactoryWii()
{
}
