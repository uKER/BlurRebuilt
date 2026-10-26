#ifndef EDITORLIB_LEVEL_PACK_H
#define EDITORLIB_LEVEL_PACK_H

namespace EditorLib
{

class I_Level;

class C_LevelPack
{
public:
	C_LevelPack(XtId packId);

	C_XtLevel* NewLevel();
	C_Status LoadLevel(XtId recordId);
	C_XtLevel* GetLevel(XtId recordId);

	XtId GetId() const;
	Btl::String GetName() const;
	Btl::Vector<C_XtLevel*> GetLevels() const;

private:
	C_Status Open();
	C_Status Close();

	Btl::OpenHashMap<XtId, C_XtLevel*> m_Levels;

	XtId m_PackId;

	FRIEND_CLASS C_LevelManager;
};

}

#endif