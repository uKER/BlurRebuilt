#ifndef APP_AFTK_SCENE_DATA_H
#define APP_AFTK_SCENE_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

class C_DataAftkScene : public Tools::ProcessGraph::C_Data
{
public:
	C_DataAftkScene();
	~C_DataAftkScene();

	// inherited from C_Data
	const Util::C_String GetName() const;

	const Aftk::C_Scene& GetAftkScene() const;

	Aftk::C_Scene& GetAftkScene();

	void FreeData();

private:
	Aftk::C_Scene* m_pAftkScene;
};

#endif