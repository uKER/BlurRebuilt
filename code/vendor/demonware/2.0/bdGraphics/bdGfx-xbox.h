// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: Provide a Graphics class for the XBox

#ifndef BD_GFX_XBOX_H
#define BD_GFX_XBOX_H

#include <xtl.h>

#include <bdGraphics/bdFont-xbox.h>

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

// Public platform specific class members
public:
	/// Static function to convert a platform independent bdColor object into a Direct3D color.
	/// \return The Direct3D equivalent of \a color.
	static D3DCOLOR	BD_CALL ConvertColor(bdColor color);

	/// Static function to convert a Direct3D color into a platform independent bdColor object.
	/// \retunr The bdColor equivalent of \a color.
	static bdColor	BD_CALL ConvertColor(D3DCOLOR color);

	/// \return The alpha (transparency) component of \a color.
	static bdUByte8	BD_CALL GetAlpha(D3DCOLOR color);

	/// \return The red component of \a color.
	static bdUByte8 BD_CALL GetRed(D3DCOLOR color);

	/// \return The green component of \a color.
	static bdUByte8 BD_CALL GetGreen(D3DCOLOR color);

	/// \return The blue component of \a color.
	static bdUByte8 BD_CALL GetBlue(D3DCOLOR color);
	
protected:
	/// Initialize Direct3D specifics.
	/// Creates the direct3d device then stores it in the bdD3D singleton object. Initialises some direct 3d
	/// properties and sets some initial states for the direct3d device.
	/// \return
	/// True, if initialization was successful
	/// False, if initialization failed
	bdBool initDirect3D();

	/// Draw the scene.
	/// Clears the back buffer, sets up the view and projection transformation matrices and iterates through
	/// \a m_meshes, calling each mesh's draw function. Finally presents the back buffer to the front buffer.
	void render() const;

	mutable bdFont m_font;
};

#endif // BD_GFX_XBOX_H
