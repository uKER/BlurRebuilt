#ifndef MAYA_IO_IRM_POST_PROCESS_H
#define MAYA_IO_IRM_POST_PROCESS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_IrmPostProcess
	{
	public:
		static Aftk::C_Status PostProcessScene(Aftk::C_Scene& scene);
	};
}

#endif
