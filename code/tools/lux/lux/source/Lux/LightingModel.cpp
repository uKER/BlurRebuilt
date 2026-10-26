#include "stdafx.h"

#include "Lux/Lights/Light.h"
#include "Lux/Lights/DirectionalLight.h"
#include "Lux/LightingModelProperties.h"
#include "Lux/LightingModel.h"
#include "Lux/Lux.h"
#include "Lux/Parameterization/TextureAtlas.h"
#include "Lux/Parameterization/SceneUnwrapper.h"
#include "Lux/Parameterization/LMPolyAttributes.h"
#include "Lux/LightmapPacking/LightmapPacking.h"
#include "Lux/Rasterize.h"

using namespace LUX;

LightingModel::LightingModel()
{
	Reset();
}

LightingModel::~LightingModel()
{
	Clear();
}

void LightingModel::Clear()
{
	Reset();
}

void LightingModel::Reset()
{
	m_pAftkScene = 0;
	m_pHierarchy = 0;
	m_pLux = 0;
}
