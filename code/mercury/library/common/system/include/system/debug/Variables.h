#ifndef SYSTEM_DEBUG_VARIABLES_H
#define SYSTEM_DEBUG_VARIABLES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/utility/coredebug.h>

//
// ** Welcome to the debug variables header file! **
//

// Prevent warning spam

#if defined(PLATFORM_PC)
#pragma warning(disable:4355)	// This used in initialiser list
#endif

#if defined(CFG_GOLD) || defined(PLATFORM_SPU)
#define DISABLE_DEBUG_VARIABLES
#endif

// Game must set these if it wants to save/load debug variables

typedef void (*DebugVarFileCallback)();

extern DebugVarFileCallback gGameLoadDebugVariables;
extern DebugVarFileCallback gGameSaveDebugVariables;

namespace Debug
{
	//
	// Network interface - To be moved
	//

	void NetworkStart();
	void NetworkUpdate();

	//
	// Forward declares
	//

	class Iterator;
	class Description;
	class Persist;
	class Dynamic;
	class Notify;
	class RangeInt;
	class RangeFloat;
	class StepInt;
	class StepFloat;

	//
	// VarType
	//

	enum VarType
	{
		kVtRoot		= 0x0100,
		kVtBool		= 0x0200,
		kVtInt		= 0x0300,
		kVtFloat	= 0x0400,
		kVtEnum		= 0x0500,
		kVtString	= 0x0600,
		kVtFunction	= 0x0700,
		kVtVector2	= 0x0800,
		kVtVector3	= 0x0900,
		kVtVector4	= 0x0A00,
		kVtColour	= 0x0B00
	};

	//
	// VarFlags
	//

	const int kVfRange		= 0x0001;
	const int kVfStep		= 0x0002;
	const int kVfDirty		= 0x0004;
	const int kVfPersist	= 0x0008;
	const int kVfHide		= 0x0010;
	const int kVfDynamic	= 0x0020;

	//
	// Option
	//

	struct Option
	{
		int			value;
		const char*	name;
	};

	template<typename EnumType>
	struct OptionList
	{
		static Option options[];
	};

	//
	// Miscellaneous structures
	//

	struct VectorStruct2
	{
		float vx, vy;

	public:
		VectorStruct2() : vx(0.0f), vy(0.0f) { }
		VectorStruct2(float vx_, float vy_) : vx(vx_), vy(vy_) { }

	public:
		bool operator!=(const VectorStruct2& rhs) const { return (vx != rhs.vx) || (vy != rhs.vy); }
	};

	struct VectorStruct3
	{
		float vx, vy, vz;

	public:
		VectorStruct3() : vx(0.0f), vy(0.0f), vz(0.0f) { }
		VectorStruct3(float vx_, float vy_, float vz_) : vx(vx_), vy(vy_), vz(vz_) { }

	public:
		bool operator!=(const VectorStruct3& rhs) const { return (vx != rhs.vx) || (vy != rhs.vy) || (vz != rhs.vz); }
	};

	struct VectorStruct4
	{
		float vx, vy, vz, vw;

	public:
		VectorStruct4() : vx(0.0f), vy(0.0f), vz(0.0f), vw(0.0f) { }
		VectorStruct4(float vx_, float vy_, float vz_, float vw_) : vx(vx_), vy(vy_), vz(vz_), vw(vw_) { }

	public:
		bool operator!=(const VectorStruct4& rhs) const { return (vx != rhs.vx) || (vy != rhs.vy) || (vz != rhs.vz) || (vw != rhs.vw); }
	};

	struct ColourStruct
	{
		float r, g, b, a;

	public:
		ColourStruct() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) { }
		ColourStruct(float r_, float g_, float b_, float a_) : r(r_), g(g_), b(b_), a(a_) { }

