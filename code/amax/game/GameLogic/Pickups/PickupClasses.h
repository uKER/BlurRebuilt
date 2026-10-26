#ifndef PICKUP_CLASSES_H
#define PICKUP_CLASSES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{
	enum E_PickupClass
	{
		E_PickupClass_Unknown = -1,
		E_PickupClass_ShuntProjectile,				
		E_PickupClass_ShuntProjectileReverse,		
		E_PickupClass_DoubleShunt,					
		E_PickupClass_DoubleShuntReverse,			
		E_PickupClass_NitrousReverse,				
		E_PickupClass_DoubleNitrous,					
		E_PickupClass_BlindfireProjectile,			
		E_PickupClass_BlindfireProjectileReverse,	
		E_PickupClass_BlindfireShotgun,				
		E_PickupClass_BlindfireShotgunReverse,		
		E_PickupClass_Barge,							
		E_PickupClass_Shock,
		E_PickupClass_DoubleShock,
		E_PickupClass_Bomb,							
		E_PickupClass_DoubleBomb,					
		E_PickupClass_BombReverse,					
		E_PickupClass_MasterMineBomb,				
		E_PickupClass_MiniBomb,						
		E_PickupClass_Shield,						
		E_PickupClass_SuperShield			
	};
}

#endif //PICKUP_CLASSES_H
