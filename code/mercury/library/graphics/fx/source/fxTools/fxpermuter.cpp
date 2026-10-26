#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <set>
#include <windows.h>

#include "static_strings.h"
#include "fxfile.h"
#include "fxpermuter.h"

static hString hash_sampler("sampler");
static hString hash_texture("texture");
static hString hash_permute("permute");
static hString hash_pDefault("pDefault");
static hString hash_pDepend("pDepend");
static hString hash_pValueExpr("pValueExpr");
static hString hash_pDepDef("pDepDef");
static hString hash_pDepFalseDef("pDepFalseDef");
static hString hash_pDepTrueDef("pDepTrueDef");
static hString hash_pDef("pDef");
static hString hash_pFalseDef("pFalseDef");
static hString hash_pTrueDef("pTrueDef");

void FxPermuter::Init(const string &szFile,map<string,string> *pDefines,list<string> *pIncludePaths)
{
	if(isInited)
		return;

	if(pDefines) {
		defines = *pDefines;
	} else {
		defines.clear();
	}

	if(pIncludePaths) {
		includes = *pIncludePaths;
	} else {
		includes.clear();
	}

	defines["PERMUTING"] = "1";

	FxFile::Init(szFile,&defines,&includes);
	nPermVars = 0;
	for(size_t i=0;i<varList.size();i++) {
		if(varList[i]->FindAnnoVar(hash_permute))
			nPermVars++;
	}
}


void 
FxPermuter::WriteFX(FILE *pFile,const string permName,Params *pParams)
{
	Permutation &perm = MakePermutation(permName,false);

	Params params;
	if(pParams == NULL) {
		pParams = &params;
	}
	size_t idxNow = 0;
	vector<Var*>::iterator varIter;
	for(varIter = varList.begin();varIter != varList.end();varIter++) {
		Var *pVar = *varIter;
		map<hString,AnnoVar>::iterator semIter = pVar->annovars.find(hash_permute); 

		if(	semIter != pVar->annovars.end() && semIter->second.type == hash_bool && semIter->second.value == hash_true) {
#if 0
			map<hString,Var>::iterator permVarIter = perm.vars.find(pVar->name);
			if(permVarIter != perm.vars.end()) {
				deque<Tok>::iterator tokIter;
				for(tokIter= permVarIter->second.begin;tokIter != permVarIter->second.end;tokIter++) {
					tokIter->output = false;
				}
				permVarIter->second.end->output = false;
			}
#endif

			char c = permName[idxNow++];
			if(c == 'x')
				c = '0';
			pParams->Set(pVar->name,c-'0');
		}
	}

	perm.WriteFX(pFile,pParams);
}


string FxPermuter::GetPermName(const Params &Param)
{
	Params paramCopy = Param;
	string permName;
	permName.reserve(64);

	vector<Var*>::iterator varIter;
	for(varIter = varList.begin();varIter != varList.end();varIter++) {
		Var *pVar = *varIter;
		map<hString,AnnoVar>::iterator semIter = pVar->annovars.find(hash_pDefault);
		if(semIter != pVar->annovars.end()) {
			if(paramCopy.GetParams().find(pVar->name) != paramCopy.GetParams().end())
				continue;
			paramCopy.Set(pVar->name,semIter->second.Eval(paramCopy));
		}
	}

	for(varIter = varList.begin();varIter != varList.end();varIter++) {
		Var *pVar = *varIter;
		map<hString,AnnoVar>::iterator annoIter = pVar->annovars.find(hash_permute);
		if(	annoIter != pVar->annovars.end() && annoIter->second.type == hash_bool && annoIter->second.value == hash_true) {
			double param = paramCopy[pVar->name];
			bool depend = true;

			annoIter = pVar->annovars.find(hash_pDepend);
			if(annoIter != pVar->annovars.end()) {
				depend = (annoIter->second.Eval(paramCopy) != 0.0);
				if(!depend) {
					paramCopy.Set(pVar->name,0.0);
				}
			}

			if(depend) {
				double value = param;

				annoIter = pVar->annovars.find(hash_pValueExpr);
				if(annoIter != pVar->annovars.end()) {
					value = annoIter->second.Eval(paramCopy);
				}

				char szResult[256];
				sprintf(szResult,"%d",long(value));
				permName += szResult;
			} else {
				permName += "x";
			}
		}
	}

	return permName;
}

