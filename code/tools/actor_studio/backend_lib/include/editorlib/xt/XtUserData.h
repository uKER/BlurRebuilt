#ifndef EDITOR_LIB_XT_USER_DATA_H
#define EDITOR_LIB_XT_USER_DATA_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_XtNullStorage;

	class C_XtUserData
	{
	public:
		C_XtUserData();
		virtual ~C_XtUserData();
		bool Open(const Btl::String& path);
		Xt::DomComponent* ObtainPackUserData(U32 pack);
		Xt::DomComponent* ObtainRecordUserData(U32 pack, U32 record);
		LarvaSchema* GetSchema();

	private:
		bool				m_IsInitialized;
		Xt::DomDb*			m_pDb;
		C_XtNullStorage*	m_pStorage;
		Xt::DomArray*		m_LevelDataArray;
	};
}

#endif
