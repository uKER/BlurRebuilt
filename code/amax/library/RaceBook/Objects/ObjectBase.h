#ifndef RACEBOOK_OBJECT_BASE_H
#define RACEBOOK_OBJECT_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/serialise/BinarySerialiseAutoEndian.h>

namespace RaceBook
{

class C_ObjectBase
{
public:
	C_ObjectBase() : mHot(false), mNew(true), mObjectId(0) {};

	bool mHot;
	U32  mObjectId;

	void Serialise( Sys::BinarySerialiserAutoEndian& stream )     { Sys::BinarySerialiserAutoEndian out( stream ); out << mObjectId << (S32&)mHot << (S32&)mNew; };
	void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream ) { Sys::BinaryDeserialiserAutoEndian in( stream ); in >> mObjectId >> (S32&)mHot >> (S32&)mNew; };

	bool IsNew()	  { return mNew; };
	void SeenObject() { mNew = false; };

protected:
	bool mNew;
};

// Might be overkill for one variable ...
class C_MessageIdxTracker
{
public:
	static C_MessageIdxTracker& Instance(){ return s_instance; };

	void AssignId( C_ObjectBase* obj );
	void RemoveId( int i );
	void ClearIds()						{ objectList.Clear(); };
	void AddObject( C_ObjectBase* obj ) { objectList.PushBack(obj); };
	void ReorderObjects();


private:
	static C_MessageIdxTracker s_instance;
	Btl::Vector<C_ObjectBase*> objectList;
};

} // namespace RaceBook

#endif // RACEBOOK_OBJECT_BASE_H
