#include "stdafx.h"

#include <windows.h>
#include <core/maths.h>
#include "fx/fxTools/static_strings.h"
#include "fx/fxTools/fxfile.h"
#include "fx/fx.h"
#include "fx/effect.h"
#include <algorithm>
#include <system/Perf/PerfEvent.h>

#define CPP "wave.exe"

static Debug::Logger &g_Log = Debug::GetLogger("graphics.fx.fxfile");

using namespace std;

class Stream {
	FILE *pFile;
	char szText[1024*10];
	char *szTextNow;
	char szPushBack[1024];
	char *szPushBackNow;
	int nLine;

	char internalGetC() 
	{ 
		if(szPushBackNow != szPushBack) {
			return *(--szPushBackNow);
		}

		if(*szTextNow) 
			return *szTextNow++;

		szTextNow = szText;

		char *szRead = fgets(szTextNow,sizeof(szText)-1,pFile);
		size_t nRead = 0;
		if(szRead)
			nRead = strlen(szRead);


		if(nRead)
			return *szTextNow++;

		return 0;
	}
public:
	Stream(FILE *_pFile): pFile(_pFile),szTextNow(szText),szPushBackNow(szPushBack) ,nLine(0)
	{
		szText[0] = 0;
	}

	char GetC()
	{
		char c = internalGetC();
		if(c == '\n') {
			nLine++;
		}
		return c;
	}

	int GetLine() 
	{
		return nLine;
	}

	void UnGetC(char c) 
	{
		if(c == 'n') {
			nLine--;
		}
		*szPushBackNow++ = c;
	}
	void Flush() {
		while(fread(szText,1,sizeof(szText),pFile) == sizeof(szText))
			;
	}
};

void
FxFile::WriteFX(FILE *pFile)
{
	deque<FxFile::Tok>::iterator tokIter;
	bool bSkippedPrev = false;
	bool bPrevWasNewLine = true;
	for(tokIter=toks.begin();tokIter != toks.end();tokIter++) {
		FxFile::Tok &tok = *tokIter;
		if(!tok.output) {
			// Keep the line numbers the same for compiling
			if(tok.c_str()[0] == '\n' || tok.type == Tok::PRECOMP) {
				fprintf(pFile,"%s",tok.c_str());
			}
			bSkippedPrev = true;
			continue;
		}

		if (bSkippedPrev && !bPrevWasNewLine)
		{
			fprintf(pFile, " "); // ensure we don't accidentally concatenate tokens
			bSkippedPrev = false;
		}

		// attempt to restore indentation (it was stripped out by the preprocessor)
		if (bPrevWasNewLine && tok.type != Tok::PRECOMP)
		{
			for (int n=0;n<tok.depth;n++)
				fprintf(pFile, "\t");
		}

		if(tok.type == Tok::QUOTE) {
			fprintf(pFile,"\"%s\"",tok.c_str());
		} else {
			fprintf(pFile,"%s",tok.c_str());
		}
		bPrevWasNewLine = (tok.c_str()[0] == '\n' || tok.type == Tok::PRECOMP);
	}
	fprintf(pFile,"\n");
}

void 
FxFile::WriteFX(FILE *pFile,Params *pParams)
{
	{
		deque<Tok>::iterator tokIter;
		for(tokIter = toks.begin();tokIter != toks.end();tokIter++) {
			tokIter->output = true;
		}
	}

	vector<Var*>::iterator varIter;
	for(varIter = varList.begin();varIter != varList.end();varIter++) {
		Var *pVar = *varIter;
		if(pParams->Defined(pVar->name))
		{
			deque<Tok>::iterator tokIter;
			for(tokIter= pVar->begin;tokIter != pVar->end;tokIter++) {
				tokIter->output = false;
			}
			pVar->end->output = false;
			double v = (*pParams)[pVar->name];
			fprintf(pFile,"#define %s %g\n",pVar->name.c_str(),v);
		}
	}

	WriteFX(pFile);
}

