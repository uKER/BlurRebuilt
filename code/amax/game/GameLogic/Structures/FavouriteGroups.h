#ifndef FAVOURITE_GROUPS_H_
#define FAVOURITE_GROUPS_H_

#include <Ui\game\Multiplayer\MultiplayerDefines.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

namespace Game
{
class FavouriteGroups
{
public:
	const static U64 EMPTY_PROFILE_FAVOURITE_GROUP;
	const static int MAX_PROFILE_FAVOURITE_GROUPS = 10;
	typedef Btl::Vector<UI::TEventID, Btl::VectorParameters_Static<MAX_PROFILE_FAVOURITE_GROUPS> > FavouriteGroupArray;

	void										Initialise();

	void										Serialise( Sys::BinarySerialiserAutoEndian& stream );
	void										Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	const UI::TEventID							GetFavouriteGroupByIndex(const U8 favouriteGroupIndex) const;		//Return the group at index...
	const U8									GetIndexByFavouriteGroup(const UI::TEventID tgroupId) const;		//Search through the groups looking for tgroupId and return the index

	const bool									AddFavouriteGroup(const UI::TEventID tgroupId, const U8 insertAtIndex = 0);	//Add a new favourite group, by default adds it as the MOST favourite group evar (index 0) and knocks on off the least favourite (index MAX -1)
	const bool									RemoveFavouriteGroup(const UI::TEventID tgroupId);					//Remove group by tgroupId
	const bool									RemoveFavouriteGroupByIndex(const U8 indexToBeRemoved);				//Remove group by index
	
	const bool									MoveFavouriteGroup(const UI::TEventID tgroupId, const U8 destinationIndex);	//Promote group
	const bool									MoveFavouriteGroupByIndex(const U8 sourceIndex, const U8 destinationIndex);

	const S32									Size() const;

private:
	void										UpdateSize();

private:

	FavouriteGroupArray							mFavouriteGroups;

	S32											mSize;					//This is the number of groups currently stored not the physical size (which is always MAX_PROFILE_FAVOURITE_GROUPS)
};

}; //namespace Game

#endif //FAVOURITE_GROUPS_H_
