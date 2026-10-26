#ifndef __TEXTURE_EXPORT_UTILITY_H__
#define __TEXTURE_EXPORT_UTILITY_H__

#include <system/debuglog.h>

// forward declarations
namespace ActTools
{
class Texture;
};
namespace Attk
{
class C_Image;
};

namespace TextureExport
{

extern Debug::Logger &g_Log;

/// load a texture using the DevIL library
ActTools::Texture *LoadTexture(const char *pFilename);

// Convert a 2d Attk::C_Image to an ActTools::Texture
// the returned texture is in A8R8G8B8 format (which is actually R8G8B8A8 in ATTK speak)
ActTools::Texture *TextureFromAttkImage(Attk::C_Image *pImage);

// Convert a 2d Attk::C_Image to an ActTools::Texture
// the returned texture is in A32R32G32B32 floating point format
ActTools::Texture *TextureFromAttkImageF(Attk::C_Image *pImage);

// Convert a ActTools::Texture to an Attk::C_Image
Attk::C_Image *AttkImageFromTexture(ActTools::Texture *pInTexture);

};

#endif
