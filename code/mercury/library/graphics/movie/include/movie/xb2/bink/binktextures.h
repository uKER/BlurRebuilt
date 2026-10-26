#ifndef RENDER_BINK_TEXTURES_H
#define RENDER_BINK_TEXTURES_H

#ifdef _MSC_VER
#pragma once
#endif

/*

This is RAD's high level API for using 3D hardware to do color conversion.
It is supported on PS3, Xbox, Xbox 360, Wii, Windows and GameCube.

It's a nice and simple API, but you should see your platform's example code
to see all the fine details.


There are three main cross platform functions:

  Create_Bink_textures:  This function takes a BINKTEXTURESET structure and
    creates the texture resources to render it quickly.
  
  Free_Bink_textures:  Frees the resources allocated in Create_Bink_textures.

  Draw_Bink_textures:  Renders the textures onto the screen.


There are also a few platform specific functions:

  Wait_for_Bink_textures:  On Wii, Xbox, Xbox 360 and PS3, this function 
    waits for the GPU to finish using the given texture set. Call
    before BinkOpen.

  Sync_Bink_textures:  On Wii, Xbox 360, and PS3, this function stores the 
    CPU L2 cache entries for the texture memory, so that the GPU can see it.
    Call this function after BinkDoFrame.
    

  Create_Bink_shaders:  On PS3, Xbox 360 and Windows, this function creates
    the pixel shaders we use to do the color conversion. Call this function
    before the first call to Create_Bink_textures.

  Free_Bink_shaders:  Frees the pixel shaders created in Create_Bink_shaders.


  Lock_Bink_textures:  On Windows, locks the textures so that BinkDoFrame can
    decompress into them.

  Unlock_Bink_textures:  On Windows, unlocks the textures after BinkDoFrame.


So, basically, playback works like this:

  1) Create the pixel shaders on the platforms that need it (PS3, Xbox 360, Win).
  
  2) Open the Bink file with the BINKNOFRAMEBUFFERS flag.  We will use our API
     to create the frame buffers that Bink will use.

  3) Call BinkGetFrameBuffersInfo to get the details on the frame buffers 
     that we need.

  4) Call Create_Bink_textures to create the textures.

  5) Call BinkRegisterFrameBuffers to register these new textures with Bink.

  6) Call Wait_for_Bink_textures before BinkDoFrame (or Lock_Bink_textures 
     on Windows).

  7) Call BinkDoFrame to decompress a video frame.

  8) Call Sync_Bink_textures to flush the cache for the GPU (or 
     Unlock_Bink_textures on Windows).

  9) Draw the frame using Draw_Bink_textures.


And that's it! (Skipping over a few details - see the examples for all 
the details...)

Should drop in really quickly and it hides a ton of platform specific ugliness!

*/

#include "bink.h"

#if defined(__RADPS3__) || defined(__RADWII__) || defined(__RADNGC__)
  #define if_used_3d_device // no global 3D device handle on ps3, wii or ngc
#else
  #define if_used_3d_device LPDIRECT3DDEVICE9 d3d_device,
#endif


typedef struct BINKFRAMETEXTURES
{
#ifdef __RADPS3__

  UINTa Ytexture;
  UINTa cRtexture;
  UINTa cBtexture;
  UINTa Atexture;
  UINTa fence;

#elif defined( __RADWII__ ) || defined( __RADNGC__ )

  GXTexObj Ytexture;
  GXTexObj cRtexture;
  GXTexObj cBtexture;
  GXTexObj Atexture;

#else

  U32 Ysize;
  U32 cRsize;
  U32 cBsize;
  U32 Asize;


    // xenon and windows use dx9
    LPDIRECT3DTEXTURE9 Ytexture;
    LPDIRECT3DTEXTURE9 cRtexture;
    LPDIRECT3DTEXTURE9 cBtexture;
    LPDIRECT3DTEXTURE9 Atexture;


#endif
} BINKFRAMETEXTURES;


