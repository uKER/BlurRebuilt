#ifndef XT_IMPL_XTDOM_H
#define XT_IMPL_XTDOM_H

#ifdef _MSC_VER
#pragma once
#endif

struct ExportSchema;
struct LarvaSchema;
struct LarvaType;
struct LarvaField;
struct LtStruct;

namespace Xt
{

	// Xt::Storage

	class Storage
	{
	public:
		virtual bool New(const char* filename) = 0;
		virtual bool Dirty(const char* filename) = 0;
		virtual bool Rename(const char* oldFilename, const char* newFilename) = 0;
		virtual bool Remove(const char* filename) = 0;
	};

	// Xt::StatusMessage

	struct StatusMessage
	{
	public:
		enum Severity
		{
			kInfo,
			kWarning,
			kError,
			kTrace
		};

	public:
		Severity	severity;
		Sz8			message;

	public:
		StatusMessage(Severity severity_, const Sz8& message_) :
			severity(severity_),
			message(message_)
		{
		}
	};

	// Xt::StatusCallback

	class StatusCallback
	{
	public:
		virtual ~StatusCallback();

	public:
		virtual void Status(const StatusMessage& msg);
		virtual void Progress(float progress);

	public:
		virtual bool IsCancelled();

	public:
		static void Info(StatusCallback* status, const char* msg, ...);
		static void Warning(StatusCallback* status, const char* msg, ...);
		static void Error(StatusCallback* status, const char* msg, ...);
		static void Trace(StatusCallback* status, const char* msg, ...);
		static void Progress(StatusCallback* status, float progress);
	};

	// Xt::DomKind

	enum DomKind
	{
		kDomField,
		kDomRef,
		kDomBlock,
		kDomArray,
		kDomComponent,
		kDomRecord,
		kDomPack,
		kDomDb
	};

	// Xt::DomElement

	struct DomMonitor;

	struct DomElement
	{
		DomKind						kind;
		DomElement*					parent;
		XtId						parentId;
		XtString					parentKey;
		XtUt::LinkList<DomMonitor>	monitors;

	public:
		DomElement(DomKind kind_);
		virtual ~DomElement();

	private:
		XT_DECLARE_PRIVATE_COPY_CONSTRUCTORS(DomElement)
	};

	// Xt::DomField

	struct DomField : public DomElement
	{
		XtString value;

	public:
		DomField();
		virtual ~DomField();
	};

	// Xt::DomRef

	struct DomRef : public DomElement
	{
		XtId value;

	public:
		DomRef();
		virtual ~DomRef();
	};

	// Xt::DomBlock

	struct DomBlock : public DomElement
	{
		XtUt::Dictionary<XtString, DomElement> elements;

	public:
		typedef struct DomBlockIterator Iterator;

	public:
		DomBlock();
		virtual ~DomBlock();
	};

	// Xt::DomArray

	struct DomArray : public DomElement
	{
		XtUt::Dictionary<XtId, DomElement>	elements;
		XtUt::Array<XtId>					index;

	public:
		typedef struct DomArrayElementIterator	ElementIterator;
		typedef struct DomArrayIndexIterator	IndexIterator;

	public:
		DomArray();
		virtual ~DomArray();
	};

	// Xt::DomComponent

	struct DomComponent : public DomElement
	{
		XtId		id;
		XtString	type;
		DomBlock	data;

	public:
		DomComponent();
		virtual ~DomComponent();
	};

	// Xt::DomRecord

	struct DomRecord : public DomElement
	{
		XtId			id;
		XtString		name;
		DomComponent	root;
		bool			dirty;

		XtUt::Dictionary<XtId, XtString> summaryCache;

	public:
		DomRecord();
		virtual ~DomRecord();
	};

	// Xt::DomPack

	struct DomPack : public DomElement
	{
		XtId								id;
		XtString							name;
		DomBlock							properties;
		XtUt::Dictionary<XtId, DomRecord>	records;
		XtString							filename;
		XtFileTime							filetime;
		bool								dirty;

	public:
		typedef struct DomPackIterator Iterator;

	public:
		DomPack();
		virtual ~DomPack();
	};

	// Xt::BuildStructure

	struct BuildStructure
	{
		XtString packs;
		XtString localisation;
	};

	// Xt::BuildPlatform

