#include "SystemPch.h"
#include <Core/Core.h>
#include <System/Debug/Debug.h>
#include <System/Artery/Artery.h>

/*

	TODO:

	DEBUG_PAGE(CheatSpawn, "cheat.spawn")

		DP_ITEM(gCheatSpawnType)

		DP_GROUP(kInBox, kHorizontal)
			DP_ITEM(gCheatSpawn)
			DP_ITEM(gCheatSpawnCircle)
		DP_END

		DP_SEPERATOR

		DP_ITEM(gCheatSpawnCount)

	END_DEBUG_PAGE

	void main()
	{
		DEBUG_PAGE_REGISTER(CheatSpawn);
	}

*/

//
// kDebugVarNetId
//

const char* kDebugVarNetId = "DebugVariables";

//
// Helper algorithms
//

namespace Debug
{
	template<typename Type> inline Type Clamp(const Type& value, const Type& low, const Type& high)
	{
		return (low > value) ? low : ((value > high) ? high : value);
	}

	template<typename Type> __forceinline Type Max(const Type& lhs, const Type& rhs)
	{
		return (rhs > lhs) ? rhs : lhs;
	}

	template<typename Type> __forceinline Type Min(const Type& lhs, const Type& rhs)
	{
		return (lhs > rhs) ? rhs : lhs;
	}

	bool IsDebugVariableNameValid(const char* name)
	{
		if (*name == '\0')
			return false;

		if (!(
				((*name >= 'A') && (*name <= 'Z')) ||
				((*name >= 'a') && (*name <= 'z')) ||
				(*name == '_')
			))
		{
			return false;
		}

		for(const char* p = name; *p != '\0'; p++)
		{
			if (!(
					((*p >= 'A') && (*p <= 'Z')) ||
					((*p >= 'a') && (*p <= 'z')) ||
					((*p >= '0') && (*p <= '9')) ||
					(*p == '.') || (*p == '_')
				))
			{
				return false;
			}
		}

		for(const char* p = name + 1; *p != '\0'; p++)
		{
			if (*p == '.')
			{
				if ((p[-1] == '.') || (p[1] == '\0'))
					return false;
			}
		}

		return true;
	}
}

//
// Globals
//

namespace Debug
{
	class Root : public VarBase
	{
	public:
		Root() :
			VarBase(0, kVtRoot)
		{
		}

	public:
	};
};

Debug::Root	gDebugVariables;
bool		gDebugVariablesListDirty;
bool		gDebugVariablesDirty;

//
// Helper functions
//

int Debug::StringCompare(const char* lhs, const char* rhs)
{
	return strcmp(lhs, rhs);
}

//
// Debug::VarBase
//

Debug::VarBase::VarBase(const char* name, VarType type) :
	_name(name),
	_description(""),
	_flags(type),
	_revision(0),
	_notifyCallback(0),
	_notifyContext(0)
{
	if (gDebugVariables._next == 0)
	{
		gDebugVariables._next	= this;
		gDebugVariables._prev	= this;
		gDebugVariables._flags	= kVtRoot;
	}

	if (Type() != kVtRoot)
	{
		_next = this;
		_prev = this;

		Attach(&gDebugVariables);

		gDebugVariablesListDirty = true;
	}

#if !defined(CFG_GOLD) && defined(PLATFORM_PC)
	if ((Type() != kVtRoot) && !IsDebugVariableNameValid(name))
	{
		char buf[256];
		_snprintf(buf, sizeof(buf), "***\r\n*** Debug variable names must resemble C identifiers: '%s'\r\n***\r\n", name);
		buf[255] = 0;
		OutputDebugString(buf);
		
		// If you've been caught by this DebugBreak then you need to fix the names of your
		// debug variables which aren't compatible with the debug system. Please fix them before commiting your code.
		DebugBreak();
	}
#endif
}

Debug::VarBase::~VarBase()
{
	Detach();

	gDebugVariablesListDirty = true;

	if (_flags & kVfDynamic)
		delete [] _name;
}

//
// Debug::VarBase::SetDirty
//

