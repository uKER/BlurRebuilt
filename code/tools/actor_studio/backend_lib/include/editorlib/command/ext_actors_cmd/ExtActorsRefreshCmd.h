#ifndef EDITOR_LIB_EXT_ACTORS_REFRESH_CMD_H
#define EDITOR_LIB_EXT_ACTORS_REFRESH_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

#include "ExtActorsRefresh.h"

namespace EditorLib
{
	class C_ExtActorsRefreshCmd : public I_Command
	{
	public:
		C_ExtActorsRefreshCmd(U32 type);
		virtual ~C_ExtActorsRefreshCmd();

		virtual C_Status Do();

		virtual bool IsUndoable() const;

		static I_Command*		Create(U32 type);
		static C_Syntax*		CreateSyntax();
		static const char*		GetName();

	private:
		C_Status	ProcessOnline(
			C_ExtActorsRecordStats& stats,
			const C_AftkRef* pRef);

		bool BuildOnlineRefInfo(
			Btl::Vector<C_AftkActorPlaceholder>& aftkActorPlaceholders,
			const C_AftkRef* pRef) const;

		bool DoRecord(C_ExtActorsRecordStats& stats, Xt::DomRecord* pRecord);
	};
}

#endif
