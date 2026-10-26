// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_GFX_PS2_H
#define BD_GFX_PS2_H

#include <revolution.h>
#include <revolution/mem.h>
#include <revolution/tpl.h>
#include <revolution/gx.h>



#define DEFAULT_FIFO_SIZE (256 * 1024)

class bdGfxImpl : public bdGfxBase
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	// Default Constructor
	bdGfxImpl();

	// Default Destructor
	virtual ~bdGfxImpl();

	/// Initializes the graphics class.
	/// \return
	/// - True, if initialization was successful
	/// - False, if initialization failed
	virtual bdBool init(bdInt32 *argc, bdNChar8 *argv[]);

	/// Run one frame of the application.
	/// \return
	/// - True, if the application should continue execution
	/// - False, if a shutdown request was received
	virtual bdBool runFrame() const;

	/// Cleanup for shutting down
	virtual void cleanup();

	/// Set the background color of the buffer.
	/// \param color[in] The new background color for the application.
	virtual void setBackgroundColor(const bdColor &color);

	void startVI( void );
	void initGX( void );
	void beforeRender( void )const;
	void doneRender( void ) const;
	void swapBuffers( void ) const;
	void setRenderMode( GXRenderModeObj* mode );
	void configureMem();
	void CameraInit (int mode) const ;

protected:
	/// Draw the scene.
	/// Clears the back buffer, sets up the view and projection transformation matrices and iterates through
	/// \a m_meshes, calling each mesh's draw function. Finally presents the back buffer to the front buffer.
	void render() const;
  
	// Needs to be mutable since render() is const
	mutable GXBool  m_firstFrame;
	GXRenderModeObj *m_Rmode;
	GXRenderModeObj m_Rmodeobj;
	void*        m_FifoBuffer;
	GXFifoObj*   m_FifoObj;
	u32 m_allocatedFrameBufferSize;
	MEMAllocator	m_Allocator1;
	MEMAllocator	m_Allocator2;

	// These need to be mutable for double buffering
	mutable void*   m_frameBuffer1;
	mutable void*   m_frameBuffer2;
	mutable void*   m_currentBuffer;
	mutable Mtx     m_v;   // view matrix
};

#endif // BD_GFX_PS2_H
