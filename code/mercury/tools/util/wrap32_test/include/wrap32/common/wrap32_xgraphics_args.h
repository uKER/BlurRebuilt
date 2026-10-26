#pragma once

struct Wrap32_XGTileTextureLevel64Arguments
{
    UINT			Width;			// Width of the texture (base level) in texels (not blocks)
    UINT			Height;			// Height of the texture (base level) in texels (not blocks)
    UINT			Level;			// Mip level of the destination
    DWORD			GpuFormat;		// Gpu format of the texture
    DWORD			Flags;			// Tiling flags (border; non-packed; etc.)
    WRAP32_SHARED_MEMORY_OFFSET
					pDestination;	// Address of the first texel in the destination (tiled) level
    WRAP32_SHARED_MEMORY_OFFSET 
					pPoint;			// Offset in the destination level (in texels) to place the tiled image
    WRAP32_SHARED_MEMORY_OFFSET 
					pSource;		// Address of the source (linear) image
    UINT			RowPitch;		// Distance in bytes between one row of the source image and the next
    WRAP32_SHARED_MEMORY_OFFSET     
					pRect;			// Rect within the source image to copy (in texels)

	UINT			srcSize;
	WRAP32_SHARED_MEMORY_OFFSET
					pDestinationBaseAddress;
	UINT			dstSize;
};

struct Wrap32_XGTileSurface64Arguments
{
    WRAP32_SHARED_MEMORY_OFFSET	
					pDestination; // Base address of the destination (tiled) image
    UINT			Width;        // Width of the destination image
    UINT			Height;       // Height of the destination image
    WRAP32_SHARED_MEMORY_OFFSET
					pPoint;       // Offset in the destination image to place the tiled rectangle
    WRAP32_SHARED_MEMORY_OFFSET
					pSource;      // Base address of the source (linear) image
    UINT			RowPitch;     // Distance in bytes between one row of the source image and the next
    WRAP32_SHARED_MEMORY_OFFSET
					pRect;        // Rectangle within the source image to copy
    UINT			TexelPitch;   // Size of an image texel/block in bytes

	UINT			srcSize;
	WRAP32_SHARED_MEMORY_OFFSET
					pDestinationBaseAddress;
	UINT			dstSize;
};

struct Wrap32_XGGetMipTailLevelOffsetArguments
{
    UINT  Width;                // Width of the texture
    UINT  Height;               // Height of the texture
    UINT  Depth;                // Depth of the texture
    UINT  Level;                // Mip level of the texture
    DWORD GpuFormat;            // Gpu format of the texture
    BOOL  Tiled;                // TRUE if the texture is tiled; FALSE if linear
    BOOL  Border;               // TRUE if the texture has a border; FALSE if it does not
};

struct Wrap32_XGGetMipTailBaseLevelArguments
{
    UINT Width;                 // Width of the texture
    UINT Height;                // Height of the texture
    BOOL Border;                // TRUE if the texture has a border; FALSE if it does not
};

struct Wrap32_XGGetTextureDescArguments
{
    WRAP32_SHARED_MEMORY_OFFSET 
		pTexture;   // Texture to describe
    UINT            
		Level;      // Mip level to describe
    WRAP32_SHARED_MEMORY_OFFSET 
		pDesc;      // Structure to be filled with texture description
};

struct Wrap32_XGSetTextureHeaderArguments
{
    UINT                    Width;
    UINT                    Height;
    UINT                    Levels;
    DWORD                   Usage;
    D3DFORMAT               Format;
    D3DPOOL                 Pool;
    UINT                    BaseOffset;
    UINT                    MipOffset;
    UINT                    Pitch;
    WRAP32_SHARED_MEMORY_OFFSET 
							pTexture;
    WRAP32_SHARED_MEMORY_OFFSET     
							pBaseSize;
    WRAP32_SHARED_MEMORY_OFFSET     
							pMipSize;
};
