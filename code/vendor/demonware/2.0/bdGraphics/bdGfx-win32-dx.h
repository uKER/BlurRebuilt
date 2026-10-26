// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_GFX_WIN32_DX
#define BD_GFX_WIN32_DX

struct ID3DXFont;

/// The Win32-DirectX specific implementation of the Graphics class.
/// This class provides functionality to display a 3D scene.
class bdGfxImpl : public bdGfxBase
{
// Public non-platform specific class members
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	/// Default empty constructor.
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

	/// Cleanup for shutting down
	virtual void cleanup();

	/// Displays a text string on the screen.
	/// \param str[in] The string to display on the screen.
	virtual void setText(const bdNChar8 *const str);

// Public platform specific class members
public:
	/// Static function to convert a platform independent bdColor object into a Direct3D color.
	/// \return The Direct3D equivalent of \a color.
	static DWORD	BD_CALL ConvertColor(bdColor color);

	/// Static function to convert a Direct3D color into a platform independent bdColor object.
	/// \retunr The bdColor equivalent of \a color.
	static bdColor	BD_CALL ConvertColor(DWORD color);

	/// \return The alpha (transparency) component of \a color.
	static bdUByte8	BD_CALL GetAlpha(DWORD color);

	/// \return The red component of \a color.
	static bdUByte8 BD_CALL GetRed(DWORD color);

	/// \return The green component of \a color.
	static bdUByte8 BD_CALL GetGreen(DWORD color);

	/// \return The blue component of \a color.
	static bdUByte8 BD_CALL GetBlue(DWORD color);
	
	/// \return The Win32 Handle to the application's main window.
	HWND getWindowHandle() const;

protected:
	/// Initialize Win32 specifics.
	/// Registers a new Window Class and creates the main window for the application.
	/// \return
	/// - True, if initialization was successful
	/// - False, if initialization failed
	bdBool initWindows();

	/// Initialize Direct3D specifics.
	/// Creates the direct3d device then stores it in the bdD3D singleton object. Initializes some direct 3d
	/// properties and sets some initial states for the direct3d device.
	/// \return
	/// True, if initialization was successful
	/// False, if initialization failed
	bdBool initDirect3D();

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

	/// Determines whether the bdGfx object has completed initialization. If,
	/// - True, Initialization of the object is complete.
	/// - False, Initialization of the object is not complete.
	bdBool m_initialized;

	/// Pointer to the font object.
	/// Used to display text on the screen.
	ID3DXFont *m_font;

	/// The height of the text on screen (in pixels)
	bdInt m_textHeight;

	/// The width of the text on screen (in pixels)
	bdUInt m_textWidth;

	/// The weight of the text on screen.
	/// This value can be between 0 and 1000. The larger the number, the more bold the text
	bdUInt m_textWeight;

	/// The position of the right of the text's bounding rectangle.
	bdLong m_textRight;

	/// The position of the bottom of the text's bounding rectangle.
	bdLong m_textBottom;
};

#endif	// BD_GFX_WIN32_DX
