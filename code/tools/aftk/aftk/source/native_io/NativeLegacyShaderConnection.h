#ifndef NATIVE_LEGACY_SHADER_CONNECTION_H
#define NATIVE_LEGACY_SHADER_CONNECTION_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	struct C_NativeLegacyShaderConnection
	{
		I_Plug* pPlug;
		S32 shaderIndex;
		S32 plugIndex;
		S32 plugChildIndex;
	};
}

#endif
