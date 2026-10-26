class LocFormMdiWindow;

class CLocCSVParser
{
public:
	CLocCSVParser();
	~CLocCSVParser();

	bool LoadFile(const char* fileName);

	bool Parse(AsyncOp* op, LocFormMdiWindow* locForm);

protected:
	WCHAR NextChar();
	void SkipChar();
	WCHAR PeekChar();
	bool EndOfLine();
	bool EndOfFile();
	bool SkipToNextLine();

	XtString ReadParam();
	Xt::DomRecord* FindRecord(XtString recordType, XtString recordName);
	void ReadAndSet(Xt::DomBlock* block, XtString name, bool setCondition = true);
	void RemoveNullArrayEntries(Xt::DomArray* arr);

private:
	bool ReadFirstLine();
	bool ReadLine();

protected:
	bool					mUnicode;
	int						mBufferSize;
	BYTE*					mBuf;
	CHAR*					mBufPtr;
	WCHAR*					mBufPtrW;

	XtUt::Array<XtString>	mLanguages;

	AsyncOp*				mAsyncOp;
	LocFormMdiWindow*		mLocForm;
};
