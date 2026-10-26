#ifndef EDITOR_LIB_DG_NODE_HELPERS_H
#define EDITOR_LIB_DG_NODE_HELPERS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	namespace DgNodeHelpers
	{
		void BuildNodeList(
			NodeIdArray& ids,
			const Btl::String& selection, 
			bool useName,
			bool componentMode);

		void BuildPath(Btl::String& path, const I_DgObject* pDgObject);
	}
}

#endif
