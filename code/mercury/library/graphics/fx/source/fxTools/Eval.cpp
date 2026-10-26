#include "stdafx.h"

#include "fx/fxTools/eval.h"
#include <string>
#include <stack>
using namespace std;

namespace Expr {

struct Oper
{
	int				precedence;
	char			*szText;
	ExprFunc		*pEval;
};

class Command {
public:
	enum Type {
		VAL,
		VAR,
		OPER,
	};
	Type type;
	union {
		double		v;
		ExprFunc	*pOper;
		const char	*szVar;
	} data;
	Command() {};
	Command(double v): type(VAL) { data.v = v; }
	Command(const Oper &o): type(OPER) { data.pOper = o.pEval;}
	Command(hString var): type(VAR) { data.szVar = var.c_str(); }
};

double NotEval(ValueStack &st) 		{ return !st.pop(); }
double MulEval(ValueStack &st) 		{ return st.pop()*st.pop(); }
double DivEval(ValueStack &st) 		{ double b = st.pop(), a = st.pop(); return a/b; }
double ModEval(ValueStack &st) 		{ double b = st.pop(), a = st.pop(); return long(a)%long(b); }
double AddEval(ValueStack &st) 		{ double b = st.pop(), a = st.pop(); return a+b; }
double SubEval(ValueStack &st) 		{ double b = st.pop(), a = st.pop(); return a-b; }
double LTEval(ValueStack &st) 		{ double b = st.pop(), a = st.pop(); return a<b; }
double LTEEval(ValueStack &st) 		{ double b = st.pop(), a = st.pop(); return a<=b; }
double GTEval(ValueStack &st) 		{ double b = st.pop(), a = st.pop(); return a>b; }
double GTEEval(ValueStack &st) 		{ double b = st.pop(), a = st.pop(); return a>=b; }
double EQEval(ValueStack &st) 		{ double b = st.pop(), a = st.pop(); return a==b; }
double NEQEval(ValueStack &st) 		{ double b = st.pop(), a = st.pop(); return a!=b; }
double LANDEval(ValueStack &st) 	{ double b = st.pop(), a = st.pop(); return a&&b; }
double LOREval(ValueStack &st) 		{ double b = st.pop(), a = st.pop(); return a||b; }

static Oper aOpers[] = {
	{7,		"<=",	LTEEval	},
	{7,		">", 	GTEval 	},
	{7,		">=",	GTEEval	},
	{9,		"==",	EQEval 	},
	{9,		"!=",	NEQEval	},
	{13,	"&&",	LANDEval},
	{14,	"||",	LOREval	},

	{3,		"!", 	NotEval	},
	{5,		"*", 	MulEval	},
	{5,		"/", 	DivEval	},
	{5,		"%", 	ModEval	},
	{6,		"+", 	AddEval	},
	{6,		"-", 	SubEval	},
	{7,		"<", 	LTEval 	},
	{17,	",",	NULL	},
};
	
static Oper Bracket = {18,"(",NULL};

static map<string,Oper> funcs;
void AddFunction(const char *szName,ExprFunc *pFunc)
{
	Oper &fop = funcs[szName];
	fop.precedence=1;
	fop.szText = NULL;
	fop.pEval = pFunc;
}

bool Expr::Init(const hString expr)
{
	vector<Command> prog;
	stack<const Oper*> opers;
	size_t idx = 0;
	bool bLastOper = true;
	size_t strLen = strlen(expr.c_str());
	const char *szExpr = expr.c_str();
	while(idx < strLen) {
		if(szExpr[idx] == ' ')
		{
			idx++;
			continue;
		}
		
		if(szExpr[idx] == '(') {
			opers.push(&Bracket);
			idx++;
			bLastOper = true;
			goto next;
		}

		if(szExpr[idx] == ')') {
			while(opers.top() != &Bracket) {
				prog.push_back(Command(*opers.top()));
				opers.pop();
			}
			opers.pop();
			idx++;
			bLastOper = false;
			goto next;
		}

		if(bLastOper) {
			double v;
			int toklen;
			int nProcessed;
			if((nProcessed = sscanf(&szExpr[idx],"%lf%n",&v,&toklen)) == 1) {
				prog.push_back(Command(v));
				idx += toklen;
				bLastOper = false;
				goto next;
			} else {
				char c = szExpr[idx];
				if(isCName(c) || c == '.') {
					size_t idxStart = idx;
					char szVarName[1024];
					do {
						szVarName[idx-idxStart] = c;
						idx++;
						c = szExpr[idx];
					} while(isCNameNum(c) || c == '.');
					szVarName[idx-idxStart] = 0;

					if(strcmp(szVarName,"true") == NULL) {
						prog.push_back(Command(1.0));
					} else if(strcmp(szVarName,"false") == NULL) {
						prog.push_back(Command(0.0));
					} else {
						map<string,Oper>::const_iterator iter = funcs.find(szVarName);
						if(iter != funcs.end()) {
							opers.push(&iter->second);
						} else {
							prog.push_back(Command(hString(szVarName)));
						}
					}
					bLastOper = false;
					goto next;
				}
			}
		}
		for(size_t i=0;i<NUMELEMENTS(aOpers);i++) {
			Oper &oper = aOpers[i];
			if(oper.szText[0] == szExpr[idx] && (!oper.szText[1] || oper.szText[1] == szExpr[idx+1])) {
				while(!opers.empty() && opers.top()->precedence <= oper.precedence) {
					prog.push_back(Command(*opers.top()));
					opers.pop();
				}
				if(oper.pEval) {
					opers.push(&oper);
				}
				idx += oper.szText[1]?2:1;
				bLastOper = true;
				goto next;
			}
		}
		return false;
next:
		continue;
	}
	while(!opers.empty()) {
		prog.push_back(Command(*opers.top()));
		opers.pop();
	}

	delete []program;
	programSize = prog.size();
	program = new Command[programSize];

	for(size_t i=0;i<programSize;i++)
	{
		program[i] = prog[i];
	}

	return true;
}

void Expr::Eval(ValueStack &st,const map<hString,double> *vars)
{
	for(size_t i=0;i<programSize;i++) {
		Command &cmd = program[i];
		switch(cmd.type) {
		case Command::VAL:
			st.push(cmd.data.v);
			break;
		case Command::OPER:
			st.push(cmd.data.pOper(st));
			break;
		case Command::VAR:
			if(vars) {
				map<hString,double>::const_iterator iter = vars->find(hString(cmd.data.szVar,true));
				if(iter != vars->end()) {
					st.push(iter->second);
					break;
				} 
			}
			st.push(0);
			break;
		}
	}
}

Expr::~Expr()
{
	delete []program;
}

}
