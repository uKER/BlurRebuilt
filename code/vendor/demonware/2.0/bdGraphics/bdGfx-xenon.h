// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: Overall graphics class. Provides an application with platform independent graphics.

#ifndef BD_GFX_XENON_H
#define BD_GFX_XENON_H

#include <xtl.h>

#include <bdGraphics/bdFont-xenon.h>

/// Overall graphics class. Provides an application with platform independent graphics.
/// Initializes all that is required to display a 3D scene.
class bdGfxImpl : public bdGfxBase
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	// Default constructor.
	bdGfxImpl();

	// Default destructor.
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
	static D3DCOLOR	ConvertColor(bdColor color);
	static bdColor	ConvertColor(D3DCOLOR color);
	static bdUByte8	GetAlpha(D3DCOLOR color);
	static bdUByte8 GetRed(D3DCOLOR color);
	static bdUByte8 GetGreen(D3DCOLOR color);
	static bdUByte8 GetBlue(D3DCOLOR color);

protected:
	bdBool initDirect3D();

	void render() const;

	mutable bdFont m_font;
	bdUInt m_deviceWidth;
	bdUInt m_deviceHeight;
};

#endif // BD_GFX_XENON_H