FxPermuter::Permutation &FxPermuter::MakePermutation(string uberName,bool bCompileShaders)
{
	map<string,Permutation>::iterator iter;
	Permutation *pRet = NULL;
	iter = Permutations.find(uberName);
	if(iter == Permutations.end()) {
		Params params;

		map<string,string> defines(this->defines);
		vector<Var*>::iterator varIter;


		size_t idx=0;
		for(varIter = varList.begin();varIter != varList.end();varIter++) {
			Var *pVar = *varIter;
			map<hString,AnnoVar>::iterator annoIter = pVar->annovars.find(hash_permute);
			if(	annoIter != pVar->annovars.end() && annoIter->second.type == hash_bool && annoIter->second.value == hash_true) {
				char c = uberName[idx++];
				annoIter = pVar->annovars.end();
				if(c == 'x') {
					c = '0';
				} else {
					if(c == '0') {
						annoIter = pVar->annovars.find(hash_pDepFalseDef);
					} else {
						annoIter = pVar->annovars.find(hash_pDepTrueDef);
					}
					if(annoIter == pVar->annovars.end()) {
						annoIter = pVar->annovars.find(hash_pDepDef);
					}

					if(annoIter != pVar->annovars.end()) {
						hString str = annoIter->second.value;
						char szValue[1024];
						strcpy(szValue,str.c_str());
						char *szDef = strtok(szValue,",");
						do {
							defines[szDef] = c;
							szDef = strtok(0,",");
						} while(szDef);
					}
				}

				if(c == '0') {
					annoIter = pVar->annovars.find(hash_pFalseDef);
				} else {
					annoIter = pVar->annovars.find(hash_pTrueDef);
				}
				if(annoIter == pVar->annovars.end()) {
					annoIter = pVar->annovars.find(hash_pDef);
				}

				if(annoIter != pVar->annovars.end()) {
					hString str = annoIter->second.value;
					char szValue[1024];
					strcpy(szValue,annoIter->second.value.c_str());
					char *szDef = strtok(szValue,",");
					do {
						defines[szDef] = c;
						szDef = strtok(0,",");
					} while(szDef);
				}

				params.Set(pVar->name,c-'0');
			}
		}
		Permutation &perm = Permutations[uberName];
		perm.Init(fileName.c_str(),&defines);

		pRet = &perm;
	} else {
		pRet = &(iter->second);
	}

#if 0
	if(bCompileShaders && !pRet->bCompiled) {
		pRet->bCompiled = true;
		char tempName[1024];
		GetTempFileName(getenv("temp"),"perm",0,tempName);

		WriteFX(tempName,uberName);

		list<TechniqueDesc>::iterator techIter;
		for(techIter = pRet->techDescs.begin();techIter != pRet->techDescs.end();techIter++) {
			pRet->techs.push_back(Technique());
			Technique &tech = pRet->techs.back();
			tech.name = techIter->name;
			list<PassDesc>::iterator passIter;
			for(passIter = techIter->passes.begin();passIter != techIter->passes.end();passIter++) {
				tech.passes.push_back(Pass());

				Pass &pass = tech.passes.back();
				pass.name = passIter->name;

				DXCompileVertexShader(tempName,passIter->VertexEntry.c_str(),&pass.VertexShader);
				DXCompilePixelShader(tempName,passIter->PixelEntry.c_str(),&pass.PixelShader);
			}
		}
		_unlink(tempName);
	}
#endif

	return *pRet;
}

void FxPermuter::Permutation::Init(const string &szFile,map<string,string> *pDefines)
{
	FxFile::Init(szFile,pDefines);
	vector<Var*>::const_iterator iter;
	for(iter = varList.begin();iter!=varList.end();iter++) {
		Var *pVar = *iter;
		if(	*(pVar->begin) == hash_sampler )
			continue;
		vector<Tok*>::const_iterator tokIter;
		for(tokIter = pVar->body.begin();tokIter != pVar->body.end();tokIter++) {
			Tok *pTok = *tokIter;
			if(pTok->depth != 1 || *pTok != hash_texture)
				continue;
			tokIter++;
			tokIter++;
			tokIter++;
			varRename[**tokIter] = pVar->name;
		}
	}
}
