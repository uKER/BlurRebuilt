// [Blur2PC] Official AMAX client lifetime and nonblocking stream bridge.
#pragma once
namespace AmaxClientBridge {
    enum Status { Stopped, Starting, Ready, Failed };
    // [Blur2PC] Recheck the extracted official client whenever the menu opens.
    bool IsAvailable();
    bool Start(const char* username, const char* password);
    Status GetStatus();
    const char* Error();
    void Stop();
}
