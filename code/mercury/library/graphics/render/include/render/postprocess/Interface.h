#ifndef RENDER_POSTPROCESS_H
#define RENDER_POSTPROCESS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace PostProcess
{

static const S32 MAX_RENDER_TEXTURES = 16;

typedef Ut::UtStackVector<const Display::C_RenderTexture2D*, MAX_RENDER_TEXTURES>		RenderTexturesConst;
typedef Ut::UtStackVector<Display::C_RenderTexture2D*, MAX_RENDER_TEXTURES>				RenderTextures;

enum E_Type
{
	E_Type_Intermediate,
	E_Type_Final
};

enum E_Chainable
{
	E_Chainable_True,
	E_Chainable_False
};

enum E_Resolve
{
	E_Resolve_True,
	E_Resolve_False
};

class C_Manager;

class I_PostProcess
{
public:

	I_PostProcess(C_Manager& pManager);
	virtual ~I_PostProcess();

	void SetType(const E_Type eType);
	void SetResolve(const E_Resolve eResolve);
	void SetChainable(const E_Chainable eChainable);
	void SetTextureFilter(Display::E_TextureFilter textureFilter);

	void SetShader(const Display::C_PixelShader* pShader) const;

	C_Manager&	 Manager();

	virtual bool Create() = 0;
	virtual void Destroy() = 0;
	virtual bool CreateVolatile() = 0;
	virtual void DestroyVolatile() = 0;

	virtual void Update() = 0;

protected:

	void Release();
		
	void SetViewport() const;
	void Resolve() const;
	void DisplayIntermediate(const Display::C_PixelShader* pPixelShader = NULL, Display::E_TextureFilter textureFilter = Display::E_TextureFilter_Unknown) const;
	void DisplayFinal(const Display::C_PixelShader* pPixelShader = NULL, Display::E_TextureFilter textureFilter = Display::E_TextureFilter_Unknown) const;

	mutable const Display::C_PixelShader*	m_pShader;
	mutable Display::E_TextureFilter		m_TextureFilter;
	C_Manager&								m_Manager;
									
	E_Resolve								m_eResolve;
	E_Type									m_eType;
	E_Chainable								m_eChainable;
	bool									m_bDepthEnable;

};

INLINE C_Manager& I_PostProcess::Manager()
{
	return m_Manager; 
}

INLINE void I_PostProcess::SetType(const E_Type eType)
{
	m_eType = eType;
}

INLINE void I_PostProcess::SetChainable(const E_Chainable eChainable)
{
	m_eChainable = eChainable;
}

INLINE void I_PostProcess::SetResolve(const E_Resolve eResolve)
{
	m_eResolve = eResolve;
}

INLINE void I_PostProcess::SetShader(const Display::C_PixelShader* pShader) const
{
	m_pShader = pShader;
}

INLINE void I_PostProcess::SetTextureFilter(Display::E_TextureFilter textureFilter)
{
	m_TextureFilter = textureFilter;
}

} // namespace PostProcess

#endif // RENDER_POSTPROCESS_H
