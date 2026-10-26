#ifndef EDITOR_LIB_VERIFICATION_H
#define EDITOR_LIB_VERIFICATION_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{

enum E_VerificationResultDataType
{
	E_VerificationResultDataType_Actor,
	E_VerificationResultDataType_Xt,
	E_VerificationResultDataType_Position,
	E_VerificationResultDataType_Unknown,
};

class I_VerificationResultData
{
public:
	virtual E_VerificationResultDataType GetType() const { return E_VerificationResultDataType_Unknown; }
};

class C_VerificationResultDataActor : public I_VerificationResultData
{
public:
	C_VerificationResultDataActor(Xt::DomComponent* pComponent);

	virtual E_VerificationResultDataType GetType() const { return E_VerificationResultDataType_Actor; }

	Xt::DomComponent* GetXtData() const;
	Btl::String GetName() const;
	EditorLib::C_SceneNode* GetActor() const;

private:
	Xt::DomComponent* m_pComponent;
};

class C_VerificationResultDataXt : public I_VerificationResultData
{
public:
	C_VerificationResultDataXt(Xt::DomElement* pElement);

	virtual E_VerificationResultDataType GetType() const { return E_VerificationResultDataType_Xt; }

	Xt::DomElement* GetXtData() const;

private:
	Xt::DomElement* m_pElement;
};

class C_VerificationResultDataPosition : public I_VerificationResultData
{
public:
	C_VerificationResultDataPosition(const Maths::Vector3& position);

	virtual E_VerificationResultDataType GetType() const { return E_VerificationResultDataType_Position; }

	const Maths::Vector3& GetPosition() const;

private:
	Maths::Vector3 m_Position;
};

enum E_VerificationResultLevel
{
	E_VerificationResultLevel_Debug,
	E_VerificationResultLevel_Info,
	E_VerificationResultLevel_Warning,
	E_VerificationResultLevel_Error,
	E_VerificationResultLevel_Fatal,
};

class C_VerificationResult
{
public:
	C_VerificationResult(E_VerificationResultLevel errorLevel, const Btl::String& msg, LarvaField* field, Xt::DomElement* dom);
	C_VerificationResult(E_VerificationResultLevel errorLevel, const Btl::String& msg);
	~C_VerificationResult();

	Btl::String AsString() const;

	E_VerificationResultLevel GetErrorLevel() const;

	void AddData(I_VerificationResultData* data);
	const Btl::Vector<I_VerificationResultData*>& GetData() const;

private:
	void ParseRefLocation(Btl::String& location, Xt::DomElement* dom);

	Btl::Vector<I_VerificationResultData*> m_Data;
	
	E_VerificationResultLevel m_ErrorLevel;

	Btl::String m_Msg;
};

class C_VerificationRecord
{
public:
	C_VerificationRecord(XtId xtId);
	~C_VerificationRecord();

	XtId GetXtId() const;

	void SetName(const Btl::String& name);
	const Btl::String& GetName() const;

	void AddResult(C_VerificationResult* result);
	void PrintToLog(bool detailed=false);

	const Btl::Vector<C_VerificationResult*>& GetResults() const;

private:
	XtId m_XtId;
	Btl::String m_Name;
	Btl::Vector<C_VerificationResult*> m_Results;
};

class C_VerificationGroup
{
public:
	typedef Btl::OpenHashMap<XtId, C_VerificationRecord*> XtIdRecordHash;

	C_VerificationGroup();
	~C_VerificationGroup();

	void SetName(const Btl::String& name);
	const Btl::String& GetName() const;

	C_VerificationRecord& GetRecord(XtId xtId);
	const XtIdRecordHash& GetRecords() const;

	void PrintToLog(bool detailed=false);

private:
	Btl::String m_Name;
	XtIdRecordHash m_Records;
};

class C_VerificationSystem
{
public:
	typedef Btl::OpenHashMap<Btl::String, C_VerificationGroup*, Btl::HashKeyOpBtlString> StringGroupHash;

	~C_VerificationSystem();

	void RegisterCallback(Ut::Delegate<bool(void*, XtId)> pHandler, void* pData);

	void Clear();

	C_Status VerifyLevel(XtId level, bool clearPreviousResults=true);
	C_Status VerifyAllLevels();
	C_Status VerifyDatabase();

	C_VerificationGroup& GetGroup(const Btl::String& name);
	const StringGroupHash& GetGroups() const;
	void PrintToLog(bool detailed=false);

private:
	StringGroupHash m_Groups;

	Btl::List<Btl::Pair<Ut::Delegate<bool(void*, XtId)>, void*> > m_Callbacks;
};


}

#endif