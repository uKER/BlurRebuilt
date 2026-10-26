#ifndef RENDERCONTEXT_9DC71DED_3710_4C00_BEFC_1DF9DBBC981D_H
#define RENDERCONTEXT_9DC71DED_3710_4C00_BEFC_1DF9DBBC981D_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/context/I_Anything.h>
#include <misc_shared/uid/FourCC.h>


namespace SceneGraph { class C_SceneGraph; }
namespace GameResource { class C_LevelSceneManager; }

namespace GameContext
{

class I_RenderContext : public Sys::C_Anything<FOURCC_U32('IARC')>
{
public:
	virtual SceneGraph::C_SceneGraph* GetSceneGraph() const = 0;
	virtual GameResource::C_LevelSceneManager* GetLevelSceneManager() const = 0;

protected:
	explicit I_RenderContext(const Sys::C_AnythingUid typeUid) : C_Super(typeUid) {}
};

class C_RenderContext : public Sys::C_Anything<FOURCC_U32('CARC'), I_RenderContext>
{
public:
	explicit C_RenderContext(SceneGraph::C_SceneGraph* pSceneGraph, GameResource::C_LevelSceneManager* pLevelSceneManager) : m_pSceneGraph(pSceneGraph), m_pLevelSceneManager(pLevelSceneManager) {}

	virtual SceneGraph::C_SceneGraph* GetSceneGraph() const {return m_pSceneGraph;} // from I_RenderContext
	virtual GameResource::C_LevelSceneManager* GetLevelSceneManager() const {return m_pLevelSceneManager; }

private:
	SceneGraph::C_SceneGraph* m_pSceneGraph;
	GameResource::C_LevelSceneManager* m_pLevelSceneManager;
};

} // namespace GameContext


#endif // RENDERCONTEXT_9DC71DED_3710_4C00_BEFC_1DF9DBBC981D_H
