#ifndef GAME_DATA_DEFS_H
#define GAME_DATA_DEFS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <xt/xtgame.h>
#include <SafeString.inl>
#include "ui/Graphics/Localisation/Loc.h"


struct GAMEDATA_ID
{
	XtId id;

public:
	explicit GAMEDATA_ID(XtId val = 0) : id(val) { }

public:
	operator XtId()				{ return id; }
	void operator=(XtId val)	{ id = val; }
};

//XtEventType, XtRecord*, void*, XtCallbackContext*
template<typename T,typename C> void GameDataHandler(XtEventType event, XtRecord* record, T* object, XtCallbackContext*)
{
	switch(event)
	{
	case kXtEtConstruct:
		object->ID=record->id;
		C()()->Add(object);
		break;

	case kXtEtDestruct:
		C()()->Destroy((GAMEDATA_ID)object->ID);
		break;
	}
}

//XtEventType, XtRecord*, void*, XtCallbackContext*
template<typename T> void GameIDHandler(XtEventType event, XtRecord* record, T* object, XtCallbackContext*)
{
	switch(event)
	{
		case kXtEtConstruct:
			object->ID=record->id;
			break;
	}
}

struct AmaxDLCBase
{
	U8		dlcPack;
	S32		dlcLicenseBit;
	bool	dlcLicensed;

	S32		dlcUnlockId;
};

struct AmaxGameDataObj : AmaxDLCBase
{
	XtSz8	tag;
	XtId	ID;
	int		index;
	XtLocId loc_string;
};

