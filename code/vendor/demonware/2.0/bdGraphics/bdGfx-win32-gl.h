// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_GFX_WIN32_GL_H
#define BD_GFX_WIN32_GL_H

#ifdef BD_WIN32_GL

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

// Public platform specific class members
public:
	/// \return The Win32 Handle to the application's main window.
	HWND getWindowHandle() const;

protected:
	/// Initialize Win32 specifics.
	/// Registers a new Window Class and creates the main window for the application.
	/// \return
	/// - True, if initialization was successful
	/// - False, if initialization failed
	bdBool initWin32();

	/// Initializes any OpenGL specifics.
	/// \return
	/// - True, if initialization was successful
	/// - False, if initialization failed
	bdBool initGL();

	/// Draw the scene.
	/// Clears the back buffer, sets up the view and projection transformation matrices and iterates through
	/// \a m_meshes, calling each mesh's draw function. Finally presents the back buffer to the front buffer.
	void render() const;

	/// The callback window procedure to process messages from Windows
	/// \param hWnd[in] The handle to the window the message is being sent to.
	/// \param msg[in] The identifier of the message being sent.
	/// \param wParam[in] A parameter for the message. Message Dependent.
	/// \param lParam[in] A parameter for the message. Message Dependent.
	/// \return The meaning of the return value is dependent on the message.
	static LRESULT WINAPI MessageProc(	HWND hWnd,
										UINT msg,
										WPARAM wParam,
										LPARAM lParam);

protected:
	/// Handle to the main window.
	/// Required for creating/using a window by Win32.
	HWND m_mainWindowHandle;

	/// Handle to a device context for the main window
	HDC m_deviceContext;

	/// Handle to a rendering context for the main window
	HGLRC m_renderContext;
};

#endif // BD_WIN32_GL

#endif	// BD_GFX_WIN32_GL_H
