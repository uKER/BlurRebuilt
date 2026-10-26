#ifndef IRLCOMPONENT_H
#define IRLCOMPONENT_H

//Defines the components and the order inwhich they're updated
enum E_RLCOMPONENTTYPE
{
	E_RLCOMPONENTTYPE_NONE = -1,

	//CONTROLLERS
	E_RLCOMPONENTTYPE_DATABASE_CONTROLLER,										//Get any data from the Db first
																				//thats going to happen this frame to happen
	//CONTROLLERS

	E_RLCOMPONENTTYPE_MAX
};

namespace Game
{
namespace Actors
{
class RaceLogicCore;

class IRLComponent
{
//Functions
public:
	IRLComponent(){};
	virtual ~IRLComponent(){};

	virtual void								PreUpdate(){};
	virtual void								Update(){};
	virtual void								PostUpdate(){};

	virtual void								Render(){};

	void										SetRLC(RaceLogicCore* pRLC){m_pRLC = pRLC;}


	const enum E_RLCOMPONENTTYPE				GetType() const {return meType;}

//Variables
protected:
	enum E_RLCOMPONENTTYPE						meType;

	RaceLogicCore*								m_pRLC;
};
};//namespace Actors
};//namespace Game

#endif //IRLCOMPONENT_H
