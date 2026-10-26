#ifndef FBX_IO_IMPORT_OPTIONS_H
#define FBX_IO_IMPORT_OPTIONS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace FbxIo
{
	class C_ImportOptions
	{
	public:
		C_ImportOptions()
		{
			Clear();
		}


		void Clear()
		{
			takeName = "default";
			takeIndex = 0;
			password = "";
			targetFPS = 0; //use source FPS

		}
		
		std::string takeName;
		U32 takeIndex;
		std::string password;
		U32 targetFPS;
	};
}

#endif
