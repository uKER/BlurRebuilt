#include "stdafx.h"
#include <cg/cgc.h>

#include "Movie/PS3/Bink/binktextures.h"
#include "Movie/PS3/PS3Bink.h"
#include <Display/Device/GCM/GCMDeviceLabels.h>

#define USE_OWN_BINK_SHADERS
#define USE_OWN_BINK_TEXTURES

static U32 PosIndex = 0;
static U32 TexIndex = 8;

static U32 * bink_texture_fence_addr = NULL;
static U32 bink_texture_fence_level = 0;

typedef struct vtype
{
  F32 x, y;
  F32 u, v;
} vtype;

static vtype vertex[ 4 ] =
{
  { 0.0f, 1.0f, 0.0f, 0.0f },
  { 1.0f, 1.0f, 1.0f, 0.0f },
  { 0.0f, 0.0f, 0.0f, 1.0f },
  { 1.0f, 0.0f, 1.0f, 1.0f }
};

static U32 divisible128( U32 val )
{ 
  return( ( val + 127 ) & ~127 );
}

//############################################################################
//##                                                                        ##
//## Free the shaders that we use.                                          ##
//##                                                                        ##
//############################################################################

void Free_Bink_shaders( Movie::C_PS3Shaders& shaders )
{
	shaders.DestroyShaders();

	if(shaders.vertex_size > 0)
	{
		Movie::C_PS3Shaders::FreeVertexMemory(shaders.vertex_size, shaders.vertex_address);
		shaders.vertex_size = 0;
		shaders.vertex_offset = 0;
		shaders.vertex_address = NULL;
	}
}

//############################################################################
//##                                                                        ##
//## Create the three shaders that we use.                                  ##
//##                                                                        ##
//############################################################################

RADDEFFUNC S32 Create_Bink_shaders( U32 width, U32 height, Movie::C_PS3Shaders& shaders )
{
	shaders.screen_width = (F32) width;  
	shaders.screen_height = (F32) height;
	
	if(!shaders.CreateShaders())
	{
		LOGERROR(Display::g_Log, "Failed to create the bink shaders\n");
		return 0;
	}
	
	shaders.vertex_size = sizeof( vertex );
	if(!Movie::C_PS3Shaders::AllocVertexMemory(shaders.vertex_size, &shaders.vertex_address))
	{
		LOGERROR(Display::g_Log, "Failed to alloc the bink vertex memory %i\n", shaders.vertex_size);
		return 0;
	}
	memcpy( shaders.vertex_address, vertex, sizeof( vertex ) );
	cellGcmAddressToOffset( shaders.vertex_address, &shaders.vertex_offset );

	// create a fence to use 
	if(!bink_texture_fence_addr)
	{
		bink_texture_fence_addr = cellGcmGetLabelAddress( g_BinkTexturesWaitLabel );
		*bink_texture_fence_addr = bink_texture_fence_level;
	}

  return( 1 );
}


//############################################################################
//##                                                                        ##
//## Free the textures that we allocated.                                   ##
//##                                                                        ##
//############################################################################

void Free_Bink_textures( BINKTEXTURESET * set_textures )
{
  int i;
  BINKFRAMETEXTURES * bt;

  if ( set_textures->pbo )
  {
	Movie::C_PS3Shaders::FreeTextureMemory(set_textures->pbs, (U8*)set_textures->pbo);
    set_textures->pbs = 0;
	set_textures->pbo = 0;
  }
}


//############################################################################
//##                                                                        ##
//## Texture helper functions.                                              ##
//##                                                                        ##
//############################################################################

