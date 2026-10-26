
#ifndef CAMERA2_CAMERASWITCH_H
#define CAMERA2_CAMERASWITCH_H

namespace Cam2
{

class C_CamOption
{
public:
	C_CamOption(S32 type, const Util::C_String& name = "");

	S32 m_Type;
	Util::C_String m_Name;
};

enum E_SwitchKey
{
	E_SwitchKey_Up = 0,
	E_SwitchKey_Down,
	E_SwitchKey_Left,
	E_SwitchKey_Right,
	E_SwitchKey_ShowMenu,
	E_SwitchKey_Confirm,
	E_SwitchKey_Count
};

class C_CameraSwitch
{
public:
	C_CameraSwitch();
	void Reset();
	void AddGroup();
	void AddOption(S32 type, const Util::C_String& name);

	void Update(F32 period);
	void RenderFullscreen();
	S32 Choice();
	char m_Key[E_SwitchKey_Count];
	bool UsingInput() { return m_ShowList; }
	void ShowAndFade() { m_Fader = 1.f; }
	
protected:
	S32 m_CamGroup, m_CamSel;
	S32 m_Choice, m_PrevChoice;
	bool m_ShowList;
	F32 m_Fader;
	F32 m_ScrollY;
	
	void SyncCameraChoice();

private:
	Ut::Vector< Ut::Vector< C_CamOption > > m_CamList;
};

}

#endif
