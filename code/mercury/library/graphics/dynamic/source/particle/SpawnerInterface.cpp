#include "stdafx.h"

#include "Dynamic/Particle/SpawnerInterface.h"
#include "Dynamic/Particle/EffectInterface.h"
#include "Dynamic/Particle/UserParticleLibrary.h"

namespace DynamicEffect
{
namespace Particle
{

void I_Spawner::CreateParticle(U8* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings)
{
/*	const C_ParticleFormat* pParticleFormat = pSpawnSettings->GetParticleFormat();

	for(S32 j = 0; j < pParticleFormat->GetNumElements(); ++j)
		CreateElement(pParticle, random, pEmitter, pSpawnSettings, pParticleFormat->GetElement(j));*/
}
	
void I_Spawner::CreateParticles(U8* pParticles, S32 num, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings)
{
/*	const C_ParticleFormat* pParticleFormat = pSpawnSettings->GetParticleFormat();
	const U32 elementSize = pParticleFormat->GetElementSize();

	for(S32 i = 0; i < num; ++i)
	{
		U8* pParticle = &pParticles[i * elementSize];

		for(S32 j = 0; j < pParticleFormat->GetNumElements(); ++j)
			CreateElement(pParticle, random, pEmitter, pSpawnSettings, pParticleFormat->GetElement(j));
	}*/
}

void I_Spawner::CreateElement(U8* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, const ParticleElement& element)
{
/*	static bool doMeshVariant = true;
	switch(element.type)
	{
		case E_ParticleElement_Position3:
			{
				MathsStorage::Float3* pPosition = (MathsStorage::Float3*)&pParticle[element.offset];
			#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
				XMVECTOR position = pEmitter->GetSpawnPosition(random);
				XMStoreFloat3A16((XMFLOAT3A16*)pPosition, position);
			#endif
			}
			break;
		
		case E_ParticleElement_Life:
			{
				F32* pLife = (F32*)&pParticle[element.offset];
				*pLife = 0.0f;
			}
			break;
		
		case E_ParticleElement_Velocity3:
			{
				MathsStorage::Float3* pVelocity = (MathsStorage::Float3*)&pParticle[element.offset];
				
			#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
				XMVECTOR velocity = pEmitter->GetSpawnVelocity(random);
				XMStoreFloat3A16((XMFLOAT3A16*)pVelocity, velocity);
			#endif
			}
			break;
		
		case E_ParticleElement_Variant:
			{
				U8* pVarient = (U8*)&pParticle[element.offset];

				if (pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape == E_ParticleShape_Mesh && doMeshVariant)
				{
					const C_UserShape3DVariants& variants = C_UserParticleLibrary::Instance().GetShape3DVariants(pEffect->GetRenderSettings().vertexBuildSettings.iParticleLibraryIndex);
					(*pVarient) = random.Rand(variants.GetShape3DArray().Size());
				}
				else
				{
					(*pVarient) = random.Rand(pSpawnSettings->atlasSize) + pSpawnSettings->atlasOffset;
				}

			}
			break;

		case E_ParticleElement_Size:
			{
				F32* pSize = (F32*)&pParticle[element.offset];
				if (pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape == E_ParticleShape_Mesh && doMeshVariant)
				{
					const C_UserShape3DVariants& variants = C_UserParticleLibrary::Instance().GetShape3DVariants(pEffect->GetRenderSettings().vertexBuildSettings.iParticleLibraryIndex);

					#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
						XMVECTOR vSize	= pEmitter->GetSpawnSize(random);	//Vec4 X duplicated as size

						vSize = XMVectorMultiply(	vSize,	XMVectorSplatZ(variants.GetScale()) );
						XMStoreScalar(pSize, vSize);						//store X as scalar
					#endif
				}
				else
				{
					#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
						XMVECTOR size = pEmitter->GetSpawnSize(random);
						XMStoreScalar(pSize, size);
					#endif

				}

			}
			break;

		case E_ParticleElement_LifeModifier:
			{
				F32* pLifeModifier = (F32*)&pParticle[element.offset];

			#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
				XMVECTOR lifeModifier = pEmitter->GetSpawnLifeVariance(random);
				XMStoreScalar(pLifeModifier, lifeModifier);
			#endif
			}
			break;

		case E_ParticleElement_Colour:
			{
				MathsStorage::Float4* pColour = (MathsStorage::Float4*)&pParticle[element.offset];
				
			#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
				XMVECTOR colour = pEmitter->GetSpawnColour(random);
				XMStoreFloat4A16((XMFLOAT4A16*)pColour, colour);
			#endif
			}
			break;

		case E_ParticleElement_Rotation:
			{
				F32* pRotation = (F32*)&pParticle[element.offset];
				*pRotation = pEmitter->GetSpawnRotation(random);
			}
			break;

		case E_ParticleElement_RotationRate:
			{
				F32* pRotationRate = (F32*)&pParticle[element.offset];
				*pRotationRate = pEmitter->GetSpawnRotationRate(random);
			}
			break;
		
		case E_ParticleElement_RotationQuaternion:
			{
				MathsStorage::Float4* pQuaternion = (MathsStorage::Float4*)&pParticle[element.offset];

			#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
				XMVECTOR quaternion = pEmitter->GetSpawnOrientation(random);
				XMStoreFloat4A16((XMFLOAT4A16*)pQuaternion, quaternion);
			#endif
			}
			break;
		case E_ParticleElement_AngularVelocity:
			{
				MathsStorage::Float4* pAngularVelocity = (MathsStorage::Float4*)&pParticle[element.offset];
	
			#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
				XMVECTOR angularVelocity = pEmitter->GetSpawnAngularVelocity(random);
				XMStoreFloat4A16((XMFLOAT4A16*)pAngularVelocity, angularVelocity);
			#endif
			}

		case E_ParticleElement_CollisionPlane:
			{
				MathsStorage::Float3* pCollisionPlanePosition = (MathsStorage::Float3*)&pParticle[element.offset];
				MathsStorage::HenDN3* pCollisionPlaneNormal = (MathsStorage::HenDN3*)&pParticle[element.offset + 12];

			#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
				XMVECTOR position = pEmitter->GetSpawnCollisionPlanePosition();
				XMVECTOR normal = pEmitter->GetSpawnCollisionPlaneNormal();
				XMStoreFloat3((XMFLOAT3*)&pCollisionPlanePosition, position);
				XMStoreHenDN3((XMHENDN3*)&pCollisionPlaneNormal, normal);
			#endif
			}
			break;
	}*/
}

void I_Spawner::CreateBlock(ParticleBlock* pBlock, const C_Emitter* pEmitter)
{

}

} // Particle
} // DynamicEffect


