static S32 init_texture( U32 width, U32 height, UINTa * out_texture_addr,
                         U8 * address, U32 pitch )
{
  CellGcmTexture * out_texture;

  // the texture structure comes after the texture data itself
  out_texture = (CellGcmTexture *)( address + divisible128( width * height ) );

  out_texture->format = CELL_GCM_TEXTURE_B8 | CELL_GCM_TEXTURE_LN;
  out_texture->mipmap = 1;
  out_texture->dimension = CELL_GCM_TEXTURE_DIMENSION_2;
  out_texture->cubemap = CELL_GCM_FALSE;
  out_texture->remap = CELL_GCM_REMAP_MODE( CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
                                            CELL_GCM_TEXTURE_REMAP_FROM_A,
                                            CELL_GCM_TEXTURE_REMAP_FROM_R,
                                            CELL_GCM_TEXTURE_REMAP_FROM_G,
                                            CELL_GCM_TEXTURE_REMAP_FROM_B,
                                            CELL_GCM_TEXTURE_REMAP_REMAP,
                                            CELL_GCM_TEXTURE_REMAP_REMAP,
                                            CELL_GCM_TEXTURE_REMAP_REMAP,
                                            CELL_GCM_TEXTURE_REMAP_REMAP );
  out_texture->width = width;
  out_texture->height = height;
  out_texture->depth = 1;
  out_texture->location = CELL_GCM_LOCATION_MAIN;
  out_texture->pitch = width;
  cellGcmAddressToOffset( address, &out_texture->offset );
  if ( out_texture->offset == 0 )
    return( 0 );
  
  *out_texture_addr = (UINTa) out_texture;
  return( 1 );
}


static void select_texture( UINTa itexture, const U32 sampler )
{
  CellGcmTexture * texture;
  
  texture = ( CellGcmTexture * )itexture;

  cell::Gcm::cellGcmSetTexture( sampler, texture );

  cell::Gcm::cellGcmSetTextureControl( sampler, CELL_GCM_TRUE, 0, 0, CELL_GCM_TEXTURE_MAX_ANISO_1 );
  cell::Gcm::cellGcmSetTextureFilter( sampler, 0, CELL_GCM_TEXTURE_LINEAR, CELL_GCM_TEXTURE_LINEAR, CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX );
  cell::Gcm::cellGcmSetTextureAddress( sampler, CELL_GCM_TEXTURE_CLAMP_TO_EDGE, CELL_GCM_TEXTURE_CLAMP_TO_EDGE, CELL_GCM_TEXTURE_CLAMP_TO_EDGE, CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL, CELL_GCM_TEXTURE_ZFUNC_ALWAYS, 0 );
}

static void clear_texture( const U32 sampler )
{
  cell::Gcm::cellGcmSetTextureControl( 
                            sampler, 
                            CELL_GCM_FALSE, 0, 0, CELL_GCM_TEXTURE_MAX_ANISO_1 );
}


static void calc_layout( BINKFRAMEBUFFERS * buffers, U32 * cRoffset, U32 * cBoffset, U32 * Aoffset, 
                                                     U32 * framesize, U32 * YApitch, U32 * cRcBpitch )
{
  U32 ywidth  = ( buffers->Frames[ 0 ].YPlane.Allocate ) ? buffers->YABufferWidth : 0;
  U32 awidth = ( buffers->Frames[ 0 ].APlane.Allocate ) ? buffers->YABufferWidth : 0;
  U32 crcbwidth = ( buffers->Frames[ 0 ].cRPlane.Allocate ) ? buffers->cRcBBufferWidth : 0;
  U32 yaheight = buffers->YABufferHeight;
  U32 crcbheight = buffers->cRcBBufferHeight;

  U32 ysize, asize, crcbsize;
               
  ysize = divisible128( ywidth * yaheight );
  asize = divisible128( awidth * yaheight );
  crcbsize = divisible128( crcbwidth * crcbheight );

  if ( ysize )
    ysize += divisible128( sizeof( CellGcmTexture ) );

  if ( asize )
    asize += divisible128( sizeof( CellGcmTexture ) );

  if ( crcbsize )
    crcbsize += divisible128( sizeof( CellGcmTexture ) );

  *Aoffset = ysize;
  *cRoffset = ysize + asize;
  *cBoffset = ysize + asize + crcbsize;
  *framesize = ysize + asize + 2 * crcbsize;
  *YApitch = ywidth | awidth; // they will be the same, or one will be zero
  *cRcBpitch = crcbwidth;
}


