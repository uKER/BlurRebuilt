/*! \file Texture.h
	\brief ActToolsLib Texture module.
*/

/**
	\defgroup ACTTOOLSLIB_TEXTURE Texture Class
	@{
*/
#ifndef TEXTURE_H__INCLUDED
#define TEXTURE_H__INCLUDED

#include "texture_export/Palette.h"
#include "texture_export/Surface.h"
#include "ngl/HashStr.h"
#include "btl/containers/vector.h"

namespace ActTools 
{

struct TextureFrame
{
	ActTools::HashStr theName;
	int theTime;
};

/**
 * A Texture class
 */
class Texture
{
public:
	enum Type
	{
		INVALID,
		TEXTURE,
		VOLUMETEXTURE,
		CUBETEXTURE,
		//ANIMATED,
	};

	enum
	{
		ALPHA_UNKNOW     = 0,
		ALPHA_NONE       = 1,
		ALPHA_WHITE      = 1,
		ALPHA_BLACKWHITE = 2,
		ALPHA_GREYSCALE  = 3,

		ALPHA_PRESENSE_MASK = 0x00000003,
		ALPHA_NUMBITS_MASK = 0x0000003C
	};

	void Create( Type aType, const ActTools::TexelFormat* aFormat, size_t aWidth, size_t aHeight, size_t aDepth, unsigned aNumMipmaps );

	/** Constructor. Initializes the object as an empty texture
	 */
	Texture()
	{
		Init();
	}
	/** Destructor.
	 */
	~Texture()
	{
		Destroy();
	}

	Texture(const Texture& texture)
	{
		Init();
		CopyFrom(texture);
	}

	Texture& operator = (const Texture& texture)
	{
		return CopyFrom(texture);
	}

	/** Creates a 2D texture object with a specified format, width height and total number of mips
	 * \param aFormat             A TexelFormat for all the surfaces
	 * \param aWidth              The width of the main mip surfaces
	 * \param aHeight             The height of the main mip surfaces
	 * \param aNumMipmaps         The total number of mip surfaces including the main mip. Must be equal or greater than 1
	 */
	void CreateTexture( const ActTools::TexelFormat* aFormat, size_t aWidth, size_t aHeight, unsigned aNumMipmaps );
	/** Creates a volumetric texture object with a given format, width, height, depth and total number of mips
	 * \param aFormat             A TexelFormat for all the surfaces
	 * \param aWidth              The width of the main mip surfaces
	 * \param aHeight             The height of the main mip surfaces
	 * \param aDepth              The number of surfaces for each mip level
	 * \param aNumMipmaps         The total number of mip-maps including the main mip. Must be equal or greater than 1
	 */
	void CreateVolumeTexture( const ActTools::TexelFormat* aFormat, size_t aWidth, size_t aHeight, unsigned aDepth, unsigned aNumMipmaps );
	/** Creates a cube texture object with a given format, width and total number of mips
	 * \param aFormat             A TexelFormat for all the surfaces
	 * \param aWidth              The width of the main mip surfaces. All surfaces in the cube texture are square
	 * \param aNumMipmaps         The total number of mip-maps including the main mip. Must be equal or greater than 1
	 */
	void CreateCubeTexture( const ActTools::TexelFormat* aFormat, size_t aWidth, unsigned aNumMipmaps );
	/** Creates a an animated texture object with a given format and number of frames
	 * \param aFormat             A TexelFormat for all the surfaces
	 * \param aNumFrames          The number of frames in the animation
	 */
	//void CreateAnimated( const ActTools::TexelFormat* aFormat, unsigned aNumFrames );
	/** Transfer all the data from a specified object
	 * \param aTexture     Texture will take ownership of all the aTexture data and aTexture will become an empty texture when function returns.
	 */
	//void Acquire( Texture* aTexture );
	/** Copy all the data from a specified texure.
	 * \param aTexture            The source texture to copy data from
	 */
	Texture& CopyFrom( const Texture& aTexture );

	void DetachSurface( unsigned aSubTexture, unsigned aMipLevel, ActTools::Surface* aDetachedSurface = 0 );
	void AttachSurface( unsigned aSubTexture, unsigned aMipLevel, ActTools::Surface& aAttachSurface, ActTools::Surface* aDetachedSurface = 0 );
	/** Increas the number of mip levels in a texture 
	 * \param aNumMipmapsToAdd    The number of mip levels to add to the already exsiting ones
	 */
	void AddMipmaps( size_t aNumMipmapsToAdd );

	// After changing mip levels or surfaces, update the size of the texure.
	void SetNewSize(U32 width, U32 height) 
	{
		theWidth = width;
		theHeight = height;
	}

	// total size of the texture data
	S32 GetSize() const;

	/** Check if the texture have alpha channel. Alpha channel could be punch true(ALPHA_BLACKWHITE flag) or greysccale(ALPHA_GREYSCALE flag)
	 */
	int HasAlpha() const
	{
		return 0 != ( theAlphaFlags & ( ALPHA_BLACKWHITE|ALPHA_GREYSCALE ) );
	}
	/** Returns the TexelFormat used for all the surfaces
	 */
	const ActTools::TexelFormat* GetFormat() const
	{
		return theFormat;
	}

	const ActTools::Palette* GetPalette() const {return thePalette;}