void
FxFile::ProcessAnnotations()
{
	vector<Var*>::iterator varIter;
	for(varIter = varList.begin();varIter != varList.end();varIter++) {
//		printf("Var: %s\n",(*varIter)->name.c_str());
		Var *pVar = *varIter;
		vector<Tok*>::iterator semIter;
		bool bDecl = true;
		for(semIter = pVar->annotation.begin();semIter != pVar->annotation.end();semIter++) {
			if(!bDecl) {
				if(**semIter == hash_semicolon /*";"*/)
					bDecl = true;
				continue;
			}
			bDecl = false;
			vector<Tok*>::iterator iter = semIter;

			const hString &type = **(iter++);
			if(iter == pVar->annotation.end())	break;

			const hString &name = **(iter++);
			if(iter == pVar->annotation.end())	break;

			if(**(iter++) != hash_equals)		continue;
			if(iter == pVar->annotation.end())	break;

			const hString &value = **(iter++);
			if(iter == pVar->annotation.end())	break;

			if(**(iter) != hash_semicolon)		continue;

			bDecl = true;
			semIter = iter;

			AnnoVar *pSem = &pVar->annovars[name];
			pSem->type = type;
			pSem->value = value;
//			printf("\t%s %s = %s\n",type.c_str(),name.c_str(),value.c_str());

			if(iter == pVar->annotation.end())	break;
		}
	}
}

void
FxFile::ProcessFunction(Func *pFunc)
{
	Tok *pDeclName = pFunc->arg.front();
	map<hString,Var>::iterator vertDec = vars.find(*pDeclName);
	if(vertDec == vars.end()) {
		return;
	}

	Var *pDecl = &vertDec->second;
	vector<Tok*>::iterator tokIter;
	for(tokIter = pDecl->body.begin();tokIter != pDecl->body.end();tokIter++) {
		if(**tokIter == hash_colon) {
			vector<Tok*>::iterator nameIter = tokIter;
			hString &name = **(--nameIter);
			hString &type = **(--nameIter);
			tokIter++;
			printf("\t\t%s %s:",name.c_str(),type.c_str());
			//hString &sema = **(tokIter++);
			for(;tokIter != pDecl->body.end() && **tokIter != hash_semicolon;tokIter++) {
				hString &prop = **(tokIter++);
				if((*tokIter)->type != Tok::CBRACKET || **tokIter != hash_bracket_start)
					break;
				tokIter++;
				hString &value = **(tokIter++);
				if((*tokIter)->type != Tok::CBRACKET || **tokIter != hash_bracket_end)
					break;
				tokIter++;
				printf("%s %s",prop.c_str(),value.c_str());
			}
			printf("\n");
		}
	}
}

void
FxFile::ProcessTechnique(Var *pVar)
{
	techDescs.push_back(TechniqueDesc());
	TechniqueDesc  &techDesc = techDescs.back();
	techDesc.pVar = pVar;
	techDesc.name = pVar->name;

//	printf("Technique: %s\n",pVar->name.c_str());

	vector<Tok*>::iterator bodyIter;
	PassDesc *pPass = NULL;
	for(bodyIter = pVar->body.begin();bodyIter != pVar->body.end();bodyIter++) {
		if((*bodyIter)->depth == 1 && **bodyIter == hash_pass) {
			techDesc.passes.push_back(PassDesc());
			pPass = &techDesc.passes.back();
			bodyIter++;
			pPass->name = **bodyIter;
		}
		
		if((*bodyIter)->depth == 2 && (*bodyIter)->type == Tok::COPERATOR && *(*bodyIter) == hash_equals) {
			vector<hString> &states = pPass->states[bodyIter[-1]->LC()];
			bodyIter++;
			while((*bodyIter)->type != Tok::COPERATOR || *(*bodyIter) != hash_semicolon) {
				states.push_back(**bodyIter);
				bodyIter++;
			}
		}
	}

}

void
FxFile::Var::InitStates()
{
	bStatesInited = true;
	vector<Tok*>::iterator bodyIter;
	if(body.begin() == body.end())
		return;
	for(bodyIter = ++body.begin();bodyIter != body.end();bodyIter++) {
		if((*bodyIter)->type == Tok::COPERATOR && *(*bodyIter) == hash_equals) {
			vector<hString> &states = this->states[bodyIter[-1]->LC()];
			bodyIter++;
			while((*bodyIter)->type != Tok::COPERATOR || *(*bodyIter) != hash_semicolon) {
				states.push_back(**bodyIter);
				bodyIter++;
			}
		}
	}
}

void
FxFile::ProcessTechniques()
{
	vector<Var*>::iterator varIter;

	for(varIter = varList.begin();varIter != varList.end();varIter++) {
		Var *pVar = *varIter;
		vector<Tok *>::iterator declIter;
		for(declIter = pVar->decl.begin();declIter != pVar->decl.end();declIter++) {
			if(**declIter == hash_technique) {
				ProcessTechnique(pVar);
				break;
			}
		}
	}
}