//############################################################################
//##                                                                        ##
//## Create 2 sets of textures for Bink to decompress into...               ##
//##                                                                        ##
//############################################################################

S32 Create_Bink_textures( BINKTEXTURESET * set_textures )
{
  int i;

  U32 cRoffset;
  U32 cBoffset;
  U32 Aoffset;
  U32 framesize;
  U32 YApitch;
  U32 cRcBpitch;

  calc_layout( &set_textures->bink_buffers,
               &cRoffset,
               &cBoffset,
               &Aoffset,
               &framesize,
               &YApitch,
               &cRcBpitch);

  U8 * data = NULL;

  //
  // Allocate all texture memory at once
  //

	U32 size = framesize * set_textures->bink_buffers.TotalFrames;
	U8* pAddress = NULL;
	if(!Movie::C_PS3Shaders::AllocTextureMemory(size, &pAddress))
	{
		LOGERROR(Display::g_Log, "Failed to alloc the bink texture memory %i\n", size);
		set_textures->pbo = NULL;		/* stop horrific crash later on when it tired to free unitinialised memory */
		goto fail;
	}
	set_textures->pbs = size;
	set_textures->pbo = (UINTa)pAddress;

  data = (U8*) set_textures->pbo;
  if ( data == 0 )
    goto fail;

  // don't allow video ram
  if ( ( ( (UINTa) data ) >= 0xc0000000 ) && ( ( (UINTa) data ) < ( 0xc0000000 + 256 * 1024 * 1024 ) ) )
    goto fail;

  // align the start
  data = (U8*) divisible128( (U32) data );

  //
  // Create our textures
  //

  for( i = 0 ; i < set_textures->bink_buffers.TotalFrames ; i++ )
  {
    if ( set_textures->bink_buffers.Frames[ i ].YPlane.Allocate )
    {
      set_textures->bink_buffers.Frames[ i ].YPlane.Buffer = data;
      set_textures->bink_buffers.Frames[ i ].YPlane.BufferPitch = YApitch;
      
      // create Y plane
      if ( ! init_texture( set_textures->bink_buffers.YABufferWidth, 
                           set_textures->bink_buffers.YABufferHeight,
                           &set_textures->textures[ i ].Ytexture, 
                           data,
                           YApitch ) )
        goto fail;
    }
                                              
    if ( set_textures->bink_buffers.Frames[ i ].cRPlane.Allocate )
    {
      set_textures->bink_buffers.Frames[ i ].cRPlane.Buffer = data + cRoffset;
      set_textures->bink_buffers.Frames[ i ].cRPlane.BufferPitch = cRcBpitch;
      
      // create cR plane
      if ( ! init_texture( set_textures->bink_buffers.cRcBBufferWidth, 
                           set_textures->bink_buffers.cRcBBufferHeight,
                           &set_textures->textures[ i ].cRtexture, 
                           data + cRoffset,
                           cRcBpitch ) )
        goto fail;
    }

    if ( set_textures->bink_buffers.Frames[ i ].cBPlane.Allocate )
    {
      set_textures->bink_buffers.Frames[ i ].cBPlane.Buffer = data + cBoffset;
      set_textures->bink_buffers.Frames[ i ].cBPlane.BufferPitch = cRcBpitch;

      // create cB plane
      if ( ! init_texture( set_textures->bink_buffers.cRcBBufferWidth, 
                           set_textures->bink_buffers.cRcBBufferHeight,
                           &set_textures->textures[ i ].cBtexture, 
                           data + cBoffset,
                           cRcBpitch ) )
        goto fail;
    }
    
    if ( set_textures->bink_buffers.Frames[ i ].APlane.Allocate )
    {
      set_textures->bink_buffers.Frames[ i ].APlane.Buffer = data + Aoffset;
      set_textures->bink_buffers.Frames[ i ].APlane.BufferPitch = YApitch;
      
      // create alpha plane
      if ( ! init_texture( set_textures->bink_buffers.YABufferWidth, 
                           set_textures->bink_buffers.YABufferHeight,
                           &set_textures->textures[ i ].Atexture, 
                           data + Aoffset,
                           YApitch ) )
        goto fail;
    }  

    set_textures->textures[ i ].fence = 0;
    data += framesize;
  }

  set_textures->framesize = framesize;

  return( 1 );
  
 fail:

  Free_Bink_textures( set_textures );
  return( 0 );
}