	struct BuildPlatform
	{
		XtString name;
		XtEndian endian;

	public:
		BuildPlatform() : endian(kXtBigEndian) { }
	};

	// Xt::BuildStage

	struct BuildStage
	{
		XtString		name;
		BuildPlatform*	platform;
		XtString		root;

	public:
		BuildStage() : platform(0) { }
	};

	// Xt::BuildDeploy

	struct BuildDeploy
	{
		XtString	name;
		BuildStage*	stage;
		XtString	method;
		XtString	root;

	public:
		BuildDeploy() : stage(0) { }
	};

	// Xt::DomBuild

	struct DomBuild
	{
		BuildStructure				structure;
		XtUt::List<BuildPlatform>	platforms;
		XtUt::List<BuildStage>		staging;
		XtUt::List<BuildDeploy>		deployment;
	};

	// Xt::DomLanguage

	struct DomLanguage
	{
		XtString name;
		XtString code;
	};

	// Xt::DomDb

	struct DomDb : public DomElement
	{
		bool								open;
		XtString							path;
		XtString							name;
		XtUt::Dictionary<XtId, DomPack>		packs;
		XtUt::Dictionary<XtId, XtUt::Block>	pristine;
		LarvaSchema*						schema;
		ExportSchema*						exportSchema;
		bool								dirty;
		XtUt::List<DomLanguage>				languages;
		DomBuild							build;
		DomBlock							properties;
		XtUt::Array<XtString>				removedPacks;
		XtSimpleFile						lock;

	public:
		typedef struct DomDbIterator Iterator;

	public:
		DomDb();
		virtual ~DomDb();
	};

	// Xt::DomParseError

	struct DomParseError
	{
	public:
		enum Type
		{
			kOkay,
			kIdentifierExpected,
			kStringMissingEndQuote,
			kBadCharacterInIdentifier,
			kBadCharctersAfterString,
			kMalformedBlock,
			kMalformedValue,
			kMalformedRef,
			kMalformedArray,
			kMalformedEnd,
			kUnknownIdentifier,
			kNestingTooDeep,
			kUnexpectedEnd,
			kRecordUnclosed,
			kBlockUnclosed,
			kArrayUnclosed,
			kRecordOnlyValidAtRoot,
			kOnlyClassValidAsRoot,
			kOneClassPerFile,
			kOutOfMemory,
			kCannotNameArrayItems,
			kBlockMustHaveValidName,
			kRecordMustHaveValidName,
			kFieldMustHaveValidName,
			kRefMustHaveValidName,
			kArrayMustHaveValidName,
			kCouldntOpenFile,
			kErrorReadingFile,
			kNameTooLong,
			kUnresolvedConflict
		};

	public:
		Type	type;
		int		line;

	public:
		DomParseError(Type type_ = kOkay, int line_ = 0) : type(type_), line(line_) { }

	public:
		operator bool() const { return type == kOkay; }

	public:
		const char* AsText();
	};

	// Xt::EventCode

	enum DomEventCode
	{
		kDomRemoved,
		kDomFieldChanged,
		kDomRefChanged,
		kDomBlockChanged,
		kDomArrayChanged,
		kDomComponentChanged,
		kDomRecordChanged,
		kDomRecordParentChanged,
		kDomPackChanged,
		kDomPackParentChanged,
		kDomDbChanged,
		kDomDbOpened,
		kDomDbClosing
	};

	// Xt::Event

	struct DomEvent
	{
		DomEventCode	code;
		DomElement*		from;

	public:
		DomEvent(DomEventCode code_, DomElement* from_) : code(code_), from(from_) { }
	};

	struct DomRemoved : public DomEvent
	{
		DomRemoved(DomElement* el) : DomEvent(kDomRemoved, el) { }
	};

	struct DomFieldChanged : public DomEvent
	{
		DomFieldChanged(DomField* field) : DomEvent(kDomFieldChanged, field) { }
	};

	struct DomRefChanged : public DomEvent
	{
		DomRefChanged(DomRef* ref) : DomEvent(kDomRefChanged, ref) { }
	};

	struct DomBlockChanged : public DomEvent
	{
		DomElement*		child;
		const XtString&	key;

