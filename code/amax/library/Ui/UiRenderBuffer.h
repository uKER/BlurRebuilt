#ifndef UIRENDERBUFFER_H
#define UIRENDERBUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace UI
{

class C_UiRenderBuffer : public Display::I_Renderer
{
public:
										C_UiRenderBuffer();
										~C_UiRenderBuffer();

	bool 								OnCreateVolatile();
	void 								OnDestroyVolatile();

	void 								SetWidth( const U32 width );
	void 								SetHeight( const U32 height );
	void 								SetMultisample( const Display::E_Multisample multisample );
	void 								SetColourFormat( const Display::E_Format colourFormat );
	void 								SetDepthStencilFormat( const Display::E_Format depthStencilFormat );

	bool								Bind();
	void								Unbind( const bool bDoResolve = true );
	void								FinalResolve();

	bool								IsBound() const 
										{
											return m_Bound; 
										}

	U32									GetWidth() const 
										{ 
											return m_Width; 
										}

	U32									GetHeight() const 
										{
											return m_Height; 
										}

	Display::E_Multisample				GetMultisample() const 
										{ 
											return m_Multisample; 
										}

	Display::E_Format					GetColourFormat() const 
										{ 
											return m_ColourFormat; 
										}

	Display::E_Format					GetDepthStencilFormat() const 
										{ 
											return m_DepthStencilFormat; 
										}

	Display::C_Texture2D&				GetTexture() 
										{ 
											return m_ColourRenderTexture.GetTexture(); 
										}

	const Display::C_Texture2D&			GetTexture() const 
										{ 
											return m_ColourRenderTexture.GetTexture(); 
										}

private:
	bool								m_Bound;
	U32									m_Width;
	U32									m_Height;
	Display::E_Multisample				m_Multisample;
	Display::E_Format					m_ColourFormat;

	Display::E_Format					m_DepthStencilFormat;
	Display::C_Surface					m_DepthStencilSurface;

	Display::C_RenderTexture2D			m_ColourRenderTexture;
};

} // namespace UI

#endif
