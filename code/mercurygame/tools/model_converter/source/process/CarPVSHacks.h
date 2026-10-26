#ifndef __APP_CAR_PVS_HACKS_H__
#define __APP_CAR_PVS_HACKS_H__

class C_PVSDataEditor
{
public:
	/// constructor
	C_PVSDataEditor(const Aftk::C_Scene *pScene, C_DataPvs *pData);

	/// destructor
	~C_PVSDataEditor();

	/// unify PVS data for all polygons on the object, such that if any polygon is visible, the entire object is visible
	void Merge(const Aftk::C_SceneNode *pNode, int iMinCell = 0, int iMaxCell = 1000000);

	/// force all polygons on the object to be visible for the given range of PVS cells
	void ForceVisible(const Aftk::C_SceneNode *pNode, int iMinCell = 0, int iMaxCell = 1000000);

private:
	const Aftk::C_Scene *m_pScene;
	C_DataPvs *m_pData;
};

// hack the PVS data slightly to account for PVS problems on damaged cars
void ApplyCarPVSHacks(const Aftk::C_Scene *pScene, C_DataPvs *pData);

#endif

