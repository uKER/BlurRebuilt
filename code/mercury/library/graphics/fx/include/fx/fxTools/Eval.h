// Simple expression evaluator.
// Johan Kohler Feb-9-2007

#ifndef NUMELEMENTS
#define NUMELEMENTS(XXX) (sizeof(XXX)/sizeof((XXX)[0]))
#endif

#include "util.h"
#include <map>
#include <string>

namespace Expr {
	class ValueStack
	{
		double	st[1024];
		size_t	idx;
	public:
		ValueStack(): idx(0) {}
		double push(double v)
		{
			st[idx++] = v;
			if(idx > NUMELEMENTS(st))
				throw("Eval: Stack out of bounds\n");
			return v;
		}
		double pop() 
		{ 
			if(idx > NUMELEMENTS(st) || idx == 0)
				throw("Eval: Stack out of bounds\n");
			return st[--idx]; 
		}
		size_t size()
		{
			return idx;
		}
	};

	typedef double ExprFunc(ValueStack &st);

	void AddFunction(const char *szName,ExprFunc *pFunc);

	class Command;

	class Expr {
		Command *program;
		size_t	programSize;
	public:
		Expr():program(NULL),programSize(0){}
		~Expr();
		bool Init(const hString expr);
		bool IsInited() { return programSize != 0; }
		void Eval(ValueStack &st,const std::map<hString,double> *vars=NULL);
		double Eval(const std::map<hString,double> *vars=NULL)
		{
			ValueStack st;
			Eval(st,vars);
			if(st.size() != 1) {
				printf("Eval: Stack not empty after expr ran\n");
				throw("Eval: Stack not empty after expr ran\n");
			}
			return st.pop();
		}
	};
}
