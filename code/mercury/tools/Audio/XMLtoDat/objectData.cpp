#include "stdafx.h"
#include "objectdata.h"


//------------------------------------------------------------------------------
//  IsData
//------------------------------------------------------------------------------
bool ObjectDataFile::IsData(DWORD dwBlockID)
{
    if (IsString(dwBlockID))
    {
        if (strcmp(GetString(dwBlockID), "ScriptCode") == 0)
            return true;
        else
            return StringDataFile::IsData(dwBlockID);
    }
    else
        return StringDataFile::IsData(dwBlockID);
}
