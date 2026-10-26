
#ifndef CAMERA2_INPUTVIEW_H
#define CAMERA2_INPUTVIEW_H

namespace Render
{
	namespace Font
	{
		class C_FontRenderer;
	}
}

namespace DebugSystems
{

class C_InputView
{
public:
	C_InputView();
	~C_InputView();
	void Init();
	void Render();

private:
	Render::Font::C_FontRenderer* m_pFontRenderer;

	class C_ButtonPlace
	{
	public:
		F32 x, y, r, g, b;
		const char* text;
		F32 width;
	};

	static S32 s_ButtonId[16], s_AxisId[4];
	static C_ButtonPlace s_ButtonPlace[16];

	void DrawStick(F32 sx, F32 sy, F32 ax, F32 ay);
};

}

#endif