namespace FX
{
bool CharIsSpace(const char c)
{
	return c == ' ';
}
bool CharIsCharacter(const char c)
{
	return ((c >= 'A' && c <= 'Z') || 
				(c >= 'a' && c <= 'z') || 
				(c >= '0' && c <= '9') ||
				c == '=' || c == '.' || c == ':' || c == ',' ||
				c == '_');
}
bool StringIsNumber(const char *psz)
{
	while (*psz)
	{
		if (*psz == ' ' || (*psz>='0' && *psz<='9') || *psz == '.')
			psz++;
		else
			return false;
	}
	return true;
}

// parse annotations from a string and set the to a shader input semantic
void WriteInputSemanticAnnotations(Btl::Vector<Annotation> &Annotations, const char *pszString)
{
	int iMode = 0;
	const char *pszName;
	const char *pszNameEnd;
	const char *pszValue;
	const char *pszValueEnd;
	while (1)
	{
		if (*pszString == 0)
		{
			if (iMode == 1)
			{
				// name without value
				pszNameEnd = pszString;
				pszValue = pszNameEnd;
				pszValueEnd = pszNameEnd;
				iMode = 8;
			}
			else if (iMode != 8)
				break;
		}

		switch (iMode)
		{
		case 8:
			// add an annotation
			{
				char _pszName[1024];
				char _pszValue[1024];
				int n = pszNameEnd-pszName;
				memcpy(_pszName, pszName, n);
				_pszName[n] = 0;
				n = pszValueEnd-pszValue;
				memcpy(_pszValue, pszValue, n);
				_pszValue[n] = 0;

				Annotation item;
				item.Name = SharedString::GetString(_pszName);
				if (StringIsNumber(_pszValue))
				{
					item.Type = NGLFX_ANNOTATION_INT;
					item.Int = atoi(_pszValue);
				}
				else if (!stricmp(_pszValue, "true") || !stricmp(_pszValue, "false"))
				{
					item.Type = NGLFX_ANNOTATION_BOOL;
					item.Bool = stricmp(_pszValue, "true")==0;
				}
				else if (strlen(_pszValue) == 0)
				{
					item.Type = NGLFX_ANNOTATION_NONE;
				}
				else
				{
					item.Type = NGLFX_ANNOTATION_STRING;
					item.String = _strdup(_pszValue);
				}
				Annotations.PushBack(item);
				iMode++;
			}
			// fallthrough
		case 0:
		case 2:
		case 4:
		case 6:
		case 9:
			if (CharIsSpace(*pszString))
				pszString++;
			else
			{
				iMode++;
				if (iMode == 1)
					pszName = pszString;
				else if (iMode == 5)
					pszValue = pszString;
			}
			break;
		case 1: // name / value
		case 5:
			if (CharIsCharacter(*pszString) || CharIsSpace(*pszString))
				pszString++;
			else
			{
				if (iMode == 1)
					pszNameEnd = pszString;
				else if (iMode == 5)
					pszValueEnd = pszString;
				iMode++;
			}
			break;
		case 3:
			if (*pszString == '|')
			{
				// name without value
				pszValue = pszNameEnd;
				pszValueEnd = pszNameEnd;
				iMode = 8;
				break;
			}
			if (*pszString != '(')
			{
				printf("ERROR: expected '(' while parsing annotation string '%s'.\n", pszString);
				return;
			}
			pszString++;
			iMode++;
			break;
		case 7:
			if (*pszString != ')')
			{
				printf("ERROR: expected ')' while parsing annotation string '%s'.\n", pszString);
				return;
			}
			pszString++;
			iMode++;
			break;
		case 10:
			if (*pszString != '|')
			{
				printf("ERROR: expected '|' while parsing annotation string '%s'.\n", pszString);
				return;
			}
			pszString++;
			iMode = 0;
			break;
		}
	}
}
}

int DoesDeclHaveAnnotation(vector<FxFile::Tok*> &decl)
{
	for (unsigned int i=0; i<decl.size()-1; i++)
	{
		if (!stricmp(decl[i]->c_str(), "___VERTEX_DECLARATION_ANNOTATION___"))
			return i;
	}
	return -1;
}

