#ifndef MULTIPLAYER_AWARDS_H
#define MULTIPLAYER_AWARDS_H

#include "UI/game/Multiplayer/MultiplayerDefines.h"
#include "GameLogic/GameData/MultiplayerData.h"
#include <Network\Misc\NetDefines.h>
#include "system\misc\Random.h"

struct MultiplayerAwardData;

enum
{
	E_MultiplayerAward_MaxAwards = 5,				// Ensure no sorcery award distribution spec never gives more than this!
};

namespace Game
{
	//The decision on who got what - to be sent over the network
	struct AwardSelection
	{
		UI::E_MultiplayerAward		m_Awards[E_MultiplayerAward_MaxAwards];
		U8							m_Owners[E_MultiplayerAward_MaxAwards];
		F32							m_AwardValues[E_MultiplayerAward_MaxAwards];
		bool						m_AwardTied[E_MultiplayerAward_MaxAwards];

		void Reset()
		{
			for(int i = 0; i < E_MultiplayerAward_MaxAwards; i++)
			{
				m_Awards[i]			= UI::E_MultiplayerAward_None;
				m_Owners[i]			= 0;
				m_AwardValues[i]	= 0.f;
				m_AwardTied[i]		= false;
			}
		}
	};

	//Holds information about an individual award
	class C_MultiplayerAward
	{	
	public:
		// Public so C_MultiplayerAwardS class can also use
		struct MultiplayerAwardStatInformation
		{
			U8		joinId;
			float	statValue;
			U8		finishPosition;
			bool	tied;
		};

		C_MultiplayerAward(){}
		C_MultiplayerAward( const XtHandle<MultiplayerAwardData> pAwardData );

		MultiplayerAwardStatInformation	CalculateWinner(S8 numPlayersAtEndOfRace) const;
		const F32								CalculateDesirability() const;
		const MultiplayerAwardData*				GetData() const;
		
	private:
		XtHandle<MultiplayerAwardData>			m_Handle;

		typedef Btl::Vector< MultiplayerAwardStatInformation, Btl::VectorParameters_Array< RacerNet::NET_MAX_VEHICLES > > MultiplayerAwardStats;
	};

	typedef Btl::Vector< C_MultiplayerAward, Btl::VectorParameters_Array< UI::E_MultiplayerAward_Count > > MultiplayerAwards;

	struct AwardResult
	{
		AwardResult()
		{
			Init();
		}

		void Init() 
		{
			mAwardType = UI::E_MultiplayerAward_None;
			mAwardCategory = UI::E_MultiplayerAwardCategory_None;
			mDesirability = 0.f;
			mDefinedYet = false;
			mJoinID = -1;
			mWinnersScore = 0.f;
			mFinishPosition = 0;
			mTied = false;
		}

		enum UI::E_MultiplayerAward					mAwardType;
		enum UI::E_MultiplayerAwardCategory			mAwardCategory;
		F32											mDesirability;
		bool										mDefinedYet;
		U8											mJoinID;
		float										mWinnersScore;
		U8											mFinishPosition;
		bool										mTied;
	};

	//Holds information on all awards including current selection
	class C_MultiplayerAwards
	{
	public:
		typedef Btl::Vector< AwardResult, Btl::VectorParameters_Array< UI::E_MultiplayerAward_Count > > AwardsResults;
													C_MultiplayerAwards();
		void										Init();
		void										ResetSelection();
		const C_MultiplayerAward&					GetAward(const UI::E_MultiplayerAward award) const;
		const MultiplayerAwards&					GetAwards() const { return m_Awards; };
		void										GiftAward(const S32 awardIndex);
		void										CalculateWinners(S8 numPlayersAtEndOfRace);
		void										CalculateDesirability();
		const AwardSelection&						CalculateAwards(U32 seed);
		void										SetAwards(const	AwardSelection& awardSelection);
		void										SelectDesirableAwardsDebug();
		void										SelectDesirableAwards(  U8 awardsToChoose, U8 spoonsToChoose, U8  randomAwardsOrSpoonsToChoose );
		const AwardSelection&						GetAwardSelection() const { return m_CurrentSelection; };
		void										ConvertToLocalIndices();
		void										ResolveTiedAwards();

		int											NumAwardsDelt() const { return m_AwardsToGive; }

	private:
		int					m_NumPlayersAtEndOfRace;
		MultiplayerAwards	m_Awards;				//All of the potential awards
		AwardSelection		m_CurrentSelection;		//The selection of each award used and its winner
		AwardsResults		m_AwardResults;			//Results for each award (Both used and not)

		int					m_AwardNumber;
		int					m_AwardsToGive;
		Util::C_Random		m_Random;

		void										EnforceWinLimit(const U8 numPlayers, const U8 maxAwardsPerPlayer);
		void										TallyUpWinsPerPlayer(  Btl::Vector< Btl::Vector<UI::E_MultiplayerAward> >* numWinsArray, Btl::Map<U8,U8>& LocalIdToFinishPosMap,const U8 numPlayers );
		void										DeprioritiseInvalidAwards(Btl::Vector<UI::E_MultiplayerAward>* winnersAwards, const U8 maxAwardsToAllowThisWinner);
		void										BuildLocalIdToFinishPosMap( Btl::Map<U8,U8>& LocalIdToFinishPosMap );

		// Award sorting comparison functor
		class SortAwardsByDesirability
		{
		public:
			bool operator ()(const AwardResult& LHS, const AwardResult& RHS)
			{
				return (LHS.mDesirability > RHS.mDesirability);
			}
		};

		// Helper Functions
		void										BuildSpoonAndAwardLists( Btl::Vector< U8 >* desirableAwards,   Btl::Vector< U8 >* desirableSpoons,
																			 Btl::Vector< U8 >* undesirableAwards, Btl::Vector< U8 >* undesirableSpoons);

		void										GiftRequiredNumberFromSelections( U8& numToGift, Btl::Vector< U8 >* desirables, Btl::Vector< U8 >* undesirables);
		void										GiftRandomsFromSelection( Btl::Vector< U8 >* selection, U8& countRemaining, Btl::Vector< U8 >* otherSelection = NULL);
		void										GiftRandomAwardOrSpoon( Btl::Vector< U8 >* theDesirableAwardIndices, Btl::Vector< U8 >* theDesirableSpoonIndices,
																		    Btl::Vector< U8 >* theUndesirableAwardIndices, Btl::Vector< U8 >* theUndesirableSpoonIndices);
		void										DisableNonsensicalAwards(); 

		const XtHandle<MultiplayerAwardDistributionSpec>	GetAwardDistributionSpec( U8 numPlayersToDistributeTo ) const;

	};



} //End namespace Game

#endif
