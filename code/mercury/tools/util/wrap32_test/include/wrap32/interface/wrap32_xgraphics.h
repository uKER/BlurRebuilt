XBOXAPI
VOID 
WINAPI 
XGTileTextureLevel64(
    UINT             Width,        // Width of the texture (base level) in texels (not blocks)
    UINT             Height,       // Height of the texture (base level) in texels (not blocks)
    UINT             Level,        // Mip level of the destination
    DWORD            GpuFormat,    // Gpu format of the texture
    DWORD            Flags,        // Tiling flags (border, non-packed, etc.)
    VOID*            pDestination, // Address of the first texel in the destination (tiled) level
    CONST POINT*     pPoint,       // Offset in the destination level (in texels) to place the tiled image
    CONST VOID*      pSource,      // Address of the source (linear) image
    UINT             RowPitch,     // Distance in bytes between one row of the source image and the next
    CONST RECT*      pRect,         // Rect within the source image to copy (in texels)

	CONST VOID*		pSourceBaseAddress,
	UINT			srcSize,
	VOID*			pDestinationBaseAddress,
	UINT			dstSize
    );

XBOXAPI
VOID 
WINAPI  
XGTileSurface64(
    VOID*        pDestination, // Base address of the destination (tiled) image
    UINT         Width,        // Width of the destination image
    UINT         Height,       // Height of the destination image
    CONST POINT* pPoint,       // Offset in the destination image to place the tiled rectangle
    CONST VOID*  pSource,      // Base address of the source (linear) image
    UINT         RowPitch,     // Distance in bytes between one row of the source image and the next
    CONST RECT*  pRect,        // Rectangle within the source image to copy
    UINT         TexelPitch,   // Size of an image texel/block in bytes

	CONST VOID*		pSourceBaseAddress,
	UINT			srcSize,
	VOID*			pDestinationBaseAddress,
	UINT			dstSize
    );

