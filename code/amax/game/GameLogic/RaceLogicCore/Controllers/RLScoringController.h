#ifndef RLSCORING_CONTROLLER_H
#define RLSCORING_CONTROLLER_H

#include "../Components/IRLComponent.h"
#include "../Data/RLDefinitions.h"

#include <ui/util/WideString.h>


struct RLScoringRuleType;
struct RLScoringType;
struct RLScoringRuleValue;

namespace Game
{
namespace Actors
{
class RLScoringController : public IRLComponent
{
//Functions
public:
	RLScoringController();
	~RLScoringController();

	void										PreUpdate();
	void										Update();
	void										PostUpdate();

	void										AddScoringEvent(const U8 vehicleIndex, 
																const RLScoringType& scoringType, 
																const float multiplier = 1.f, 
																const UI::C_WideString64 overrideScoreTypeName = L"");

private:
	const bool									ValidateDatabase();

	void										Initialise();

	void										PreUpdateVehicle(const U8 vehicleIndex);
	void										UpdateVehicle(const U8 vehicleIndex);
	void										PostUpdateVehicle(const U8 vehicleIndex);

	const bool									EvaluateRule(	const U8 vehicleIndex, 
																const RLScoringRuleType& rule, 
																const bool invertRule, 
																const bool outputDebugInfo = false, 
																const bool conditionalDisplay = false, 
																const bool displayWhenTrue = false) const;
	
	const bool									EvaluateValue(	const RLScoringRuleValue& ruleValue, 
																const float valueToBeEvaluated, 
																const bool outputDebugInfo = false, 
																const bool conditionalDisplay = false, 
																const bool displayWhenTrue = false) const;

	

};

};	//namespace Actors
};	//namespace Game


#endif //RLSCORING_CONTROLLER_H
