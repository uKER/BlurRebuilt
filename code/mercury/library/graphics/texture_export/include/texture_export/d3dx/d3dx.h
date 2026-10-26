#ifndef D3DX_H
#define D3DX_H

#ifndef SWIG
 #ifdef D3DX_EXPORT
  #define D3DX_API __declspec(dllexport)
 #else
  #define D3DX_API __declspec(dllimport)
 #endif // D3DX_EXPORT
#else
 #define D3DX_API
#endif

namespace ActTools
{
	class Scene;
	class Mesh;
	class Texture;
	class TexelFormat;
}


struct IDirect3DBaseTexture9;
struct _D3DXFRAME;
struct _D3DXMESHCONTAINER;

namespace d3dx
{


//------ Tangent generation --------------------------------------------------
// Tangent generation
D3DX_API ACTRESULT GenerateTangentsBinormals(
	ActTools::Mesh* aMesh,
	int aGenerateTangent      , int aGenerateBinormal      , int aScaleToRangeMinus1ToPlus1   = 0,
	int aGenerateTangent_2 = 0, int aGenerateBinormal_2 = 0, int aScaleToRangeMinus1ToPlus1_2 = 0,
	int aGenerateTangent_3 = 0, int aGenerateBinormal_3 = 0, int aScaleToRangeMinus1ToPlus1_3 = 0,
	int aGenerateTangent_4 = 0, int aGenerateBinormal_4 = 0, int aScaleToRangeMinus1ToPlus1_4 = 0
);

//------ Mesh conversion, loading/saving -------------------------------------
// Loads _D3DXMESHCONTAINER* from .x file
D3DX_API _D3DXMESHCONTAINER* LoadMeshContainerFromX( const char* filename );

// Saves _D3DXMESHCONTAINER* to .x file
D3DX_API ACTRESULT SaveMeshContainerToX( const char* filename, _D3DXMESHCONTAINER* mesh );

// Copies the vertex and triangle data of the D3DXMESH to Mesh
//D3DX_API ACTRESULT CopyD3DXMeshToMesh( ID3DXMesh* d3dxmesh, ActTools::Mesh* mesh );
D3DX_API ACTRESULT CopyD3DXMeshToMesh( ID3DXMesh* d3dxmesh, ActTools::Mesh* &old_mesh );

// Copies only the vertex and triangle data from  Mesh into D3DXMesh
// Assume that all components have the same number of elements and 
// the faces indices are grouped.
D3DX_API ID3DXMesh* CreateD3DXMeshFromMesh( ActTools::Mesh* mesh );

// Converts a _D3DXMESHCONTAINER* into Mesh
D3DX_API ActTools::Scene* CreateSceneFromMeshContainer( _D3DXMESHCONTAINER* mesh );

// Converts a Mesh into _D3DXMESHCONTAINER*
D3DX_API _D3DXMESHCONTAINER* CreateMeshContainerFromScene( ActTools::Scene* scene );


//------ Multi-mesh conversion, loading/saving -------------------------------
// Loads frame hierarchy to from .x file
D3DX_API _D3DXFRAME* LoadFrameHierarchyFromX( const char* filename );

// Saves Loads frame hierarchy from .x file
D3DX_API ACTRESULT SaveFrameHierarchyToX( const char* filename, _D3DXFRAME* frame );

// Converts a Scene into frame hierarchy
D3DX_API _D3DXFRAME* CreateFrameHierarchyFromScene( ActTools::Scene* scene );

// Converts a frame hierarchy into a Scene
D3DX_API ActTools::Scene* CreateSceneFromFrameHierarchy( _D3DXFRAME* frame );


//------ Texture conversion, loading/saving ----------------------------------
// Loads IDirect3DTexture9 from any file format supported by D3DX.
D3DX_API IDirect3DBaseTexture9* LoadD3DTexture( const char* filename );

// Saves IDirect3DTexture9 to any file format supported by D3DX.
D3DX_API ACTRESULT SaveD3DTexture( const char* filename, IDirect3DBaseTexture9* texture );

// Converts IDirect3DTexture9 to Texture
D3DX_API ActTools::Texture* CreateTextureFromD3DTexture( IDirect3DBaseTexture9* texture, const char* name, const char* filename );

// Converts Texture to IDirect3DTexture9
D3DX_API IDirect3DBaseTexture9* CreateD3DTextureFromTexture( ActTools::Texture* texture );

// Creates procedural texture from HLSL shader.
D3DX_API IDirect3DBaseTexture9* CreateD3DTextureFromShader( const char* shadername,
                                                            const char* function,
                                                            const char* target,
                                                            const char* type,
                                                            int width,
                                                            int height,
                                                            int depth,
                                                            int levels,
                                                            D3DFORMAT format );

D3DX_API ActTools::Texture* CreateTextureFromShader( const char* name,
                                                     const char* shadername,
                                                     const char* function,
                                                     const char* target,
                                                     const char* type,
                                                     int width,
                                                     int height,
                                                     int depth,
                                                     int levels,
                                                     D3DFORMAT format );

// Destroys IDirect3DTexture9 by calling Release()
void DestroyD3DTexture( IDirect3DBaseTexture9* texture );

// Calculates the amount of memory used by a surface.
D3DX_API size_t GetSurfaceBytes( D3DFORMAT format, size_t w, size_t h );

// Convert from D3DFORMAT <-> string.
D3DX_API D3DFORMAT GetD3DFormat( const char* id );
D3DX_API const char* GetD3DFormatName( D3DFORMAT format );

// Get TexelFormat class object from D3DFORMAT.
D3DX_API ActTools::TexelFormat* GetTexelFormat( D3DFORMAT format );
D3DX_API ActTools::TexelFormat* GetTexelFormat( const char* format );

} // namespace d3dx

#endif // D3DX_H

