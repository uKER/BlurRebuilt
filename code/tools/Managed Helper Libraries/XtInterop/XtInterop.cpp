// This is the main DLL file.

#include "stdafx.h"
#include "XtInterop.h"

class MyStatusCallback : public Xt::StatusCallback
{
public:
	MyStatusCallback() : _errorString(NULL)
	{
	}

	~MyStatusCallback()
	{
		if (_errorString)
			free((void*)_errorString);
	}

	virtual void Status(const Xt::StatusMessage& msg)
	{
		_errorString = _strdup(msg.message);
	}

	const char* ErrorString()
	{
		return _errorString;
	}

private:
	const char* _errorString;
};

class NullStorage : public Xt::Storage
{
public:
	virtual bool Start()
	{
		return true;
	}

	virtual bool New(const char* filename)
	{
		return true;
	}

	virtual bool Dirty(const char* filename)
	{
		return true;
	}

	virtual bool Rename(const char* oldFilename, const char* newFilename)
	{
		return true;
	}

	virtual bool Remove(const char* filename)
	{
		return true;
	}

	virtual bool IsModified(const char* filename)
	{
		return false;
	}

	virtual bool Stop()
	{
		return true;
	}
};

String^ ToManaged(const char* text)
{
	return Marshal::PtrToStringAnsi((IntPtr)(char*)text);
}

class ToNative
{
public:
	ToNative(String^ text)
	{
		_text = (char*)Marshal::StringToHGlobalAnsi(text).ToPointer();
	}

	~ToNative()
	{
		Marshal::FreeHGlobal(IntPtr(_text));
	}

public:
	operator XtString()
	{
		return XtString(_text);
	}

private:
	char* _text;
};

namespace XtInterop
{
	/*

	public class XtElement
	{
	}

	public class XtField : XtElement
	{
		string value;
	}

	public class XtBlock : XtElement
	{
		Dictionary<string, XtElement> els;
	}

	public class XtArray : XtElement
	{
		List<string, XtElement> els;
	}
	*/

	//
	// XtElement
	//

	public ref class XtElement
	{
	};

	//
	// XtField
	//

	public ref class XtField : public XtElement
	{
	public:
		property String^ Value
		{
			String^ get()
			{
				return _value;
			}

			void set(String^ value)
			{
				_value = value;
			}
		}

	internal:
		String^ _value;
	};

	//
	// XtRef
	//

	public ref class XtRef : public XtElement
	{
	public:
		property unsigned int Value
		{
			unsigned int get()
			{
				return _value;
			}

			void set(unsigned int value)
			{
				_value = value;
			}
		}

	internal:
		unsigned int _value;
	};

	//
	// XtBlock
	//

	public ref class XtBlock : public XtElement
	{
	public:
		XtBlock() :
			_elements(gcnew Dictionary<String^, XtElement^>)
		{
			
		}

	public:
		property XtElement^ default[String^]
		{
			XtElement^ get(String^ key)
			{
				return _elements[key];
			}
		}

		property Dictionary<String^, XtElement^>::KeyCollection^ Keys
		{
			Dictionary<String^, XtElement^>::KeyCollection^ get()
			{
				return _elements->Keys;
			}
		}

	internal:
		Dictionary<String^, XtElement^>^ _elements;
	};

	//
	// XtArray
	//

	public ref class XtArray : public XtElement
	{
	public:
		XtArray() :
			_elements(gcnew Dictionary<XtId, XtElement^>),
			_index(gcnew List<XtId>)
		{
		}

	public:
		property XtElement^ default[int]
		{
			XtElement^ get(int index)
			{
				return _elements[_index[index]];
			}
		}

		property int Count
		{
			int get()
			{
				return _elements->Count;
			}
		}

		unsigned int GetIdByIndex(int index)
		{
			return _index[index];
		}

	internal:
		Dictionary<XtId, XtElement^>^ _elements;
		List<XtId>^ _index;
	};

	//
	// XtComponent
	//

	public ref class XtComponent : public XtElement
	{
	public:
		XtComponent()
		{
		}

	public:
		property String^ Type
		{
			String^ get()
			{
				return _type;
			}

			void set(String^ newType)
			{
				_type = newType;
			}
		}

		property XtBlock^ Data
		{
			XtBlock^ get()
			{
				return _data;
			}

			void set(XtBlock^ data)
			{
				_data = data;
			}
		}

		property unsigned int Id
		{
			unsigned int get()
			{
				return _id;
			}

			void set(unsigned int id)
			{
				_id = id;
			}
		}

	internal:
		unsigned int _id;
		String^ _type;
		XtBlock^ _data;
	};

	//
	// Clone
	//

	XtField^ Clone(Xt::DomField* domField)
	{
		XtField^ field = gcnew XtField();

		field->_value = ToManaged(domField->value);

		return field;
	}

