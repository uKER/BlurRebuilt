#include <map>
#include <deque>
#include <list>

#include <Btl/Containers/vector.h>

#include "util.h"
#include "Eval.h"

namespace Ut
{
template<typename T>
class Vector;
}
namespace FX
{
struct Annotation;
}

using namespace std;

class Params {
	map<hString,double>	params;
public:
	Params() {}
	double operator[](const hString var) const {
		map<hString,double>::const_iterator iter = params.find(var);
		if(iter == params.end())
			return 0;
		return iter->second;
	}
	bool Defined(const hString var)
	{
		map<hString,double>::const_iterator iter = params.find(var);
		if(iter == params.end())
			return false;
		return true;
	}
	void Set(const hString var,double value) 
	{
		params[var] = value;
	}
	const map<hString,double>	&GetParams() const { return params;} 
};

class FxFile {
public:
	class Tok: public hString {
	public:
		enum Type {
			PRECOMP,
			QUOTE,
			CNAME,
			CNUM,
			COPERATOR,
			CBRACKET,
			WS,
		};
		int		depth;
		Type	type;
		bool	output;

		Tok(Type type,int depth,hString str): depth(depth), type(type),output(true),hString(str) {}
	};

	class AnnoVar {
	public:
		double Eval(const Params &param) {
			if(!expr.IsInited()) {
				if(!expr.Init(value)) {
					printf("Invalid expression: '%s'\n",value.c_str());
					throw("Invalid expression\n");
				}
			}
			return expr.Eval(&param.GetParams());
		}
		void Eval(Expr::ValueStack &st,const Params &param) {
			if(!expr.IsInited()) {
				if(!expr.Init(value)) {
					printf("Invalid expression: '%s'\n",value.c_str());
					throw("Invalid expression\n");
				}
			}
			return expr.Eval(st,&param.GetParams());
		}
		hString		type;
		hString		value;
		Expr::Expr	expr;
	};

	class States {
	public:
		States(): bStatesInited(false) {}
		bool bStatesInited;
		map<hString,vector<hString> > states;
		virtual void InitStates() = 0;
		const vector<hString> *FindState(hString hash_state) 
		{
			if(!bStatesInited)
				InitStates();
			map<hString,vector<hString> >::const_iterator iter = states.find(hash_state.LC());
			if(iter != states.end()) {
				return &iter->second;
			}
			return NULL;
		}
	};

	class Var: public States {
	public:
		void InitStates();
		hString					name;
		hString					type;
		vector<Tok*> 			decl;
		vector<Tok*> 			annotation;
		vector<Tok*> 			body;
		map<hString,AnnoVar>	annovars;
		deque<Tok>::iterator	begin;
		deque<Tok>::iterator	end;
	public:
		AnnoVar *FindAnnoVar(hString hash_anno) 
		{
			map<hString,AnnoVar>::iterator iter = annovars.find(hash_anno);
			if(iter == annovars.end())
				return NULL;
			return &iter->second;
		}
	};

	class Func {
	public:
		hString					name;
		vector<Tok*>			semantic;
		vector<Tok*> 			decl;
		vector<Tok*> 			arg;
		vector<Tok*> 			body;
		deque<Tok>::iterator	begin;
		deque<Tok>::iterator	end;
	};

	class PassDesc: public States {
	public:
		PassDesc() { bStatesInited = true; }
		void InitStates() {}
		hString name;
	};

	class MemberDesc
	{
	public:
		hString name;
		hString type;
		Btl::Vector<FX::Annotation> annotations;
	};

	class StructDesc
	{
	public:
		Var	*pVar;
		hString name;
		list<MemberDesc> members;
		Btl::Vector<FX::Annotation> annotations;
	};

	class TechniqueDesc {
	public:
		Var		*pVar;
		hString	name;
		list<PassDesc> passes; 
		PassDesc *FindPass(hString hash_pass)
		{
			list<FxFile::PassDesc>::const_iterator iter;
			for(iter = passes.begin();iter != passes.end();iter++) {
				if(iter->name == hash_pass) {
					return (PassDesc*)&(*iter);
				}
			}
			return NULL;
		}
	};
public:
	bool					isInited;
	string					name;
	string					fileName;
	list<TechniqueDesc>		techDescs;
	list<StructDesc>		structDescs;

	deque<Tok>				toks;
	vector<Var*>			varList;
	map<hString,Var>		vars;
	map<hString,Func>		funcs;

protected:
	void ProcessTechniques();
	void ProcessTechnique(Var *pVar);
	void ProcessAnnotations();
	void ProcessFunction(Func *pFunc);
	void ProcessStruct(Var *pVar);
	void ProcessStructs();

public:
	FxFile(): isInited(false) {}
	void Init(const string &szFile,map<string,string> *pDefines = NULL,list<string> *pIncludePaths = NULL);
	void WriteFX(FILE *pFile);
	void WriteFX(const char *szFile,Params *pParams)
	{
		FILE *pFile = fopen(szFile,"w");
		WriteFX(pFile,pParams);
		fclose(pFile);
	}
	void WriteFX(FILE *pFile,Params *pParams);

	Var *FindVar(hString hash_var)
	{
		map<hString,Var>::iterator iter = vars.find(hash_var);
		if(iter == vars.end())
			return NULL;
		return &iter->second;
	}
	Func *FindFunc(hString hash_func)
	{
		map<hString,Func>::iterator iter = funcs.find(hash_func);
		if(iter == funcs.end())
			return NULL;
		return &iter->second;
	}
	TechniqueDesc *FindTechnique(hString hash_tech)
	{
		list<TechniqueDesc>::const_iterator iter;
		for(iter = techDescs.begin();iter != techDescs.end();iter++) {
			if(iter->name == hash_tech) {
				return (TechniqueDesc*)&(*iter);
			}
		}
		return NULL;
	}
};


