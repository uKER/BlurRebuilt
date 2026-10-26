#ifndef ATTK_DEVIL_UTILITY_H
#define ATTK_DEVIL_UTILITY_H

#ifdef _MSC_VER_
#pragma once
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS

namespace Attk
{
	class C_DevILUtility
	{
	public:
		static ILenum GetDevILType(const Attk::C_Image& img);
	
		static ILenum GetDevILFormat(const Attk::C_Image& img);

		///\brief Copies to DevIL - flattens image and changes format if needed.
		///
		static C_Status CopyToDevIL(
			const Attk::C_Image& source,
			DevIL::Image& destination);

		static C_Status CopyToAttk(
			const DevIL::Image& source,
			Attk::C_Image& destination);

		static C_Status Resize(
			Attk::C_Image& image, 
			U32 width, 
			U32 height);
	};
}

#endif

#endif
