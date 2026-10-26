#include "stdafx.h"
#include "FavouriteGroups.h"

namespace Game
{
const U64 FavouriteGroups::EMPTY_PROFILE_FAVOURITE_GROUP = -1;

void FavouriteGroups::Initialise()
{
	mFavouriteGroups.Clear();
	for(U8 groupIndex = 0; groupIndex < FavouriteGroups::MAX_PROFILE_FAVOURITE_GROUPS; groupIndex++)
	{
		mFavouriteGroups.PushBack(FavouriteGroups::EMPTY_PROFILE_FAVOURITE_GROUP);
	}

	UpdateSize();
}

void FavouriteGroups::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	Sys::BinarySerialiserAutoEndian out( stream );

	//The favourite group array is a fixed size so we don't need to output the size of this sucker
	for(int groupIndex = 0; groupIndex < mFavouriteGroups.Size(); groupIndex++)
	{
		out << mFavouriteGroups[groupIndex];
	}
}

void FavouriteGroups::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	Sys::BinaryDeserialiserAutoEndian in( stream );

	//The favourite group array is a fixed size so we don't need to output the size of this sucker
	for(int groupIndex = 0; groupIndex < mFavouriteGroups.Size(); groupIndex++)
	{
		in >> mFavouriteGroups[groupIndex];
	}

	UpdateSize();
}

//Return the group at index...
const UI::TEventID FavouriteGroups::GetFavouriteGroupByIndex(const U8 favouriteGroupIndex) const
{
	BIZ_ASSERT_MSG(favouriteGroupIndex < mFavouriteGroups.Size(), "FavouriteGroups::GetFavouriteGroupByIndex() > Invalid group index :( KAPOW!!!");

	return mFavouriteGroups[favouriteGroupIndex];
}

 //Search through the groups looking for tgroupId and return the index
const U8 FavouriteGroups::GetIndexByFavouriteGroup(const UI::TEventID tgroupId) const
{
	for(U8 i = 0; i < mFavouriteGroups.Size(); i++)
	{
		if(mFavouriteGroups[i] == tgroupId)
		{
			return i;
		}
	}

	//It might be useful to know that we've not found the required group so we don't need to assert here, I think
	return 0xff;
}

//Add a new favourite group, by default adds it as the MOST favourite group evar (index 0) and knocks on off the least favourite (index MAX -1)
const bool FavouriteGroups::AddFavouriteGroup(const UI::TEventID tgroupId, const U8 insertAtIndex)	
{
	if ( Size() == MAX_PROFILE_FAVOURITE_GROUPS )
		return false;

	//Insert at index 
	for(U8 i = mFavouriteGroups.Size()-1; i > insertAtIndex; i--)
	{
		mFavouriteGroups[i] = mFavouriteGroups[i-1];
	}
	mFavouriteGroups[insertAtIndex] = tgroupId;

	//Update the count of elements 
	UpdateSize();

	return true;
}

const bool FavouriteGroups::RemoveFavouriteGroup(const UI::TEventID tgroupId) //Remove group by tgroupId
{
	bool success = false;

	//Find the index of the condemned
	const U8 index = GetIndexByFavouriteGroup(tgroupId);
	if(index == 0xff)
	{
		//Don't need to assert, I think
		return success;
	}

	return RemoveFavouriteGroupByIndex(index);
}

const bool FavouriteGroups::RemoveFavouriteGroupByIndex(const U8 indexToBeRemoved) //Remove group by index
{
	//We know that the array is always the same size so we know this isn't going to fail
	int lastIndex = mFavouriteGroups.Size()-1;
	for(U8 i = indexToBeRemoved; i < lastIndex; i++)
	{
		mFavouriteGroups[i] = mFavouriteGroups[i+1];
	}
	mFavouriteGroups[lastIndex] = FavouriteGroups::EMPTY_PROFILE_FAVOURITE_GROUP;
	
	//Update the count of elements
	UpdateSize();
	
	return true;
}

const bool FavouriteGroups::MoveFavouriteGroup(const UI::TEventID tgroupId, const U8 destinationIndex) //Promote group
{
	//Find the index of the condemned
	const U8 sourceIndex = GetIndexByFavouriteGroup(tgroupId);
	BIZ_ASSERT_MSG(sourceIndex < 0xff, "FavouriteGroups::MoveFavouriteGroup() > Couldn't find group to promote :(");

	return MoveFavouriteGroupByIndex(sourceIndex, destinationIndex);
}

const bool FavouriteGroups::MoveFavouriteGroupByIndex(const U8 sourceIndex, const U8 destinationIndex)
{
	BIZ_ASSERT_MSG(sourceIndex < mFavouriteGroups.Size(), "FavouriteGroup::MoveFavouriteGroupByIndex() > Source index is invalid, wtf!! KAPPOWWWW!!!");
	BIZ_ASSERT_MSG(destinationIndex < mFavouriteGroups.Size(), "FavouriteGroup::MoveFavouriteGroupByIndex() > Destination index is invalid, wtf!! ZZZZAAPPPP!!");

	//Get the value of the group we're moving
	UI::TEventID tgroupId = mFavouriteGroups[sourceIndex];

	//Remove the group from its old index
	RemoveFavouriteGroupByIndex(sourceIndex);

	//Now add it in at its new index
	AddFavouriteGroup(tgroupId, destinationIndex);

	return true;
}

void FavouriteGroups::UpdateSize() 
{
	mSize = 0;
	//Go through each group and count all the ones that are in their default values
	for(U8 groupIndex = 0; groupIndex < FavouriteGroups::MAX_PROFILE_FAVOURITE_GROUPS; groupIndex++)
	{
		//
		if(mFavouriteGroups[groupIndex] != FavouriteGroups::EMPTY_PROFILE_FAVOURITE_GROUP)
		{
			//We have a valid group (unless someone creates a group who's value is FavouriteGroups::EMPTY_PROFILE_FAVOURITE_GROUP)
			mSize++;
		}
	}
}

const S32 FavouriteGroups::Size() const
{

	return mSize;
}

}; //namespace Game
