#ifndef EDITOR_LIB_EXT_ACTORS_OFFLINE_REFRESH_H
#define EDITOR_LIB_EXT_ACTORS_OFFLINE_REFRESH_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <Btl/Containers/Set.h>
#include "ExtActorsRefresh.h"

namespace EditorLib
{
	class C_ExtActorsOfflineRefreshCmd : public I_Command
	{
	public:
		C_ExtActorsOfflineRefreshCmd(U32 type);
		virtual ~C_ExtActorsOfflineRefreshCmd();

		virtual C_Status Do();

		virtual bool IsUndoable() const;

		static I_Command*		Create(U32 type);
		static C_Syntax*		CreateSyntax();
		static const char*		GetName();

	private:
		struct C_LevelRefInfo
		{
			C_LevelRefInfo()
			{
				pRefBlock = 0;
			}
			C_LevelRefInfo(const C_LevelRefInfo& rhs)
			{
				*this = rhs;
			}
			const C_LevelRefInfo& operator=(const C_LevelRefInfo& rhs)
			{
				refFilePath = rhs.refFilePath;
				pRefBlock = rhs.pRefBlock;
				return *this;
			}

			Btl::String		refFilePath;
			Xt::DomBlock*	pRefBlock;
		};

		struct C_LevelInfo
		{
			C_LevelInfo()
			{
				pRecord = 0;
			}

			C_LevelInfo(const C_LevelInfo& rhs)
			{
				*this = rhs;
			}

			const C_LevelInfo& operator=(const C_LevelInfo& rhs)
			{
				pRecord = rhs.pRecord;
				refInfo = rhs.refInfo;
				return *this;
			}

			Xt::DomRecord*					pRecord;
			Btl::Vector<C_LevelRefInfo>		refInfo;
		};

		void BuildRecordInfo(C_LevelInfo& levelInfo, Xt::DomRecord* pRec);

		void BuildPackInfo(C_ExtActorsPackStats& stats, Btl::Vector<C_LevelInfo>& levelInfoArray, Xt::DomPack* pPack);

		bool ProcessOffline(C_ExtActorsPackStats& stats, const Btl::Vector<C_LevelInfo>& levelInfoArray);

		bool DoDb(C_ExtActorsDbStats& stats);

		bool DoPack(C_ExtActorsPackStats& stats, Xt::DomPack* pPack);

		bool DoRecord(C_ExtActorsRecordStats& stats, Xt::DomRecord* pRecord);

	private:
		bool m_autoflagExtActorRefs;
	};
}

#endif
