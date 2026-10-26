#ifndef NEXUS_OVERLAY2D_H
#define NEXUS_OVERLAY2D_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Nexus
{
	class C_ViewportDisplay;

	class C_Overlay2D
	{
	public:
		C_Overlay2D(C_ViewportDisplay& viewport);

		bool Draw();

	private:
		void Print(
			S32 fontIndex, 
			S32 x, S32 y, 
			const char* text, 
			const Maths::HVec4& colour = Maths::HVec4(1.0f,1.0f,1.0f,1.0f));

		bool DrawMeshInfo(S32& heightOffset);

		bool DrawAnimationInfo(S32& heightOffset);

		bool DrawCameraInfo(S32& heightOffset);

		C_ViewportDisplay& m_viewport;
	};
}

#endif