template <typename GD> class AmaxGameData
{
public:
	const GD operator [](GAMEDATA_ID id) const
	{
		for(int a=0;a<mData.Size();a++)
		{
			if(mData[a]->ID == (XtId)id)
			{
				return mData[a];
			}
		}

		return NULL;
	}
	const GD operator [](const char *str) const
	{
		for(int a=0;a<mData.Size();a++)
		{
			if(!strcmp(mData[a]->tag,str))
			{
				return mData[a];
			}
		}

		return NULL;
	}

	const inline GD operator [](int a) const
	{
		return mData[a];
	}


	GD FindIDFromIndex(int index)
	{
		for(int a=0;a<mData.Size();a++)
		{
			if (mData[a]->index == index)
				return mData[a];
		}
		return NULL;
	}


	inline void Add(GD data)
	{
		mData.PushBack(data);
	}
	inline void Destroy(GD data)
	{
		for(int a=0;a<mData.Size();a++)
		{
			if(mData[a] == data)
			{
				mData.EraseByIndex(a);
				return;
			}
		}
	}
	inline void Destroy(GAMEDATA_ID id)
	{
		for(int a=0;a<mData.Size();a++)
		{
			if(mData[a]->ID == (U32)id)
			{
				mData.EraseByIndex(a);
				return;
			}
		}

		//assert(!"Could no Erase ID in AmaxGameData");
	}
	inline void DestroyAll()
	{
		for(int a=0;a<mData.Size();a++)
		{
			mData.EraseByIndex(a);
		}
	}

	inline int Number()
	{
		return mData.Size();
	}

	inline void Init()
	{
		mData.Clear();
	}

	void Dump()
	{
		//  LOGDEBUG( gGameLogicDataLogger, "Loaded %d Objects\n", mData.Size());
		//  for(int a=0;a<mData.Size();a++)
		//  {
		//if(mSortedData.Size())
		//	LOGDEBUG( gGameLogicDataLogger,"%d: %d %s\n",a,mData[mSortedData[a]]->ID,mData[mSortedData[a]]->tag);
		//else
		//	LOGDEBUG( gGameLogicDataLogger,"%d: %d %s\n",a,mData[a]->ID,mData[a]->tag);


		//  }
	}

	void DumpEvent()
	{
		//LOGDEBUG( gGameLogicDataLogger, "Dumping events...errrggghhhh!!!! Loaded %d Objects\n", mData.Size());
		//   for(int a=0;a<mData.Size();a++)
		//   {
		//	if(mSortedData.Size())
		//	{
		//		LOGDEBUG( gGameLogicDataLogger,"%d: %d %s %i %i\n",a,mData[mSortedData[a]]->ID,mData[mSortedData[a]]->tag, mData[mSortedData[a]]->racemode_params->laps, mData[mSortedData[a]]->racemode_params->time_limit);
		//	}
		//	else
		//		LOGDEBUG( gGameLogicDataLogger,"%d: %d %s\n",a,mData[a]->ID,mData[a]->tag);


		//   }
	}

	void AlphaSort()
	{
		mSortedData.Clear();
		mSortedData.Resize(mData.Size());

		for(int a=0;a<mSortedData.Size();a++)
		{
			mSortedData[a]=a;
		}

		bool sorting=true;
		while(sorting)
		{
			sorting=false;

			for(int a=0;a<mSortedData.Size()-1;a++)
			{
				WCHAR name1[128];
				WCHAR name2[128];

				Amax::C_SafeString::WideStringPrintF(name1, lengthof(name1), L"%s",UI::C_TextStringID(((AmaxGameDataObj *)mData[mSortedData[a]])->loc_string.id).String());
				Amax::C_SafeString::WideStringPrintF(name2, lengthof(name2), L"%s",UI::C_TextStringID(((AmaxGameDataObj *)mData[mSortedData[a+1]])->loc_string.id).String());

				int b=0;

				while(name1[b] && name2[b] && (tolower(name1[b]) == tolower(name2[b])))
				{
					b++;

					if(name2[b] == NULL)
					{
						int temp=mSortedData[a];
						mSortedData[a]=mSortedData[a+1];
						mSortedData[a+1]=temp;

						sorting=true;

						if(!wcscmp(name1,name2))
						{
							sorting=false;
							//LOGWARN( gGameLogicDataLogger,"%S == %S\n",name1,name2);
							//assert(!"There are two names the same in Sorcery");
						}
					}
				}

				if(name1[b] && name2[b] && (tolower(name1[b]) > tolower(name2[b])))
				{
					int temp=mSortedData[a];
					mSortedData[a]=mSortedData[a+1];
					mSortedData[a+1]=temp;

					sorting=true;

					if(!wcscmp(name1,name2))
					{
						sorting=false;
						//LOGWARN( gGameLogicDataLogger,"%S == %S\n",name1,name2);
						//assert(!"There are two names the same in Sorcery");
					}
				}
			}
		}
	}

	void IndexSort()
	{
		mSortedData.Clear();
		mSortedData.Resize(mData.Size());

		for(int a=0;a<mSortedData.Size();a++)
		{
			mSortedData[a]=a;
		}

		bool sorting=true;
		while(sorting)
		{
			sorting=false;

			for(int a=0;a<mSortedData.Size()-1;a++)
			{
				if(((AmaxGameDataObj *)mData[mSortedData[a]])->index > ((AmaxGameDataObj *)mData[mSortedData[a+1]])->index)
				{
					int temp=mSortedData[a];
					mSortedData[a]=mSortedData[a+1];
					mSortedData[a+1]=temp;

					sorting=true;
				}
			}
		}
	}

	void TagSort()
	{
		mSortedData.Clear();
		mSortedData.Resize(mData.Size());

		for(int a=0;a<mSortedData.Size();a++)
		{
			mSortedData[a]=a;
		}

		bool sorting=true;
		while(sorting)
		{
			sorting=false;

			for(int a=0;a<mSortedData.Size()-1;a++)
			{
				char name1[128];
				char name2[128];

				Amax::C_SafeString::StringPrintF(name1, lengthof(name1), "%s",((AmaxGameDataObj *)mData[mSortedData[a]])->tag.string);
				Amax::C_SafeString::StringPrintF(name2, lengthof(name2), "%s",((AmaxGameDataObj *)mData[mSortedData[a+1]])->tag.string);

				int b=0;

				while(name1[b] && name2[b] && (tolower(name1[b]) == tolower(name2[b])))
				{
					b++;

					if(name2[b] == NULL)
					{
						int temp=mSortedData[a];
						mSortedData[a]=mSortedData[a+1];
						mSortedData[a+1]=temp;

						sorting=true;

						if(!strcmp(name1,name2))
						{
							sorting=false;
							//LOGWARN( gGameLogicDataLogger,"%S == %S\n",name1,name2);
							//assert(!"There are two names the same in Sorcery");
						}
					}
				}

				if(name1[b] && name2[b] && (tolower(name1[b]) > tolower(name2[b])))
				{
					int temp=mSortedData[a];
					mSortedData[a]=mSortedData[a+1];
					mSortedData[a+1]=temp;

					sorting=true;

					if(!strcmp(name1,name2))
					{
						sorting=false;
						//LOGWARN( gGameLogicDataLogger,"%S == %S\n",name1,name2);
						//assert(!"There are two names the same in Sorcery");
					}
				}
			}
		}
	}

	Ut::Vector<int> mSortedData;

private:

	Ut::Vector<GD> mData;
};



#endif // GAME_DATA_DEFS_H
