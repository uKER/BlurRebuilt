#ifndef EDITOR_LIB_XT_MANAGER_H
#define EDITOR_LIB_XT_MANAGER_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <Btl/Containers/OpenHashSet.h>
#include <Btl/Utility/String.h>

#include <EditorLib/Misc/Status.h>

namespace EditorLib
{
	class C_XtManager
	{
	public:
		C_XtManager();
		~C_XtManager();

		C_Status Close();

		C_Status Open(const Btl::String& dbPath, bool ensureUnique);

		C_Status Save();

		void Revert();

//		C_Status Export();

		Xt::DomDb* GetDb();

		const Xt::DomDb* GetDb() const;

		XtId CreateRecord(XtId packId, const Btl::String& typeName);

		bool DeleteRecord(XtId id);

		void RevertRecord(XtId id);

		Xt::DomPack* FindPack(const Btl::String& name);

		const Xt::DomPack* FindPack(const Btl::String& name) const;

		Xt::DomPack* FindPack(XtId id);

		const Xt::DomPack* FindPack(XtId id) const;

		Xt::DomRecord* FindRecord(XtId id);

		const Xt::DomRecord* FindRecord(XtId id) const;

		Xt::DomElement* FindElementByPath(
			LtStruct* pRecordType,
			Xt::DomElement* element,
			const char* path);

		void EnsureUniqueUids(XtId recordId);

		void EnsureUniqueUids(bool onlyIfDirty);

		void ClearPacksPendingDeployment();

		void GetPacksPendingDeployment(Btl::Vector<Xt::DomPack*>& packs);

	private:
		static void StaticOnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent, 
			C_XtManager* pThis);

		Xt::DomDb*					m_pDb;
		Btl::String					m_dbPath;
		Xt::DomMonitor				m_dbMonitor;
		Btl::OpenHashSet<XtId>		m_packsPendingDeployment;
	};
}

#endif
