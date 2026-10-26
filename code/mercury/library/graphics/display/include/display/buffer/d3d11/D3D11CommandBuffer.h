#ifndef DISPLAY_D3D11_COMMAND_BUFFER_H
#define DISPLAY_D3D11_COMMAND_BUFFER_H

// [Blur1PCDX11] PC render tasks submit directly on the immediate context.
// The PC D3D9 implementation also had no recorded command buffer.
namespace Display
{
class C_CommandBuffer
{
public:
    bool IsCreated() const { return false; }
    void Create(U32) {}
    void Create() {}
    void Destroy() {}
    void Bind() {}
    void Unbind() {}
    void Run() {}
    void SetEmptyCommandBuffer(bool) {}
};
}
#endif
