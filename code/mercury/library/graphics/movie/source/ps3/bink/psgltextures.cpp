#include <PSGL/psgl.h>
#include <cg/cgGL.h>

#include "binktextures.h"


//
// pointers to our local vertex and pixel shader
//

static CGprogram  YCrCbToRGBNoPixelAlpha = 0;
static CGprogram  YCrCbAToRGBA = 0;
static CGprogram  PositionAndTexCoordPassThrough = 0;

static GLuint vbo = 0;
static CGcontext context = 0;
static F32 screen_width, screen_height;

static CGparameter AdjustParam = 0;

static CGparameter YplaneParam = 0;
static CGparameter cRplaneParam = 0;
static CGparameter cBplaneParam = 0;
static CGparameter AplaneParam = 0;
static CGparameter AlphaParam = 0;
static CGparameter YplaneNPAParam = 0;
static CGparameter cRplaneNPAParam = 0;
static CGparameter cBplaneNPAParam = 0;
static CGparameter AlphaNPAParam = 0;

static CGprofile VertexProfile;
static CGprofile FragmentProfile;

//
// simple pass through vertex shader
//

static const char StrPositionAndTexCoordPassThrough[] = 
" struct Output                                         "
" {                                                     "
"     float4 Pos : POSITION;                            "
"     float2 T0: TEXCOORD0;                             "
" };                                                    "
"                                                       "
" uniform float4 Adjust;                                "
"                                                       "
" Output main( Output In )                              "
" {                                                     "
"     In.Pos.x *= Adjust.z;"
"     In.Pos.y *= Adjust.w;"
"     In.Pos.x += Adjust.x;"
"     In.Pos.y += Adjust.y;"
"     return In;                                        "
" }                                                     ";

//
// simple pixel shader to apply the yuvtorgb matrix
//

static const char StrYCrCbToRGBNoPixelAlpha[] =
" uniform sampler2D YplaneNPA;          "
" uniform sampler2D cRplaneNPA;         "
" uniform sampler2D cBplaneNPA;         "
" uniform float AlphaNPA;               "
"                                       "
" float4 main( float2 texcoord: TEXCOORD0 ) : COLOR      "
" {                                     "
"   float4 c;                           "
"   float4 p;                           "
"   const float4  tor = float4( 1.164123535f, 1.595794678f, 0.0f, -0.87065506f ); "
"   const float4  tog = float4( 1.164123535f, -0.813476563f, -0.391448975f, 0.529705048f ); "
"   const float4  tob = float4( 1.164123535f,  0.0f, 2.017822266f, -1.081668854f ); "
"   c.x = tex2D( YplaneNPA, texcoord ).x;  "
"   c.y = tex2D( cRplaneNPA, texcoord ).x; "
"   c.z = tex2D( cBplaneNPA, texcoord ).x; "
"   c.w = 1.0f;                         "
"   p.x = dot( tor, c );                "
"   p.y = dot( tog, c );                "
"   p.z = dot( tob, c );                "
"   p.w = AlphaNPA;                     "
"   return p;                           "
" }                                     ";

//
// simple pixel shader to apply the yuvtorgb matrix with alpha
//

static const char StrYCrCbAToRGBA[] =
" uniform sampler2D Yplane;             "
" uniform sampler2D cRplane;            "
" uniform sampler2D cBplane;            "
" uniform sampler2D Aplane;             "
" uniform float Alpha;                  "
"                                       "
" float4 main( float2 texcoord: TEXCOORD0 ) : COLOR      "
" {                                     "
"   float4 c;                           "
"   float4 p;                           "
"   const float4  tor = float4( 1.164123535f, 1.595794678f, 0.0f, -0.87065506f ); "
"   const float4  tog = float4( 1.164123535f, -0.813476563f, -0.391448975f, 0.529705048f ); "
"   const float4  tob = float4( 1.164123535f,  0.0f, 2.017822266f, -1.081668854f ); "
"   c.x = tex2D( Yplane, texcoord ).x;  "
"   c.y = tex2D( cRplane, texcoord ).x; "
"   c.z = tex2D( cBplane, texcoord ).x; "
"   c.w = 1.0f;                         "
"   p.w = tex2D( Aplane, texcoord ).x;  "
"   p.x = dot( tor, c );                "
"   p.y = dot( tog, c );                "
"   p.z = dot( tob, c );                "
"   p.w *= Alpha;                       "
"   return p;                           "
" }                                     ";