	U32 GetWidth() const {return theWidth;}
	U32	GetHeight() const {return theHeight;}
	U32	GetDepth() const {return theDepth;}
	U32	GetNumMipMaps() const {return theNumMipmaps;}
	Type GetType() const {return theType;}

	const ActTools::Surface& GetSurface(U32 i) const {return theSurfaces[i];}

	/** Returns a specific mip level
	 * \param aSubTexture         A sub-texture whose mip level to return. Must be 0 for 2D texrure, [0..5] for CUBE texture and [0..Dept) for VOLUME texture
	 */
	ActTools::Surface* GetMipmap( unsigned aSubTexture, unsigned aMipLevel )
	{
		assert( aMipLevel < theNumMipmaps );
		assert( aSubTexture < theDepth );
		return &theSurfaces[aMipLevel * theDepth + aSubTexture];
	}

	const ActTools::Surface* GetMipmap( unsigned aSubTexture, unsigned aMipLevel ) const
	{
		assert( aMipLevel < theNumMipmaps );
		assert( aSubTexture < theDepth );
		return &theSurfaces[aMipLevel * theDepth + aSubTexture];
	}
	/** Sets the name
	*/
	void SetName( const char* aName )
	{
		theName.Set( aName );
	}

	void AttachPalette( ActTools::Palette* aPalette );

	/** Converts texture to a specified TexelFormat
	 *  
	 * \param aFromat             The new texel format for all surfaces in the texture.
	 */
	void ConvertTo( const ActTools::TexelFormat* aFromat );
	/** Retrieves the alpha presence flag: ALPHA_UNKNOW, ALPHA_NONE, ALPHA_BLACKWHITE or ALPHA_GREYSCALE
	*/
	int GetAlphaPresence() const
	{
		return theAlphaFlags & ALPHA_PRESENSE_MASK;
	}
	/** Set ALPHA presence flat for a texture
	 * \param aPresenceFlag       Must be ALPHA_UNKNOW, ALPHA_NONE, ALPHA_BLACKWHITE or ALPHA_GREYSCALE
	 */
	void SetAlphaPresence( int aPresenceFlag )
	{
		theAlphaFlags &= ~ALPHA_PRESENSE_MASK;
		theAlphaFlags |= aPresenceFlag & ALPHA_PRESENSE_MASK;
	}
	/** Set SRGB flag for a texture (default is false)
	 * \param bSRGB       false for SRGB off and true for SRGB
	 */
	void SetSRGBFlag( int bSRGB )
	{
		bSRGBTexture = bSRGB;
	}
	/** Retrieves the SRGB texture flag
	*/
	int GetSRGBFlag() const
	{
		return bSRGBTexture;
	}
	/** Retrieves the number of bits used to code the alpha channel for a texture that has a greysscale alpha
	*/
	int GetAlphaNumBits() const
	{
		return (theAlphaFlags & ALPHA_PRESENSE_MASK) >> 2;
	}
	/** Sets the number of bits used to code the alpha channel for a texture that has a greysscale alpha
	*/
	void SetAlphaNumBits( int aNumAlphaBits )
	{
		theAlphaFlags &= ~ALPHA_NUMBITS_MASK;
		theAlphaFlags |= aNumAlphaBits & ALPHA_NUMBITS_MASK;
	}
	/** Write the texture to an archive
	*/
	template<typename Archive>
	void Serialise(Archive &ar)
	{
		ar	<< theName << theFilePath << (int)theType << theWidth << theHeight << theDepth
			<< theAlphaFlags << theNumMipmaps << theNumFrames << Util::C_String(theFormat->GetId()) << bSRGBTexture;
		int nSurfaces = theNumMipmaps * theDepth;
		for (int i=0; i<nSurfaces; i++)
			ar << theSurfaces[i];
	}
	/** Read the texture from an archive
	*/
	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		if (theFormat)
			delete theFormat;
		if (thePalette)
			delete thePalette;
		Init();
		Util::C_String formatId;
		ar	>> theName >> theFilePath >> (int&)theType >> theWidth >> theHeight >> theDepth
			>> theAlphaFlags >> theNumMipmaps >> theNumFrames >> formatId >> bSRGBTexture;
		int nSurfaces = theNumMipmaps * theDepth;
		theSurfaces.Resize(nSurfaces);
		for (int i=0; i<nSurfaces; i++)
		{
			ar >> theSurfaces[i];
			theSurfaces[i].theTexture = this;
		}
		theFormat = new UnknownTexelFormat(formatId);
	}

	friend class Surface;

private:
	void Init();
	void Destroy();

	HashStr     theName;
	HashStr     theFilePath;

	Type        theType;
	U32			theWidth;
	U32			theHeight;
	U32			theDepth; // 0:animated, 1:textures, 6:cubetexture, volume:variable
	u32         theAlphaFlags;

	U32			theNumMipmaps;
	U32			theNumFrames;
	U8			bSRGBTexture;

	const ActTools::TexelFormat* theFormat;
	ActTools::Palette* thePalette;
	Btl::Vector<ActTools::Surface> theSurfaces;
	//ActTools::TextureFrame* theFrames;
};
// ACTTOOLSLIB_TEXTURE
//@}

}

#endif // TEXTURE_H__INCLUDED

