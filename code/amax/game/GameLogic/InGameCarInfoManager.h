#ifndef INGAME_CAR_INFO_MANAGER_H
#define INGAME_CAR_INFO_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>

#include <misc_shared/Messaging/Messaging.h>
#include <misc_shared\Messaging\MessageMultiHandler.h>

#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>

#include <Ui\Graphics\Font\FontStyle.h>
#include <Ui\Graphics\UIColourClasses.h>

namespace Game
{

class C_InGameCarInfoManager : public Messaging::C_MessageMultiHandler
{
public:

	C_InGameCarInfoManager()
	{
	};

	void Init();
	static C_InGameCarInfoManager& Instance(){return s_instance;};

	// For game messages
	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	void			StoreCarCoords(int index, Maths::HVec3 pos, float fontsize, float alpha);

	Maths::HVec3&	GetCarCoords( S32 sIndex )														{ return mCarScreenCoords[ sIndex ]; }
	F32				GetFontSize( S32 sIndex )														{ return mCarNameHeights[ sIndex ]; }
	F32				GetAlpha( S32 sIndex )															{ return mCarNameAlphas[ sIndex ]; }

private:
	bool					mShowNames;
	UI::C_FontStylePasses	mFontStylePasses;
	UI::C_UISpriteColours	mFontColour;

	Btl::Vector<Maths::HVec3,Btl::VectorParameters_Array<MAX_VEHICLES> > mCarScreenCoords;
	Btl::Vector<float,Btl::VectorParameters_Array<MAX_VEHICLES> > mCarNameHeights;
	Btl::Vector<float,Btl::VectorParameters_Array<MAX_VEHICLES> > mCarNameAlphas;
	
	static C_InGameCarInfoManager s_instance;
};

} // namespace Game

#endif // INGAME_CAR_INFO_MANAGER_H