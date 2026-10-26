#ifndef AMAX_GRAPHICS_LIGHTING_JOBQUEUEPREPASS_H
#define AMAX_GRAPHICS_LIGHTING_JOBQUEUEPREPASS_H

namespace Scheduler
{
	struct jqModule;
};

namespace Prepass
{
namespace LightingData
{

struct ALIGN16 PointLight
{
	union
	{
		F32 f[12];
		struct
		{
			F32	fX;
			F32	fY;
			F32	fZ;
			F32	fIntensity;			// TODO: [STOVE] Swap intensity and radius? Makes more sense.
			F32	fR;
			F32	fG;
			F32	fB;
			F32	fRadius;
			F32 fInvRadiusSq[4];
		};
	};
};

struct ALIGN128 View
{
	F32	fViewProjMatrix[16];
	F32	fViewMatrix[16];
	F32	fProjMatrix[16];
	F32	fViewProjInverseMatrix[16];
	F32	fViewInverseMatrix[16];
	F32	fProjInverseMatrix[16];
	F32	fNearClip;
	F32	fFarClip;
	F32 fPadding[2];
};

struct ALIGN128 Fog
{
	F32	fDepthFogDensity;
	F32	fDepthFogNearDistance;
	F32	fHeightFogDensity;
	F32	fHeightFogFalloff;
	F32	fHeightFogPlane[4];
};


struct ALIGN16 WorkerInput
{
	S32*	pJobIndex;
	U32		jobCount;
	U32		gateAddress;
	U32		gateValue;
	U32		freeAddress;
	U32		freeValue;
	U32		pointLightsAddress;
	U32		pointLightsCount;
	U32		viewAddress;
	U32		fogAddress;
	U32		textureWidth;
	U32		textureHeight;
	U32		lightingBufferAddress;
	U32		normalDepthBufferAddress;
	U32		normalDepthBufferBankOffset;
};

}; // namespace LightingData
}; // namespace Prepass

extern Scheduler::jqModule JobQueuePrepassMasterModule;
extern Scheduler::jqModule JobQueuePrepassWorkerModule;

#endif // AMAX_GRAPHICS_LIGHTING_JOBQUEUEPREPASS_H
