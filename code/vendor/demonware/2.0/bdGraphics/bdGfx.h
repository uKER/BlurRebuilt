// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 XBOX XENON PS2 PSP-INFRA PSP-ADHOC PS3 WII OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_GFX_H
#define BD_GFX_H

#include <bdCore/bdCore.h>
#include <bdGraphics/bdColor.h>
#include <bdGraphics/bdMesh.h>

/// The Mathematical constant pi
static const bdFloat32 BD_PI = 3.141592654f;
static const bdUInt BD_MAX_SCREEN_TEXT = 1024;

#ifdef BD_PLATFORM_WIN32
	// Win32 needs a class name to distinguish the window class that we create from others
#define BD_MAIN_WINDOW_CLASS_NAME "DemonWare Graphics Window Class"
#include <bdGraphics/bdGfxLib.h>
#endif // BD_PLATFORM_WIN32

/// The abstract base class for the DemonWare Graphics Engine. \a init() should be called before any other method to
/// initialize the object. \a runFrame() should be called when you wish to render the scene. If the engine detects a
/// request to quit or close the application, \a runFrame() shall return \a false. \a cleanup() should be called when
/// shutting down the application to perform any kind of releasing of resources. \a addMesh() and \a removeMesh() are
/// used to add and remove objects (represented as \a bdMesh objects) from the scene.
class bdGfxBase
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	/// Default constructor. Initializes m_backgroundColor.
	bdGfxBase();

	virtual ~bdGfxBase();

	/// Initializes the graphics class.
	/// \return
	/// - True, if initialization was successful
	/// - False, if initialization failed
	virtual bdBool init(bdInt32 *argc = BD_NULL, bdNChar8 *argv[] = BD_NULL) = 0;

	/// Run one frame of the application.
	/// \return
	/// - True, if the application should continue execution
	/// - False, if a shutdown request was received
	virtual bdBool runFrame() const = 0;

	/// Cleanup on shutdown.
	virtual void cleanup() = 0;

	/// Displays a text string on the screen.
	/// \param str[in] The string to display on the screen.
	virtual void setText(const bdNChar8 *const str);

	/// Retrieves the text that is being drawn to the screen.
	void getText(bdNChar8 *const buffer, bdUInt bufferSize);

	/// Sets the position of the top-left of the text on the screen.
	/// \param x[in] The x-coordinate of the text.
	/// \param y[in] The y-coordinate of the text.
	virtual void setTextPosition(bdFloat32 x, bdFloat32 y);

	/// Gets the position of the top-left of the text drawn to the screen.
	/// \param x[out] The x-coordinate of the text.
	/// \param y[out] The y-coordinate of the text.
	void getTextPosition(bdFloat32 &x, bdFloat32 &y);

	/// Set text color.
	/// \param color[in] The new color of the text
	void setTextColor(bdColor color);

	/// Retrieves the color of the text on the screen.
	/// \return The color of the text
	bdColor getTextColor();

	/// Get the buffer's background color.
	/// \return The application's current background color.
	bdColor getBackgroundColor() const;

	/// Set the background color of the buffer.
	/// \param color[in] The new background color for the application.
	virtual void setBackgroundColor(const bdColor &color);

	/// Add a mesh to the scene.
	/// \param mesh[in] The mesh to add the scene.
	/// \return
	/// - True, if \a mesh was added to the scene
	/// - False, if the mesh was already in the scene and so was not added again
	bdBool addMesh(bdMesh *mesh);

	/// Remove a mesh from the scene.
	/// \return
	/// - True, if \a mesh was removed from the scene
	/// - False, if \a mesh was not in the scene
	bdBool removeMesh(bdMesh *mesh);

protected:
	/// The background color to use when clearing the draw buffer.
	bdColor m_backgroundColor;

	/// The array of meshes to be drawn by the engine.
	bdFastArray<bdMesh*> m_meshes;

	/// Text to be displayed on the screen.
	bdNChar8 m_text[BD_MAX_SCREEN_TEXT];

	/// The position of the left of the text's bounding rectangle.
	bdFloat32 m_textPositionX;

	/// The position of the top of the text's bounding rectangle.
	bdFloat32 m_textPositionY;

	/// The color to draw the text on the screen.
	bdColor m_textColor;
};

// Include the correct header for the current platform
#if defined BD_PLATFORM_WIN32 && defined BD_WIN32_DIRECTX
//#	pragma message ("Platform: Win32 - DirectX")
#	include <bdGraphics/bdGfx-win32-dx.h>
#elif defined BD_PLATFORM_WIN32 && defined BD_WIN32_GL
//#	pragma message ("Platform: Win32 - GL")
#	include <bdGraphics/bdGfx-win32-gl.h>
#elif defined BD_PLATFORM_XENON
//#	pragma message ("Platform: Xenon")
#	include <bdGraphics/bdGfx-xenon.h>
#elif defined BD_PLATFORM_XBOX
//#	pragma message ("Platform: XBOX")
#	include <bdGraphics/bdGfx-xbox.h>
#elif defined BD_PLATFORM_PS2
//#	pragma message ("Platform: PS2")
#	include <bdGraphics/bdGfx-ps2.h>
#elif defined BD_PLATFORM_PSP
//#	pragma message ("Platform: PSP")
#	include <bdGraphics/bdGfx-psp.h>
#elif defined BD_PLATFORM_PS3
//# pragma message ("Platform: PS3")
#	include <bdGraphics/bdGfx-ps3.h>
#elif defined BD_PLATFORM_WII
//# pragma message ("Platform: WII")
#	include <bdGraphics/bdGfx-wii.h>
#elif defined BD_PLATFORM_APPLE
//# pragma message ("Platform: APPLE")
#	include <bdGraphics/bdGfx-apple.h>
#else
#	error Unknown platform, not supported.
#endif

/// Class to provide access to singleton graphics object.
class bdGfx : public bdSingleton<bdGfxImpl>
{
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	protected:
		bdGfx();
};

#endif // BD_GFX_H
