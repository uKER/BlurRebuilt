#ifndef EFFECTS_H
#define EFFECTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIDefines.h"
#include "ui/Graphics/UIColourClasses.h"
// #include "ui/Game/UIMaths.h"

namespace UI
{

class C_GadgetBaseData;

class UIEffect
{
public:
	UIEffect(){};
	~UIEffect(){};

	//void Add(ExporterEffect *effect);
	void Update(F32 time,C_GadgetBaseData *mData,SCREEN_TIME stime,bool overide=false, const S32 sCurve = -1 );

	void DoEffect(F32 timer,F32 *to_var1,F32 *to_var2,F32 *to_var3);
	void DoEffect(F32 timer,F32 *to_var1,F32 *to_var2);
	void DoEffect(F32 timer,F32 *to_var1);
	void DoEffect(F32 timer,S32 *to_var);

	bool CanWeDoEffect(SCREEN_TIME stime);

	S32			m_TimerUsed;
	F32			m_Var[6];		//0-2 is FROM 3-5 is TO
	C_UIColour	m_Col[2];

	S32 m_Type;
	S32 m_Sound;
	S32 m_EndSound;
	S32 m_When;
    F32 m_StartTime;
	F32 m_PrevTime;
	F32 m_Duration;

private:
};

}; // namespace UI
#endif // EFFECTS_H
