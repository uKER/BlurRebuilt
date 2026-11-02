// [Blur2PC] T259. Fixed-width local IPC between the game and its x86 AMAX host.
#pragma once
#include <stdint.h>
namespace AmaxClientProtocol {
    enum { Magic = 0x58414d31, MaxTransfer = 65536, MaxCredential = 64 };
    enum Operation { Create = 1, Connect, Send, Receive, Close };
    struct Login { uint32_t magic; char username[MaxCredential + 1]; char password[MaxCredential + 1]; };
    struct Command { uint32_t operation; int32_t handle; uint32_t size; };
    enum Result { Accepted = 0, BadProtocol = -100, HostLayout = -101, LoadClient = -102,
                  HooksMissing = -103, LoginRejected = -104 };
}
