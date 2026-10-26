// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: Provide a Graphics class for the PS3

#ifndef BD_GFX_PS3_H
#define BD_GFX_PS3_H

#include <PSGL/psgl.h>
#include <PSGL/psglu.h>
//#include <PSGL/psglfx.h>

class bdGfxImpl : public bdGfxBase
{
// Public non-platform specific class members
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	bdGfxImpl();

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
	
	/// Cleanup on shutdown.
	virtual void cleanup();

	protected:
		PSGLdevice *m_glDevice;
		PSGLcontext *m_glContext;

		bdUInt m_deviceWidth;
		bdUInt m_deviceHeight;
};

#endif // BD_GFX_PS3_H
