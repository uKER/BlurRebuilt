#ifndef _SPLIT_SCREEN_SCORING_H_
#define _SPLIT_SCREEN_SCORING_H_

namespace Game
{

class SplitScreenScoring
{
const static int MAX_SPLIT_SCREEN_PLAYERS = 4;

public:

	typedef Btl::Vector<int, Btl::VectorParameters_Array<MAX_SPLIT_SCREEN_PLAYERS> > RaceResults;
	typedef Btl::Vector<int, Btl::VectorParameters_Array<MAX_SPLIT_SCREEN_PLAYERS> > Scores;

	SplitScreenScoring();

	void Reset();

	//Results are now in viewport order, external
	void RaceFinished(const RaceResults& racePositionResults, const RaceResults& fanResults);	
	
	const Scores& GetTotalScores() const;
	const Scores& GetScoresForRace() const;

	const RaceResults& GetRaceResulsts() const;

	const Scores& GetTotalFans() const;
	const Scores& GetFansForRace() const;	// Not sorted either

	const int GetTotalRaces() const;
	const Scores& GetTotalWins() const;

	void SetNoViewports(const int noViewports);

private:
	const int GetSplitScreenScore(const int racePosition, const bool includesAIs) const;

	Scores m_totalScores;
	Scores m_scoresForRace;

	Scores m_fansForRace;
	Scores m_totalFans;

	Scores m_TotalWins;

	RaceResults m_RaceResults;

	int m_noViewports;
};

}

#endif // _SPLIT_SCREEN_SCORING_H_