	public:
		DomBlockChanged(DomBlock* block, DomElement* child_, const XtString& key_) :
			DomEvent(kDomBlockChanged, block), child(child_), key(key_)
		{ }
	};

	struct DomArrayChanged : public DomEvent
	{
		XtId id;

	public:
		DomArrayChanged(DomArray* arr, XtId id_) : DomEvent(kDomArrayChanged, arr), id(id_) { }
	};

	struct DomComponentChanged : public DomEvent
	{
		DomComponentChanged(DomComponent* component) : DomEvent(kDomComponentChanged, component) { }
	};

	struct DomRecordChanged : public DomEvent
	{
		DomRecordChanged(DomRecord* record) : DomEvent(kDomRecordChanged, record) { }
	};

	struct DomPackChanged : public DomEvent
	{
		DomPackChanged(DomPack* pack) : DomEvent(kDomPackChanged, pack) { }
	};

	struct DomDbChanged : public DomEvent
	{
		DomDbChanged(DomDb* db) : DomEvent(kDomDbChanged, db) { }
	};

	struct DomDbOpened : public DomEvent
	{
		DomDbOpened(DomDb* db) : DomEvent(kDomDbOpened, db) { }
	};

	struct DomDbClosing : public DomEvent
	{
		DomDbClosing(DomDb* db) : DomEvent(kDomDbClosing, db) { }
	};

	// Xt::DomMonitor

	struct DomMonitor
	{
	public:
		typedef void (*Callback)(DomMonitor*, DomEvent*, void*);

	public:
		DomElement*						element;
		XtUt::Link<DomMonitor>			linkElement;
		Callback						callback;
		void*							context;
		XtUt::WeakAnchor<DomMonitor>	self;

	public:
		DomMonitor() :
			element(0),
			linkElement(this),
			callback(0),
			context(0),
			self(this)
		{
		}

		template<typename Type>
		DomMonitor(void (*callback_)(DomMonitor*, DomEvent*, Type*), Type* context_) :
			element(0),
			linkElement(this),
			callback((Callback)callback_),
			context(context_),
			self(this)
		{
		}

	public:
		template<typename Type>
		void SetCallback(void (*callback_)(DomMonitor*, DomEvent*, Type*), Type* context_)
		{
			callback	= (Callback)callback_;
			context		= context_;
		}

	public:
		DomElement*			operator->()		{ return element; }
		const DomElement*	operator->() const	{ return element; }

	public:
		operator DomElement*()				{ return element; }
		operator const DomElement*() const	{ return element; }

	public:
		DomElement* Element()				{ return element; }
		const DomElement* Element() const	{ return element; }

	XT_DECLARE_PRIVATE_COPY_CONSTRUCTORS(DomMonitor)
	};

	//
	// Xt::DomElement
	//

	void Notify(DomElement* element, DomEvent* event);
	void SetDirty(DomElement* element, bool forceNotify);

	// Xt::DomElement::As*

	inline DomField*		AsField(DomElement* element)		{ return element && (element->kind == kDomField) ? (DomField*)element : 0;			}
	inline DomRef*			AsRef(DomElement* element)			{ return element && (element->kind == kDomRef) ? (DomRef*)element : 0;				}
	inline DomBlock*		AsBlock(DomElement* element)		{ return element && (element->kind == kDomBlock) ? (DomBlock*)element : 0;			}
	inline DomArray*		AsArray(DomElement* element)		{ return element && (element->kind == kDomArray) ? (DomArray*)element : 0;			}
	inline DomComponent*	AsComponent(DomElement* element)	{ return element && (element->kind == kDomComponent) ? (DomComponent*)element : 0;	}
	inline DomRecord*		AsRecord(DomElement* element)		{ return element && (element->kind == kDomRecord) ? (DomRecord*)element : 0;		}
	inline DomPack*			AsPack(DomElement* element)			{ return element && (element->kind == kDomPack) ? (DomPack*)element : 0;			}
	inline DomDb*			AsDb(DomElement* element)			{ return element && (element->kind == kDomDb) ? (DomDb*)element : 0;				}
	inline const XtString&	AsText(DomElement* element)			{ return element && (element->kind == kDomField) ? ((DomField*)element)->value : kXtNullString; }
	inline XtId				AsId(DomElement* element)			{ return element && (element->kind == kDomRef) ? ((DomRef*)element)->value : 0;		}