//############################################################################
//##                                                                        ##
//## Free the shaders that we use.                                          ##
//##                                                                        ##
//############################################################################

void Free_Bink_shaders( void )
{
  if ( YCrCbToRGBNoPixelAlpha )
  {
    cgDestroyProgram( YCrCbToRGBNoPixelAlpha );
    YCrCbToRGBNoPixelAlpha = 0;
  }
  
  if ( YCrCbAToRGBA )
  {
    cgDestroyProgram( YCrCbAToRGBA );
    YCrCbAToRGBA = 0;
  }
  
  if ( PositionAndTexCoordPassThrough )
  {
    cgDestroyProgram( PositionAndTexCoordPassThrough );
    PositionAndTexCoordPassThrough = 0;
  }
  
  if ( context )
  {
    cgDestroyContext( context );
    context = 0;
  }
  
  if ( vbo )
  {
    glDeleteBuffers( 1, &vbo );
    vbo = 0;
  }
}

// quad (minX,minY)->(maxX,maxY)
static GLfloat vertex[12]=
{
  0.0f,0.0f,0.0f,
  1.0f,0.0f,0.0f,
  0.0f,-1.0f,0.0f,
  1.0f,-1.0f,0.0f,
}; 

// UVs (0,0)->(1,1)
static GLfloat tc[8]=
{
  0.f,0.f,
  1.f,0.f,
  0.f,1.f,
  1.f,1.f,
};


//############################################################################
//##                                                                        ##
//## Create the three shaders that we use.                                  ##
//##                                                                        ##
//############################################################################

RADDEFFUNC S32 Create_Bink_shaders( U32 width, U32 height, void * local_mem )
{
  if ( vbo == 0 )
  {
    // store vertex data into a VBO so that we don't use the bus each frame
    glGenBuffers(1,&vbo);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertex)+sizeof(tc),NULL,GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex), vertex);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertex), sizeof(tc), tc);
  }

  screen_width = (F32) width;
  screen_height = (F32) height;


  cgRTCgcInit();

  if ( context == 0 )
    context = cgCreateContext();

  VertexProfile = cgGLGetLatestProfile( CG_GL_VERTEX );
  FragmentProfile = cgGLGetLatestProfile( CG_GL_FRAGMENT );

  cgGLEnableProfile( VertexProfile );

  if ( PositionAndTexCoordPassThrough == 0 )
    PositionAndTexCoordPassThrough = cgCreateProgram( context, CG_SOURCE, StrPositionAndTexCoordPassThrough, VertexProfile,0,0);

  AdjustParam = cgGetNamedParameter( PositionAndTexCoordPassThrough, "Adjust" );

  cgGLDisableProfile( VertexProfile );

  
  cgGLEnableProfile( FragmentProfile );

  if ( YCrCbToRGBNoPixelAlpha == 0 )
    YCrCbToRGBNoPixelAlpha = cgCreateProgram( context, CG_SOURCE, StrYCrCbToRGBNoPixelAlpha, FragmentProfile,0,0);
    
  if ( YCrCbAToRGBA == 0 )
    YCrCbAToRGBA = cgCreateProgram( context, CG_SOURCE, StrYCrCbAToRGBA, FragmentProfile,0,0);


  YplaneNPAParam = cgGetNamedParameter( YCrCbToRGBNoPixelAlpha, "YplaneNPA" );
  cRplaneNPAParam = cgGetNamedParameter( YCrCbToRGBNoPixelAlpha, "cRplaneNPA" );
  cBplaneNPAParam = cgGetNamedParameter( YCrCbToRGBNoPixelAlpha, "cBplaneNPA" );
  AlphaNPAParam = cgGetNamedParameter( YCrCbToRGBNoPixelAlpha, "AlphaNPA" );

  YplaneParam = cgGetNamedParameter( YCrCbAToRGBA, "Yplane" );
  cRplaneParam = cgGetNamedParameter( YCrCbAToRGBA, "cRplane" );
  cBplaneParam = cgGetNamedParameter( YCrCbAToRGBA, "cBplane" );
  AplaneParam = cgGetNamedParameter( YCrCbAToRGBA, "Aplane" );
  AlphaParam = cgGetNamedParameter( YCrCbAToRGBA, "Alpha" );

  cgGLDisableProfile( FragmentProfile );

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
    
  // free the texture memory and then the textures directly
  bt = set_textures->textures;
  for( i = 0 ; i < set_textures->bink_buffers.TotalFrames ; i++ )
  {
    if ( bt->Ytexture )
    {
      glDeleteTextures( 1, &bt->Ytexture );
      bt->Ytexture = 0;
    }
    if ( bt->cRtexture )
    {
      glDeleteTextures( 1, &bt->cRtexture );
      bt->cRtexture = 0;
    }
    if ( bt->cBtexture )
    {
      glDeleteTextures( 1, &bt->cBtexture );
      bt->cBtexture = 0;
    }
    if ( bt->Atexture )
    {
      glDeleteTextures( 1, &bt->Atexture );
      bt->Atexture = 0;
    }
    ++bt;
  }

  if ( set_textures->pbo )
  {
    glBindBuffer( GL_TEXTURE_REFERENCE_BUFFER_SCE, set_textures->pbo );
    glUnmapBuffer( GL_TEXTURE_REFERENCE_BUFFER_SCE );

    glDeleteBuffers( 1, &set_textures->pbo );

    set_textures->pbo = 0;
  }
}