	public:
		bool operator!=(const ColourStruct& rhs) const { return (r != rhs.r) || (g != rhs.g) || (b != rhs.b) || (a != rhs.a); }
	};

	//
	// Helper functions
	//

	int StringCompare(const char* lhs, const char* rhs);

	//
	// Notification
	//

	typedef void (*NotifyCallback)(class VarBase*, void*);

	//
	// VarBase
	//

	class VarBase
	{
	public:
		VarBase(const char* name, VarType type);
		virtual ~VarBase();

	public:
		VarType Type() const			{ return (VarType)(_flags & 0xFF00);	}
		const char* Name() const		{ return _name;							}
		const char* Description() const	{ return _description;					}
		int Flags() const				{ return _flags;						}
		int Revision() const			{ return _revision;						}

	public:
		void SetDirty();
		void SetHide(bool hide = true);

	public:
		void DoNotify();

	protected:
		void Attach(VarBase* before);
		void Detach();

	protected:
		bool SetDynamic();
		virtual bool OnSetDynamic();

	protected:
		virtual void SetRange(int low, int high);
		virtual void SetRange(float low, float high);
		virtual void SetStep(int step);
		virtual void SetStep(float step);

	protected:
		VarBase* _next;
		VarBase* _prev;

	protected:
		const char*		_name;
		const char*		_description;
		int				_flags;
		int				_revision;
		NotifyCallback	_notifyCallback;
		void*			_notifyContext;

	friend class Iterator;
	friend class Description;
	friend class Persist;
	friend class Dynamic;
	friend class Notify;
	friend class RangeInt;
	friend class RangeFloat;
	friend class StepInt;
	friend class StepFloat;
	friend void NetworkUpdate();
	};

	//
	// Meta macros
	//

	#define DV_META_ARGS_2		const MetaBase& m00, const MetaBase& m01 = MetaBase()
	#define DV_META_ARGS_ND_2	const MetaBase& m00, const MetaBase& m01
	#define DV_META_APPLY_2()	do { m00.Apply(*this); m01.Apply(*this); } while(0)

	#define DV_META_ARGS_6		const MetaBase& m00, const MetaBase& m01, const MetaBase& m02, const MetaBase& m03 = MetaBase(), const MetaBase& m04 = MetaBase(), const MetaBase& m05 = MetaBase()
	#define DV_META_ARGS_ND_6	const MetaBase& m00, const MetaBase& m01, const MetaBase& m02, const MetaBase& m03, const MetaBase& m04, const MetaBase& m05
	#define DV_META_APPLY_6()	do { m00.Apply(*this); m01.Apply(*this); m02.Apply(*this); m03.Apply(*this); m04.Apply(*this); m05.Apply(*this); } while(0)

	//
	// Iterator
	//

	class Iterator
	{
	public:
		Iterator();

	public:
		operator VarBase*()					{ return _current; }
		operator VarBase&()					{ assert(_current != 0); return *_current;	}
		VarBase* operator->()				{ assert(_current != 0); return _current;	}
		const VarBase* operator->() const	{ assert(_current != 0); return _current;	}
		VarBase& operator*()				{ assert(_current != 0); return *_current;	}
		const VarBase& operator*() const	{ assert(_current != 0); return *_current;	}

		void operator++()
		{
			if ((_current = _current->_next)->Type() == kVtRoot)
				_current = 0;
		}

	private:
		VarBase* _current;
	};

	//
	// EnumBase
	//

	class EnumBase : public VarBase
	{
	public:
		EnumBase(const char* name, const Option* options);

	public:
		const Option* Options() const { return _options; }

	public:
		bool SetValueByName(const char* name);
		const char* ValueAsName() const;
		const char* DefaultAsName() const;

	public:
		void SetOptions(const Option* options);

	public:
		virtual void SetRawValue(int rawValue) = 0;
		virtual int RawValue() const = 0;
		virtual int RawDefault() const = 0;

	protected:
		const Option* _options;

	template<typename> friend class Enum;
	};

	//
	// OptionIterator
	//

	class OptionIterator
	{
	public:
		OptionIterator(const EnumBase* enumBase) : _current(enumBase->Options()) { }

	public:
		operator const Option*()			{ return _current->name ? _current : 0;				}
		const Option* operator->() const	{ assert(_current->name != 0); return _current;		}
		const Option& operator*() const		{ assert(_current->name != 0); return *_current;	}
		void operator++()					{ _current++; }

	private:
		const Option* _current;
	};

	//
	// MetaBase
	//

	class MetaBase
	{
	public:
		virtual void Apply(class VarBase& varBase) const;
		virtual ~MetaBase() { }
	};

	//
	// Bool
	//

	class Bool : public VarBase
	{
	public:
		Bool(const char* name, bool value);
		Bool(const char* name, bool value, DV_META_ARGS_2);
		Bool(const char* name, bool value, DV_META_ARGS_6);

	public:
		Bool& operator=(const Bool& rhs)	{ SetValue(rhs._value); return *this;	}
		bool operator=(bool value)			{ SetValue(value); return _value;		}
		operator bool() const				{ return _value; }

	public:
		bool Value() const		{ return _value;	}
		bool Default() const	{ return _default;	}

	public:
		void SetValue(bool value);

	private:
		bool _value;
		bool _default;
	};

	//
	// Int
	//

	class Int : public VarBase
	{
	public:
		Int(const char* name, int value);
		Int(const char* name, int value, DV_META_ARGS_2);
		Int(const char* name, int value, DV_META_ARGS_6);

	public:
		Int& operator=(const Int& rhs)	{ SetValue(rhs._value); return *this;	}
		int operator=(int value)		{ SetValue(value); return _value;		}
		operator int() const			{ return _value; }

	public:
		int Value() const	{ return _value;	}
		int Default() const	{ return _default;	}
		int Low() const		{ return _low;		}
		int High() const	{ return _high;		}
		int Step() const	{ return _step;		}

	public:
		void SetValue(int value);

	protected:
		virtual void SetRange(int low, int high);
		virtual void SetRange(float low, float high);
		virtual void SetStep(int step);
		virtual void SetStep(float step);

	private:
		int _value;
		int _default;
		int _low;
		int _high;
		int _step;
	};

	//
	// Float
	//

	class Float : public VarBase
	{
	public:
		Float(const char* name, float value);
		Float(const char* name, float value, DV_META_ARGS_2);
		Float(const char* name, float value, DV_META_ARGS_6);

	public:
		Float& operator=(const Float& rhs)	{ SetValue(rhs._value); return *this;	}
		float operator=(float value)		{ SetValue(value); return _value;		}
		operator float() const				{ return _value; }

	public:
		float Value() const		{ return _value;	}
		float Default() const	{ return _default;	}
		float Low() const		{ return _low;		}
		float High() const		{ return _high;		}
		float Step() const		{ return _step;		}

	public:
		void SetValue(float value);

	protected:
		virtual void SetRange(int low, int high);
		virtual void SetRange(float low, float high);
		virtual void SetStep(int step);
		virtual void SetStep(float step);

	private:
		float _value;
		float _default;
		float _low;
		float _high;
		float _step;
	};

	//
	// Enum
	//

	template<typename EnumType>
	class Enum : public EnumBase
	{
	public:
		Enum(const char* name, const EnumType& value) :
			EnumBase(name, OptionList<EnumType>::options),
			_value(value),
			_default(value)
		{
		}

		Enum(const char* name, const EnumType& value, DV_META_ARGS_2) :
			EnumBase(name, OptionList<EnumType>::options),
			_value(value),
			_default(value)
		{
			DV_META_APPLY_2();
		}

		Enum(const char* name, const EnumType& value, DV_META_ARGS_6) :
			EnumBase(name, OptionList<EnumType>::options),
			_value(value),
			_default(value)
		{
			DV_META_APPLY_6();
		}

	public:
		Enum<EnumType>& operator=(const Enum<EnumType>& rhs)	{ SetValue(rhs._value); return *this;	}
		const EnumType& operator=(const EnumType& value)		{ SetValue(value); return _value;		}
		operator const EnumType&() const						{ return _value; }

	public:
		const EnumType& Value() const	{ return _value;	}
		const EnumType& Default() const	{ return _default;	}

	public:
		void SetValue(const EnumType& value)
		{
			if (_value != value)
			{
				_value = value;
				SetDirty();
			}
		}

	public:
		virtual void SetRawValue(int rawValue)	{ SetValue((EnumType)rawValue);	}
		virtual int RawValue() const			{ return _value;	}
		virtual int RawDefault() const			{ return _default;	}

	private:
		EnumType _value;
		EnumType _default;
	};

	//
	// String
	//

	class String : public VarBase
	{
	public:
		String(const char* name, const char* value);
		String(const char* name, const char* value, DV_META_ARGS_2);
		String(const char* name, const char* value, DV_META_ARGS_6);
		virtual ~String();

	public:
		String& operator=(const String& rhs)		{ SetValue(rhs._value); return *this;	}
		const char* operator=(const char* value)	{ SetValue(value); return _value;		}
		operator const char*() const				{ return _value; }

	public:
		const char* Value() const	{ return _value;	}
		const char* Default() const	{ return _default;	}

	public:
		void SetValue(const char* value);

	protected:
		virtual bool OnSetDynamic();

	private:
		void SetImpl(const char* text);

	private:
		const char* _value;
		const char* _default;
	};

	//
	// Function
	//

	class Function : public VarBase
	{
	public:
		Function(const char* name, void (*callback)(void*), void* context);
		Function(const char* name, void (*callback)(void*), void* context, DV_META_ARGS_2);
		Function(const char* name, void (*callback)(void*), void* context, DV_META_ARGS_6);

	public:
		void Call();

	private:
		typedef void (*CallbackType)(void*);

	private:
		CallbackType	_callback;
		void*			_context;
	};

	//
	// Vector2
	//

	class Vector2 : public VarBase
	{
	public:
		Vector2(const char* name, float vx, float vy);
		Vector2(const char* name, float vx, float vy, DV_META_ARGS_2);
		Vector2(const char* name, float vx, float vy, DV_META_ARGS_6);

	public:
		Vector2& operator=(const Vector2& rhs)						{ SetValue(rhs._value); return *this; }
		const VectorStruct2& operator=(const VectorStruct2& value)	{ SetValue(value); return _value; }
		operator const VectorStruct2&() const						{ return _value; }
		const VectorStruct2* operator->() const						{ return &_value; }

	public:
		const VectorStruct2& Value() const		{ return _value; }
		const VectorStruct2& Default() const	{ return _default; }

	public:
		void SetValue(const VectorStruct2& value);
		void SetValue(float vx, float vy) { SetValue(VectorStruct2(vx, vy)); }

	private:
		VectorStruct2 _value;
		VectorStruct2 _default;
	};

	//
	// Vector3
	//

	class Vector3 : public VarBase
	{
	public:
		Vector3(const char* name, float vx, float vy, float vz);
		Vector3(const char* name, float vx, float vy, float vz, DV_META_ARGS_2);
		Vector3(const char* name, float vx, float vy, float vz, DV_META_ARGS_6);

	public:
		Vector3& operator=(const Vector3& rhs)						{ SetValue(rhs._value); return *this; }
		const VectorStruct3& operator=(const VectorStruct3& value)	{ SetValue(value); return _value; }
		operator const VectorStruct3&() const						{ return _value; }
		const VectorStruct3* operator->() const						{ return &_value; }

	public:
		const VectorStruct3& Value() const		{ return _value; }
		const VectorStruct3& Default() const	{ return _default; }

	public:
		void SetValue(const VectorStruct3& value);
		void SetValue(float vx, float vy, float vz) { SetValue(VectorStruct3(vx, vy, vz)); }

	private:
		VectorStruct3 _value;
		VectorStruct3 _default;
	};

	typedef Vector3 Vector; // Backwards compat.

	//
	// Vector4
	//

	class Vector4 : public VarBase
	{
	public:
		Vector4(const char* name, float vx, float vy, float vz, float vw);
		Vector4(const char* name, float vx, float vy, float vz, float vw, DV_META_ARGS_2);
		Vector4(const char* name, float vx, float vy, float vz, float vw, DV_META_ARGS_6);

	public:
		Vector4& operator=(const Vector4& rhs)						{ SetValue(rhs._value); return *this; }
		const VectorStruct4& operator=(const VectorStruct4& value)	{ SetValue(value); return _value; }
		operator const VectorStruct4&() const						{ return _value; }
		const VectorStruct4* operator->() const						{ return &_value; }

	public:
		const VectorStruct4& Value() const		{ return _value; }
		const VectorStruct4& Default() const	{ return _default; }

	public:
		void SetValue(const VectorStruct4& value);
		void SetValue(float vx, float vy, float vz, float vw) { SetValue(VectorStruct4(vx, vy, vz, vw)); }

	private:
		VectorStruct4 _value;
		VectorStruct4 _default;
	};

	//
	// Colour
	//

	class Colour : public VarBase
	{
	public:
		Colour(const char* name, float r, float g, float b, float a);
		Colour(const char* name, float r, float g, float b, float a, DV_META_ARGS_2);
		Colour(const char* name, float r, float g, float b, float a, DV_META_ARGS_6);

	public:
		Colour& operator=(const Colour& rhs)						{ SetValue(rhs._value); return *this;	}
		const ColourStruct& operator=(const ColourStruct& value)	{ SetValue(value); return _value;		}
		operator const ColourStruct&() const						{ return _value;	}
		const ColourStruct* operator->() const						{ return &_value;	}

	public:
		const ColourStruct& Value() const	{ return _value;	}
		const ColourStruct& Default() const	{ return _default;	}
		float Gamma() const					{ return _gamma;	}

	public:
		void SetValue(const ColourStruct& value);
		void SetValue(float r, float g, float b, float a)	{ SetValue(ColourStruct(r, g, b, a)); }
		void SetGamma(float gamma)							{ _gamma = gamma; }

	private:
		ColourStruct	_value;
		ColourStruct	_default;
		float			_gamma;
	};

	//
	// Description
	//

	class Description : public MetaBase
	{
	public:
		Description(const char* description) : _description(description) { }

	public:
		virtual void Apply(class VarBase& varBase) const;

	private:
		const char* _description;
	};

	//
	// Persist
	//

	class Persist : public MetaBase
	{
	public:
		virtual void Apply(class VarBase& varBase) const;
	};

	//
	// Dynamic
	//

	class Dynamic : public MetaBase
	{
	public:
		virtual void Apply(class VarBase& varBase) const;
	};

	//
	// Notify
	//

	class Notify : public MetaBase
	{
	public:
		Notify(NotifyCallback callback, void* context = 0) : _callback(callback), _context(context) { }

	public:
		virtual void Apply(class VarBase& varBase) const;

	private:
		NotifyCallback	_callback;
		void*			_context;
	};

	//
	// Range
	//

	class RangeInt : public MetaBase
	{
	public:
		RangeInt(int low, int high);

	public:
		virtual void Apply(class VarBase& varBase) const;

	private:
		int _low;
		int _high;
	};

	class RangeFloat : public MetaBase
	{
	public:
		RangeFloat(float low, float high);

	public:
		virtual void Apply(class VarBase& varBase) const;

	private:
		float _low;
		float _high;
	};

	inline RangeInt Range(int low, int high)		{ return RangeInt(low, high);	}
	inline RangeFloat Range(float low, float high)	{ return RangeFloat(low, high);	}

	//
	// Step
	//

	class StepInt : public MetaBase
	{
	public:
		StepInt(int step);

	public:
		virtual void Apply(class VarBase& varBase) const;

	private:
		int _step;
	};

	class StepFloat : public MetaBase
	{
	public:
		StepFloat(float step);

	public:
		virtual void Apply(class VarBase& varBase) const;

	private:
		float _step;
	};

	inline StepInt Step(int step)		{ return StepInt(step);		}
	inline StepFloat Step(float step)	{ return StepFloat(step);	}

	//
	// Gamma
	//

	class Gamma : public MetaBase
	{
	public:
		Gamma(float gamma = 2.2f) : _gamma(gamma) { }

	public:
		virtual void Apply(class VarBase& varBase) const;

	private:
		float _gamma;
	};

	//
	// Helper functions
	//

	VarBase* FindByName(const char* name);
	void Adjust(VarBase* varBase, int factor);
	void SetToDefault(VarBase* varBase);
	void SetToLow(VarBase* varBase);
	void SetToHigh(VarBase* varBase);
	bool IsDebugVariableNameValid(const char* name);

	//
	// DebugLog
	//

	enum Severity
	{
		kSeverityInherit,
		kSeverityCritical,
		kSeverityHigh,
		kSeverityNormal,
		kSeverityLow,
		kSeverityTrace
	};

	typedef void (*DebugLogCallback)(const char* name, Severity reportSeverity);	// Called when severity changes
	void RegisterDebugLogCallback(DebugLogCallback callback);						// Register callback for severity change
	bool RegisterDebugLogChannel(const char* name, Severity reportSeverity);		// Register channel under severity
	void DebugLogLn(const char* channel, Severity severity, const char* text);		// Output to channel with severity

	//
	// Console
	//

	struct ConsoleEvent
	{
		const char* name;
		const char* text;
	};

	typedef void (*ConsoleCallback)(ConsoleEvent* event, void* context);

	void ConsoleLn(const char* name, const char* text);
	void ConsolePrompt(const char* name, const char* prompt);
	void RegisterConsole(const char* name, ConsoleCallback callback, void* context);

	//
	// Debug variables network version
	//

	const unsigned int kNetVerServer0 = 0x00010000;
	const unsigned int kNetVerServer1 = 0x00020000;
	const unsigned int kNetVerServer2 = 0x00030000;

	const unsigned int kNetVerClient0 = 0x00010000;
};

