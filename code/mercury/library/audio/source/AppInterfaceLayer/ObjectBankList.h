#ifndef _OBJECTBANKLIST_H_
#define _OBJECTBANKLIST_H_

namespace Audio
{

typedef Btl::Vector<Btl::String> BankList;

struct C_ObjectBankList
{
public:
	C_ObjectBankList();
	~C_ObjectBankList();

	bool Load(const Sys::C_FilePath& strPath);						// load all wbl files in a specified path

	const BankList* Lookup(const Btl::String& strObjectName) const;

private:
	bool LoadBankList(const Sys::C_FilePath& strFname);
	void Add(const Btl::String& strFname, const Btl::String& str);
	
	void Clear();
	const Btl::String StripCarriageReturns(const Btl::String& strFname);

private:
	Btl::Map<StringHash, BankList> m_ObjectBankMap;
};

} // namespace Audio

#endif	//	#ifndef _OBJECTBANKLIST_H_