//############################################################################
//##                                                                        ##
//## Create a texture while allocating the memory ourselves.                ##
//##                                                                        ##
//############################################################################

static S32 make_texture( U32 width, U32 height,
                         GLuint * out_texture, U32 pbo_offset, U32 pitch )
{
  GLuint texture;

  glGenTextures( 1, &texture );
  if ( glGetError() != GL_NO_ERROR )
    return( 0 );

  glBindTexture( GL_TEXTURE_2D, texture );
  glTextureReferenceSCE( GL_TEXTURE_2D, 1, width, height, 1, GL_LUMINANCE, pitch, pbo_offset );

  //
  // Turn on texture filtering and texture clamping
  //

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glBindTexture( GL_TEXTURE_2D, 0 );

  if ( glGetError() != GL_NO_ERROR )
  {
    glDeleteTextures( 1, &texture );
    return( 0 );
  }

  *out_texture = texture;
  return( 1 );
}


static U32 divisible128( U32 val )
{
  return( ( val + 127 ) & ~127 );
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
  U32 pbo_offset;

  glGetError();
  
  calc_layout( &set_textures->bink_buffers,
               &cRoffset,
               &cBoffset,
               &Aoffset,
               &framesize,
               &YApitch,
               &cRcBpitch);

  //
  // Allocate all texture memory at once
  //

  set_textures->pbo = 0;
  
  glGenBuffers( 1, &set_textures->pbo );
  if ( glGetError() != GL_NO_ERROR )
    goto fail;
  
  glBindBuffer( GL_TEXTURE_REFERENCE_BUFFER_SCE, set_textures->pbo );
  glBufferData( GL_TEXTURE_REFERENCE_BUFFER_SCE, framesize * set_textures->bink_buffers.TotalFrames, NULL, GL_SYSTEM_DRAW_SCE );
  if ( glGetError() != GL_NO_ERROR )
    goto fail;

  U8 * data = (U8*) glMapBuffer( GL_TEXTURE_REFERENCE_BUFFER_SCE, GL_READ_WRITE );
  if ( data == 0 )
    goto fail;

  // don't allow video ram
  if ( ( ( (UINTa) data ) >= 0xc0000000 ) && ( ( (UINTa) data ) < ( 0xc0000000 + 256 * 1024 * 1024 ) ) )
    goto fail;


  pbo_offset = 0;
  
  //
  // Create our textures
  //

  for( i = 0 ; i < set_textures->bink_buffers.TotalFrames ; i++ )
  {
    set_textures->textures[ i ].Ytexture = 0;
    set_textures->textures[ i ].cBtexture = 0;
    set_textures->textures[ i ].cRtexture = 0;
    set_textures->textures[ i ].Atexture = 0;

    if ( set_textures->bink_buffers.Frames[ i ].YPlane.Allocate )
    {
      set_textures->bink_buffers.Frames[ i ].YPlane.Buffer = data;
      set_textures->bink_buffers.Frames[ i ].YPlane.BufferPitch = YApitch;
      
      // create Y plane
      if ( ! make_texture( set_textures->bink_buffers.YABufferWidth, 
                           set_textures->bink_buffers.YABufferHeight,
                           &set_textures->textures[ i ].Ytexture, 
                           pbo_offset,
                           YApitch ) )
        goto fail;
    }
                                              
    if ( set_textures->bink_buffers.Frames[ i ].cRPlane.Allocate )
    {
      set_textures->bink_buffers.Frames[ i ].cRPlane.Buffer = data + cRoffset;
      set_textures->bink_buffers.Frames[ i ].cRPlane.BufferPitch = cRcBpitch;
      
      // create cR plane
      if ( ! make_texture( set_textures->bink_buffers.cRcBBufferWidth, 
                           set_textures->bink_buffers.cRcBBufferHeight,
                           &set_textures->textures[ i ].cRtexture, 
                           pbo_offset + cRoffset,
                           cRcBpitch ) )
        goto fail;
    }

    if ( set_textures->bink_buffers.Frames[ i ].cBPlane.Allocate )
    {
      set_textures->bink_buffers.Frames[ i ].cBPlane.Buffer = data + cBoffset;
      set_textures->bink_buffers.Frames[ i ].cBPlane.BufferPitch = cRcBpitch;

      // create cB plane
      if ( ! make_texture( set_textures->bink_buffers.cRcBBufferWidth, 
                           set_textures->bink_buffers.cRcBBufferHeight,
                           &set_textures->textures[ i ].cBtexture, 
                           pbo_offset + cBoffset,
                           cRcBpitch ) )
        goto fail;
    }
    
    if ( set_textures->bink_buffers.Frames[ i ].APlane.Allocate )
    {
      set_textures->bink_buffers.Frames[ i ].APlane.Buffer = data + Aoffset;
      set_textures->bink_buffers.Frames[ i ].APlane.BufferPitch = YApitch;
      
      // create alpha plane
      if ( ! make_texture( set_textures->bink_buffers.YABufferWidth, 
                           set_textures->bink_buffers.YABufferHeight,
                           &set_textures->textures[ i ].Atexture, 
                           pbo_offset + Aoffset,
                           YApitch ) )
        goto fail;
    }  

    glGenFencesNV( 1, &set_textures->textures[ i ].fence );
    glSetFenceNV( set_textures->textures[ i ].fence, GL_ALL_COMPLETED_NV );

    data += framesize;
    pbo_offset += framesize;
  }

  set_textures->framesize = framesize;

  glBindBuffer( GL_TEXTURE_REFERENCE_BUFFER_SCE, 0 );
  
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
                         F32 alpha_level )
{
  cgGLEnableProfile( VertexProfile );
  cgGLEnableProfile( FragmentProfile );

  cgGLBindProgram( PositionAndTexCoordPassThrough );

  glBindBuffer( GL_TEXTURE_REFERENCE_BUFFER_SCE, set_textures->pbo );
  glBindBuffer( GL_ARRAY_BUFFER, vbo );

  glEnableClientState( GL_VERTEX_ARRAY );
  glVertexPointer( 3, GL_FLOAT, 0, NULL );

  glEnableClientState( GL_TEXTURE_COORD_ARRAY );
  glTexCoordPointer( 2, GL_FLOAT, 0, (GLubyte *)NULL + sizeof(vertex) );

  //
  // turn off Z buffering, culling, and projection (since we are drawing orthographically)
  //

  glDisable( GL_DEPTH_TEST );
  glDisable( GL_CULL_FACE );

  cgGLSetParameter4f( AdjustParam, ( x_offset / screen_width ) * 2.0f - 1, ( 1 - ( y_offset / screen_height ) ) * 2 - 1, x_scale * 2.0f * ( (F32) width ) / screen_width, y_scale * 2.0f * ( (F32) height ) / screen_height );

  //
  // are we using an alpha plane? if so, turn on the 4th texture and set our pixel shader
  //
  
  if ( set_textures->textures[ set_textures->bink_buffers.FrameNum ].Atexture )
  {
    //
    // turn on our pixel shader
    //
  
    cgGLBindProgram( YCrCbAToRGBA );

    cgGLSetTextureParameter( YplaneParam, set_textures->textures[ set_textures->bink_buffers.FrameNum ].Ytexture ); 
    cgGLEnableTextureParameter( YplaneParam );

    cgGLSetTextureParameter( cRplaneParam, set_textures->textures[ set_textures->bink_buffers.FrameNum ].cRtexture ); 
    cgGLEnableTextureParameter( cRplaneParam );

    cgGLSetTextureParameter( cBplaneParam, set_textures->textures[ set_textures->bink_buffers.FrameNum ].cBtexture ); 
    cgGLEnableTextureParameter( cBplaneParam );

    cgGLSetTextureParameter( AplaneParam, set_textures->textures[ set_textures->bink_buffers.FrameNum ].Atexture ); 
    cgGLEnableTextureParameter( AplaneParam );

    cgGLSetParameter1f( AlphaParam, alpha_level );

    goto do_alpha;
  }
  else
  {
    //
    // turn on our pixel shader
    //
  
    cgGLBindProgram( YCrCbToRGBNoPixelAlpha );

    cgGLSetTextureParameter( YplaneNPAParam, set_textures->textures[ set_textures->bink_buffers.FrameNum ].Ytexture ); 
    cgGLEnableTextureParameter( YplaneNPAParam );

    cgGLSetTextureParameter( cRplaneNPAParam, set_textures->textures[ set_textures->bink_buffers.FrameNum ].cRtexture ); 
    cgGLEnableTextureParameter( cRplaneNPAParam );

    cgGLSetTextureParameter( cBplaneNPAParam, set_textures->textures[ set_textures->bink_buffers.FrameNum ].cBtexture ); 
    cgGLEnableTextureParameter( cBplaneNPAParam );

    cgGLSetParameter1f( AlphaNPAParam, alpha_level );
  }

  //
  // are we completely opaque or somewhat transparent?
  //
  
  if ( alpha_level >= 0.999f )
  {
    glDisable( GL_BLEND );
  }
  else
  {
   do_alpha:
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }
  
  //
  // Draw the vertices.
  //

  glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

  //
  // clear the textures
  //
  
  if ( set_textures->textures[ set_textures->bink_buffers.FrameNum ].Atexture )
  {
    cgGLDisableTextureParameter( YplaneParam );
    cgGLDisableTextureParameter( cRplaneParam );
    cgGLDisableTextureParameter( cBplaneParam );
    cgGLDisableTextureParameter( AplaneParam );
  }
  else
  {
    cgGLDisableTextureParameter( YplaneNPAParam );
    cgGLDisableTextureParameter( cRplaneNPAParam );
    cgGLDisableTextureParameter( cBplaneNPAParam );
  }


  //
  // set a fence so that we know when we are done
  //
  
  glSetFenceNV( set_textures->textures[ set_textures->bink_buffers.FrameNum ].fence, GL_ALL_COMPLETED_NV );


  //
  // clear the profiles
  //

  cgGLUnbindProgram( VertexProfile );
  cgGLUnbindProgram( FragmentProfile );
  
  cgGLDisableProfile( VertexProfile );
  cgGLDisableProfile( FragmentProfile );
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
    // block until the texture is ready
    //

    glFinishFenceNV( set_textures->textures[ next ].fence );
  }
}