void FxFile::ProcessStruct(Var *pVar)
{
	structDescs.push_back(StructDesc());
	StructDesc &desc = structDescs.back();
	desc.pVar = pVar;
	desc.name = pVar->name;

	// does the declaration have an annotation in?
	int i = DoesDeclHaveAnnotation(pVar->decl);
	if (i != -1)
	{
		// struct with annotation
		pVar->name = pVar->decl[1]->c_str();
		desc.name = pVar->name;
		const char *pszAnnotation = pVar->decl[i+1]->c_str();
		FX::WriteInputSemanticAnnotations(desc.annotations, pszAnnotation);
	}

	vector<Tok*>::iterator bodyIter;
	PassDesc *pPass = NULL;
	int iAnnotationMode = 0;
	for(bodyIter = pVar->body.begin();bodyIter != pVar->body.end();bodyIter++)
	{
		if ((*bodyIter)->depth != 1)
			continue;

		MemberDesc item;
		item.type = (*bodyIter)->c_str();
		bodyIter++;
		item.name = (*bodyIter)->c_str();
		bodyIter++;

		while(*(*bodyIter) != hash_semicolon && bodyIter != pVar->body.end())
		{
			if ((*bodyIter)->type != FxFile::Tok::WS)
			{
				switch(iAnnotationMode)
				{
				case 0:
					if (!stricmp((*bodyIter)->c_str(), "___VERTEX_DECLARATION_ANNOTATION___"))
						iAnnotationMode = 1;
					break;
				case 1:
					if ((*bodyIter)->type == FxFile::Tok::QUOTE)
					{
						const char *pszAnnotation = (*(bodyIter))->c_str();
						FX::WriteInputSemanticAnnotations(item.annotations, pszAnnotation);
						iAnnotationMode = 0;
					}
					break;
				}
			}

			bodyIter++;
		}

		desc.members.push_back(item);
	}
}

void
FxFile::ProcessStructs()
{
	vector<Var*>::iterator varIter;

	for(varIter = varList.begin();varIter != varList.end();varIter++) {
		Var *pVar = *varIter;
		vector<Tok *>::iterator declIter;
		for(declIter = pVar->decl.begin();declIter != pVar->decl.end();declIter++) {
			if(**declIter == hash_struct) {
				ProcessStruct(pVar);
				break;
			}
		}
	}
}

