#ifndef __SPATIALDIVIDER_ALPHATEXTURES_H__
#define __SPATIALDIVIDER_ALPHATEXTURES_H__

namespace SD
{

// class that generates alpha texture info structures for given texture filenames
class I_AlphaTextureManager
{
public:
									/// virtual destructor
	virtual							~I_AlphaTextureManager() {};
									/// get alpha texture index structure given an index
	virtual AlphaTextureInfo		*GetAlphaTextureInfo(int i) = 0;
									/// get an index for a texture filename
									/// returns -1 if the texture has no alpha
	virtual int						FindAlphaTextureInfo(const std::string &sTextureFilename) = 0;
									/// retrieve the list and free the contents of the class
	virtual Ut::Vector<AlphaTextureInfo> *TakeOwnershipOfList() = 0;
};

}

#endif
