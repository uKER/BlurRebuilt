#ifndef DEFERRED_MASK_H
#define DEFERRED_MASK_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Deferred
{

struct C_MaskShaders
{
	bool Create();
	void Destroy();

	Display::C_VertexShader	m_VertexShader;
	Display::C_PixelShader	m_PixelShader;
};

class C_Mask
{
public:

	C_Mask();
	~C_Mask();

	void Begin() const;
	void End() const;

	void MaskAfter(const F32 fBegin) const;
	void MaskBefore(const F32 fEnd) const;
	void MaskBetween(const F32 fBegin, const F32 fEnd) const;

	void ReloadHiStencil() const;

private:

	void DrawDepth(const F32 fZ) const;

private:

	static S32				m_RefCount;
	static C_MaskShaders	m_Shaders;

	mutable F32				m_MaskBegin;
	mutable F32				m_MaskEnd;

};

}; // namespace Deferred

#endif // DEFERRED_MASK_H