void FxFile::Init(const string &szFile,map<string,string> *pDefines,list<string> *pIncludePaths)
{
	PERF_EVENT("FxFile.Init");

	if(isInited)
		return;

//	printf("Processing: %s\n",szFile.c_str());
	isInited = true;
	fileName = szFile;

	name = Path_GetFileName(szFile);

	const char *pszVS80 = getenv("VS80COMNTOOLS");
	if (pszVS80 == NULL)
	{
		pszVS80 = getenv("VS90COMNTOOLS");
		if (pszVS80 == NULL)
		{
			printf( "VS90COMNTOOLS environment variable not found. This application requires Visual Studio 2008's C++ preprocessor to be installed.\n");
			exit(-1);
		}
	}

	const char* pszPATH = getenv("PATH");

	Util::C_String newPath;
	newPath.Printf("PATH=%s..\\ide;%s..\\..\\vc\\bin;%s", pszVS80, pszVS80, pszPATH);
	putenv(newPath.Chars());

	Util::C_String szTL = Util::C_String("\" \"") + pszVS80 + "..\\..\\vc\\bin\\cl.exe\" /nologo /E ";

	string cmd(szTL.Chars());

	if(pDefines) {
		map<string,string>::iterator iter;
		for(iter = pDefines->begin();iter != pDefines->end();iter++) {
			char szDef[1024];
			sprintf(szDef,"-D%s=%s ",iter->first.c_str(),iter->second.c_str());
			cmd += szDef;
		}
	}

	if(pIncludePaths) {
		list<string>::iterator iter;
		for(iter = pIncludePaths->begin();iter != pIncludePaths->end();iter++) {
			char szDef[1024];
			sprintf(szDef,"-I\"%s\" ",iter->c_str(),iter->c_str());
			cmd += szDef;
		}
	}

	cmd += szFile;
	cmd += "\"";

	{
	PERF_EVENT("FxFile.Main");

	std::string cmdOutput;
	FILE *pFile;
	{
		PERF_EVENT("FxFile._popen");
		pFile = _popen(cmd.c_str(),"rt");
		if (!pFile)
		{
			perror("Failed to spawn CL.EXE to preprocess .fx file");
			isInited = false;
			return;
		}
	}
	bool bFirstLine = true;
	Stream stream(pFile);
	int depth = 0;
	while(true) {
		char c = stream.GetC();

		if (bFirstLine)
			cmdOutput = cmdOutput + c;

		if(c == 0)
			break;

		// Skip lines beginning with #
		if(c == '#') {
			while(true) {
				char c = stream.GetC();
				if(c == 0 || c == '\n')
					break;
			}
			continue;
		}

		// String quotations.
		if(c == '"' || c == '\'') {
			std::string strTok;
			strTok.reserve(1024);
			while(true) {
				char ch = stream.GetC();
				if (bFirstLine)
					cmdOutput = cmdOutput + ch;
				if(ch == 0)
					break;

				if(ch == c) {
					break;
				}

				strTok += ch;

				if(ch == '\\') {
					char _c = stream.GetC();
					strTok += _c;
					if (bFirstLine)
						cmdOutput = cmdOutput + _c;
				}
			}
			Tok tok(Tok::QUOTE,depth,hString(strTok.c_str()));
			deque<Tok>::iterator iter;
			bool bAdd = true;
			for(iter = --toks.end();iter != toks.begin();--iter) {
				if(iter->type == Tok::WS)
					continue;
				if(iter->type == Tok::QUOTE) {
					string str(iter->c_str());
					str += tok.c_str();
					(hString)*iter = hString(str.c_str());
					bAdd = false;
				}
				break;
			}
			if(bAdd) {
				toks.push_back(tok);
			}
			continue;
		}

		if(isCBracketStart(c)) {
			Tok tok(Tok::CBRACKET,depth,hString(c));
			depth++;
			toks.push_back(tok);
			continue;
		}

		if(isCBracketEnd(c)) {
			depth--;
			Tok tok(Tok::CBRACKET,depth,hString(c));
			toks.push_back(tok);
			continue;
		}

		// COperator Text token
		if(isCOperator(c)) {
			Tok tok(Tok::COPERATOR,depth,hString(c));
			/*while(true){
				c = stream.GetC();
				if(!isCOperator(c)) {
					stream.UnGetC(c);
					break;
				}
				tok += c;
			}*/
			toks.push_back(tok);
			continue;
		}

		// CName Text token
		if(isCName(c)) {
			std::string strTok;
			strTok.reserve(1024);
			strTok += c;
			while(true){
				c = stream.GetC();
				if(!isCNameNum(c)) {
					stream.UnGetC(c);
					break;
				}
				if (bFirstLine)
					cmdOutput = cmdOutput + c;
				strTok += c;
			}
			Tok tok(Tok::CNAME,depth,hString(strTok.c_str()));
			toks.push_back(tok);
			continue;
		}

		// CNum Text token
		if(isCNum(c)) {
			std::string strTok;
			strTok.reserve(1024);
			strTok += c;
			while(true){
				c = stream.GetC();
				if(!isCNum(c)) {
					stream.UnGetC(c);
					break;
				}
				if (bFirstLine)
					cmdOutput = cmdOutput + c;
				strTok += c;
			}
			Tok tok(Tok::CNUM,depth,hString(strTok.c_str()));
			toks.push_back(tok);
			continue;
		}

		if(isWS(c)) {
			while(true){
				c = stream.GetC();
				if (c == '\n')
					bFirstLine = false;
				if(!isWS(c)) {
					stream.UnGetC(c);
					break;
				}
			}
			Tok tok(Tok::WS,depth,hString(" "));
			toks.push_back(tok);
			continue;
		}
		throw("Unknown character in file");
	}
	stream.Flush();
	int errcode = _pclose(pFile);
	if (errcode)
	{
		printf("ERROR: Command '%s' returned error code %d.\n", cmd.c_str(), errcode);
		printf("%s", cmdOutput.c_str());
		isInited = false;
		return;
	}

	}

	deque<Tok>::iterator tokIter;
	deque<Tok>::iterator startIter = toks.end();

	enum NameSpace {
		DECL,
		VAR,
		VAR_ANNOTATION,
		VAR_SEMANTIC,
		FUNC_ARGS,
		FUNC_BODY,
	};
	NameSpace ns=DECL;
	Var *pVar=NULL;
	Func *pFunc=NULL;
	vector<Tok*> decl;
	bool bVarHasOperator = false;

	{
	PERF_EVENT("FxFile.TokIter");

	for(tokIter = toks.begin();tokIter != toks.end();tokIter++) {
		if(tokIter->type == Tok::WS || tokIter->type == Tok::PRECOMP)
			continue;

		bool bAgain = true;
		while(bAgain) {
			bAgain = false;
			switch(ns) {
			case DECL:
				if(startIter == toks.end()) {
					startIter = tokIter;
				}

				if(tokIter->type == Tok::CNAME || tokIter->type == Tok::QUOTE) {
					decl.push_back(&*tokIter);
				}

				if(decl.empty()) {
					break;
				}

				if(tokIter->type == Tok::CBRACKET) {
					if(*tokIter == hash_bracket_start) {
//												LOGDEBUG( g_Log, "Func: %s\n",(*decl.back()).c_str());

						pFunc = &funcs[*decl.back()];
						pFunc->name = *decl.back();
						pFunc->decl = decl;
						pFunc->begin = startIter;
						startIter = toks.end();
						decl.clear();

						ns = FUNC_ARGS;
						break;
					}
				}
				if(tokIter->type == Tok::CBRACKET || tokIter->type == Tok::COPERATOR) {
//										LOGDEBUG( g_Log, "Var: %s\n",(*decl.back()).c_str());

					hString varname = *decl.back();
					bool bAnnotation = DoesDeclHaveAnnotation(decl)!=-1;
					if (bAnnotation)
					{
						varname = *decl[1];
					}

					pVar = &vars[varname];
					pVar->name = varname;

					if(decl.size() >= 2)
					{
						if (!bAnnotation)
							pVar->type = *decl[decl.size()-2];
						else
							pVar->type = *decl[0];
					}
					else
					{
						pVar->name = hString();
					}
					pVar->decl = decl;
					pVar->begin = startIter;
					startIter = toks.end();
					decl.clear();
					varList.push_back(pVar);

					ns = VAR;
					bVarHasOperator = false;
					bAgain = true;
				}
				break;
			case VAR:
				if(!tokIter->depth && tokIter->type == Tok::COPERATOR) {
					if(*tokIter == hash_semicolon) {
						pVar->end = tokIter+1;
						pVar = NULL;
						ns = DECL;
						break;
					}
					if(*tokIter == hash_less) {
						ns = VAR_ANNOTATION;
						break;
					}
					if(*tokIter == hash_colon) {
						ns = VAR_SEMANTIC;
						break;
					}
					if(tokIter->type == Tok::COPERATOR) {
						bVarHasOperator = true;
					}
				}
				pVar->body.push_back(&*tokIter);
				if(tokIter->depth == 0 && !bVarHasOperator && *tokIter == hash_braces_end) {
					tokIter++;
					pVar->end = tokIter;
					pVar = NULL;
					ns = DECL;
					bAgain = true;
					break;
				}
				break;
			case VAR_SEMANTIC:
				if(tokIter->type == Tok::COPERATOR && *tokIter == hash_equals) {
					ns = VAR;
					tokIter--;
					break;
				}
				if(tokIter->type == Tok::COPERATOR && *tokIter == hash_less) {
					ns = VAR_ANNOTATION;
				}
				if(tokIter->type == Tok::COPERATOR && *tokIter == hash_semicolon) {
					pVar->end = tokIter+1;
					pVar = NULL;
					ns = DECL;
				}
				break;
			case VAR_ANNOTATION:
				if(tokIter->type == Tok::COPERATOR && *tokIter == hash_more) {
					ns = VAR;
					break;
				}
				pVar->annotation.push_back(&*tokIter);
				break;
			case FUNC_ARGS:
				if(tokIter->depth) {
					pFunc->arg.push_back(&*tokIter);
					break;
				} else {
					if(tokIter->type == Tok::COPERATOR && *tokIter == hash_colon) {
						pFunc->semantic.push_back(&*tokIter);
						do {
							tokIter++;
						} while(tokIter->type == Tok::WS);
						pFunc->semantic.push_back(&*tokIter);
					}
				}
				if(tokIter->Tok::CBRACKET && *tokIter == hash_braces_start) {
					ns = FUNC_BODY;
				} 
				break;
			case FUNC_BODY:
				if(tokIter->depth) {
					pFunc->body.push_back(&*tokIter);
					break;
				}
				pFunc->body.push_back(&*tokIter);
				ns = DECL;
				pFunc->end = tokIter+1;
				pFunc = NULL;
				break;
			}
		}
	}
	}
	
	{
		PERF_EVENT("FxFile.Finish");

		ProcessAnnotations();
		ProcessTechniques();
		ProcessStructs();
	}
}

