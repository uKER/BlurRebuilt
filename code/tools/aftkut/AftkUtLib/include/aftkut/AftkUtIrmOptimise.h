#ifndef AFTK_UT_IRM_OPTMIMISE_H
#define AFTK_UT_IRM_OPTMIMISE_H

namespace AftkUt
{
	class C_IrmOptimise
	{
	public:
		static void WriteOriginDebug(const Aftk::C_Scene* pScene, const char* outputFilePath = 0);

		static Aftk::C_Status RemoveDuplicates(Aftk::C_Scene* pScene);
	};
}

#endif
