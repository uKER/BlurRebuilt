#include "stdafx.h"
#include <Python.h>

#include <EditorLib/Command/CommandProcessor.h>

#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Python/XtFuncs.h>

#include <EditorLib/Xt/Export/XtBuildLevelPack.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtObject.h>

#include <EditorLib/CoreConfig.h>
#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

#include <svn_wrapper.h>

namespace EditorLib
{
namespace XtFuncs
{
PyObject* GetXtPackId(PyObject* pSelf, PyObject* pArgs)
{
	char* path = 0;
	if (PyArg_ParseTuple(pArgs, "s", &path) && path)
	{
		Xt::DomDb *pDomDb = C_Core::Instance().GetXtManager().GetDb();
		for(Xt::DomDb::Iterator itPack(pDomDb); itPack; ++itPack)
		{	
			if (stricmp(itPack->filename.Text(), path)==0)
				return Py_BuildValue("i", itPack->id);			
		}
	}

	LOGERROR(C_Core::Instance().GetLogger(), "Invalid pack path %s", path);
	return Py_BuildValue("i", 0);
}

PyObject* GetXtPackList(PyObject* pSelf, PyObject* pArgs)
{
	Ut::UtVector< Ut::DynamicVector<XtId> > packIds;

	Xt::DomDb *pDomDb = C_Core::Instance().GetXtManager().GetDb();
	for(Xt::DomDb::Iterator itPack(pDomDb); itPack; ++itPack)
		packIds.PushBack(itPack->id);

	if (packIds.Size())
	{
		PyObject *pPyList = PyList_New(packIds.Size());
		for (S32 i=0; i<packIds.Size(); ++i)
			PyList_SetItem(pPyList, i, Py_BuildValue("i", packIds[i]));

		return pPyList;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* GetXtPackRecordList(PyObject* pSelf, PyObject* pArgs)
{
	XtId packId = 0;
	char* type = 0;

	if (PyArg_ParseTuple(pArgs, "is", &packId, &type) && packId && type)
	{
		Xt::DomPack *pPack = C_Core::Instance().GetXtManager().FindPack(packId);
		if (pPack)
		{
			Ut::UtVector< Ut::DynamicVector<XtId> > recordIds;
			for (Xt::DomPack::Iterator itRec(pPack); itRec; ++itRec)
			{
				Xt::DomRecord *pRecord = itRec;
				if (strcmp(pRecord->root.type.Text(), type)==0)
					recordIds.PushBack(pRecord->id);
			}

			if (recordIds.Size())
			{
				PyObject *pPyList = PyList_New(recordIds.Size());
				for (S32 i=0; i<recordIds.Size(); ++i)
					PyList_SetItem(pPyList, i, Py_BuildValue("i", recordIds[i]));

				return pPyList;
			}
		}
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* BuildPack(PyObject* pSelf, PyObject* pArgs)
{
	XtId packId = 0;
	if (PyArg_ParseTuple(pArgs, "i", &packId) && packId)
	{
		C_PackExportParams params;
		params.packId = packId;

		if (XtBuildLevelPack::Build((HWND)C_Core::Instance().GetWindowHandle(), &params))
			return Py_BuildValue("i", 1);
		else
			return Py_BuildValue("i", 0);
	}
	
	return Py_BuildValue("i", 0);
}


class NonInteractiveSvnXtContext : public Vcs::VcsContext
{
public:
	NonInteractiveSvnXtContext()
	{
	}

	NonInteractiveSvnXtContext(const Btl::String user, const Btl::String& password)
	: m_user(user)
	, m_password(password)
	{
	}

	virtual bool GetLogin(Btl::String& username, Btl::String& password, const Btl::String& realm, bool& maySave)
	{
		username = m_user;
		password = m_password;
		maySave = false;

		return true;
	}

private:
	Btl::String m_user;
	Btl::String m_password;
};


static void SvnProgressCallback(const Vcs::Notify& info, void* baton)
{
	if (info.error.code == Vcs::Status::kSuccess)
		LOGINFO(C_Core::Instance().GetLogger(), "Success: %s", info.path);
	else
		LOGERROR(C_Core::Instance().GetLogger(), "Failed: %s", info.path);
}

struct SvnUpdateContext
{
	SvnUpdateContext()
	: m_hasConflicts(false)
	{
	}

	bool m_hasConflicts;
};

static void SvnUpdateInfoCallback(const Vcs::StatusEntry& info, void* baton)
{
	if (info.textStatus == Vcs::kStatus_Conflicted)
	{
		SvnUpdateContext *pUpdateContext = static_cast<SvnUpdateContext*>(baton);
		pUpdateContext->m_hasConflicts = true;
		LOGINFO(C_Core::Instance().GetLogger(), "Conflict: %s", info.path);
	}
}


PyObject* VersionControlUpdate(PyObject* pSelf, PyObject* pArgs)
{
	XtId currentRecord = C_Core::Instance().GetLevelManager()->GetActiveLevel() ?
		C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtRecordId() : 0;

	if (C_Core::Instance().GetXtManager().GetDb()->open && 
		!C_Core::Instance().GetCommandProcessor().Execute("xtdb", Btl::String("-s")))
	{
		LOGERROR(C_Core::Instance().GetLogger(), "Failed to save database prior to updating");
		return Py_BuildValue("i", 0);
	}
	
	if (currentRecord)
		C_Core::Instance().GetLevelManager()->Close();

	if (C_Core::Instance().GetXtManager().GetDb()->open &&
		!C_Core::Instance().GetCommandProcessor().Execute("xtdb", Btl::String("-c")))
	{
		LOGERROR(C_Core::Instance().GetLogger(), "Failed to close database prior to updating");
		return Py_BuildValue("i", 0);
	}

	LOGINFO(C_Core::Instance().GetLogger(), "Updating...");

	//assume anonymous svn updates are permitted
	Vcs::Svn svn(new NonInteractiveSvnXtContext());
	if (svn)
	{
		SvnUpdateContext updateContext;

		const C_CoreConfig& coreConfig = C_Core::Instance().GetConfig();
		bool result = svn.Update(coreConfig.xtDbPath.Chars(), Vcs::kDepth_Infinity, Vcs::kRevision_Head, SvnProgressCallback);
		if (result)
			result = svn.Info(coreConfig.xtDbPath.Chars(), Vcs::kDepth_Infinity, SvnUpdateInfoCallback, &updateContext);

		if (updateContext.m_hasConflicts)
		{
			//TODO: resolve conflicts
			//There doesn't seem to be a non-interactive way of calling SorceryResolveConflics
		}

		if (result)
			LOGINFO(C_Core::Instance().GetLogger(), "Update Complete");
		else
			LOGINFO(C_Core::Instance().GetLogger(), "Update Failed");
	}
	else
	{
		LOGERROR(C_Core::Instance().GetLogger(), "Update Failed: Failed to intialise svn context");
	}

	//reload database
	{
		const C_CoreConfig& coreConfig = C_Core::Instance().GetConfig();
		if(!C_Core::Instance().GetCommandProcessor().Execute("xtdb", Btl::String("-o ") + coreConfig.xtDbPath))
			LOGERROR(C_Core::Instance().GetLogger(), "Unable to reload the database after update");
			return Py_BuildValue("i", 0);
	}

	//reload level
	if (currentRecord && !EditorLib::C_Core::Instance().GetLevelManager()->Open(currentRecord))
	{
		LOGERROR(C_Core::Instance().GetLogger(), "Failed to reload level after update");
		return Py_BuildValue("i", 0);
	}
	
	return Py_BuildValue("i", 1);
}

PyObject* VersionControlCommit(PyObject* pSelf, PyObject* pArgs)
{
	EditorLib::C_XtManager& xtManager = EditorLib::C_Core::Instance().GetXtManager();
	
	if (!xtManager.Save())
	{
		LOGERROR(C_Core::Instance().GetLogger(), "Failed to save database prior to committing");
		return Py_BuildValue("i", 0);
	}

	char *pUser=0, *pPassword=0, *pComment=0;
	PyArg_ParseTuple(pArgs, "sss", &pUser, &pPassword, &pComment);

	LOGINFO(C_Core::Instance().GetLogger(), "Committing...");

	Vcs::Svn svn(new NonInteractiveSvnXtContext());
	bool result = false;
	if (svn)
	{
		const C_CoreConfig& coreConfig = C_Core::Instance().GetConfig();
		result = svn.Commit(coreConfig.xtDbPath.Chars(), pComment, Vcs::kDepth_Infinity, SvnProgressCallback);
	}

	if (result)
	{
		LOGINFO(C_Core::Instance().GetLogger(), "Commit Complete");
		return Py_BuildValue("i", 1);
	}
	else
	{
		LOGERROR(C_Core::Instance().GetLogger(), "Commit Failed");
		return Py_BuildValue("i", 0);
	}
}

static PyMethodDef g_xtHelpersDef[] = 
{
	{"GetXtPackId", GetXtPackId, METH_VARARGS, "Returns xtid of pack at path"},
	{"GetPackList", GetXtPackList, METH_NOARGS, "Returns list of pack ids"},
	{"GetXtPackRecordList", GetXtPackRecordList, METH_VARARGS, "Returns list of record ids of the given type in the specified pack"},
	{"BuildPack", BuildPack, METH_VARARGS, "Builds pack using the specified xtid"},
	{"VersionControlUpdate", VersionControlUpdate, METH_NOARGS, "Update the database from version control"},
	{"VersionControlCommit", VersionControlCommit, METH_VARARGS, "Commit database changes to version control"},
	{NULL, NULL, 0, NULL}        //Sentinel
};

PyMethodDef* GetXtHelpersDef()
{
	return g_xtHelpersDef;
}

}

}
