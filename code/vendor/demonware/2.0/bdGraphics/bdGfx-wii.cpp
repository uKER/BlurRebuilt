// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>


#ifdef BD_PLATFORM_WII

#include <bdGraphics/bdGfx.h>
#include <bdGraphics/bdFont-wii.h>

enum { PERSPECTIVE, ORTHOGRAPHIC};

BD_REGISTER_SINGLETON(bdGfxImpl)

bdGfxImpl::bdGfxImpl()
{
	m_firstFrame = GX_TRUE;
	m_textPositionX = 0.5;
	m_textPositionY = 0.5;
}

bdGfxImpl::~bdGfxImpl()
{
   
}
void bdGfxImpl::setRenderMode( GXRenderModeObj* mode )
{
    // If an application specific render mode is provided,
    // override the default render mode
    if (mode != NULL) 
    {
        m_Rmodeobj = *mode;
        m_Rmode    = &m_Rmodeobj;
    }
    else
    {
        switch (VIGetTvFormat())
        {
          case VI_NTSC:
            m_Rmode = &GXNtsc480IntDf;
            break;
          case VI_PAL:
            m_Rmode = &GXPal528IntDf;
            break;
          case VI_EURGB60:
            m_Rmode = &GXEurgb60Hz480IntDf;
            break;
          case VI_MPAL:
            m_Rmode = &GXMpal480IntDf;
            break;
          default:
            OSHalt("DEMOInit: invalid TV format\n");
            break;
        }

        // Trim off from top & bottom 16 scan lines (which will be overscanned).
        // So almost all demos actually render only 448 lines (in NTSC case.)
        // Since this setting is just for SDK demos, you can specify this
        // in order to match your application requirements.
        GXAdjustForOverscan(m_Rmode, &m_Rmodeobj, 0, 16);
        
        m_Rmode = &m_Rmodeobj;
    }
}

void bdGfxImpl::beforeRender( void ) const
{
    // Set up viewport (This is inappropriate for full-frame AA.)
    if (m_Rmode->field_rendering)
    {
        GXSetViewportJitter(
          0.0F, 0.0F, (float)m_Rmode->fbWidth, (float)m_Rmode->efbHeight, 
          0.0F, 1.0F, VIGetNextField());
    }
    else
    {
        GXSetViewport(
          0.0F, 0.0F, (float)m_Rmode->fbWidth, (float)m_Rmode->efbHeight, 
          0.0F, 1.0F);
    }

    // Invalidate vertex cache in GP
    GXInvalidateVtxCache();
    // Invalidate texture cache in GP
    GXInvalidateTexAll();
}

void bdGfxImpl::doneRender( void ) const
{
    // Set Z/Color update to make sure eFB will be cleared at GXCopyDisp.
    // (If you want to control these modes by yourself in your application,
    //  please comment out this part.)
//    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
 //   GXSetColorUpdate(GX_TRUE);
    
    // Issue display copy command
    GXCopyDisp(m_currentBuffer, GX_TRUE);

    // Wait until everything is drawn and copied into XFB.
    GXDrawDone();

    // Set the next frame buffer
    swapBuffers();
}

void bdGfxImpl::swapBuffers( void ) const
{
    // Display the buffer which was just filled by GXCopyDisplay
    VISetNextFrameBuffer(m_currentBuffer);

    // If this is the first frame, turn off VIBlack
    if(m_firstFrame)
    {
        VISetBlack(FALSE);
        m_firstFrame = GX_FALSE;
    }
    
    // Tell VI device driver to write the current VI settings so far
    VIFlush();
    
    // Wait for vertical retrace.
    VIWaitForRetrace();

    // Swap buffers
    if(m_currentBuffer == m_frameBuffer1)
        m_currentBuffer = m_frameBuffer2;
    else
        m_currentBuffer = m_frameBuffer1;
}

 void bdGfxImpl::initGX( void )
{
    u16     xfbHeight;
    f32     yScale;

    /*----------------------------------------------------------------*
     *  GX configuration by a render mode obj                         *
     *----------------------------------------------------------------*/
    // These are necessary function calls to take a render mode
    // object and set up relevant GX configuration.

    GXSetViewport(0.0F, 0.0F, (f32)m_Rmode->fbWidth, (f32)m_Rmode->efbHeight, 0.0F, 1.0F);
    GXSetScissor(0, 0, (u32)m_Rmode->fbWidth, (u32)m_Rmode->efbHeight);

    yScale = GXGetYScaleFactor(m_Rmode->efbHeight, m_Rmode->xfbHeight);
    xfbHeight = (u16)GXSetDispCopyYScale(yScale);
    GXSetDispCopySrc(0, 0, m_Rmode->fbWidth, m_Rmode->efbHeight);
    GXSetDispCopyDst(m_Rmode->fbWidth, xfbHeight);
    GXSetCopyFilter(m_Rmode->aa, m_Rmode->sample_pattern, GX_TRUE, m_Rmode->vfilter);
    GXSetDispCopyGamma(GX_GM_1_0);

    if (m_Rmode->aa)
        GXSetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    else
        GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	// Below is the comment from the original Nintendo Demo, this might be relevant for widescreen? - eman
    // Note that following is an appropriate setting for full-frame AA mode.
    // You should use "xfbHeight" instead of "efbHeight" to specify actual
    // view size. Since this library doesn't support such special case, please
    // call these in each application to override the normal setting.
#if 0
    GXSetViewport(0.0F, 0.0F, (f32)m_Rmode->fbWidth, (f32)m_Rmode->xfbHeight, 
                  0.0F, 1.0F);
    GXSetDispCopyYScale(1.0F);
#endif

    // Clear embedded framebuffer for the first frame
    GXCopyDisp(m_frameBuffer2, GX_TRUE);
}