	// Xt::DomElement::CanCompose

	inline bool CanCompose(DomKind kind)
		{ return (kind == kDomField) || (kind == kDomRef) || (kind == kDomBlock) || (kind == kDomArray) || (kind == kDomComponent); }

	inline bool CanCompose(DomElement* element)
		{ return element && CanCompose(element->kind); }

	//
	// Xt::DomField
	//

	bool Set(DomField* field, const XtString& value);
	bool Set(DomField* field, const char* value);

	//
	// Xt::DomRef
	//

	bool Set(DomRef* ref, XtId value);

	//
	// Xt::DomBlock
	//

	DomElement* Insert(DomBlock* block, const XtString& key, DomElement* element);
	DomElement* Replace(DomBlock* block, const XtString& key, DomElement* element);
	DomElement* Set(DomBlock* block, const XtString& key, DomElement* element);
	bool Remove(DomBlock* block, const XtString& key);
	void Clear(DomBlock* block);

	DomField* AcquireField(DomBlock* block, const XtString& key);
	DomRef* AcquireRef(DomBlock* block, const XtString& key);
	DomBlock* AcquireBlock(DomBlock* block, const XtString& key);
	DomArray* AcquireArray(DomBlock* block, const XtString& key);
	DomComponent* AcquireComponent(DomBlock* block, const XtString& key);

	DomField* Set(DomBlock* block, const XtString& key, const XtString& value);
	DomField* Set(DomBlock* block, const XtString& key, const char* value);
	DomRef* Set(DomBlock* block, const XtString& key, XtId id);

	// Xt::DomBlock::Count

	inline size_t Count(DomBlock* block)
		{ return block ? Count(&block->elements) : 0; }

	// Xt::DomBlock::At

	inline DomElement* At(DomBlock* block, const XtString& key)
		{ return block ? block->elements[key] : 0; }

	// Xt::DomBlock template overloads

	template<typename Type> inline Type* Insert(DomBlock* block, const XtString& key, Type* element)
		{ return static_cast<Type*>(Insert(block, key, static_cast<DomElement*>(element))); }

	template<typename Type> inline Type* Replace(DomBlock* block, const XtString& key, Type* element)
		{ return static_cast<Type*>(Replace(block, key, static_cast<DomElement*>(element))); }

	template<typename Type> inline Type* Set(DomBlock* block, const XtString& key, Type* element)
		{ return static_cast<Type*>(Set(block, key, static_cast<DomElement*>(element))); }

	//
	// Xt::DomArray
	//

	XtId GenerateUniqueArrayId(DomArray* array);
	XtId GenerateUniqueArrayIdForLoc(DomArray* array);

	DomElement* AppendWithId(DomArray* array, XtId id, DomElement* element);
	DomElement* Append(DomArray* array, DomElement* element);
	DomElement* Insert(DomArray* array, size_t before, DomElement* element);
	DomElement* ReplaceById(DomArray* array, XtId id, DomElement* element);
	DomElement* Replace(DomArray* array, size_t index, DomElement* element);
	DomElement* SetById(DomArray* array, XtId id, DomElement* element);
	bool RemoveById(DomArray* array, XtId id);
	bool Remove(DomArray* array, size_t index);
	void Clear(DomArray* array);
	bool Swap(DomArray* array, size_t i, size_t j);

	DomField* AcquireField(DomArray* array, int index);
	DomRef* AcquireRef(DomArray* array, int index);
	DomBlock* AcquireBlock(DomArray* array, int index);
	DomArray* AcquireArray(DomArray* array, int index);
	DomComponent* AcquireComponent(DomArray* array, int index);

	DomField* AcquireFieldById(DomArray* array, XtId id);
	DomRef* AcquireRefById(DomArray* array, XtId id);
	DomBlock* AcquireBlockById(DomArray* array, XtId id);
	DomArray* AcquireArrayById(DomArray* array, XtId id);
	DomComponent* AcquireComponentById(DomArray* array, XtId id);

	int IndexOf(DomArray* array, XtId id);

	// Xt::DomArray::Count

	inline size_t Count(DomArray* array)
		{ return array ? array->index.count : 0; }

	// Xt::DomArray::At

	inline DomElement* At(DomArray* array, size_t index)
		{ return (array && (index < array->index.count)) ? array->elements[array->index[index]] : 0; }