//
// DEBUG_ENUM
//

#define DEBUG_ENUM(enumType)	template<> Debug::Option Debug::OptionList< enumType >::options[] = {
#define DE_OPTION(value, name)	{ (value), (name) },
#define END_DEBUG_ENUM			{ -1, 0 } };

// Debug variable auto remove in gold wrappers

#ifndef DISABLE_DEBUG_VARIABLES

	#define DECLARE_DEBUG_BOOL(var, name, value, ...)					Debug::Bool var((name), (value), ## __VA_ARGS__);
	#define DECLARE_DEBUG_INT(var, name, value, ...)					Debug::Int var((name), (value), ## __VA_ARGS__);
	#define DECLARE_DEBUG_FLOAT(var, name, value, ...)					Debug::Float var((name), (value), ## __VA_ARGS__);
	#define DECLARE_DEBUG_ENUM(type, var, name, value, ...)				Debug::Enum<type> var((name), (value), ## __VA_ARGS__);
	#define DECLARE_DEBUG_STRING(var, name, value, ...)					Debug::String var((name), (value), ## __VA_ARGS__);
	#define DECLARE_DEBUG_FUNCTION(var, name, callback, context, ...)	Debug::Function var((name), (callback), (context), ## __VA_ARGS__);
	#define DECLARE_DEBUG_VECTOR2(var, name, vx, vy, ...)				Debug::Vector2 var((name), (vx), (vy), ## __VA_ARGS__);
	#define DECLARE_DEBUG_VECTOR3(var, name, vx, vy, vz, ...)			Debug::Vector3 var((name), (vx), (vy), (vz), ## __VA_ARGS__);
	#define DECLARE_DEBUG_VECTOR4(var, name, vx, vy, vz, vw, ...)		Debug::Vector4 var((name), (vx), (vy), (vz), (vw), ## __VA_ARGS__);
	#define DECLARE_DEBUG_COLOUR(var, name, r, g, b, a, ...)			Debug::Colour var((name), (r), (g), (b), (a), ## __VA_ARGS__);

	#define EXTERN_DEBUG_BOOL(var)			extern Debug::Bool var;
	#define EXTERN_DEBUG_INT(var)			extern Debug::Int var;
	#define EXTERN_DEBUG_FLOAT(var)			extern Debug::Float var;
	#define EXTERN_DEBUG_ENUM(type, var)	extern Debug::Enum<type> var;
	#define EXTERN_DEBUG_STRING(var)		extern Debug::String var;
	#define EXTERN_DEBUG_FUNCTION(var)		extern Debug::Function var;
	#define EXTERN_DEBUG_VECTOR2(var)		extern Debug::Vector2 var;
	#define EXTERN_DEBUG_VECTOR3(var)		extern Debug::Vector3 var;
	#define EXTERN_DEBUG_VECTOR4(var)		extern Debug::Vector4 var;
	#define EXTERN_DEBUG_COLOUR(var)		extern Debug::Colour var;