void Debug::VarBase::SetDirty()
{
	_flags |= kVfDirty;

	gDebugVariablesDirty = true;
}

//
// Debug::VarBase::SetHide
//

void Debug::VarBase::SetHide(bool hide)
{
	if (hide)
		_flags |= kVfHide;
	else
		_flags &= ~kVfHide;
}

//
// Debug::VarBase::Notify
//

void Debug::VarBase::DoNotify()
{
	if (_notifyCallback)
		_notifyCallback(this, _notifyContext);
}

//
// Debug::VarBase::Attach/Detach
//

void Debug::VarBase::Attach(VarBase* before)
{
	assert(Type() != kVtRoot);
	assert(before != 0);

	Detach();

	_next			= before;
	_prev			= before->_prev;
	_prev->_next	= this;
	_next->_prev	= this;
}

void Debug::VarBase::Detach()
{
	_next->_prev	= _prev;
	_prev->_next	= _next;
	_next			= this;
	_prev			= this;
}

//
// Debug::VarBase::SetDynamic/OnSetDynamic
//

bool Debug::VarBase::SetDynamic()
{
	if (_flags & kVfDynamic)
		return true;

	int		length	= strlen(_name) + 1;
	char*	name	= new char[length];

	if (!name)
		return false;

	if (!OnSetDynamic())
	{
		delete [] name;
		return false;
	}

	memcpy(name, _name, length);

	_flags |= kVfDynamic;
	_name = name;

	return true;
}

bool Debug::VarBase::OnSetDynamic()
{
	return true;
}

//
// Debug::VarBase::SetRange
//

void Debug::VarBase::SetRange(int low, int high)
{
}

void Debug::VarBase::SetRange(float low, float high)
{
}

void Debug::VarBase::SetStep(int step)
{
}

void Debug::VarBase::SetStep(float step)
{
}

//
// Iterator
//

Debug::Iterator::Iterator() :
	_current(&gDebugVariables)
{
	operator++();
}

//
// Debug::EnumBase
//

Debug::EnumBase::EnumBase(const char* name, const Option* options) :
	VarBase(name, kVtEnum),
	_options(options)
{
}

bool Debug::EnumBase::SetValueByName(const char* name)
{
	for(const Debug::Option* op = _options; op->name; op++)
	{
		if (strcmp(name, op->name) == 0)
		{
			SetRawValue(op->value);
			return true;
		}
	}

	return false;
}

const char* Debug::EnumBase::ValueAsName() const
{
	int value = RawValue();

	for(const Debug::Option* op = _options; op->name; op++)
		if (value == op->value)
			return op->name;

	return "";
}

const char* Debug::EnumBase::DefaultAsName() const
{
	int value = RawDefault();

	for(const Debug::Option* op = _options; op->name; op++)
		if (value == op->value)
			return op->name;

	return "";
}

void Debug::EnumBase::SetOptions(const Option* options)
{
	_options = options;
	gDebugVariablesListDirty = true;
}

//
// Debug::MetaBase
//

void Debug::MetaBase::Apply(VarBase& varBase) const
{
}

//
// Debug::Bool
//

Debug::Bool::Bool(const char* name, bool value) :
	VarBase(name, kVtBool),
	_value(value),
	_default(value)
{
}

Debug::Bool::Bool(const char* name, bool value, DV_META_ARGS_ND_2) :
	VarBase(name, kVtBool),
	_value(value),
	_default(value)
{
	DV_META_APPLY_2();
}

Debug::Bool::Bool(const char* name, bool value, DV_META_ARGS_ND_6) :
	VarBase(name, kVtBool),
	_value(value),
	_default(value)
{
	DV_META_APPLY_6();
}

void Debug::Bool::SetValue(bool value)
{
	if (_value != value)
	{
		_value = value;
		SetDirty();
	}
}

//
// Debug::Int
//

Debug::Int::Int(const char* name, int value) :
	VarBase(name, kVtInt),
	_value(value),
	_default(value),
	_low(0),
	_high(0),
	_step(0)
{
}

