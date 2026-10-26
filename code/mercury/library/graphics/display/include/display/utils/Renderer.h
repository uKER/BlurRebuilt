#ifndef DISPLAY_UTILS_RENDERER_H
#define DISPLAY_UTILS_RENDERER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
	namespace ResourceBundler
	{
		class I_ResourceBundle;
	}

enum E_RendererType
{
	E_RendererType_Unknown = 0,
	E_RendererType_Persistent,
	E_RendererType_Dynamic,
};

enum E_RendererState
{
	E_RendererState_Involatile = 0,
	E_RendererState_Volatile,
	E_RendererState_Count,
};

class I_Renderer
{
public:

	I_Renderer(const Util::C_String& name);
	virtual ~I_Renderer();

	const bool IsRendererCreated() const;
	const bool IsRendererCreated(const E_RendererState state) const;

	const Util::C_String& GetRendererName() const;
	const Util::C_String& GetAssetPath() const;
	ResourceBundler::I_ResourceBundle* GetResourceBundle();

	const E_RendererType GetRendererType() const;
	void SetRendererType(const E_RendererType type);
	void SetAssetPath(const Util::C_String& path);
	void SetResourceBundle(ResourceBundler::I_ResourceBundle* pResourceBundle);

	bool Create();
	bool CreateVolatile();
	void DestroyVolatile();
	void Destroy();

	bool Initialise();
	bool Reinitialise();
	void Shutdown();
	void Reset();

	bool PreResize();
	bool PostResize();

	bool Created(const E_RendererState type) const;
	bool Initialised() const;

	void DestroyRenderer();

protected:

	virtual bool OnCreate();
	virtual bool OnCreateVolatile();
	virtual void OnDestroyVolatile();
	virtual void OnDestroy();

	virtual bool OnInitialise();
	virtual void OnShutdown();
	virtual void OnReset();

	virtual bool OnPreResize();
	virtual bool OnPostResize();

private:
	Util::C_String			m_Name;
	Util::C_String			m_AssetPath;
	E_RendererType			m_Type;
	bool					m_Created[E_RendererState_Count];
	bool					m_Initialised;
	ResourceBundler::I_ResourceBundle*	
							m_pResourceBundle;
};

}; // namespace Display

#include "Display/Utils/Renderer.inl"

#endif // DISPLAY_UTILS_RENDERER_H
