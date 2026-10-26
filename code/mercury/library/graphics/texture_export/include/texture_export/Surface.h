/*! \file Surface.h
	\brief ActToolsLib Surface module.
*/

/**
	\defgroup ACTTOOLSLIB_SURFACE Surface Class
	@{
*/
#ifndef SURFACE_H__INCLUDED
#define SURFACE_H__INCLUDED

#include "ngl/system.h"
#include "ngl/defs.h"
#include "texture_export/TexelFormat.h"
#include "btl/containers/vector.h"

namespace ActTools 
{

class Texture;
class Palette;

/**
 * A Surface class
 */
class Surface
{
public:

public:
	/** Constructor. Initializes the object as an empty surface
	 */
	Surface()
	{
		Init();
	}
	/** Destructor.
	 */
	~Surface()
	{
		Destroy();
	}

	Surface(const Surface& surface);
	Surface& operator = (const Surface& surface);

	/** Creates a surface with a specified width and height.
	 * \param aWidth              The width for the surface
	 * \param aHeight             The height for the surface
	 */
	void Create( size_t aWidth, size_t aHeight );
	/** Helper to reallocate memory for the surface texels.
	 * \param theSize             The numbe of bytes allocated for the surface
	 */
	void Resize( size_t aSize );
	/** Frees allocated memory
	 */
	void Destroy();
	/** Transfer all the data from a specified object.
	 * \param aSurface            Surface will take ownership of all the aSurface data and aSurface will become an empty surafce when function returns.
	 */
	void Acquire( ActTools::Surface* aSurface );

	/** Retrieves Texture that holds this surface
	 */

	const ActTools::TexelFormat* GetFormat() const;
	const ActTools::Palette* GetPalette() const {return 0;}
	U32 GetAlphaPresence() const;

	/** Use this member function to gain direct access to the texel of a surface
	 */
	const u8* GetTexels() const {return theTexels;}
	u8* GetTexels() {return theTexels;}

	U32 GetWidth() const {return theWidth;}
	U32 GetHeight() const {return theHeight;}
	U32 GetSize() const {return theSize;}

	/** Write the texture to an archive
	*/
	template<typename Archive>
	void Serialise(Archive &ar)
	{
		ar << theWidth << theHeight << theSize;
		ar.Write(theTexels, theSize);

	}
	/** Read the texture from an archive
	*/
	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		ar >> theWidth >> theHeight >> theSize;
		if (theTexels)
			delete theTexels;
		theTexels = new u8[ theSize ];
		ar.Read(theTexels, theSize);
	}

	void SetTexture(const ActTools::Texture* aTexture)
	{
		assert(theSize == 0 || aTexture == theTexture);
		theTexture = aTexture;
		delete theFormat;
	}

	friend class Texture;

private:
	/** Initializes object as an empy one with no pixel data
	 */
	void Init();

	/** Copy all the data from a specified surface.
	 * \param aSurface            The source surface to copy data from
	 */
	Surface& CopyFrom( const Surface& aSurface );

	const ActTools::Texture* theTexture;
	const ActTools::TexelFormat* theFormat;

	U32 theWidth;   /// Width of the surface
	U32 theHeight;  /// Height of the surface

	U32 theSize;    /// Number of bytes alocated for rusface texels
	u8* theTexels;     /// Pointer to the memory for texels
};

// For Vector
inline void Move(void* pDst, Surface* pSrc) 
{
	Btl::MoveByMemcopy<Surface>::Move(pDst, pSrc);
}

inline void MoveArray(void* pDst, Surface* pSrc, int n) 
{
	Btl::MoveByMemcopy<Surface>::MoveArray(pDst, pSrc, n);
}

}

#endif // SURFACE_H__INCLUDED

// ACTTOOLSLIB_SURFACE
//@}