	// Xt::DomArray::FindById

	inline DomElement* FindById(DomArray* array, XtId id)
		{ return array ? array->elements[id] : 0; }

	// Xt::DomArray template overloads

	template<typename Type> inline Type* AppendWithId(DomArray* array, XtId id, Type* element)
		{ return static_cast<Type*>(AppendWithId(array, id, static_cast<DomElement*>(element))); }

	template<typename Type> inline Type* Append(DomArray* array, Type* element)
		{ return static_cast<Type*>(Append(array, static_cast<DomElement*>(element))); }

	template<typename Type> inline Type* Insert(DomArray* array, size_t before, Type* element)
		{ return static_cast<Type*>(Insert(array, before, static_cast<DomElement*>(element))); }

	template<typename Type> inline Type* ReplaceById(DomArray* array, XtId id, Type* element)
		{ return static_cast<Type*>(ReplaceById(array, id, static_cast<DomElement*>(element))); }

	template<typename Type> inline Type* Replace(DomArray* array, size_t index, Type* element)
		{ return static_cast<Type*>(Replace(array, index, static_cast<DomElement*>(element))); }

	//
	// Xt::DomComponent
	//

	DomComponent* ComponentOf(DomElement* element);
	DomBlock* AsComponentData(DomElement* element);
	XtId GenerateUniqueComponentId();
	bool SetId(DomComponent* component, XtId id);
	bool SetType(DomComponent* component, const XtString& type);

	//
	// Xt::DomRecord
	//

	DomRecord* RecordOf(DomElement* element);
	DomComponent* AsRecordRoot(DomElement* element);
	bool SetName(DomRecord* record, const XtString& name);
	void SetDirty(DomRecord* record, bool forceNotify);

	//
	// Xt::DomPack
	//

	DomPack* PackOf(DomElement* element);
	DomBlock* AsPackProperties(DomElement* element);
	bool SetName(DomPack* pack, const XtString& name);

	XtId GenerateUniqueRecordId(DomPack* pack);
	XtId GenerateUniqueRecordId(DomDb* db);

	DomRecord* NewRecord(DomPack* pack, const XtString& name, const XtString& type);
	DomRecord* AddRecord(DomPack* pack, DomRecord* record);
	bool RemoveRecord(DomPack* pack, XtId id);

	DomRecord* DuplicateRecord(LarvaSchema* schema, DomRecord* record);
	bool MoveRecord(DomRecord* record, DomPack* dstPack);

	void SetDirty(DomPack* pack, bool forceNotify);

	// Xt::DomPack::At

	inline DomRecord* At(DomPack* pack, XtId id)
		{ return pack ? pack->records[id] : 0; }

	//
	// Xt::DomDb
	//

	enum OpenResult
	{
		kOpenSuccess,
		kOpenFailed,
		kOpenLocked
	};

	DomDb* DbOf(DomElement* element);
	DomBlock* AsDbProperties(DomElement* element);
	void SetDirty(DomDb* db, bool forceNotify);

	OpenResult Open(DomDb* db, const XtString& path, Storage* storage, StatusCallback* status);
	bool Flush(DomDb* db, Storage* storage, StatusCallback* status);
	void Close(DomDb* db, Storage* storage, StatusCallback* status);
	void Free(DomDb* db);

	bool ReloadSchema(DomDb* db, StatusCallback* status);

	DomPack*	NewPack(DomDb* db, const XtString& name);
	bool		AddPack(DomDb* db, DomPack* pack);
	bool		RemovePack(DomDb* db, XtId id);

	bool RemoveRecord(DomDb* db, XtId id);

	// Xt::Dom helpers

	DomElement* Import2(const char* filename, XtFileTime* ft, StatusCallback* sc, XtUt::Block* pristine = 0);
	bool Import2(DomBlock* block, const char* filename, XtFileTime* ft, StatusCallback* sc);

	DomPack* ImportPack(const char* filename, StatusCallback* status, XtUt::Block* pristine = 0);
	bool ExportPack(DomPack* pack, const char* filename, XtUt::Block* pristine = 0);

	bool Export2(DomElement* element, const char* filename, XtUt::Block* pristine = 0);

