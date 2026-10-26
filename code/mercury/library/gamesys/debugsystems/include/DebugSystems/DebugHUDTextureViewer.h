#ifndef DEBUGHUDTEXTUREVIEWER_H
#define DEBUGHUDTEXTUREVIEWER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/vector.h>
#include <display/texture/Texture2D.h>

namespace Render { namespace Font { class C_FontRenderer;}}

namespace DebugSystems
{
//Interface to control the DebugHUDTextureViewer
class I_ControllerTextureViewer
{
public:
	// Debounced movement (preferably with delay & repeat)
	virtual const bool 		MoveUp( ) = 0;
	virtual const bool 		MoveDown( ) = 0;
	virtual const bool 		MoveLeft( ) = 0;
	virtual const bool 		MoveRight( ) = 0;
	// Debounced selection
	virtual const bool 		Confirm( ) = 0;
	virtual const bool 		Cancel( ) = 0;
	//Delta position
	virtual const bool		PositionMove(F32& deltaX, F32& deltaY) = 0;
};

enum E_State
{
	E_State_SingleTexture,
	E_State_Zoom,
	E_State_Unzoom,
	E_State_Browser
};

enum E_FullScreen_State
{
	E_FullScreen_State_Colour,
	E_FullScreen_State_Alpha,
	E_FullScreen_State_1x1Pixel,
	E_FullScreen_State_Mipmaps,
	E_FullScreen_State_Off,
	E_FullScreen_State_Total
};

class C_DebugHUDTextureViewer
{
public:
	C_DebugHUDTextureViewer(I_ControllerTextureViewer* pController);
	~C_DebugHUDTextureViewer();

	void							Update( const Maths::C_TimeS& timeStep );
	void							Render( );

	static const void*				GetStreamDebugTexture() { return s_pStreamDebugTexture;};
private:
	
	void Up();
	void Down();
	void Left();
	void Right();

	void Confirm();
	void Cancel();

	void RenderTexture(Maths::HVec2 PCREF position, Maths::HVec2 PCREF size, F32 intensity, F32 alpha, Display::C_Texture2D* pTexture);

	void ClearTextureNameSelected();

	I_ControllerTextureViewer*			m_pController;
	Render::Font::C_FontRenderer*		m_pFontRenderer;
	E_State								m_ViewerState;
	F32									m_ZoomPosition;
	E_FullScreen_State					m_FullScreenState;
	S32									m_CurrentMipMap;
	bool								m_ColourGammaEnabled;
	Util::C_String						m_CurrentTextureName;
	const void*							m_CurrentTexturePointer;
	Btl::Vector<Display2::C_ResourceListIt>	m_CachedTextureVector;

	//Stream debug texture
	static const void*					s_pStreamDebugTexture;
	//Device resources
	Display::C_PixelShaderParameter m_psTextureViewerShaderParams;
	Display::C_PixelShader			m_PixelShader;
	Display::C_VertexShader			m_VertexShader;
	Display::C_VertexDeclaration*	m_pVertexDeclaration2DTex;
};

} // namespace DebugSystems

#endif
