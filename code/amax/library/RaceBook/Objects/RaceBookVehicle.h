#ifndef RACEBOOK_VEHICLE_H
#define RACEBOOK_VEHICLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <RaceBook\Profile\RaceBookProfile.h>
#include <RaceBook\Objects\ObjectBase.h>

namespace RaceBook
{

	class C_RaceBookProfile;

	class C_RaceBookVehicle : public C_ObjectBase
	{
	public:
		C_RaceBookVehicle() : mOwner(NULL), mID(0), mBaseModelID(0), mIsForSale(false), mCost(0), mPerkSlots(0),mDisallowedInSplitscreen(false) {}

		U32 mID;
		C_RaceBookProfile* mOwner;
		U32 mBaseModelID;
		bool mIsForSale;
		int mCost;
		bool mDisallowedInSplitscreen; 
		
		U32 mSellTextID;
		U32 mSellerID;
		U32 mPutUpForSaleAfter;
		
		int mPerkSlots;

		int mNumberOfBaseSlots;
		Btl::Vector<int> mAdditionalSlotsCost;

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

		void AddPerkSlot();
		bool MaxiumumSlots();
		float UpgradeCost();

	private:

	};

} // namespace RaceBook

#endif // RACEBOOK_VEHICLE_H
