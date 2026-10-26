#ifndef VEHICLE_PERFORMANCE_MODIFIERS_H
#define VEHICLE_PERFORMANCE_MODIFIERS_H

#include <Btl/Containers/Vector.h>
#include "../../../GameLogic/Structures/LevelStateConstants.h"
#include "../../../GameLogic/RaceLogicCore/Data/RLEnums.h"

namespace Game
{
	namespace Actors
	{
		struct VehiclePerformanceModifiers
		{
			template <typename Type> class VehiclePerformanceModifier
			{
				public:
					VehiclePerformanceModifier()
					{
						mModify=false;
					};

					void Set(bool modify, Type modifier)
					{
						mModify=modify;
						mModifier=modifier;
					}

					bool mModify;
					Type mModifier;
			};

			void InitialiseWithOverwrite()
			{
				//Used by various pickups, Nitrous, Ghost, BullBars ...
				mBoost.Set(true, 0.f);

				//Used by Drafting component
				mDraftBoost.Set(true, 0.f);

				//Used by domination
				mAssistBoost.Set(true, 0.f);

				//Used by RL Scoring Controller
				mXPMultiplier.Set(true, 1.f);

				//Used by most aggressive perks
				mForce.Set(true, Maths::HVec3());
				mPosition.Set(true, Maths::HVec3());

				//Used by Shock and Pulse
				mCutThrottle.Set(true, false);

				//Used by Shunt
				mClampSteering.Set(true, false);
				mClampedSteeringAngle.Set(true, 0.f);
			}


			void Initialise()
			{
				//Used by various pickups, Nitrous, Ghost, BullBars ...
				mBoost.Set(false, 0.f);

				//Used by Drafting component
				mDraftBoost.Set(false, 0.f);

				//Used by domination
				mAssistBoost.Set(false, 0.f);

				//Used by RL Scoring Controller
				mXPMultiplier.Set(false, 1.f);

				//Used by most aggressive perks
				mForce.Set(false, Maths::HVec3());
				mPosition.Set(false, Maths::HVec3());

				//Used by Shock and Pulse
				mCutThrottle.Set(false, false);

				//Used by Shunt
				mClampSteering.Set(false, false);
				mClampedSteeringAngle.Set(false, 0.f);
			}


			VehiclePerformanceModifier<float>			mBoost;
			
			VehiclePerformanceModifier<float>			mDraftBoost;

			VehiclePerformanceModifier<float>			mAssistBoost;					//"Boost" we apply for MP rubber-banding

			VehiclePerformanceModifier<float>			mXPMultiplier;

			VehiclePerformanceModifier<Maths::HVec3>	mForce;
			VehiclePerformanceModifier<Maths::HVec3>	mPosition;

			VehiclePerformanceModifier<bool>			mCutThrottle;

			VehiclePerformanceModifier<bool>			mClampSteering;
			VehiclePerformanceModifier<float>			mClampedSteeringAngle;
		};

	};	//namespace Actors

};	//namespace Game

#endif //VEHICLE_PERFORMANCE_MODIFIERS_H
