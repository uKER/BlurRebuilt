#ifndef AFTKRENDER_FONT_RENDERER_H
#define AFTKRENDER_FONT_RENDERER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AftkRender
{

	class C_FontRenderer : public Display::I_Renderer
	{
	public:

		C_FontRenderer();
		~C_FontRenderer();

		bool OnCreate();
		void OnDestroy();

		bool OnCreateVolatile();
		void OnDestroyVolatile();

		bool LoadFont(const char* pFont, U8 size);

		void Begin();
		void End();

		RECT GetRect(const char* pFontName, U8 size, const char* pText, S32 x, S32 y);
		void Draw(const char* pFontName, U8 size, const char* pText, S32 x, S32 y, const Maths::RGBA colour);

	private:

		typedef Ut::Pair<Util::C_String, U8> FontMapKey;
		Ut::UtMap<FontMapKey, ID3DXFont*>	m_FontMap;
		ID3DXSprite*	m_pCacheSprite;
		int				m_TextBegin;
	};
}

#endif