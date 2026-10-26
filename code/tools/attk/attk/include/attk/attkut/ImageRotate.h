#ifndef ATTK_ROTATE_IMAGE_H
#define ATTK_ROTATE_IMAGE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace AttkUt
{
	class C_ImageRotate
	{
	public:
		static void Rotate90CW(Attk::C_Image &image);

		static void Rotate90CCW(Attk::C_Image &image);
	};
}

#endif // ATTK_ROTATE_IMAGE_H