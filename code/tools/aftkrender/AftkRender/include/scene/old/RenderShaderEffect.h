#ifndef AFTK_RENDER_SHADER_EFFECT_INCLUDE
#define AFTK_RENDER_SHADER_EFFECT_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif

namespace Aftk
{
	class C_Status;
}


namespace AftkRender
{

#ifdef USER_OWENT
#pragma message("FIXME: this needs an overhaul!!!")
#endif

class I_RenderShaderEffect
{
public:
	I_RenderShaderEffect();
	virtual ~I_RenderShaderEffect();

	virtual Aftk::C_Status OnCreate()=0;
	virtual void OnEnable()=0;
	virtual void OnDisable()=0;

	// Sets vs/ps constants common to all & which only need to be set once
	static void SetGlobalConstants();

protected:
	Display::C_VertexShader m_VertexShader;
	Display::C_PixelShader  m_PixelShader;

};



class C_RenderShaderEffect_DiffuseTexturedAlpha : public I_RenderShaderEffect
{
public:
	C_RenderShaderEffect_DiffuseTexturedAlpha();

	~C_RenderShaderEffect_DiffuseTexturedAlpha();

	Aftk::C_Status OnCreate();
	void OnEnable();
	void OnDisable();
};

class C_RenderShaderEffect_DiffuseTexturedAlphaVertexLit : public I_RenderShaderEffect
{
public:
	C_RenderShaderEffect_DiffuseTexturedAlphaVertexLit();

	~C_RenderShaderEffect_DiffuseTexturedAlphaVertexLit();

	Aftk::C_Status OnCreate();
	void OnEnable();
	void OnDisable();
};


class C_RenderShaderEffect_SPTBillboardLeaf : public I_RenderShaderEffect
{
public:
	C_RenderShaderEffect_SPTBillboardLeaf();

	~C_RenderShaderEffect_SPTBillboardLeaf();

	Aftk::C_Status OnCreate();
	void OnEnable();
	void OnDisable();
};

class C_RenderShaderEffect_SPTBillboardLeafVertexLit : public I_RenderShaderEffect
{
public:
	C_RenderShaderEffect_SPTBillboardLeafVertexLit();

	~C_RenderShaderEffect_SPTBillboardLeafVertexLit();

	Aftk::C_Status OnCreate();
	void OnEnable();
	void OnDisable();
};

}
#endif