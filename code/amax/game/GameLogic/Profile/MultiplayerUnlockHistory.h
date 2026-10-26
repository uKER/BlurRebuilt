#ifndef _MULTIPLAYER_UNLOCK_HISTORY_H_
#define _MULTIPLAYER_UNLOCK_HISTORY_H_

#include "xt\XtCoreTypes.h"
#include "ui\game\Multiplayer\MultiplayerDefines.h"
#include "btl\Containers\map.h"

struct MultiplayerConfig;
struct MultiplayerLevel;

namespace Sys
{
	class BinarySerialiserAutoEndian;
	class BinaryDeserialiserAutoEndian;
}

namespace Game
{	
	class C_MultiplayerUnlockHistory
	{
	public:
		typedef Btl::Map<XtId, bool> CarsMapType;
		typedef Btl::Vector<bool, Btl::VectorParameters_Array<UI::E_MultiplayerVehicleAbilities_NumberOfAbilities> > AbilitiesContainerType;
		typedef Btl::Vector<bool, Btl::VectorParameters_Array<UI::E_ChallengePack_Count> > ChallengePacksContainerType;
		typedef Btl::Vector<bool, Btl::VectorParameters_Array<UI::E_MultiplayerFeature_Count> > FeaturesContainerType;
		typedef Btl::Vector<XtId> EventsContainerType;

		C_MultiplayerUnlockHistory();

		void Init(const MultiplayerConfig* config);						// Init Recent Unlock History on creation of profile (sets up and default appropriate containers)
		void Serialise( Sys::BinarySerialiserAutoEndian& stream ) const;
		void Deserialise(const Sys::BinaryDeserialiserAutoEndian& stream );

		// Unlock History
		void SetCarUnlocked(XtId carID,const bool val);
		void SetCarChallengeUnlocked(XtId carID,const bool val);
		void SetChallengePackUnlocked(const UI::E_ChallengePack ePackCategory  ,const bool val);
		void SetAbilityUnlocked(const UI::E_MultiplayerVehicleAbilities eAbilityType ,const bool val);
		void SetFeatureUnlocked(const UI::E_MultiplayerFeature eFeatureType ,const bool val);
		void SetEventUnlocked(XtId event ,const bool val);
		bool GetEventUnlocked(XtId event) const;

		void FlagNewlyUnlockedItems(const MultiplayerConfig* config, const int startLevel, int endLevel);		// This is whenever new things are unlocked so they can be made shiny and 'new'

		const CarsMapType& GetCars() const;
		const CarsMapType& GetCarChallenges() const;
		const AbilitiesContainerType& GetAbilities() const;
		const ChallengePacksContainerType& GetChallengePacks() const;
		const FeaturesContainerType& GetFeatures() const;
		const EventsContainerType& GetEvents() const;

	private:
		bool GetEventValid(XtId eventId) const;

		void FlagCars(const MultiplayerLevel& level);
		void FlagChallengePacks(const MultiplayerLevel& level);
		void FlagAbilities(const MultiplayerLevel& level);
		void FlagFeatures(const MultiplayerLevel& level);
		void FlagEvents(const MultiplayerConfig& config, int rank);

		void InitCars(const MultiplayerConfig* config); 
		void InitChallengePacks();
		void InitAbilities(); 
		void InitFeatures(); 

		// Unlock history consists of several maps, indicating wether the item was 'recently' unlocked.  
		CarsMapType	mCars;					// Which Cars have been unlocked?			  (accessed by XtID)
		CarsMapType	mCarChallenges;			// Which CarChallenges have been unlocked?			  (accessed by XtID)
		AbilitiesContainerType mAbilities;				// Which Abilities have been unlocked?		  (accessed by Ability Enum)
		ChallengePacksContainerType mChallengePacks;		// Which Challenge Packs have been unlocked?  (accessed by Challenge Pack Enum) (based on challenge pack name).. no simple way to do this one.. ugh
		FeaturesContainerType mFeatures;
		EventsContainerType mEvents;
	};
};

#endif // _MULTIPLAYER_UNLOCK_HISTORY_H_