Debug::Int::Int(const char* name, int value, DV_META_ARGS_ND_2) :
	VarBase(name, kVtInt),
	_value(value),
	_default(value),
	_low(0),
	_high(0),
	_step(0)
{
	DV_META_APPLY_2();
}

Debug::Int::Int(const char* name, int value, DV_META_ARGS_ND_6) :
	VarBase(name, kVtInt),
	_value(value),
	_default(value),
	_low(0),
	_high(0),
	_step(0)
{
	DV_META_APPLY_6();
}

void Debug::Int::SetValue(int value)
{
	if (_value != value)
	{
		_value = value;
		SetDirty();
	}
}

void Debug::Int::SetRange(int low, int high)
{
	_flags |= kVfRange;

	_low	= low;
	_high	= high;
}

void Debug::Int::SetRange(float low, float high)
{
	_flags |= kVfRange;

	_low	= (int)low;
	_high	= (int)high;
}

void Debug::Int::SetStep(int step)
{
	_flags |= kVfStep;

	_step = step;
}

void Debug::Int::SetStep(float step)
{
	_flags |= kVfStep;

	_step = (int)step;
}

//
// Debug::Float
//

Debug::Float::Float(const char* name, float value) :
	VarBase(name, kVtFloat),
	_value(value),
	_default(value),
	_low(0.0f),
	_high(0.0f),
	_step(0.0f)
{
}

Debug::Float::Float(const char* name, float value, DV_META_ARGS_ND_2) :
	VarBase(name, kVtFloat),
	_value(value),
	_default(value),
	_low(0.0f),
	_high(0.0f),
	_step(0.0f)
{
	DV_META_APPLY_2();
}

Debug::Float::Float(const char* name, float value, DV_META_ARGS_ND_6) :
	VarBase(name, kVtFloat),
	_value(value),
	_default(value),
	_low(0.0f),
	_high(0.0f),
	_step(0.0f)
{
	DV_META_APPLY_6();
}

void Debug::Float::SetValue(float value)
{
	if (_value != value)
	{
		_value = value;
		SetDirty();
	}
}

void Debug::Float::SetRange(int low, int high)
{
	_flags |= kVfRange;

	_low	= (float)low;
	_high	= (float)high;
}

void Debug::Float::SetRange(float low, float high)
{
	_flags |= kVfRange;

	_low	= low;
	_high	= high;
}

void Debug::Float::SetStep(int step)
{
	_flags |= kVfStep;

	_step = (float)step;
}

void Debug::Float::SetStep(float step)
{
	_flags |= kVfStep;

	_step = step;
}

//
// Debug::String
//

Debug::String::String(const char* name, const char* value) :
	VarBase(name, kVtString),
	_value(""),
	_default(value)
{
	SetImpl(value);
}

Debug::String::String(const char* name, const char* value, DV_META_ARGS_ND_2) :
	VarBase(name, kVtString),
	_value(""),
	_default(value)
{
	SetImpl(value);
	DV_META_APPLY_2();
}

Debug::String::String(const char* name, const char* value, DV_META_ARGS_ND_6) :
	VarBase(name, kVtString),
	_value(""),
	_default(value)
{
	SetImpl(value);
	DV_META_APPLY_6();
}

Debug::String::~String()
{
	if (*_value)
		delete [] _value;

	if ((_flags & kVfDynamic) && *_default)
		delete [] _default;
}

void Debug::String::SetValue(const char* value)
{
	if (StringCompare(_value, value) != 0)
	{
		SetImpl(value);
		SetDirty();
	}
}

bool Debug::String::OnSetDynamic()
{
	if (!*_default)
		return true;

	int		length	= strlen(_default) + 1;
	char*	def		= new char[length];

	if (!def)
		return false;

	memcpy(def, _default, length);

	_default = def;

	return true;
}

void Debug::String::SetImpl(const char* text)
{
	size_t	length	= strlen(text) + 1;
	char*	newText	= "";

	if (length > 1)
	{
		if ((newText = new char[length]) != 0)
			memcpy(newText, text, length);
		else
			newText = "";
	}

	if (*_value)
		delete [] _value;

	_value = newText;
}

