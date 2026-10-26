#ifndef DEFERRED_PREPASS_SPU_JOBQUEUE_H
#define DEFERRED_PREPASS_SPU_JOBQUEUE_H

namespace Scheduler
{
	struct jqModule;
};

namespace Deferred
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

struct ALIGN128 SpotLight
{
	union
	{
		F32 f[36];					//!< Storage for the floats.
		struct
		{
			F32	fPositionX[4];		//!< X position of the spot light.
			F32	fPositionY[4];		//!< Y position of the spot light.
			F32	fPositionZ[4];		//!< Z position of the spot light.
			F32	fDirectionX[4];		//!< X direction of the spot light.
			F32	fDirectionY[4];		//!< Y direction of the spot light.
			F32	fDirectionZ[4];		//!< Z direction of the spot light.
			F32	fFalloffX[4];		//!< X falloff of the spot light.
			F32	fFalloffY[4];		//!< Y falloff of the spot light.
			F32	fR;					//!< Red channel.
			F32	fG;					//!< Green channel.
			F32	fB;					//!< Blue channel.
			F32	fRadius;			//!< Radius.
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

struct ALIGN16 MasterInput
{
	U32		fenceAddress;
	U32		fenceValue;
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
	U32		spotLightsAddress;
	U32		spotLightsCount;
	U32		viewAddress;
	U32		textureWidth;
	U32		textureHeight;
	U32		depthBufferAddress;
	U32		normalBufferAddress;
	U32		lightingBufferAddress;
	U32		depthBufferBankOffset;
	U32		normalBufferBankOffset;
};

}; // namespace LightingData
}; // namespace Deferred

extern Scheduler::jqModule LightingPrepassMasterModule;
extern Scheduler::jqModule LightingPrepassWorkerModule;

#endif // DEFERRED_PREPASS_SPU_JOBQUEUE_H
