#ifndef EDITORLIB_LEVEL_MANAGER_H
#define EDITORLIB_LEVEL_MANAGER_H

#include <sigslot/sigslot.h>

#include <EditorLib/Misc/Status.h>

namespace EditorLib
{

class I_Level;
class C_XtLevel;
class C_ReferenceLevel;

class C_LevelManager
{
public:
	C_LevelManager();
	~C_LevelManager();

	C_ReferenceLevel* GetReferenceLevel();
	C_XtLevel* GetActiveLevel();
	void SetActiveLevel(C_XtLevel* level);

	C_Status Open(XtId id);
	C_Status Close();

	C_Status OpenPack(XtId packId);
	C_Status ClosePack(XtId packId);
	C_LevelPack* GetPack(XtId packId);

	C_XtLevel* GetLevel(XtId recordId);

	Btl::Vector<C_LevelPack*> GetPacks();

private:
	C_Status OpenLevel(XtId packId);

	void OnLevelOpened(const I_Level& level);
	void OnLevelClosing(const I_Level& level);
	
protected:
	class C_Slots : public sigslot::has_slots<>
	{
	public:
		void Connect();
		void Disconnect();

		void OnLevelOpenedSlot(const I_Level& level);
		void OnLevelClosingSlot(const I_Level& level);
	};

private:
	Btl::OpenHashMap<XtId, C_LevelPack*> m_Packs;
	C_XtLevel* m_pActiveLevel;

	C_ReferenceLevel* m_pReferenceLevel;

	C_Slots m_Slots;
};

}

#endif