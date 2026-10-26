#ifndef AFTK_UT_BLIND_DATA_HELPER_H
#define AFTK_UT_BLIND_DATA_HELPER_H

namespace AftkUt
{
	class C_BlindDataHelper
	{
	public:
		static void SetBlindDataResourcePtr(
			Aftk::C_Polygon& poly, 
			const Aftk::C_BlindDataResource* pRef);

		static const Aftk::C_BlindDataResource* GetBlindDataResourcePtr(
			const Aftk::C_Polygon& poly);

		static void DeleteBlindDataAttribute(
			Aftk::C_Polygon& poly, 
			const char* attributeName);

		static const Aftk::C_GenericAttribute* GetBlindDataAttribute(
			const Aftk::C_Polygon& poly, 
			const char* attributeName);

		static void SetBlindDataAttribute(
			Aftk::C_Polygon& poly, 
			const Aftk::C_GenericAttribute& attr);
	};
}

#endif
