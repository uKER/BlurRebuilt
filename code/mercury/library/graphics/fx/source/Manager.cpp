#include "stdafx.h"

#include <Display.h>

#include <system/Time/Time.h>
#include <system/Serialise.h>
#include <system/DebugLog.h>
#include <system/Debug/Variables.h>

#include "fx/Fx.h"
#include "fx/Effect.h"
#include "fx/Manager.h"
#include "fx/ReloadableEffect.h"
#include "fx/FXB.h"
#include "fx/Shared.h"
#include "fx/Technique.h"

Debug::Bool g_bReloadableShaders("graphics.fx.ReloadableFx", false, Debug::Persist());

namespace FX
{

Debug::Logger &g_LogLoadedShaders = Debug::GetLogger("Graphics.Fx.LoadedShaders");

Manager g_Manager;

int g_iOverrideTechnique = 0;

/// constructor
Manager::Manager()
	: m_pRoot(NULL)
	, m_iReloadCount(0)
	, m_bPreLoaded(false)
	, m_pReloadCheck(NULL)
	, m_pFileSystem(&Sys::C_FileSystem::Instance())
{
}

/// destructor
Manager::~Manager()
{
	ReloadableEffect *p = m_pRoot;

	while (p != NULL)
	{
		ReloadableEffect *pCurrent = p;
		p = p->m_pNext;
		delete pCurrent;
	}
}

void Manager::SetFileSystem(Sys::I_FileSystem* pFs)
{
	BIZ_ASSERT(pFs);
	m_pFileSystem = pFs;
}

Sys::I_FileSystem* Manager::GetFileSystem()
{
	return m_pFileSystem;
}

/// find the given effect in the list of loaded effects
ReloadableEffect *Manager::GetEffect(const char *pszFilename)
{
	for (ReloadableEffect *p = m_pRoot; p; p = p->m_pNext)
	{
		if (!stricmp(p->m_sFilename.Chars(), pszFilename))
			return p;	// found
	}

	return NULL; // not found
}

/// interal function to load an effect from disk
ReloadableEffect *Manager::LoadEffectInternal(const char *pszFilename)
{
	// TODO: might want to get this from a centralised effects manager that supports reloadable effects etc.
	Sys::I_File *pFile = m_pFileSystem->Open(pszFilename);


	if (pFile)
	{
		ReloadableEffect *p = new ReloadableEffect;
		// [Blur1PCDX11] OnEffectLoad runs during ImportFXB and needs the variant
		// filename now, not m_sFilename assigned after the import returns.
		p->NativeFilename = SharedString::GetString(pszFilename);
		bool bSuccess = FX::ImportFXB(pFile, p);
		if (m_bPreLoaded && !g_bReloadableShaders)
			BIZ_ASSERT_FAIL("Shader not pre-loaded: %s", pszFilename);
		LOGDEBUG( g_LogLoadedShaders, "%s\n", pszFilename);
		m_iReloadCount++;
		delete pFile;
		if (bSuccess)
			return p;
		else
		{
			delete p;
			return NULL;
		}
	}
	else
	{
		LOGWARN( g_Log, "Failed to load shader: %s\n", pszFilename);
		return NULL;
	}
}

/// preload a list of effects
void Manager::LoadEffects(const Btl::Vector<const char *> &list, Btl::Vector<FX::I_Effect*> &pointers)
{
	pointers.Resize(list.Size());
#if PLATFORM_PC
	// [Blur2PC] retail (Blur.exe 1.2 FX::Manager::LoadEffects, 0x00682400) clears every entry after the resize;
	// newly loaded effects are not stored below, so without this their entries kept garbage and
	// C_GameApp::ReleaseAllGlobalResources released it on shutdown (crash on Alt-F4 / window close)
	for (int i=0; i<pointers.Size(); i++)
		pointers[i] = NULL;
#endif
	for (int i=0; i<list.Size(); i++)
	{
		const char *pszFilename = list[i];

		// re-use existing
		ReloadableEffect *p = GetEffect(pszFilename);
		if (p)
		{
			p->m_iRefCount++;
			pointers[i] = p;
			continue;
		}

		// load new effect
		p = LoadEffectInternal(pszFilename);
		if (!p)
		{
			pointers[i] = NULL;
			continue;	// failed to load
		}

		// link new effect into linked list
		p->m_pNext = m_pRoot;
		p->m_pPrev = NULL;
		if (m_pRoot)
		{
			assert(!m_pRoot->m_pPrev);
			m_pRoot->m_pPrev = p;
		}
		m_pRoot = p;

		// setup
		Sys::C_FileInfo fileinfo;
		m_pFileSystem->FileInfo(pszFilename, &fileinfo);

		p->m_sFilename = pszFilename;
		p->m_iTime = fileinfo.m_FileTime;
		p->m_iRefCount = 1;
	}

	g_SharedEffect.UpdateSharedBindingBitfields();

	SetPreLoaded(true);
}

/// load an effect
Effect *Manager::LoadEffect(const char *pszFilename)
{
	// re-use existing
	ReloadableEffect *p = GetEffect(pszFilename);
	if (p)
	{
		p->m_iRefCount++;
		return p;
	}

	// load new effect
	p = LoadEffectInternal(pszFilename);
	if (!p)
		return NULL;	// failed to load

	// link new effect into linked list
	p->m_pNext = m_pRoot;
	p->m_pPrev = NULL;
	if (m_pRoot)
	{
		assert(!m_pRoot->m_pPrev);
		m_pRoot->m_pPrev = p;
	}
	m_pRoot = p;

	// setup
	Sys::C_FileInfo fileinfo;
	m_pFileSystem->FileInfo(pszFilename, &fileinfo);

	p->m_sFilename = pszFilename;
	p->m_iTime = fileinfo.m_FileTime;
	p->m_iRefCount = 1;

	g_SharedEffect.UpdateSharedBindingBitfields();

	return p;
}

/// release an effect
void Manager::ReleaseEffect(Effect *_p)
{
	if (!_p)
		return;
	ReloadableEffect *p = (ReloadableEffect*)_p;
	assert(p->m_iRefCount>0);
	if (--p->m_iRefCount > 0)
		return;

	OnEffectRelease(p);

	// unlink effect from linked list
	if (p->m_pPrev)
		p->m_pPrev->m_pNext = p->m_pNext;
	else
		m_pRoot = p->m_pNext;
	if (p->m_pNext)
		p->m_pNext->m_pPrev = p->m_pPrev;

	if (p == m_pReloadCheck)
		m_pReloadCheck = NULL;

	delete p;
}

/// returns the root node of the reloadable effect linked list
ReloadableEffect *Manager::GetRootEffect()
{
	return m_pRoot;
}

/// show all conflicting bindings for the given parameter
void Manager::DumpConflictingBindings( Debug::Logger &log, const char *pszName, int nConflicts)
{
	SharedParameter *pBinding = g_SharedEffect.GetSharedBinding(pszName);
	if (!pBinding)
		return; // not a shared parameter

	LOGDEBUG( log, "Conflicting bindings for parameter %s (%d)", pszName, pBinding->Handle);

	for (ReloadableEffect *p = m_pRoot; p; p = p->m_pNext)
	{
		if (nConflicts<0)
			return;
		g_SharedEffect.DumpConflictingBindings( log, p, pBinding, nConflicts);
	}
}

/// reload any updated effects
void Manager::ReloadEffects(bool bForce)
{
#ifndef CFG_GOLD
	g_iOverrideTechnique = g_TechniqueManager.GetOverrideTechnique();
#endif
	g_SharedEffect.RestoreDefaultData();

#ifndef CFG_GOLD
	if (!g_bReloadableShaders && !bForce)
		return;

	ReloadableEffect *p = bForce?m_pRoot:m_pReloadCheck;
	if (!p)
		p = m_pRoot;

	bool bBlocked = false;
	
	// only check 20 shaders at a time
	for (int iCheck=0; bForce||iCheck<20; iCheck++)
	{
		if (!p)
		{
			m_pReloadCheck = NULL;
			return;	// finished
		}

		// check this shader
		Sys::C_FileInfo fileinfo;
		if (bForce || 
			(m_pFileSystem->FileInfo(p->m_sFilename, &fileinfo) &&
			fileinfo.m_FileTime > p->m_iTime))
		{
			Sys::I_File *pFile = m_pFileSystem->Open(p->m_sFilename);
			if (pFile)
			{
#if defined(PLATFORM_PS3)
				if (!bBlocked)
				{
					Display::Device()->BlockUntilIdle();
					bBlocked = true;
				}
#endif
				// reload this shader
				p->Effect::~Effect();
				new (p) Effect;
				bool bResult = FX::ImportFXB(pFile, p);
				assert(bResult); // this will assert if the shader that was reloaded is for the wrong target platform
				g_SharedEffect.UpdateSharedBindingBitfields();
				m_iReloadCount++;
				delete pFile;
			}
		}
	
		p = p->m_pNext;
	}
	m_pReloadCheck = p;	// next frame, start checking at this shader
#endif
}

}
