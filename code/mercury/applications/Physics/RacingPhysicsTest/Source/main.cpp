#include "stdafx.h"
#include "racingphysicstest.h"

void main()
{
	RacingPhysicsTestApp::Instance().Create();

    for(;;) // loop forever
    {
        RacingPhysicsTestApp::Instance().Update();

        RacingPhysicsTestApp::Instance().Render();
    }
}