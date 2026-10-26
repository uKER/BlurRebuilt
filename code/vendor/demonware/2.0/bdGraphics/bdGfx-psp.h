// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC PSP-INFRA
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Provides graphics for the PSP.

#ifndef BD_GFX_PSP_H
#define BD_GFX_PSP_H

#include <psptypes.h>

class bdGfxImpl : public bdGfxBase
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	/// Default Constructor.
	bdGfxImpl();

	/// Default Destructor.
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

	/// Displays a text string on the screen.
	/// \param str[in] The string to display on the screen.
	virtual void setText(const bdNChar8 *const str);

	/// Set the background color of the buffer.
	/// \param color[in] The new background color for the application.
	virtual void setBackgroundColor(const bdColor &color);

protected:
	/// Draw the scene.
	/// Clears the back buffer, sets up the view and projection transformation matrices and iterates through
	/// \a m_meshes, calling each mesh's draw function. Finally presents the back buffer to the front buffer.
	void render() const;

	static const bdUInt projMatrixStackSize = 2;
	static const bdUInt viewMatrixStackSize = 2;
	static const bdUInt worldMatrixStackSize = 8;
	static const bdUInt texMatrixStackSize = 0;

	/// The display list.
	mutable bdNChar8 m_displayList[0x1000];

	/// The matrix stack
	ScePspFMatrix4 m_matrixStack[projMatrixStackSize + viewMatrixStackSize +
								worldMatrixStackSize + texMatrixStackSize];
};

#endif // BD_GFX_PSP_H
