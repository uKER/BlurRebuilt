#ifndef DISPLAY_D3D11_OCCLUSION_QUERY_H
#define DISPLAY_D3D11_OCCLUSION_QUERY_H

// [Blur1PCDX11] Native D3D11 pixel-count occlusion query.
#include <d3d11.h>
#include "Display/Occlusion/OcclusionQueryTypes.h"
namespace Display { namespace D3D11 {
class C_OcclusionQuery
{
public:
    C_OcclusionQuery();
    ~C_OcclusionQuery();
    bool IsCreated() const { return m_Query != nullptr; }
    bool Create();
    void Destroy();
    void Begin();
    void End();
    E_OcclusionQueryResult Visible() const;
    E_OcclusionQueryResult Visible(U32& count) const;
    E_OcclusionQueryResult Visible(F32& fraction) const;
private:
    ID3D11Query* m_Query;
    bool m_Issued; // [Blur1PCDX11] End() has been called since the last Begin()
};
} }
#endif
