INLINE void C_PS3GLDevice::SetPixelShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount)
{
	cgGLSetFragmentRegisterBlock(startRegister, registerCount, pData);
}

INLINE void C_PS3GLDevice::SetVertexShaderConstantB(const U32 startRegister, const BOOL* pData, const U32 registerCount)
{
}

INLINE void C_PS3GLDevice::SetVertexShaderConstantI(const U32 startRegister, const S32* pData, const U32 registerCount)
{
}

INLINE void C_PS3GLDevice::SetVertexShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount)
{
	cgGLSetVertexRegisterBlock(startRegister, registerCount, pData);
}

INLINE void C_PS3GLDevice::SetPixelShaderConstantB(const U32 startRegister, const BOOL* pData, const U32 registerCount)
{
}

INLINE void C_PS3GLDevice::SetPixelShaderConstantI(const U32 startRegister, const S32* pData, const U32 registerCount)
{
}
