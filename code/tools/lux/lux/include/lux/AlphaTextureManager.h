#pragma once

namespace LUX
{

typedef Ut::Vector<AlphaTextureInfo> TAlphaTextureVector;
class Lux;

class AlphaTextureManager : public SD::I_AlphaTextureManager, public FTM::FTMWrapper
{
public:
							/// constructor
							AlphaTextureManager();

							/// destructor
							~AlphaTextureManager();
							/// get alpha texture index structure given an index
	AlphaTextureInfo		*GetAlphaTextureInfo(int i);
							/// get an index for a texture filename
							/// returns -1 if the texture has no alpha
	int						FindAlphaTextureInfo(const std::string &sTextureFilename);
							/// get alpha texture info list
	Ut::Vector<AlphaTextureInfo> *TakeOwnershipOfList();

protected:
	bool					ReadAlphaTexture(const std::string &s, AlphaTextureInfo *pATI);
	int						GenerateNewAlphaTextureInfo(const std::string &s);
	void					Clear();

	TAlphaTextureVector		*m_pAlphaTextures;
	int						m_iMaxResolution;

	// map texture filenames to alpha texture list indices
	std::map<std::string, int>	m_map;
};

}
