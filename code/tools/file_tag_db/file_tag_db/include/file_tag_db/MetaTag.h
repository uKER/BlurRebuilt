#ifndef FILE_TAG_METATAG_H
#define FILE_TAG_METATAG_H

#pragma once

#include <System/Serialise/FilePath.h>

namespace FileTag
{
	class C_MetaTag
	{
	public:
		struct C_Input
		{
			Sys::C_FilePath	file;
			S64				fileTime;
		};

		C_MetaTag();

		// Returns the built time of the current tag.
		S64					GetBuildTime() const;

		bool				GetInputs(Ut::Vector<C_Input>& inputs) const;

		const C_Tag*		GetTagPtr() const;
		
		C_Tag*				GetTagPtr();

		// Set the build time to be written to file.
		void				SetBuildTime(S64 buildTime);

		void				SetInputs(const Ut::Vector<C_Input>& inputs);

	private:
		C_MetaTag(C_Tag* pTag);

		C_MetaTag(const C_MetaTag& rhs);

		const C_MetaTag& operator=(const C_MetaTag& rhs);

		C_Tag* m_pTag;

		friend class C_MetaManager;
	};
}

#endif

