#ifndef SIMPLETIMER_H
#define SIMPLETIMER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace UI
{

class SimpleTimer
{
public:
	SimpleTimer() : time_scale(1.0f) {}
	void Initialise(){thisTickCount=lastTickCount=0;pulse=0.0f;pulse_dirc=1;ticktock=false;ticktockcount=0;anim_timer=0.0f;time_scale=1.0f;};
	void Update( const Maths::C_TimeS& time );
	float FrameTime(){return timer;}; //returns m/s since last frame
	float FrameTimex2(){return timer*2.0f;}; //returns m/s since last frame
	float FrameTimex4(){return timer*4.0f;}; //returns m/s since last frame
	float FrameTimex8(){return timer*8.0f;}; //returns m/s since last frame
	float Pulse(){return pulse;};

	float ActualFrameTime(){return actual_timer;}; //returns m/s since last frame

	bool TickTock() {return ticktock;};
	bool SlowTickTock() {return slowticktock;};

	void ResetPulse(){pulse=0.0f;};
	float AnimTime() {return anim_timer;};
	void ResetAnim(){anim_timer=0.0f;};

	float GameTime(){return 1.f/60.f;};
	//float GameTime(){return timer;};

	void SetTimeScale(float scale) { time_scale = scale; }

private:
	int thisTickCount,lastTickCount;
	float timer;
	float actual_timer;
	float pulse;
	char pulse_dirc;
	bool ticktock;
	float ticktockcount;
	float anim_timer;
	float time_scale;

	bool slowticktock;
	float slowticktockcount;
};

extern SimpleTimer theFETimer;

}; // namespace UI

#endif // SIMPLETIMER_H
