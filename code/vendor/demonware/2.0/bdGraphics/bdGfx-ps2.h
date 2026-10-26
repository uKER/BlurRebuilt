// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_GFX_PS2_H
#define BD_GFX_PS2_H

#include <external/ps2shell/include/shellCore.h>
#include <external/ps2shell/include/shellPacket.h>
#include <external/ps2shell/include/shellDebug.h>

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

	static inline void *CachePointer(void *ptr)
	{
		return reinterpret_cast<void *>(reinterpret_cast<bdUInt>(ptr) & 0x0fffffff);
	}

	static inline void *UncachePointer(void *ptr)
	{
		return reinterpret_cast<void *>((reinterpret_cast<bdUInt>(ptr) & 0x0fffffff) | 0x20000000);
	}

	/// GS Packet memory for meshes.
	/// Pointer to memory used to create GS Packets for meshes.
	u_long *m_packetMem;

protected:
	/// Draw the scene.
	/// Clears the back buffer, sets up the view and projection transformation matrices and iterates through
	/// \a m_meshes, calling each mesh's draw function. Finally presents the back buffer to the front buffer.
	void render() const;

	/// Console Packet memory.
	/// Memory used to create the console's GS Packet.
	u_char *m_pConsolePacket;

	/// The console object.
	mutable ShellDebugConsole m_console;

	/// The console's font object.
	mutable ShellDebugFontInfo m_font;

	/// Width of the screen.
	const bdUInt m_screenWidth;

	/// Height of the screen.
	const bdUInt m_screenHeight;
};

#endif // BD_GFX_PS2_H
