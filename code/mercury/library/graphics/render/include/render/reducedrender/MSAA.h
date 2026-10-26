#ifndef RENDER_REDUCEDRENDER_MSAA_H
#define RENDER_REDUCEDRENDER_MSAA_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
namespace Reduced
{

class C_MSAA
{
public:

	C_MSAA();
	~C_MSAA();

	void		Begin() const;
	void		End() const;

private:

	mutable const Display::C_Surface*	m_pColourSurface;
	mutable const Display::C_Surface*	m_pDepthStencilSurface;

#if defined(PLATFORM_PS3)
	mutable CellGcmSurface				m_gcmSurface;
#endif

};

} // namespace Reduced
} // namespace Render

#endif // RENDER_REDUCEDRENDER_MSAA_H