void bdGfxImpl::startVI( void )
{
    // Configure VI with given render mode
    VIConfigure(m_Rmode);

    // Double-buffering initialization
    VISetNextFrameBuffer(m_frameBuffer1);
    m_currentBuffer = m_frameBuffer2;

    // Tell VI device driver to write the current VI settings so far
    VIFlush();
    
    // Wait for retrace to start first frame
    VIWaitForRetrace();

    // Because of hardware restriction, we need to wait one more
    // field to make sure mode is safely changed when we change
    // INT->DS or DS->INT. (VIInit() sets INT mode as a default)
    if ( (u32)m_Rmode->viTVmode & 1 )
        VIWaitForRetrace();
}

void bdGfxImpl::configureMem()
{
    u32 fbSize;

    fbSize = VIPadFrameBufferWidth(m_Rmode->fbWidth) * m_Rmode->xfbHeight * (u32)VI_DISPLAY_PIX_SZ;
    m_allocatedFrameBufferSize = fbSize;
    m_FifoBuffer   = bdMemory::alignedAllocate(DEFAULT_FIFO_SIZE, 32); 
    m_frameBuffer1 = bdMemory::alignedAllocate(fbSize, 32); 
    m_frameBuffer2 = bdMemory::alignedAllocate(fbSize, 32); 
}

bdBool bdGfxImpl::init(bdInt32 *, bdNChar8 *[])
{
	GXRenderModeObj* mode = BD_NULL;
    m_allocatedFrameBufferSize = 0;
    
    // Initializes disc drive interface.
    DVDInit();      // Initializes disc.
    
    // Initializes video interface.
    VIInit();

    // Set up rendering mode
    // (which reflects the GX/VI configurations and XFB size below)
    setRenderMode(mode);

	configureMem();

    // Initializes graphics
    m_FifoObj = GXInit(m_FifoBuffer, DEFAULT_FIFO_SIZE);
    initGX();
	
    bdFont::init();

    // Starts VI
    startVI();

	setBackgroundColor(m_backgroundColor);
	return true;
}

bdBool bdGfxImpl::runFrame() const
{
    beforeRender();
	render();
	doneRender();
	return true;
}

void bdGfxImpl::cleanup()
{
   
}

void bdGfxImpl::setBackgroundColor(const bdColor &color)
{
	bdGfxBase::setBackgroundColor(color);
	GXColor col = {	color.getRed(), 
				   	color.getGreen(), 
					color.getBlue(), 
				   	color.getAlpha()};

    GXSetCopyClear(col, 0x00FFFFFF);

}
    
 void bdGfxImpl::CameraInit (int mode) const
{
    Mtx44   p;      // projection matrix
    Vec     up      = {0, 1, 0};
    Vec     camLoc  = {0.0F, 0.0F, 15.0F};
    Vec     objPt   = {0,0,0};
    f32     left    = 24.0F;
    f32     top     = 32.0F;
    f32     znear   = 1.0F;
    f32     zfar    = 1000.0F;
	f32		fov 	= 45.0F;
	f32		aspect	= (float)m_Rmode->fbWidth/ (float)m_Rmode->efbHeight;
	
	if(mode == PERSPECTIVE)
	{
		MTXPerspective(p,fov,aspect,znear,zfar);
		GXSetProjection(p, GX_PERSPECTIVE);
		MTXLookAt(m_v, &camLoc, &up, &objPt);   
	}
	else if(mode == ORTHOGRAPHIC)
	{
		MTXOrtho( p, (float)m_Rmode->efbHeight * -0.5f,(float)m_Rmode->efbHeight * 0.5f, (float)m_Rmode->fbWidth * -0.5f,(float)m_Rmode->fbWidth * 0.5f, -10.0f,10.0f ) ;
		GXSetProjection( p, GX_ORTHOGRAPHIC ) ;
	}   
}


void bdGfxImpl::render() const
{
    Mtx  mv; // Modelview matrix.
    Mtx  m;  // Model matrix.

	// Meshes in Perspective mode
	MTXIdentity( m_v ) ;
	CameraInit(PERSPECTIVE);
    for (bdUInt i = 0; i < m_meshes.getSize(); i++)
	{

		MTXIdentity(m);
		MTXConcat(m_v, m, mv); 
		GXLoadPosMtxImm(mv, GX_PNMTX0);
		
		// Draw the mesh
	    m_meshes[i]->draw();
	}

	// If there is text to display, do so in Orthographic mode
	if(m_text[0] != '\0')
	{
		MTXIdentity( m_v ) ;
		CameraInit(ORTHOGRAPHIC);

	    MTXIdentity(m);
	    MTXConcat(m_v, m, mv); 
	    GXLoadPosMtxImm( m_v, GX_PNMTX0 ) ;
	    GXSetCurrentMtx( GX_PNMTX0 ) ;
	
	    bdFont::init_draw_font(m_Rmode->fbWidth,m_Rmode->efbHeight,10,14);  
		GXColor col = {	m_textColor.getRed(), 
						m_textColor.getGreen(), 
						m_textColor.getBlue(), 
						m_textColor.getAlpha()}; 
	    bdFont::drawString(&m_text[0], m_textPositionX, m_textPositionY, col);
	}
}

#endif // BD_PLATFORM_WII