	XtRef^ Clone(Xt::DomRef* domRef)
	{
		XtRef^ ref = gcnew XtRef();

		ref->_value = domRef->value;

		return ref;
	}

	XtArray^ Clone(Xt::DomArray* domArray);
	XtComponent^ Clone(Xt::DomComponent* domComponent);

	XtBlock^ Clone(Xt::DomBlock* domBlock)
	{
		XtBlock^ block = gcnew XtBlock();

		for(Xt::DomBlock::Iterator it(domBlock); it; ++it)
		{
			XtElement^ v;

			if (Xt::DomField* childField = AsField(it))
				v = Clone(childField);
			else if (Xt::DomRef* childRef = AsRef(it))
				v = Clone(childRef);
			else if (Xt::DomBlock* childBlock = AsBlock(it))
				v = Clone(childBlock);
			else if (Xt::DomArray* childArray = AsArray(it))
				v = Clone(childArray);
			else if (Xt::DomComponent* childComponent = AsComponent(it))
				v = Clone(childComponent);
			else
				continue;

			block->_elements->Add(ToManaged(it.Key()), v);
		}

		return block;
	}
	
	XtArray^ Clone(Xt::DomArray* domArray)
	{
		XtArray^ arr = gcnew XtArray();

		for(Xt::DomArray::IndexIterator it(domArray); it; ++it)
		{
			XtElement^ v;

			if (Xt::DomField* childField = AsField(it))
				v = Clone(childField);
			else if (Xt::DomRef* childRef = AsRef(it))
				v = Clone(childRef);
			else if (Xt::DomBlock* childBlock = AsBlock(it))
				v = Clone(childBlock);
			else if (Xt::DomArray* childArray = AsArray(it))
				v = Clone(childArray);
			else if (Xt::DomComponent* childComponent = AsComponent(it))
				v = Clone(childComponent);
			else
				continue;

			arr->_elements->Add(it.Key(), v);
			arr->_index->Add(it.Key());
		}

		return arr;
	}

	XtComponent^ Clone(Xt::DomComponent* domComponent)
	{
		XtComponent^ component = gcnew XtComponent();

		component->Id = domComponent->id;
		component->Type = ToManaged(domComponent->type);
		component->Data = Clone(&domComponent->data);

		return component;
	}

	//
	// XtRecord
	//

	public ref class XtRecord
	{
	public:
		XtRecord(Xt::DomRecord* record)
		{
			_mon = new Xt::DomMonitor;

			Xt::Monitor(_mon, record);
		}

		~XtRecord()
		{
			delete _mon;
		}

	public:
		property XtId Id
		{
			XtId get()
			{
				Xt::DomRecord* record = AsRecord(*_mon);

				return record ? record->id : 0;
			}
		}

		property String^ Name
		{
			String^ get()
			{
				Xt::DomRecord* record = AsRecord(*_mon);

				return record ? ToManaged(record->name) : "";
			}

			void set(String^ newName)
			{
				if (Xt::DomRecord* record = AsRecord(*_mon))
					record->name = ToNative(newName);
			}
		}

		property String^ PackName
		{
			String^ get()
			{
				Xt::DomPack* pack = PackOf(*_mon);

				return pack ? ToManaged(pack->name) : "";
			}
		}

		property String^ Type
		{
			String^ get()
			{
				Xt::DomRecord* record = AsRecord(*_mon);

				return record ? ToManaged(record->root.type) : "";
			}

			void set(String^ newType)
			{
				if (Xt::DomRecord* record = AsRecord(*_mon))
					record->root.type = ToNative(newType);
			}
		}

		property XtComponent^ Root
		{
			XtComponent^ get()
			{
				Xt::DomRecord* record = AsRecord(*_mon);

				return Clone(&record->root);
			}
		}

	private:
		Xt::DomMonitor* _mon;
	};

	//
	// XtDom
	//

	public ref class XtDb
	{
	public:
		XtDb()
		{
			_db = new Xt::DomDb;
		}

		~XtDb()
		{
			delete _db;
		}

		bool Open(String^ path)
		{
			MyStatusCallback status;

			Xt::OpenResult result = Xt::Open(_db, ToNative(path), &NullStorage(), &status);

			if (result != Xt::kOpenSuccess)
			{
				_errorString = ToManaged(status.ErrorString());
			}
			else
			{
				_errorString = "";
			}

			return (result == Xt::kOpenSuccess);
		}

		String^ GetErrorString()
		{
			return _errorString;
		}

		property List<XtRecord^>^ Records
		{
			List<XtRecord^>^ get()
			{
				List<XtRecord^>^ list = gcnew List<XtRecord^>;

				for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator pit(&_db->packs); pit; ++pit)
				{
					for(XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator it(&pit->records); it; ++it)
					{
						XtRecord^ rec = gcnew XtRecord(it);

						list->Add(rec);
					}
				}

				return list;
			}
		}

	private:
		Xt::DomDb* _db;
		String^ _errorString;
	};
}
