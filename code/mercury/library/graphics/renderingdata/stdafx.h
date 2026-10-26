#include <Core/Core.h>
#include "core/maths.h"

#include "btl/utility/autopointer.h"
#include "btl/containers/vector.h"
#include "btl/containers/bitvector.h"
#include "btl/utility/string.h"
#include "btl/utility/scopedpointer.h"
#include "btl/utility/pair.h"
#include "btl/utility/hash.h"
#include "btl/allocators/stackallocator.h"

#include "System/DebugLog.h"
#include "System/Debug/Debug.h"

#if !PLATFORM_SPU

#include "misc_shared/archive/archive.h"
#include "system/serialise/i_file.h"
#include <Display/Core/Includes.h>

#else

#define FRIEND_SERIALISE(x)
#define VIRTUAL_SERIALISATION_KEY_DECL

#if 1
#include <Display.h>
#else
namespace Display
{
	struct C_IndexBuffer {};
	struct C_VertexBuffer {};
	struct C_Texture2D 
	{
		const void*& GetGPUTexture() const;
	};
	struct C_VertexDeclaration {};
	struct C_CommandBuffer {};
	struct C_PixelShaderParameter {};
	struct C_VertexShader {};
	struct C_PixelShader {};
	struct C_StencilState {};
	struct C_View {};
	struct C_Device 
	{
		C_View* GetView();
	};

	typedef int E_ColourWriteEnable;

	C_Device* Device();
}

#include <display/device/DeviceTypesEnums.h>
#endif

#endif
