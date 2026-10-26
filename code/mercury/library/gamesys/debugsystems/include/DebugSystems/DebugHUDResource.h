#ifndef DEBUGHUDRESOURCE_H
#define DEBUGHUDRESOURCE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render { namespace Font { class C_FontRenderer;}}

namespace DebugSystems
{


class C_DebugHUDResource
{
public:
	C_DebugHUDResource();
	~C_DebugHUDResource();

	void							Update( const Maths::C_TimeS& /*timeStep*/ ){};
	void							Render( );
private:
	
	Render::Font::C_FontRenderer*	m_pFontRenderer;
};

} // namespace DebugSystems

#endif
