#include "stdafx.h"

#include "fx/fxTools/util.h"

using namespace std;

string Path_GetFileName(string file)
{
	for(int i=0;i<(int)file.size();i++) {
		if(file[i] == '\\') {
			file[i] = '/';
		}
	}

	size_t nFind = file.rfind('/');
	if(nFind != file.npos) {
		file = file.substr(nFind+1);
	}

	nFind = file.rfind('.');
	if(nFind != file.npos) {
		file = file.substr(0,nFind);
	}
	return file;
}

string Path_GetPath(string file)
{
	for(int i=0;i<(int)file.size();i++) {
		if(file[i] == '\\') {
			file[i] = '/';
		}
	}

	size_t nFind = file.rfind('/');
	if(nFind != file.npos) {
		file = file.substr(0,nFind+1);
	} else {
		file = "";
	}

	return file;
}


#pragma init_seg(lib)
hString::hash2string_c hString::hash2string;

hString::hash2string_c::~hash2string_c() 
{
	std::map<unsigned long,vector<const char *> >::iterator iter;
	for(iter = begin();iter != end();iter++) {
		vector<const char *> &vec = iter->second;
		for(size_t i=0;i<vec.size();i++) {
			free((char *)(vec[i]-4));
		}
	}
}

const char *hString::hash2string_c::add(const char *szStr) {
	unsigned long hash = string_hash_sdbm(szStr);
	std::map<unsigned long,vector<const char *> >::iterator iter = find(hash);
	if(iter != end()) {
		vector<const char *> &vec = iter->second;
		for(size_t i=0;i<vec.size();i++) {
			if(strcmp(vec[i],szStr) == 0) {
				return vec[i];
			}
		}

		char *szRet = _strdup(szStr);
		vec.push_back(szRet);
		return szRet;
		
	}

	size_t nLen = strlen(szStr);

	char *szRet = ((char *)malloc(nLen+4+1))+4;
	bool bNotLC=false;
	for(size_t i=0;i<nLen+1;i++) {
		szRet[i] = makeLC(szStr[i]);
		if(szRet[i] != szStr[i])
			bNotLC = true;
	}
	if(bNotLC) {
		((const char **)szRet)[-1] = add(szRet);
	} else {
		((char **)szRet)[-1] = szRet;
	}
	strcpy(szRet,szStr);

	std::map<unsigned long,vector<const char *> >::insert(std::pair<unsigned long,vector<const char *> >(hash,vector<const char *>(1,szRet)));
	return szRet;
}
