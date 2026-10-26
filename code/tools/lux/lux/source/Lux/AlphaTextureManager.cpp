#include "stdafx.h"

#include "Lux/Lights/Light.h"
#include "Lux/Lights/DirectionalLight.h"
#include "Lux/RenderChannel.h"
#include "Lux/LightingModelProperties.h"
#include "Lux/GlobalIllumination/GlobalIlluminationProperties.h"
#include "Lux/Lux.h"
#include "Lux/LightingModel.h"
#include "Lux/GlobalIllumination/GlobalIlluminationModel.h"
#include "il/OpenIL.h"
#include <tools_common/StringProc.h>
#include "ftm_xml_api\Headers.h"
#include "Lux/AlphaTextureManager.h"
#include <spatial_hierarchy/AlphaTextureInfo.h>
#include <spatial_hierarchy/AlphaTextureManager.h>
#include "Lux/AlphaTextureManager.h"

#define TEX_DATA_XML_FILE_NAME "TextureData.xml"

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;

// constructor
AlphaTextureManager::AlphaTextureManager()
{
	m_iMaxResolution = 128;
	m_pAlphaTextures = new Ut::Vector<AlphaTextureInfo>;
}

/// destructor
AlphaTextureManager::~AlphaTextureManager()
{
	if (m_pAlphaTextures)
		delete m_pAlphaTextures;
}

/// get alpha texture index structure given an index
AlphaTextureInfo *AlphaTextureManager::GetAlphaTextureInfo(int i)
{
	if (i == -1)
		return NULL;

	return &m_pAlphaTextures->At(i);
}

/// generate a new alpha texture info structure for this texture
/// must have already checked for duplicates when calling this
int AlphaTextureManager::GenerateNewAlphaTextureInfo(const std::string &s)
{
	// do texture tag lookup
	const FTM::FTMTexInfo &texinfo = LookupTexture(s);
	bool b;
	// no attribute found
	if (!texinfo.GetAttribute("Use alpha shadow", b))
		return -1;
	// attribute was set to false
	if (!b)
		return -1;

	// generate alpha texture info for this texture
	int i = m_pAlphaTextures->Size();
	m_pAlphaTextures->PushBack();

	// read the texture
	if (!ReadAlphaTexture(s, &m_pAlphaTextures->Back()))
	{
		m_pAlphaTextures->PopBack();
		return -1;
	}

	return i;
}

/// get an index for a texture filename
/// returns -1 if the texture has no alpha
int	AlphaTextureManager::FindAlphaTextureInfo(const std::string &sTextureFilename)
{
	// TODO: check in a hash table whether we already decided that this texture has no alpha

	std::string s = sTextureFilename;
	Tu::ToLower(s);

	// check in a map whether this texture already has an alpha allocated
	std::map<std::string, int>::iterator iter;

	// find this database
	iter = m_map.find( s );
	if (iter != m_map.end())
		return (*iter).second;

	int i = GenerateNewAlphaTextureInfo(s);

	return m_map[s] = i;
}

/// get alpha texture info list
Ut::Vector<AlphaTextureInfo> *AlphaTextureManager::TakeOwnershipOfList()
{
	Ut::Vector<AlphaTextureInfo> *p = m_pAlphaTextures;
	m_pAlphaTextures = NULL;
	Clear();

	return p;
}

/// free the contents of the class
void AlphaTextureManager::Clear()
{
	m_map.clear();
	FTMWrapper::m_map.clear();
	m_entries.Clear();
}

// read the texture in from disk and copy the alpha channel into our data buffer
bool AlphaTextureManager::ReadAlphaTexture(const std::string &s, AlphaTextureInfo *pATI)
{
	OpenIL::Image image;

	// read texture
	if (!image.Load( s.c_str() ))
		return false;

	image.Resize(32,32,1);

	// get surface size and malloc memory for the texture itself
	pATI->m_cX = image.GetWidth();
	pATI->m_cY = image.GetHeight();

	// get bits
	int c = pATI->m_cX * pATI->m_cY;
	pATI->m_pData = new U8[c];
	U32 *pTmp = new U32[c];
	image.GetBits(OpenIL::Image::R8G8B8A8, pTmp);

	// copy only the alpha channel
	U8 *p = (U8*)pTmp;
	p+=3;
	for (int i=0; i<c; i++)
		pATI->m_pData[i] = p[i*4];

	// done
	delete pTmp;
	return true;
}

