#ifndef GAMESYS_DEBUGRESOURCERENDER_H
#define GAMESYS_DEBUGRESOURCERENDER_H

#ifdef _MSC_VER
#pragma once
#endif


namespace GameResource
{
	class C_LevelSceneManager;
}

namespace Input
{
	class C_GameInput;
}


namespace GameDebug
{

class C_DebugResourceRenderTree;

class C_DebugResourceRender
{
public:
	C_DebugResourceRender();
	~C_DebugResourceRender();

	void Init(GameResource::C_LevelSceneManager* pLevelSceneManager, Input::C_GameInput* pGameInput);
	void Update(const Maths::HMat43& mCameraToWorld);
	void Render2d() const;
	void Close();

private:
	GameResource::C_LevelSceneManager*	m_pLevelSceneManager; ///< weak ref
	Input::C_GameInput*					m_pGameInput; ///< weak ref
	C_DebugResourceRenderTree*			m_pTree; ///< strong ref

	C_DebugResourceRender(const C_DebugResourceRender& copy); // can not copy
	C_DebugResourceRender& operator = (const C_DebugResourceRender& copy); // can not copy

};

} // namespace GameDebug


#endif // GAMESYS_DEBUGRESOURCERENDER_H