//
// Debug::Function
//

Debug::Function::Function(const char* name, void (*callback)(void*), void* context) :
	VarBase(name, kVtFunction),
	_callback(callback),
	_context(context)
{
}

Debug::Function::Function(const char* name, void (*callback)(void*), void* context, DV_META_ARGS_ND_2) :
	VarBase(name, kVtFunction),
	_callback(callback),
	_context(context)
{
	DV_META_APPLY_2();
}

Debug::Function::Function(const char* name, void (*callback)(void*), void* context, DV_META_ARGS_ND_6) :
	VarBase(name, kVtFunction),
	_callback(callback),
	_context(context)
{
	DV_META_APPLY_6();
}

void Debug::Function::Call()
{
	_callback(_context);
}

//
// Debug::Vector2
//

Debug::Vector2::Vector2(const char* name, float vx, float vy) :
	VarBase(name, kVtVector2),
	_value(vx, vy),
	_default(vx, vy)
{
}

Debug::Vector2::Vector2(const char* name, float vx, float vy, DV_META_ARGS_ND_2) :
	VarBase(name, kVtVector2),
	_value(vx, vy),
	_default(vx, vy)
{
	DV_META_APPLY_2();
}

Debug::Vector2::Vector2(const char* name, float vx, float vy, DV_META_ARGS_ND_6) :
	VarBase(name, kVtVector2),
	_value(vx, vy),
	_default(vx, vy)
{
	DV_META_APPLY_6();
}

void Debug::Vector2::SetValue(const VectorStruct2& value)
{
	if (_value != value)
	{
		_value = value;
		SetDirty();
	}
}

//
// Debug::Vector3
//

Debug::Vector3::Vector3(const char* name, float vx, float vy, float vz) :
	VarBase(name, kVtVector3),
	_value(vx, vy, vz),
	_default(vx, vy, vz)
{
}

Debug::Vector3::Vector3(const char* name, float vx, float vy, float vz, DV_META_ARGS_ND_2) :
	VarBase(name, kVtVector3),
	_value(vx, vy, vz),
	_default(vx, vy, vz)
{
	DV_META_APPLY_2();
}

Debug::Vector3::Vector3(const char* name, float vx, float vy, float vz, DV_META_ARGS_ND_6) :
	VarBase(name, kVtVector3),
	_value(vx, vy, vz),
	_default(vx, vy, vz)
{
	DV_META_APPLY_6();
}

void Debug::Vector3::SetValue(const VectorStruct3& value)
{
	if (_value != value)
	{
		_value = value;
		SetDirty();
	}
}

//
// Debug::Vector4
//

Debug::Vector4::Vector4(const char* name, float vx, float vy, float vz, float vw) :
	VarBase(name, kVtVector4),
	_value(vx, vy, vz, vw),
	_default(vx, vy, vz, vw)
{
}

Debug::Vector4::Vector4(const char* name, float vx, float vy, float vz, float vw, DV_META_ARGS_ND_2) :
	VarBase(name, kVtVector4),
	_value(vx, vy, vz, vw),
	_default(vx, vy, vz, vw)
{
	DV_META_APPLY_2();
}

Debug::Vector4::Vector4(const char* name, float vx, float vy, float vz, float vw, DV_META_ARGS_ND_6) :
	VarBase(name, kVtVector4),
	_value(vx, vy, vz, vw),
	_default(vx, vy, vz, vw)
{
	DV_META_APPLY_6();
}

void Debug::Vector4::SetValue(const VectorStruct4& value)
{
	if (_value != value)
	{
		_value = value;
		SetDirty();
	}
}

//
// Debug::Colour
//

Debug::Colour::Colour(const char* name, float r, float g, float b, float a) :
	VarBase(name, kVtColour),
	_value(r, g, b, a),
	_default(r, g, b, a),
	_gamma(1.0f)
{
}

