#ifndef DISPLAY_D3D11_RING_BUFFER_H
#define DISPLAY_D3D11_RING_BUFFER_H

// [Blur1PCDX11] The PC ring buffer holds no GPU command memory. D3D11 uses
// ID3D11Buffer objects and the immediate context for the actual allocations.
namespace Display
{
class C_MemorySet { public: void Clear() {} };
class C_RingBuffer
{
public:
    void Initialise(U32) {}
    void Shutdown() {}
    void EndFrame() {}
    U32 GetUsedSize() const { return 0; }
};
inline C_RingBuffer* RingBuffer()
{
    static C_RingBuffer buffer;
    return &buffer;
}
}
#endif
