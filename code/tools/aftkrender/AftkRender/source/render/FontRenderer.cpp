#include "AftkRenderPch.h"
#include "render/FontRenderer.h"

namespace AftkRender
{

C_FontRenderer::C_FontRenderer()
: Display::I_Renderer("AftkRender.FontRenderer")
, m_pCacheSprite(0)
, m_TextBegin(0)
{
}

C_FontRenderer::~C_FontRenderer()
{
}

bool C_FontRenderer::OnCreate()
{
	HRESULT hr = D3DXCreateSprite(Display::Device()->GetD3DDevice(), &m_pCacheSprite);
	if (FAILED(hr))
		return false;

	return LoadFont("system", 10);
}

void C_FontRenderer::OnDestroy()
{
	Ut::UtMap<FontMapKey, ID3DXFont*>::Iterator it = m_FontMap.Begin();
	for (it; it!=m_FontMap.End(); ++it)
		SAFE_RELEASE(it->GetValue());
	m_FontMap.Clear();

	SAFE_RELEASE(m_pCacheSprite);
}

bool C_FontRenderer::OnCreateVolatile()
{
	Ut::UtMap<FontMapKey, ID3DXFont*>::Iterator it = m_FontMap.Begin();
	for (it; it!=m_FontMap.End(); ++it)
		it->GetValue()->OnResetDevice();

	if (m_pCacheSprite)
		m_pCacheSprite->OnResetDevice();
	return true;
}

void C_FontRenderer::OnDestroyVolatile()
{
	Ut::UtMap<FontMapKey, ID3DXFont*>::Iterator it = m_FontMap.Begin();
	for (it; it!=m_FontMap.End(); ++it)
		it->GetValue()->OnLostDevice();

	if (m_pCacheSprite)
		m_pCacheSprite->OnLostDevice();
}

bool C_FontRenderer::LoadFont(const char* pFontName, U8 size)
{
	Ut::UtMap<FontMapKey, ID3DXFont*>::Iterator it = m_FontMap.Find(FontMapKey(pFontName, size));
	if (it != m_FontMap.End())
		return true;

	HRESULT hr;

    HDC hDC = GetDC( NULL );
    S32 nLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);
    ReleaseDC( NULL, hDC );

	S32 nHeight = S32(-size*((F32)nLogPixelsY/72));

	ID3DXFont *pFont;

	hr = D3DXCreateFont(Display::Device()->GetD3DDevice(), nHeight, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, pFontName, &pFont);
	if (FAILED(hr))
		return false;

	m_FontMap.Insert(Ut::UtMap<FontMapKey, ID3DXFont*>::DataType(FontMapKey(pFontName, size), pFont));
	return true;
}

void C_FontRenderer::Begin()
{
	if(m_TextBegin == 0)
	{
		m_pCacheSprite->Begin(D3DXSPRITE_ALPHABLEND|D3DXSPRITE_SORT_TEXTURE);
		Display::Device()->SetDepthEnable(Display::E_Bool_False);
	}
	m_TextBegin++;
}

void C_FontRenderer::End()
{
	m_TextBegin--;
	if(m_TextBegin == 0)
	{
		m_pCacheSprite->End();
		Display::Device()->SetDepthEnable(Display::E_Bool_True);
	}
}

void C_FontRenderer::Draw(const char* pFontName, U8 size, const char* pText, S32 x, S32 y, const Maths::RGBA colour)
{
	Ut::UtMap<FontMapKey, ID3DXFont*>::ConstIterator it;

	if (!pFontName)
		it = m_FontMap.Find(FontMapKey("system", 10));
	else
		it = m_FontMap.Find(FontMapKey(pFontName, size));

	if (it == m_FontMap.End())
		return;

	ID3DXFont* pFont = it->GetValue();

	RECT rect;
	SetRect(&rect, x, y, 0, 0);
	D3DXCOLOR c(colour.R()/255.f, colour.G()/255.f, colour.B()/255.f, colour.A()/255.f);

	pFont->DrawText(m_pCacheSprite, pText, -1, &rect, DT_CALCRECT, c);
	pFont->DrawText(m_pCacheSprite, pText, -1, &rect, DT_NOCLIP, c);
}

RECT C_FontRenderer::GetRect(const char* pFontName, U8 size, const char* pText, S32 x, S32 y)
{
	Ut::UtMap<FontMapKey, ID3DXFont*>::ConstIterator it;

	RECT rect;
	SetRect(&rect, x, y, 0, 0);

	if (!pFontName)
		it = m_FontMap.Find(FontMapKey("system", 10));
	else
		it = m_FontMap.Find(FontMapKey(pFontName, size));

	if (it == m_FontMap.End())
		return rect;

	ID3DXFont* pFont = it->GetValue();
	pFont->DrawText(0, pText, -1, &rect, DT_CALCRECT, D3DXCOLOR(0xff000000));

	return rect;
}

}
