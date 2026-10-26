#ifndef ATTKUT_IMAGE_ADJUSTMENT_H
#define ATTKUT_IMAGE_ADJUSTMENT_H

namespace AttkUt
{
	class C_ImageAdjustment
	{
	public:
		enum E_AdjustmentChannel
		{
			E_AdjustmentChannel_Red			= 0x00000001,
			E_AdjustmentChannel_Green		= 0x00000002,
			E_AdjustmentChannel_Blue		= 0x00000004,
			E_AdjustmentChannel_Alpha		= 0x00000008,
			E_AdjustmentChannel_Luminance	= 0x00000010,

			E_AdjustmentChannel_All		= 0xffffffff
		};

		static Attk::C_Status Adjust(
			Attk::C_Image& image,
			const C_ImageAdjustmentCurve<Point2>& curve,
			E_AdjustmentChannel channel = E_AdjustmentChannel_All);

		static Attk::C_Status Adjust(
			Attk::C_MipmapImageChain& mipmapChain,
			const C_ImageAdjustmentCurve<Point2>& curve,
			E_AdjustmentChannel channel = E_AdjustmentChannel_All);

#ifdef _DEBUG
		static Attk::C_Status OutputDebugImage(
			const char* pFileName,
			const C_ImageAdjustmentCurve<Point2>& curve);
#endif

	private:
		static Attk::C_Status AdjustSingleImage(
			Attk::C_Image& image,
			const C_EvaluateImageAdjustmentCurve<Point2> curveEvaluater,
			E_AdjustmentChannel channel = E_AdjustmentChannel_All);
	};
}

#endif
