#ifndef EDITOR_LIB_EXT_ACTORS_REFRESH_H
#define EDITOR_LIB_EXT_ACTORS_REFRESH_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <Btl/Containers/Set.h>

namespace EditorLib
{
	typedef Btl::Set<Btl::String> UniqueRefSet;

	struct C_ActorMapItem
	{
		Xt::DomComponent*	pRoot;
	};

	struct C_AftkActorPlaceholder
	{
		C_AftkActorPlaceholder()
		{
		}

		C_AftkActorPlaceholder(const C_AftkActorPlaceholder& rhs)
		{
			*this = rhs;
		}

		const C_AftkActorPlaceholder& operator=(const C_AftkActorPlaceholder& rhs)
		{
			path = rhs.path;
			name = rhs.name;
			refFilePath = rhs.refFilePath;
			worldMatrix = rhs.worldMatrix;
			attrs = rhs.attrs;
			return *this;
		}

		Btl::String path;
		Btl::String name;
		Btl::String refFilePath;
		Matrix44 worldMatrix;
		Btl::Vector<Util::C_KeyValue> attrs;
	};

	struct C_RefInfo
	{
		C_RefInfo()
		{
		}

		C_RefInfo(const C_RefInfo& rhs)
		{
			*this = rhs;
		}

		const C_RefInfo& operator=(const C_RefInfo& rhs)
		{
			originalFilePath = rhs.originalFilePath;
			cacheFilePath = rhs.cacheFilePath;
			actorPlaceholders = rhs.actorPlaceholders;
			return *this;
		}

		Btl::String originalFilePath;
		Btl::String cacheFilePath;
		Btl::Vector<C_AftkActorPlaceholder> actorPlaceholders;
	};




	struct C_ExtActorsRefStats
	{
		C_ExtActorsRefStats()
		{
			nActors = 0;
		}

		C_ExtActorsRefStats(const C_ExtActorsRefStats& rhs)
		{
			*this = rhs;
		}

		const C_ExtActorsRefStats& operator=(const C_ExtActorsRefStats& rhs)
		{
			originalFilePath = rhs.originalFilePath;
			nActors = rhs.nActors;
			return *this;
		}

		Btl::String originalFilePath;
		U32 nActors;
	};

	struct C_ExtActorsRecordStats
	{
		C_ExtActorsRecordStats()
		{
			nActorsCreated = 0;
		}

		C_ExtActorsRecordStats(const C_ExtActorsRecordStats& rhs)
		{
			*this = rhs;
		}

		const C_ExtActorsRecordStats& operator=(const C_ExtActorsRecordStats& rhs)
		{
			recordName = rhs.recordName;
			nActorsCreated = rhs.nActorsCreated;
			refArray = rhs.refArray;
			return *this;
		}

		Btl::String recordName;
		S32 nActorsCreated;
		Btl::Vector<C_ExtActorsRefStats> refArray;
	};

	struct C_ExtActorsPackStats
	{
		C_ExtActorsPackStats()
		{
		}

		C_ExtActorsPackStats(const C_ExtActorsPackStats& rhs)
		{
			*this = rhs;
		}

		const C_ExtActorsPackStats& operator=(const C_ExtActorsPackStats& rhs)
		{
			packName = rhs.packName;
			recordStatsArray = rhs.recordStatsArray;
			uniqueRefFilePaths = rhs.uniqueRefFilePaths;
			return *this;
		}

		Btl::String packName;
		Btl::Vector<C_ExtActorsRecordStats> recordStatsArray;
		UniqueRefSet uniqueRefFilePaths;
	};

	struct C_ExtActorsDbStats
	{
		C_ExtActorsDbStats() {}

		UniqueRefSet uniqueRefFilePaths;
		Btl::Vector<C_ExtActorsPackStats> packStatsArray;

	private:
		C_ExtActorsDbStats(const C_ExtActorsDbStats& rhs);
		const C_ExtActorsDbStats& operator=(const C_ExtActorsDbStats& rhs);
	};



	typedef Btl::OpenHashMap<Btl::String, C_ActorMapItem, Btl::HashKeyOpBtlString> ActorMap;

	void ClearDb();
	void ClearPack(Xt::DomPack* pPack);
	void ClearRecord(Xt::DomRecord* pRec);

	C_Status	CreateActor(
		LfArray* pLevelActorArrayField,
		Xt::DomArray* pLevelActorArray,
		ActorMap& actorsToMerge,
		const Btl::String& aftkNodeName,
		const Btl::String& aftkNodePath,
		const Matrix44& aftkNodeWorldMatrix,
		const Btl::String& xml);

	void		BuildActorMap(
		ActorMap& actorsToMerge, 
		Xt::DomArray* pLevelActorArray);

	Xt::DomComponent* AcquireDomActor(
		ActorMap& actorsToMerge, 
		const Btl::String& aftkNodeName,
		const Btl::String& aftkNodePath,
		const Btl::String& aftkRefFilePath,
		bool& created);

	void RemoveUnmergedActors(
		Xt::DomArray* pLevelActorArray, 
		const Btl::String& originalRefFilePath,
		const ActorMap& actorsToMerge);

	S32 CreateActors(
		Xt::DomRecord* pRec,
		const Btl::String& originalRefFilePath,
		const Btl::Vector<C_AftkActorPlaceholder>& aftkActorPlaceholderArray);

	bool PackContainsActorLists(Xt::DomPack* pPack);

	void DisplayRefreshSummary(const C_ExtActorsDbStats& stats);
}

#endif
