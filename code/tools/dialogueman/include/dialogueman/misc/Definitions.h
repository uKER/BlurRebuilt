#ifndef DMAN_DEFINITIONS_H
#define DMAN_DEFINITIONS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace XtShared
{
	class C_XtDirty;
}

namespace DMan
{
	class I_XtType;
	class C_XtTypeDatabase;
	class C_Selection;
	class C_App;
	class C_Frame;
	class C_Explorer;
	class C_DataGridBase;
	class C_DbManager;
	class C_ColumnSet;
	class C_Column;
	class C_Record;
	class C_Row;
	class C_RowSet;
	class C_DataGrid;
	class C_WxScarabForm;
	struct C_GlobalSignals;

	enum E_DataType 
	{
		E_DataType_Bool,
		E_DataType_UInt,
		E_DataType_Int,
		E_DataType_Long,
		E_DataType_Float,
		E_DataType_Double,
		E_DataType_String,
		E_DataType_Pointer,
		E_DataType_MAX
	};

#define DEF(e, str) e,
	enum E_XtType
	{
		#include "../Xt/Types/XtTypeMapping.h"
		E_XtType_Count
	};
#undef DEF

	static const char* kXtSemanticAttr		= "SceneView";
	static const char* kXtSemanticParam		= "semantic";
	static const char* kXtTypeParam			= "type";

	static const char* kVendorName = "Bizarre Creations";
	static const char* kEditorName = "DialogueMan";

	static const char* kRegKeyPath = "HKEY_CURRENT_USER\\Software\\Bizarre Creations\\DialogueMan";

	static const U32 kInvalidXtId = 0xffffffff;
	static const U8 SIGNATURE_CONVERSATION = 0xff;
	static const U8 SIGNATURE_TRIGGER = 0xfe;
	static const U8 SIGNATURE_LOCATION = 0xfd;
	static const U8 SIGNATURE_EVENT = 0xfc;

	typedef Util::C_String Character;
}

#endif //DMAN_DEFINITIONS_H