Debug::Colour::Colour(const char* name, float r, float g, float b, float a, DV_META_ARGS_ND_2) :
	VarBase(name, kVtColour),
	_value(r, g, b, a),
	_default(r, g, b, a),
	_gamma(1.0f)
{
	DV_META_APPLY_2();
}

Debug::Colour::Colour(const char* name, float r, float g, float b, float a, DV_META_ARGS_ND_6) :
	VarBase(name, kVtColour),
	_value(r, g, b, a),
	_default(r, g, b, a),
	_gamma(1.0f)
{
	DV_META_APPLY_6();
}

void Debug::Colour::SetValue(const ColourStruct& value)
{
	if (_value != value)
	{
		_value = value;
		SetDirty();
	}
}

//
// Debug::Description
//

void Debug::Description::Apply(class VarBase& varBase) const
{
	varBase._description = _description;
}

//
// Debug::Persist
//

void Debug::Persist::Apply(class VarBase& varBase) const
{
	varBase._flags |= kVfPersist;
}

//
// Debug::Dynamic
//

void Debug::Dynamic::Apply(class VarBase& varBase) const
{
	varBase.SetDynamic();
}

//
// Debug::Notify
//

void Debug::Notify::Apply(class VarBase& varBase) const
{
	varBase._notifyCallback	= _callback;
	varBase._notifyContext	= _context;
}

//
// Debug::Range
//

Debug::RangeInt::RangeInt(int low, int high) :
	_low(low),
	_high(high)
{
}

void Debug::RangeInt::Apply(class VarBase& varBase) const
{
	varBase.SetRange(_low, _high);
}

Debug::RangeFloat::RangeFloat(float low, float high) :
	_low(low),
	_high(high)
{
}

void Debug::RangeFloat::Apply(class VarBase& varBase) const
{
	varBase.SetRange(_low, _high);
}

//
// Debug::Step
//

Debug::StepInt::StepInt(int step) :
	_step(step)
{
}

void Debug::StepInt::Apply(class VarBase& varBase) const
{
	varBase.SetStep(_step);
}

Debug::StepFloat::StepFloat(float step) :
	_step(step)
{
}

void Debug::StepFloat::Apply(class VarBase& varBase) const
{
	varBase.SetStep(_step);
}

//
// Debug::Gamma
//

void Debug::Gamma::Apply(class VarBase& varBase) const
{
	if (varBase.Type() == kVtColour)
		((Debug::Colour&)varBase).SetGamma(_gamma);
}

//
// Helper functions
//

Debug::VarBase* Debug::FindByName(const char* name)
{
	for(Debug::Iterator it; it; ++it)
		if (strcmp(name, it->Name()) == 0)
			return it;

	return 0;
}

void Debug::Adjust(VarBase* varBase, int factor)
{
	switch(varBase->Type())
	{
		case kVtBool:
		{
			Debug::Bool* var = (Debug::Bool*)varBase;

			if (factor > 0)
				var->SetValue(true);
			else if (factor < 0)
				var->SetValue(false);
		}
		break;

		case kVtInt:
		{
			Debug::Int* var = (Debug::Int*)varBase;

			int theStep;

			if (var->Flags() & kVfStep)
				theStep = var->Step();
			else if (var->Flags() & kVfRange)
				theStep = Debug::Max(1, (var->High() - var->Low()) / 100);
			else
				theStep = 1;

			int newValue = var->Value() + (theStep * factor);

			if (var->Flags() & kVfRange)
				var->SetValue(Debug::Clamp(newValue, var->Low(), var->High()));
			else
				var->SetValue(newValue);
		}
		break;

		case kVtFloat:
		{
			Debug::Float* var = (Debug::Float*)varBase;

			float theStep;

			if (var->Flags() & kVfStep)
				theStep = var->Step();
			else if (var->Flags() & kVfRange)
				theStep = (var->High() - var->Low()) / 100.0f;
			else
				theStep = 1.0f;

			float newValue = var->Value() + (theStep * factor);

			if (var->Flags() & kVfRange)
				var->SetValue(Debug::Clamp(newValue, var->Low(), var->High()));
			else
				var->SetValue(newValue);
		}
		break;

		case kVtEnum:
		{
			Debug::EnumBase* var = (Debug::EnumBase*)varBase;

			const Debug::Option* opPrev = 0;
			const Debug::Option* opNext = 0;

			int value = var->RawValue();

			for(Debug::OptionIterator it(var); it; ++it)
			{
				if (value == it->value)
				{
					++it;
					opNext = it;
					break;
				}

				opPrev = it;
			}

			if ((factor > 0) && opNext)
				var->SetRawValue(opNext->value);
			else if ((factor < 0) && opPrev)
				var->SetRawValue(opPrev->value);
		}
		break;
	}
}

