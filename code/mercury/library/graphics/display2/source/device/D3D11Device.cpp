// [Blur1PCDX11] Native Display2 device and declaration ownership.
#include "StdAfx.h"
#include "Display2/Device/Device.h"

namespace Display2
{
C_D3D11DeviceParams::C_D3D11DeviceParams() : m_pd3dDevice(nullptr), m_pd3dObject(nullptr) {}
C_D3D11DeviceParams::~C_D3D11DeviceParams() {}
C_D3D11Device::C_D3D11Device() : m_Created(0), m_LastError(GPU_ERROR_SUCCESS),
    m_AllowOutOfMemory(false), m_pd3dDevice(nullptr), m_pd3dObject(nullptr) {}
C_D3D11Device::~C_D3D11Device() {}
U32 C_D3D11Device::IsCreated() const { return m_Created; }
GPUDevice* C_D3D11Device::GetD3DDevice() { return m_pd3dDevice; }
GPUObject* C_D3D11Device::GetD3DObject() { return m_pd3dObject; }
U32 C_D3D11Device::GetLastError() const { return m_LastError; }
void C_D3D11Device::SetLastError(U32 error) { m_LastError = error; }
void C_D3D11Device::AllowOutOfMemory(bool value) { m_AllowOutOfMemory = value; }
C_Context& C_D3D11Device::GetContext() { return m_Context; }
const C_Context& C_D3D11Device::GetContext() const { return m_Context; }
C_ContextNoCheck& C_D3D11Device::GetContextNoCheck() { return m_ContextNoCheck; }
const C_ContextNoCheck& C_D3D11Device::GetContextNoCheck() const { return m_ContextNoCheck; }
C_ContextNoCheckNoTrack& C_D3D11Device::GetContextNoCheckNoTrack() { return m_ContextNoCheckNoTrack; }
const C_ContextNoCheckNoTrack& C_D3D11Device::GetContextNoCheckNoTrack() const { return m_ContextNoCheckNoTrack; }
S32 C_D3D11Device::GetVertexDeclarationCount() const { return m_VertexDeclarationList.Size(); }
C_VertexDeclaration* C_D3D11Device::GetVertexDeclaration(S32 index) const { return m_VertexDeclarationList[index]; }
C_VertexDeclaration* C_D3D11Device::FindVertexDeclaration(S32 count, const C_VertexElement* elements) const
{
    for (S32 i = 0; i < m_VertexDeclarationList.Size(); ++i)
    {
        C_VertexDeclaration* candidate = m_VertexDeclarationList[i];
        if (candidate->GetElementCount() != count) continue;
        bool equal = true;
        for (S32 j = 0; j < count; ++j)
            if (candidate->m_Elements[j] != elements[j]) { equal = false; break; }
        if (equal) return candidate;
    }
    return nullptr;
}
C_VertexDeclaration* C_D3D11Device::CreateVertexDeclaration(const C_VertexFormat& format)
{ return CreateVertexDeclaration(format.m_Elements); }
bool C_D3D11Device::CreateVertexDeclaration(const C_VertexFormat& format, C_VertexDeclaration* decl)
{ return CreateVertexDeclaration(format.m_Elements, decl); }
C_VertexDeclaration* C_D3D11Device::CreateVertexDeclaration(const C_VertexElement* elements)
{
    S32 count = 0;
    while (elements[count].GetStream() != UCHAR_MAX && count < C_VertexDeclaration::m_MaxElementCount) ++count;
    if (auto existing = FindVertexDeclaration(count, elements)) return existing;
    auto decl = new C_VertexDeclaration();
    if (!CreateVertexDeclaration(elements, decl)) { delete decl; return nullptr; }
    m_VertexDeclarationList.PushBack(decl);
    return decl;
}
bool C_D3D11Device::CreateVertexDeclaration(const C_VertexElement* elements, C_VertexDeclaration* decl)
{
    if (!elements || !decl || decl->IsCreated()) return false;
    U32 count = 0;
    while (elements[count].GetStream() != UCHAR_MAX && count < C_VertexDeclaration::m_MaxElementCount)
    { decl->m_Elements[count] = elements[count]; ++count; }
    decl->m_ElementCount = count;
    // D3D11 input layouts require the active vertex shader signature and are
    // created when that shader is bound to the declaration.
    return count != 0;
}
void C_D3D11Device::DestroyVertexDeclaration(C_VertexDeclaration* decl)
{
    if (!decl) return;
    if (decl->m_pd3dVertexDeclaration) { decl->m_pd3dVertexDeclaration->Release(); decl->m_pd3dVertexDeclaration = nullptr; }
    decl->m_ElementCount = 0;
}
void C_D3D11Device::DestroyVertexDeclarations()
{
    for (S32 i = 0; i < m_VertexDeclarationList.Size(); ++i)
    { DestroyVertexDeclaration(m_VertexDeclarationList[i]); delete m_VertexDeclarationList[i]; }
    m_VertexDeclarationList.Clear();
}
C_D3D11DeviceManagerParams::C_D3D11DeviceManagerParams() {}
C_D3D11DeviceManagerParams::~C_D3D11DeviceManagerParams() {}
C_D3D11DeviceManager::C_D3D11DeviceManager() : m_pd3dObject(nullptr) {}
C_D3D11DeviceManager::~C_D3D11DeviceManager() {}
bool C_D3D11DeviceManager::IsCreated() const { return m_pd3dObject != nullptr; }
GPUObject* C_D3D11DeviceManager::GetD3DObject() { return m_pd3dObject; }
bool C_D3D11DeviceManager::CreateObject()
{ return SUCCEEDED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&m_pd3dObject))); }
void C_D3D11DeviceManager::DestroyObject()
{ if (m_pd3dObject) { m_pd3dObject->Release(); m_pd3dObject = nullptr; } }
C_DeviceManagerParams::C_DeviceManagerParams() {}
C_DeviceManagerParams::~C_DeviceManagerParams() {}
C_DeviceManager::C_DeviceManager() {}
C_DeviceManager::~C_DeviceManager() {}
void C_DeviceManager::GetParams(C_DeviceManagerParams& params) {}
void C_DeviceManager::SetParams(const C_DeviceManagerParams& params) {}
bool C_DeviceManager::Create(const C_DeviceManagerParams& params)
{ return m_pd3dObject ? true : CreateObject(); }
void C_DeviceManager::Destroy() { DestroyObject(); }
void C_DeviceManager::SetupDefaultDeviceParams(C_DeviceParams& params)
{ params.m_pd3dObject = m_pd3dObject; }
}
