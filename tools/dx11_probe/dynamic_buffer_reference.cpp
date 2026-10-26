// [Blur1PCDX11] Exercise the game's native public buffer-lock entry points.
#include "StdAfx.h"
#include "Display/Device/D3D11/D3D11Device.h"
#include "Display/Device/D3D11/D3D11Effect.h"
#include "Display2/Context/D3D11Context.h"
#include <d3d11sdklayers.h>
#include <vector>
#include "dynamic_buffer_checks.h"

int main()
{
    Display::D3D11::C_D3D11Device device;
    if (FAILED(device.Initialise())) return 1;
    Microsoft::WRL::ComPtr<ID3D11InfoQueue> queue;
    device.Device()->QueryInterface(IID_PPV_ARGS(&queue));
    if (queue) queue->ClearStoredMessages();
    if (!CheckDynamicBufferLocks(device)) return 1;
    if (queue)
        for (UINT64 i=0;i<queue->GetNumStoredMessages();++i)
        {
            SIZE_T size=0; queue->GetMessage(i,nullptr,&size);
            std::vector<unsigned char> bytes(size);
            auto* message=reinterpret_cast<D3D11_MESSAGE*>(bytes.data());
            if (SUCCEEDED(queue->GetMessage(i,message,&size)) && message->Severity <= D3D11_MESSAGE_SEVERITY_ERROR)
            { std::fprintf(stderr,"Dynamic buffer GPU error: %s\n",message->pDescription); return 1; }
        }
    return 0;
}
