#include "stdafx.h"
#include "racingphysicstest.h"

int main()
{
	RacingPhysicsTestApp::Instance().Create();

    for(;;) // loop forever
    {
        RacingPhysicsTestApp::Instance().Update();

        RacingPhysicsTestApp::Instance().Render();
    }

	return 0;
}
