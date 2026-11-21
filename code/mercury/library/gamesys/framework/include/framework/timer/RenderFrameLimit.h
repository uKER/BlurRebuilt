// [Blur2PC] T274: player settings access to the existing render-only limiter.
#pragma once

namespace Framework
{
    // Called once while loading settings; a valid environment override wins at startup.
    void LoadRenderFrameLimit(const char* savedValue);
    int GetRenderFrameLimit();
    void SetRenderFrameLimit(int framesPerSecond);
}
