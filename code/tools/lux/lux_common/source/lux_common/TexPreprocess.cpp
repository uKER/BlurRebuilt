#include "stdafx.h"

#include "lux_common\TexPreprocess.h"
#include "lux_common\LpvsUtility.h"

#include <il/openIL.h>
#include <string>
#include <System\DebugLog.h>

using namespace std;

static Debug::Logger &g_Log = Debug::GetLogger("tools.lux");

const int			TexturePreprocessor::m_iDatabaseVersion		= 2;
const int			TexturePreprocessor::m_iMaxResolution		= 32;
const char			*TexturePreprocessor::m_szDatabaseFilename	= "Gotham3/code/Texlibcache/test.dat";

TexturePreprocessor::TexturePreprocessor()
{
	m_bGotLatestDatabase = false;
	m_bDatabaseIsCheckedOut = false;
	LoadDatabase();
}

TexturePreprocessor::~TexturePreprocessor()
{
	if (m_bDatabaseIsCheckedOut)
	{
		SaveDatabase();
	}
}

TextureData *TexturePreprocessor::GetTextureData(const Aftk::C_Polygon *pPoly)
{
	// get material colour by looking up the diffuse texture in the texture library
	const Aftk::C_MaterialResource *pMatRes = GetMaterialResourcePtr(pPoly);
	if (!pMatRes)
		return NULL;

	const Aftk::C_MaterialNode *pMat = &pMatRes->GetContent();
	if (!pMat || !pMat->GetNumShaderImageNodes())
		return NULL;

	const Aftk::C_ShaderImageNode *pImgNode = pMat->GetShaderImageNodePtr(0);
	if (!pImgNode)
		return NULL;

	std::string tex;
	pImgNode->GetFilePlug().GetValue(tex);
	return LookupTexture(tex);
}

char gpszGothamPathName[512];

// load a texture, extract it's information and add it to the list
bool TexturePreprocessor::AddTexture(const std::string &name)
{
	OpenIL::Image image;

	// read texture
	if (!image.Load( name.c_str() ))
	{
		// try to load it from across the network
		char buffer[512];
		strcpy(buffer, name.c_str());
		StrToLower(buffer);

		// [Blur2PC] the assets share was a hard-coded z:; BIZARRE_ROOT names it now
		const char *pszRoot = getenv("BIZARRE_ROOT");
		char *pszRest = NULL;
		if (pszRoot && *pszRoot)
		{
			char lowerRoot[512];
			strcpy(lowerRoot, pszRoot);
			StrToLower(lowerRoot);
			char *psz = strstr(buffer, lowerRoot);
			if (psz)
				pszRest = psz + strlen(lowerRoot);
		}

		if (!pszRest)
			return false;

		char buffer2[512];
		strcpy(buffer2, gpszGothamPathName);
		strcat(buffer2, pszRest);

		if (!image.Load( buffer2 ))
			return false;
	}

	int iNewWidth = min(image.GetWidth(), m_iMaxResolution);
	int iNewHeight = min(image.GetHeight(), m_iMaxResolution);

	image.Resize( iNewWidth, iNewHeight, 1 );

	int r=1, g=1, b=1;
	int c=1;

	// create data structure
	TextureData texdat;

	// get texture contents
	texdat.m_cX = iNewWidth;
	texdat.m_cY = iNewHeight;
	c = texdat.m_cX * texdat.m_cY;

	texdat.m_pData = new U32[c];
	image.GetBits(OpenIL::Image::R8G8B8A8, texdat.m_pData);

	// calculate average
	U8 *pData = (U8*)texdat.m_pData;
	for (int i=0; i<c; i++, pData+=4)
	{
		r += pData[0];
		g += pData[1];
		b += pData[2];
	}
	
	texdat.m_name = name;
	float fc = (float)(c) * 255.0f;
	texdat.m_vAverageRGB = float3((float)r/fc,(float)g/fc,(float)b/fc);

	// add the texture to our list
	m_NameMap[name] = m_PreprocessData.Size();
	m_PreprocessData.PushBack( texdat );

	delete texdat.m_pData;

	return true;
}

// look up a texture, returns the texture info
TextureData *TexturePreprocessor::LookupTexture(std::string &name)
{
	map <string, int> :: const_iterator iter;

	while (1)
	{
		iter = m_NameMap.find(name);

		// found it
		if (iter != m_NameMap.end())
			break;

		// did we try to load it before?
		iter = m_NotFoundNameMap.find(name);
		if (iter != m_NotFoundNameMap.end())
			return NULL;
/*
		// have we got the latest version of the database off alienbrain
		if (!m_bGotLatestDatabase)
		{
			m_bGotLatestDatabase = true;
			if (m_pAB->GetLatest(m_szDatabaseFilename) && LoadDatabase())
				continue;
		}

		// check it out if it wasn't already
		if (!m_bDatabaseIsCheckedOut)
		{
			m_bDatabaseIsCheckedOut = true;
			m_pAB->CheckOut(m_szDatabaseFilename);
		}

		// get the missing texture off alienbrain
		m_pAB->GetLatest(name.c_str());
*/
		// regardless of whether we got the latest version off alienbrain - try to add it to ourlist
		if (!AddTexture(name))
		{
			LOGWARN( g_Log, "Texture %s not found!\n",name.c_str());
			m_NotFoundNameMap[name] = 0;
			return NULL;
		}
	}

	return &m_PreprocessData[(*iter).second];
}

// load texture database from file
bool TexturePreprocessor::LoadDatabase()
{
/*
	m_NameMap.clear();
	m_PreprocessData.Purge();

	CFile f;
	if( !f.Open( m_szDatabaseFilename, CFile::modeRead ) )
		return false;

	ost::Engine ar( &f, ost::Engine::load );

	// check version
	int iVersion;
	ar >> iVersion;
	if ( iVersion != m_iDatabaseVersion )
	{
		ar.Close();
		return false;
	}

	ar >> m_PreprocessData;

	ar.Close();

	// add the texture names into the map for fast lookups
	for (int i=0; i<m_PreprocessData.Size(); i++)
		m_NameMap[std::string(m_PreprocessData[i].m_name)] = i;
*/
	return true;
}

// save the database to file
bool TexturePreprocessor::SaveDatabase()
{
/*
	CFile f;
	if( !f.Open( m_szDatabaseFilename, CFile::modeCreate | CFile::modeWrite ) )
		return false;

	ost::Engine ar( &f, ost::Engine::store );

	ar << m_iDatabaseVersion;

	ar << m_PreprocessData;

	ar.Close();
*/
	return true;
}