	DomElement* Parse2(XtUt::Block* mb, StatusCallback* status, const char* filename);
	bool Parse2(DomBlock* block, XtUt::Block* mb, StatusCallback* status, const char* filename);

	DomPack* FindPackById(DomDb* db, XtId packId);
	DomRecord* FindRecordById(DomDb* db, XtId recordId);

	DomElement* FindElementByPath(DomElement* parent, const char* path);
	DomElement* AcquireElementByPath(DomElement* element, const char* path, DomKind elementKind);

	//
	// Xt::DomBlockIterator
	//

	struct DomBlockIterator : public XtUt::Dictionary<XtString, DomElement>::Iterator
	{
		DomBlockIterator(DomBlock* block) : XtUt::Dictionary<XtString, DomElement>::Iterator(&block->elements) { }
	};

	//
	// Xt::DomArrayElementIterator
	//

	struct DomArrayElementIterator : public XtUt::Dictionary<XtId, DomElement>::Iterator
	{
		DomArrayElementIterator(DomArray* array) : XtUt::Dictionary<XtId, DomElement>::Iterator(&array->elements) { }
	};

	//
	// Xt::DomArrayIndexIterator
	//

	struct DomArrayIndexIterator
	{
		DomArray*	array;
		size_t		index;

	public:
		DomArrayIndexIterator(DomArray* array_) : array(array_), index(0) { }

	public:
		operator bool()				{ return index < array->index.count; }
		operator DomElement*()		{ assert(index < array->index.count); return array->elements[array->index[index]]; }
		operator DomElement&()		{ assert(index < array->index.count); return *array->elements[array->index[index]]; }
		DomElement& operator*()		{ assert(index < array->index.count); return *array->elements[array->index[index]]; }
		DomElement* operator->()	{ assert(index < array->index.count); return array->elements[array->index[index]]; }

	public:
		size_t Index()	{ return index; }
		XtId Key()		{ assert(index < array->index.count); return array->index[index]; }

	public:
		void operator++() { index++; }
	};

	//
	// Xt::DomPackIterator
	//

	struct DomPackIterator : public XtUt::Dictionary<XtId, DomRecord>::Iterator
	{
		DomPackIterator(DomPack* pack) : XtUt::Dictionary<XtId, DomRecord>::Iterator(&pack->records) { }
	};

	//
	// Xt::DomDbIterator
	//

	struct DomDbIterator : public XtUt::Dictionary<XtId, DomPack>::Iterator
	{
		DomDbIterator(DomDb* db) : XtUt::Dictionary<XtId, DomPack>::Iterator(&db->packs) { }
	};

	//
	// Xt::DomMonitor
	//

	DomElement* Monitor(DomMonitor* monitor, DomElement* element);

	//
	// Xt::Clone
	//

	bool Clone(DomField* dst, DomField* src);
	bool Clone(DomRef* dst, DomRef* src);
	bool Clone(DomBlock* dst, DomBlock* src);
	bool Clone(DomArray* dst, DomArray* src);
	bool Clone(DomComponent* dst, DomComponent* src);

	DomElement* Clone(DomElement* element);

	//
	// Xt::DomBlock::Serialise
	//

	bool Serialise2(DomElement* element, XtString& data);

	//
	// Xt::Revert
	//

	bool Revert(DomRecord* record, DomPack* pristinePack);
	bool RevertToPristine(DomRecord* record);

	//
	// Xt::Bundle
	//

	struct BundleItem
	{
		XtString				type;
		XtString				field;
		XtUt::Ptr<DomElement>	element;
	};

	struct Bundle
	{
		XtString				source;
		XtUt::List<BundleItem>	elements;

	public:
		typedef struct BundleIterator Iterator;
	};

	struct BundleIterator : public XtUt::List<BundleItem>::Iterator
	{
		BundleIterator(Bundle* bundle) : XtUt::List<BundleItem>::Iterator(&bundle->elements) { }
	};

	BundleItem* Append(Bundle* bundle, BundleItem* item);
	BundleItem* Append(Bundle* bundle, LarvaField* field, DomElement* element);
	void Clear(Bundle* bundle);

	bool Serialise(Bundle* bundle, XtString& data);
	bool Parse(Bundle* bundle, XtUt::Block* mb, StatusCallback* status);

};

#endif // XT_IMPL_XTDOM_H