typedef struct BINKTEXTURESET
{
  // this is the GPU info for the textures
  BINKFRAMETEXTURES textures[ BINKMAXFRAMEBUFFERS ];

  // this is the Bink info on the textures
  BINKFRAMEBUFFERS bink_buffers;

  // this is specialized global data for each platform

  #if defined( __RADNT__ )
  
    // on windows, we need a second set of textures to draw with
    BINKFRAMETEXTURES tex_draw;

  #elif defined( __RADPS3__ )

    UINTa pbo;
    U32 framesize;

  #elif defined( __RADWII__ ) || defined( __RADNGC__ )
  
    void * base_ptr;
    U32 framesize;
    
    U32 YAdeswizzle_width; 
    U32 YAdeswizzle_height; 
    U32 cRcBdeswizzle_width; 
    U32 cRcBdeswizzle_height; 

    GXTexObj YAdeswizzle;
    GXTexObj cRcBdeswizzle;
    
    S32 drawing[ 2 ];

  #endif
} BINKTEXTURESET;

//=============================================================================

//
// allocate the textures that we'll need
//
RADDEFFUNC S32 Create_Bink_textures( if_used_3d_device
                                     BINKTEXTURESET * set_textures );

// frees the textures
RADDEFFUNC void Free_Bink_textures( if_used_3d_device
                                    BINKTEXTURESET * set_textures );

// draws the textures with D3D
RADDEFFUNC void Draw_Bink_textures( if_used_3d_device
                                    BINKTEXTURESET * set_textures,
                                    U32 width,
                                    U32 height,
                                    F32 x_offset,
                                    F32 y_offset,
                                    F32 x_scale,
                                    F32 y_scale,
                                    F32 alpha_level,
									F32 uEnd1 = 1.f,
									F32 vEnd1 = 1.f );

//=============================================================================


#if defined(__RADNT__)

  // On Windows, we need to use lock and unlock semantics for best performance

  // Lock the textures for use by D3D
  RADDEFFUNC void Lock_Bink_textures( BINKTEXTURESET * set_textures );

  // Unlock the textures after rendering
  RADDEFFUNC void Unlock_Bink_textures( LPDIRECT3DDEVICE9 d3d_device, BINKTEXTURESET * set_textures, HBINK Bink );

#else

  // on Xbox, Xenon, Wii, NGC and PS3, we use flush and wait semantics - 
  //   we flush the CPU memory out to the GPU and then
  //   we wait before using the frame again
  
  // make sure the GPU isn't using the next set of textures
  RADDEFFUNC void Wait_for_Bink_textures( BINKTEXTURESET * set_textures );


    //
    // on Wii, NGC, Xenon and PS3, we can flush specific textures
    //
  
    // sync the textures out to main memory (so that the GPU can see them)
    RADDEFFUNC void Sync_Bink_textures( BINKTEXTURESET * set_textures );


#endif

//=============================================================================

#if defined(__RADNT__) || defined(__RADPS3__)

  //
  // we only have to compile the shaders on Xenon, PS3 and PC, they 
  //   are statically defined on Xbox1, Wii, and NGC
  //

  // creates the couple of shaders that we use
#ifdef __RADPS3__

  // for GCM, local_mem should be a pointer that is BINK_TEXTURES_LOCAL_MEM bytes long
  // for PSGL, local mem should be zero
  RADDEFFUNC S32 Create_Bink_shaders( U32 width, U32 height, void * local_mem );

  #define BINK_TEXTURES_LOCAL_MEM 768

#else

  RADDEFFUNC S32 Create_Bink_shaders( LPDIRECT3DDEVICE9 d3d_device );

#endif

  // free our shaders
  RADDEFFUNC void Free_Bink_shaders( void );

#endif

//=============================================================================

#endif // RENDER_BINK_TEXTURES_H
