#ifndef EDITOR_I_DELETE_CONTEXT_H
#define EDITOR_I_DELETE_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace EditorLib
{
	class I_DeleteContext
	{
	public:
		I_DeleteContext() {}
		virtual ~I_DeleteContext() {}

		virtual bool Delete(I_DgNode* node, C_Selection& selection) = 0;
	};
}

#endif