//############################################################################
//##                                                                        ##
//## Draw our textures onto the screen with our vertex and pixel shaders.   ##
//##                                                                        ##
//############################################################################

void Draw_Bink_textures( BINKTEXTURESET * set_textures,
                         U32 width,
                         U32 height,
                         F32 x_offset,
                         F32 y_offset,
                         F32 x_scale,
                         F32 y_scale,
                         F32 alpha_level,
						 Movie::C_PS3Shaders& shaders,
						 F32 uEnd1,
						 F32 vEnd1 )
{
  U32 offset;

  // Set vertex shader
  shaders.SetVertexShaderBink();

  // Set vertex attribute
  cell::Gcm::cellGcmSetVertexDataArray( PosIndex, 0, sizeof( vtype ), 2, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, shaders.vertex_offset );
  cell::Gcm::cellGcmSetVertexDataArray( TexIndex, 0, sizeof( vtype ), 2, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, shaders.vertex_offset + 8 );

  //
  // turn off modes we don't need
  //

  cell::Gcm::cellGcmSetDepthTestEnable( CELL_GCM_FALSE );
  cell::Gcm::cellGcmSetCullFaceEnable( CELL_GCM_FALSE );
  cell::Gcm::cellGcmSetAlphaTestEnable( CELL_GCM_FALSE );
  cell::Gcm::cellGcmSetFrontFace( CELL_GCM_CCW );
  cell::Gcm::cellGcmSetShadeMode( CELL_GCM_SMOOTH );

  // set scale and bias for the rect
  {
    F32 adjust[ 4 ];

    adjust[ 0 ] = ( x_offset / shaders.screen_width ) * 2.0f - 1.0f;
    adjust[ 1 ] = ( y_offset / shaders.screen_height ) * 2.0f - 1.0f;
    adjust[ 2 ] = x_scale * 2.0f * ( (F32) width ) / shaders.screen_width;
    adjust[ 3 ] = y_scale * 2.0f * ( (F32) height ) / shaders.screen_height;

	shaders.SetVertexShaderBink_Adjust(adjust);
  }

  // Adjust texcoords for cr/cb textures being different aspect ratio
  {
	  F32 uvRange[ 4 ];

	  uvRange[ 0 ] = 1.f;
	  uvRange[ 1 ] = 1.f;
	  uvRange[ 2 ] = uEnd1;
	  uvRange[ 3 ] = vEnd1;

	  shaders.SetVertexShaderBink_UVRange(uvRange);
  }

  //
  // are we using an alpha plane? if so, turn on the 4th texture and set our pixel shader
  //

  if ( set_textures->bink_buffers.Frames[ 0 ].APlane.Allocate )
  {
    //
    // turn on our pixel shader
    //

	select_texture( set_textures->textures[ set_textures->bink_buffers.FrameNum ].Ytexture,		0 );
    select_texture( set_textures->textures[ set_textures->bink_buffers.FrameNum ].cRtexture,	1 );
    select_texture( set_textures->textures[ set_textures->bink_buffers.FrameNum ].cBtexture,	2 );
    select_texture( set_textures->textures[ set_textures->bink_buffers.FrameNum ].Atexture,		3 );

	shaders.SetPixelShaderBink_AlphaLevel(alpha_level);
	shaders.SetPixelShaderBink();

    goto do_alpha;
  }
  else
  {
    //
    // turn on our pixel shader
    //

	select_texture( set_textures->textures[ set_textures->bink_buffers.FrameNum ].Ytexture,		0 );
    select_texture( set_textures->textures[ set_textures->bink_buffers.FrameNum ].cRtexture,	1 );
    select_texture( set_textures->textures[ set_textures->bink_buffers.FrameNum ].cBtexture,	2 );

	shaders.SetPixelShaderBink_AlphaLevel(alpha_level);
	shaders.SetPixelShaderBink();
  }

  //
  // are we completely opaque or somewhat transparent?
  //
  
  if ( alpha_level >= 0.999f )
  {
    cell::Gcm::cellGcmSetBlendEnable( CELL_GCM_FALSE );
  }
  else
  {
   do_alpha:
    cell::Gcm::cellGcmSetBlendEnable( CELL_GCM_TRUE );
    cell::Gcm::cellGcmSetBlendFunc( CELL_GCM_SRC_ALPHA, CELL_GCM_ONE_MINUS_SRC_ALPHA, CELL_GCM_ONE, CELL_GCM_ZERO );
  }
  
  // Draw the quad
  cell::Gcm::cellGcmSetDrawArrays( CELL_GCM_PRIMITIVE_TRIANGLE_STRIP, 0, 4 );


  //
  // set a fence so that we know when we are done
  //   we use a global increasing fence value, so we only need one 
  //   fence label for all active Bink frames (consider zero an inactive fence, so skip it)
  //

  ++bink_texture_fence_level; if ( bink_texture_fence_level == 0 ) ++bink_texture_fence_level;
  set_textures->textures[ set_textures->bink_buffers.FrameNum ].fence = bink_texture_fence_level;
  cell::Gcm::cellGcmSetWriteBackEndLabel( g_BinkTexturesWaitLabel, bink_texture_fence_level );


  //
  // clear the vertex data and textures
  //

  cell::Gcm::cellGcmSetInvalidateVertexCache( );
  cell::Gcm::cellGcmSetInvalidateTextureCache( CELL_GCM_INVALIDATE_TEXTURE );

  cell::Gcm::cellGcmSetVertexDataArray( PosIndex, 0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0 );
  cell::Gcm::cellGcmSetVertexDataArray( TexIndex, 0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0 );

  if ( set_textures->bink_buffers.Frames[ 0 ].APlane.Allocate )
  {
	clear_texture( 0 );
    clear_texture( 1 );
    clear_texture( 2 );
    clear_texture( 3 );
  }
  else
  {
	clear_texture( 0 );
    clear_texture( 1 );
    clear_texture( 2 );
  }
}


