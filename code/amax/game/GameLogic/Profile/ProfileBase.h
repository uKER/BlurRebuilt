#ifndef PROFILE_BASE_H
#define PROFILE_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{

class C_ProfileBase
{
public:
	virtual void			Init(){};

	// TODO : Get rid of these functions
	virtual const S32		NumPickupSlots() const	
							{ 
								return 3; 
							}

	virtual const bool		AllowAnyPerkType() const	
							{ 
								return true; 
							}

	virtual const bool		AllowManualPerkChange()	const
							{ 
								return true; 
							}

	virtual const bool		AllowShield() const
							{
								return true;
							}

	virtual const S32		NumShieldSlots() const
							{
								return 3;
							}

	virtual const S32		NumAdditionalHealthSlots() const
							{
								return 0;
							}
	// END TODO

	virtual const S32		NumFans() const
							{
								return 0;
							}

	virtual const S32		NumPreviousFans() const
							{
								return 0;
							}
};

}

#endif //PROFILE_BASE_H