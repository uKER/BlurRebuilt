#ifndef AFTK_RENDER_SHADER_INCLUDE
#define AFTK_RENDER_SHADER_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif

namespace Aftk
{
	class C_Status;
}


namespace AftkRender
{

class I_RenderShader
{
public:
	I_RenderShader();
	virtual ~I_RenderShader();

	virtual Aftk::C_Status OnCreate()=0;
	virtual void OnEnable()=0;
	virtual void OnDisable()=0;

	// Sets vs/ps constants common to all & which only need to be set once
	static void SetGlobalConstants();

protected:
	NewDisplay::C_VertexShader m_VertexShader;
	NewDisplay::C_PixelShader  m_PixelShader;

};



class C_RenderShader_DiffuseTexturedAlpha : public I_RenderShader
{
public:
	C_RenderShader_DiffuseTexturedAlpha();

	~C_RenderShader_DiffuseTexturedAlpha();

	Aftk::C_Status OnCreate();
	void OnEnable();
	void OnDisable();

	void SetTexture(NewDisplay::C_Texture2D* pTexture);

private:
	NewDisplay::C_Texture2D* m_pTexture;
};




class C_RenderShader_Billboard : public I_RenderShader
{
public:
	C_RenderShader_Billboard();

	~C_RenderShader_Billboard();

	Aftk::C_Status OnCreate();
	void OnEnable();
	void OnDisable();

	void SetTexture(NewDisplay::C_Texture2D* pTexture);

private:
	NewDisplay::C_Texture2D* m_pTexture;
};



}
#endif