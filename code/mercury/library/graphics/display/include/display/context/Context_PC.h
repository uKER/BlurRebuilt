#ifndef __DISPLAY_CONTEXT_PC_H__
#define __DISPLAY_CONTEXT_PC_H__

#include "Display/Context/Context_D3D11.h"

namespace Display
{

DECLARE_CONTEXT(Context, C_Context);
class C_CommandBuffer; // [Blur1PCDX11] PC command buffers are not allocated.

class C_Context : public C_ContextD3D11
{
public:
	
	C_Context();

	I_Context* New() { return new C_Context; }

	void Initialise();
	
	void CloneState(const I_Context* pContext);
	void InvalidateState();

	void BeginCommandBuffer(Display::C_CommandBuffer* pCommandBuffer);
	void EndCommandBuffer();

	const C_View* GetView() const;
};

} // namespace Display

#endif // __DISPLAY_CONTEXT_PC_H__
