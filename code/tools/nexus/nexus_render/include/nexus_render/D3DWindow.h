#ifndef SR_D3D_WINDOW_H
#define SR_D3D_WINDOW_H

#define C_D3DWINDOW_NUM_FONTS 3

namespace NexusRender
{

	class C_D3DWindow
	{
	public:
		C_D3DWindow();

		~C_D3DWindow();

		bool AttachToWindow(HINSTANCE instance, HWND hWnd);
		// Destroy window and shut everything down
		void Destroy();

		void Resize(U32 width, U32 height);

		// Get Direct3D interface
		IDirect3D9*	InterfacePtr();
		// Get Direct3D	device
		IDirect3DDevice9* DevicePtr();

		NexusRender::C_D3DWindow*	C_D3DWindowPtr();
		
		HINSTANCE HInstance() const;
		
		const std::string& ClassName() const;
		
		HWND GetHWnd() const;
		
		HGLRC GetHRC() const;
		
		U32	GetWidth() const;

		U32	GetHeight() const;

		U32	GetBitsPerPixel() const;

		U32	GetDepth() const;

		bool IsVisible() const;

		bool IsFullScreen() const;

		S32	GetNumFonts() const;

		void SetHInstance(HINSTANCE hInstance);

		void SetClassName(const std::string className);

		void SetHWnd(HWND hWnd);

		void SetHRC(HGLRC hRC);

		void SetVisible(bool flag);

		void SetWidth(U32 width);

		void SetHeight(U32 height);

		void SetBitsPerPixel(U32 bitsPerPixel);

		void SetDepth(U32 depth);

		void SetFullScreen(bool flag);

		HRESULT Format(
			IDirect3D9 *D3D, 
			bool fullscreen, 
			UCHAR depth,D3DFORMAT *format);

		HRESULT	HandleLostDevice();

		LPD3DXFONT	GetFontPtr(S32 index);

	private:
		bool CreateD3DAndD3DDevice();
		void DestroyD3DAndD3DDevice();

		bool Initialise();
		void Uninitialise();

		bool CreateFonts();
		void DestroyFonts();

		HINSTANCE			m_hInstance;				// Application Instance
		std::string 		m_className;				// Application ClassName
		HWND				m_hWnd;						// Window Handle
		HGLRC				m_hRC;						// Rendering Context
		bool				m_isVisibleFlag;			// Window Visible?

		D3DFORMAT			m_d3dFormat;
		IDirect3D9			*m_pD3D;
		IDirect3DDevice9	*m_pD3DDevice;

		U32					m_width;
		U32					m_height;
		U32					m_bitsPerPixel;
		U32					m_depth;
		bool				m_fullScreenFlag;


		// Presentation parameters.
		D3DPRESENT_PARAMETERS m_pp;

		// Font stuff
		S32					m_fontSize;
		
		LPD3DXFONT			m_fonts[C_D3DWINDOW_NUM_FONTS];
		WCHAR*				m_strTextBuffer;
		ID3DXSprite*		m_pD3DxSprite;
		TCHAR				m_strFont[LF_FACESIZE];
	};
}

#endif
