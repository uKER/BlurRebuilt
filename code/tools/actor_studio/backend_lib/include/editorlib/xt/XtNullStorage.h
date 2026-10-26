#ifndef EDITOR_LIB_XT_NULL_STORAGE_H
#define EDITOR_LIB_XT_NULL_STORAGE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_XtNullStorage : public Xt::Storage
	{
	public:
		virtual bool Start() { return true; }
		virtual bool New(const char* filename) { return true; }
		virtual bool Dirty(const char* filename) { return true; }
		virtual bool Rename(const char* oldFilename, const char* newFilename) { return true; }
		virtual bool Remove(const char* filename) { return true; }
		virtual bool IsModified(const char* filename) { return true; }
		virtual bool Stop() { return true; }
	};
}

#endif
