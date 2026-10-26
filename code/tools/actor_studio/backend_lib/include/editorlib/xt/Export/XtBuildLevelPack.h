#ifndef EDITOR_LIB_XT_BUILD_LEVEL_PACK_H
#define EDITOR_LIB_XT_BUILD_LEVEL_PACK_H

#pragma once

class AsyncOp;

namespace EditorLib
{
	struct C_PackExportParams
	{
		XtId packId;
	};

	namespace XtBuildLevelPack
	{
		Btl::String BuildTempFolder(XtId packId);

		bool Build(HWND hwnd, C_PackExportParams* pExpParams);
	}
}

#endif
