#ifndef SR_D3D_WINDOW_H
#define SR_D3D_WINDOW_H

#define D3DWINDOW_NUM_FONTS 3

namespace SRD
{

	class D3dWindow
	{
	public:
		D3dWindow();
		~D3dWindow();

		bool				AttachToWindow(HINSTANCE instance, HWND hWnd);
		// Destroy window and shut everything down
		void				Destroy();

		void				Resize(U32 width, U32 height);
		void				Print(S32 fontIndex, S32 x, S32 y, const std::string& text, const Colour4& colour = Colour4(1,1,1.0,1.0));

		// Get Direct3D interface
		IDirect3D9*			GetD3dInterfacePtr();
		// Get Direct3D	device
		IDirect3DDevice9*	GetD3dDevicePtr();
		SRD::D3dWindow*		GetD3dWindowPtr();
		HINSTANCE			GetHInstance() const;
		const std::string&	GetClassName() const;
		HWND				GetHWnd() const;
		HDC					GetHDC() const;
		HGLRC				GetHRC() const;
		bool				GetIsVisibleFlag() const;
		U32					GetWidth() const;
		U32					GetHeight() const;
		U32					GetBitsPerPixel() const;
		U32					GetDepth() const;
		bool				GetFullScreenFlag() const;
		LPD3DXFONT*			GetFontPtr(S32 index);
		S32					GetNumFonts() const;

		void				SetHInstance(HINSTANCE hInstance);
		void				SetClassName(const std::string className);
		void				SetHWnd(HWND hWnd);
		void				SetHDC(HDC hDC);
		void				SetHRC(HGLRC hRC);
		void				SetIsVisibleFlag(bool flag);
		void				SetD3dDevicePtr(IDirect3DDevice9* pD3DDevice);
		void				SetWidth(U32 width);
		void				SetHeight(U32 height);
		void				SetBitsPerPixel(U32 bitsPerPixel);
		void				SetDepth(U32 depth);
		void				SetFullScreenFlag(bool flag);

		HRESULT				GetFormat(	IDirect3D9 *d3d, bool fullscreen, 
										UCHAR depth,D3DFORMAT *format);

		HRESULT				HandleLostDevice();

	private:
		bool				CreateD3dAndD3dDevice();
		void				DestroyD3dAndD3dDevice();

		bool				CreateFonts();
		void				DestroyFonts();

		D3DFORMAT			m_d3dFormat;

		HINSTANCE			m_hInstance;				// Application Instance
		std::string 		m_className;				// Application ClassName

		HWND				m_hWnd;						// Window Handle
		HDC					m_hDC;						// Device Context
		HGLRC				m_hRC;						// Rendering Context
		bool				m_isVisibleFlag;			// Window Visible?

		IDirect3D9			*m_pD3d;
		IDirect3DDevice9	*m_pD3dDevice;

		U32					m_width;
		U32					m_height;
		U32					m_bitsPerPixel;
		U32					m_depth;
		bool				m_fullScreenFlag;


		// Presentation parameters.
		D3DPRESENT_PARAMETERS m_pp;

		// Font stuff
		S32					m_fontSize;
		
		LPD3DXFONT			m_fonts[D3DWINDOW_NUM_FONTS];
		WCHAR*				m_strTextBuffer;
		ID3DXSprite*		m_pD3dxSprite;
		TCHAR				m_strFont[LF_FACESIZE];
	};
}

#endif