void Debug::SetToDefault(VarBase* varBase)
{
	switch(varBase->Type())
	{
		case kVtBool:
		{
			Debug::Bool* var = (Debug::Bool*)varBase;

			var->SetValue(var->Default());
		}
		break;

		case kVtInt:
		{
			Debug::Int* var = (Debug::Int*)varBase;

			var->SetValue(var->Default());
		}
		break;

		case kVtFloat:
		{
			Debug::Float* var = (Debug::Float*)varBase;

			var->SetValue(var->Default());
		}
		break;

		case kVtEnum:
		{
			Debug::EnumBase* var = (Debug::EnumBase*)varBase;

			var->SetRawValue(var->RawDefault());
		}
		break;

		case Debug::kVtString:
		{
			Debug::String* var = (Debug::String*)varBase;

			var->SetValue(var->Default());
		}
		break;

		case Debug::kVtVector2:
		{
			Debug::Vector2* var = (Debug::Vector2*)varBase;

			var->SetValue(var->Default());
		}
		break;

		case Debug::kVtVector3:
		{
			Debug::Vector3* var = (Debug::Vector3*)varBase;

			var->SetValue(var->Default());
		}
		break;

		case Debug::kVtVector4:
		{
			Debug::Vector4* var = (Debug::Vector4*)varBase;

			var->SetValue(var->Default());
		}
		break;

		case Debug::kVtColour:
		{
			Debug::Colour* var = (Debug::Colour*)varBase;

			var->SetValue(var->Default());
		}
		break;
	}
}

void Debug::SetToLow(VarBase* varBase)
{
	switch(varBase->Type())
	{
		case kVtBool:
		{
			Debug::Bool* var = (Debug::Bool*)varBase;

			var->SetValue(false);
		}
		break;

		case kVtInt:
		{
			Debug::Int* var = (Debug::Int*)varBase;

			if (varBase->Flags() & kVfRange)
				var->SetValue(var->Low());
		}
		break;

		case kVtFloat:
		{
			Debug::Float* var = (Debug::Float*)varBase;

			if (varBase->Flags() & kVfRange)
				var->SetValue(var->Low());
		}
		break;

		case kVtEnum:
		{
			Debug::EnumBase* var = (Debug::EnumBase*)varBase;

			for(Debug::OptionIterator it(var); it; ++it)
			{
				var->SetRawValue(it->value);
				break;
			}
		}
		break;
	}
}

void Debug::SetToHigh(VarBase* varBase)
{
	switch(varBase->Type())
	{
		case kVtBool:
		{
			Debug::Bool* var = (Debug::Bool*)varBase;

			var->SetValue(true);
		}
		break;

		case kVtInt:
		{
			Debug::Int* var = (Debug::Int*)varBase;

			if (varBase->Flags() & kVfRange)
				var->SetValue(var->High());
		}
		break;

		case kVtFloat:
		{
			Debug::Float* var = (Debug::Float*)varBase;

			if (varBase->Flags() & kVfRange)
				var->SetValue(var->High());
		}
		break;

		case kVtEnum:
		{
			Debug::EnumBase* var = (Debug::EnumBase*)varBase;

			const Debug::Option* opHigh = 0;

			for(Debug::OptionIterator it(var); it; ++it)
				opHigh = it;

			if (opHigh)
				var->SetRawValue(opHigh->value);
		}
		break;
	}
}