//
// simple routine to store the cache of a memory range
//

static void store_cache( void * ptr, U32 size )
{
  U32 i;
  
  for ( i = 0 ; i < size ; i += 128 )
  {
    asm  ("dcbst     0,%0" : : "r" (ptr));
    ptr = ( (U8*) ptr ) + 128;
  }
}
    
// sync the textures out to main memory, so that the GPU can see them (this is fast)
RADDEFFUNC void Sync_Bink_textures( BINKTEXTURESET * set_textures )
{
  store_cache( set_textures->bink_buffers.Frames[ set_textures->bink_buffers.FrameNum ].YPlane.Buffer, set_textures->framesize );
}


// make sure the GPU is done with the textures that we are about to write info
RADDEFFUNC void Wait_for_Bink_textures( BINKTEXTURESET * set_textures )
{
  S32 next;
  
  next = set_textures->bink_buffers.FrameNum + 1;
  if ( ( next >= set_textures->bink_buffers.TotalFrames ) || ( set_textures->textures[ next ].Ytexture == 0 ) )
    next = 0;

  if ( set_textures->textures[ next ].fence )
  {
    //
    // block until the texture is done beign used
    //

    while ( ( (S32) ( ( set_textures->textures[ next ].fence - *( (S32 volatile *) bink_texture_fence_addr ) ) ) ) > 0 )
    {
      sys_timer_usleep( 10 );
    }
  }
}
