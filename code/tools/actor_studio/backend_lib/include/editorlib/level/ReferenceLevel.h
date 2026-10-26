#ifndef EDITOR_LIB_REFERENCE_LEVEL_H
#define EDITOR_LIB_REFERENCE_LEVEL_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <sigslot/sigslot.h>

#include <EditorLib/Level/LevelBase.h>

namespace EditorLib
{

class C_AftkRef;

class C_ReferenceLevel : public C_LevelBase
{
public:
	C_ReferenceLevel();
	virtual ~C_ReferenceLevel();

	virtual E_LevelType GetType() const;

	void OnDatabaseEvent(Xt::DomDb& db, E_DatabaseEvent e);

protected:
	class C_Slots : public sigslot::has_slots<>
	{
	public:
		void Connect();
		void Disconnect();

		void OnDatabaseEventSlot(Xt::DomDb& db, E_DatabaseEvent e);
	};

	C_Slots m_Slots;

	FRIEND_CLASS		I_DgNode;
};

}

#endif