#else // DISABLE_DEBUG_VARIABLES

	namespace Debug
	{
		class FunctionProxy
		{
		public:
			FunctionProxy(void (*callback)(void*), void* context) : _callback(callback), _context(context) { }

		public:
			void Call() { _callback(_context); }

		private:
			void (*_callback)(void*);
			void* _context;
		};

		class VectorProxy2
		{
		public:
			VectorProxy2(float vx, float vy) : _value(vx, vy) { }

		public:
			const VectorStruct2& operator=(const VectorStruct2& value) { _value = value; return _value; }
			operator const VectorStruct2&() const { return _value; }
			const VectorStruct2* operator->() const { return &_value; }

		private:
			VectorStruct2 _value;
		};

		class VectorProxy3
		{
		public:
			VectorProxy3(float vx, float vy, float vz) : _value(vx, vy, vz) { }

		public:
			const VectorStruct3& operator=(const VectorStruct3& value) { _value = value; return _value; }
			operator const VectorStruct3&() const { return _value; }
			const VectorStruct3* operator->() const { return &_value; }

		private:
			VectorStruct3 _value;
		};

		class VectorProxy4
		{
		public:
			VectorProxy4(float vx, float vy, float vz, float vw) : _value(vx, vy, vz, vw) { }

		public:
			const VectorStruct4& operator=(const VectorStruct4& value) { _value = value; return _value; }
			operator const VectorStruct4&() const { return _value; }
			const VectorStruct4* operator->() const { return &_value; }

		private:
			VectorStruct4 _value;
		};

		class ColourProxy
		{
		public:
			ColourProxy(float r, float g, float b, float a) : _value(r, g, b, a) { }

		public:
			const ColourStruct& operator=(const ColourStruct& value) { _value = value; return _value; }
			operator const ColourStruct&() const { return _value; }
			const ColourStruct* operator->() const { return &_value; }

		private:
			ColourStruct _value;
		};
	}

	#define DECLARE_DEBUG_BOOL(var, name, value, ...)					bool var = (value);
	#define DECLARE_DEBUG_INT(var, name, value, ...)					int var = (value);
	#define DECLARE_DEBUG_FLOAT(var, name, value, ...)					float var = (value);
	#define DECLARE_DEBUG_ENUM(type, var, name, value, ...)				type var = (value);
	#define DECLARE_DEBUG_STRING(var, name, value, ...)					const char* var = (value);
	#define DECLARE_DEBUG_FUNCTION(var, name, callback, context, ...)	Debug::FunctionProxy var((callback), (context));
	#define DECLARE_DEBUG_VECTOR2(var, name, vx, vy, ...)				Debug::VectorProxy2 var((vx), (vy));
	#define DECLARE_DEBUG_VECTOR3(var, name, vx, vy, vz, ...)			Debug::VectorProxy3 var((vx), (vy), (vz));
	#define DECLARE_DEBUG_VECTOR4(var, name, vx, vy, vz, vw, ...)		Debug::VectorProxy4 var((vx), (vy), (vz), (vw));
	#define DECLARE_DEBUG_COLOUR(var, name, r, g, b, a, ...)			Debug::ColourProxy var((r), (g), (b), (a));

	#define EXTERN_DEBUG_BOOL(var)			extern bool var;
	#define EXTERN_DEBUG_INT(var)			extern int var;
	#define EXTERN_DEBUG_FLOAT(var)			extern float var;
	#define EXTERN_DEBUG_ENUM(type, var)	extern type var;
	#define EXTERN_DEBUG_STRING(var)		extern const char* var;
	#define EXTERN_DEBUG_FUNCTION(var)		extern Debug::FunctionProxy var;
	#define EXTERN_DEBUG_VECTOR2(var)		extern Debug::VectorProxy2 var;
	#define EXTERN_DEBUG_VECTOR3(var)		extern Debug::VectorProxy3 var;
	#define EXTERN_DEBUG_VECTOR4(var)		extern Debug::VectorProxy4 var;
	#define EXTERN_DEBUG_COLOUR(var)		extern Debug::ColourProxy var;

#endif // DISABLE_DEBUG_VARIABLES

#endif // !SYSTEM_DEBUG_VARIABLES_H
