using namespace std;

class FxPermuter: public FxFile {
public:
#if 0
	class Shader {
	public:
		Shader(): pShader(NULL),nShaderBytes(0) {}
		~Shader() { if(pShader) free(pShader); }
		void			*pShader;
		size_t			nShaderBytes;
		set<hString>	Uses;
		bool operator[](const hString & str) const {
			return Uses.find(str) != Uses.end();
		}
	};

	class Pass {
	public:
		bool operator[](const hString & str) const {
			return PixelShader[str] || VertexShader[str];
		}
		hString name;
		Shader	VertexShader;
		Shader	PixelShader;
	};

	class Technique {
	public:
		bool operator[](const hString & str) const {
			list<Pass>::const_iterator iter;
			for(iter = passes.begin();iter != passes.end();iter++) {
				if((*iter)[str])
					return true;
			}
			return false;
		};
		hString	name;
		list<Pass> passes;
	};
#endif
	class Permutation: public FxFile {
	public:
		Permutation(): bCompiled(false) {}
#if 0
		bool operator[](hString str) const {
			map<hString,hString>::const_iterator renIter = varRename.find(str);
			if(renIter != varRename.end())
				str = renIter->second;

			list<Technique>::const_iterator iter;
			for(iter = techs.begin();iter != techs.end();iter++) {
				if((*iter)[str])
					return true;
			}
			return false;
		}
		list<Technique>	techs;
#endif
		map<hString,hString>	varRename;
		bool bCompiled;

		void Init(const string &szFile,map<string,string> *pDefines = NULL);

		void WriteFX(FILE *pFile,Params *pParams) {
			FxFile::WriteFX(pFile,pParams);
		}
		void WriteFX(const char *szFile,Params *pParams) {
			FxFile::WriteFX(szFile,pParams);
		}
	};

private:
	map<string,Permutation>	Permutations;
	Permutation &MakePermutation(string permName,bool bCompileShaders);
public:
	int nPermVars;
	map<string,string> defines;
	list<string> includes;
	void Init(const string &szFile,map<string,string> *pDefines = NULL ,list<string> *pIncludePaths = NULL);

public:
	string GetPermName(const Params &Param);
	void WriteFX(const char *szFile,const string permName)
	{
		FILE *pFile = fopen(szFile,"w");
		WriteFX(pFile,permName);
		fclose(pFile);
	}
	void WriteFX(FILE *pFile,const string permName,Params *pParams=NULL);
	Permutation &GetPermutation(string permName) {
		return MakePermutation(permName,true);
	}
};

