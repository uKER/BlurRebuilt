using System;

namespace Windows
{
	#region Message Codes

	namespace Message
	{
		#region WM_ Messages

		/// <summary>
		/// Windows Message Codes
		/// </summary>
		public enum WM : int
		{
			Null					= 0x0000,
			Create					= 0x0001,
			Destroy					= 0x0002,
			Move					= 0x0003,
			Size					= 0x0005,

			Activate				= 0x0006,

			SetFocus				= 0x0007,
			KillFocus				= 0x0008,
			Enable					= 0x000A,
			SetRedraw				= 0x000B,
			SetText					= 0x000C,
			GetText					= 0x000D,
			GetTextLength			= 0x000E,
			Paint					= 0x000F,
			Close					= 0x0010,

			QueryEndSession			= 0x0011,
			QueryOpen				= 0x0013,
			EndSession				= 0x0016,

			Quit					= 0x0012,
			EraseBkGnd				= 0x0014,
			SysColorChange			= 0x0015,
			ShowWindow				= 0x0018,
			WinIniChange			= 0x001A,
			SettingChange			= WinIniChange,

			DevModeChange			= 0x001B,
			ActivateApp				= 0x001C,
			FontChange				= 0x001D,
			TimeChange				= 0x001E,
			CancelMode				= 0x001F,
			SetCursor				= 0x0020,
			MouseActivate			= 0x0021,
			ChildActivate			= 0x0022,
			QueueSync				= 0x0023,

			GetMinMaxInfo			= 0x0024,

			PaintIcon				= 0x0026,
			IconEraseBkGnd			= 0x0027,
			NextDlgCtl				= 0x0028,
			SpoolErStatus			= 0x002A,
			DrawItem				= 0x002B,
			MeasureItem				= 0x002C,
			DeleteItem				= 0x002D,
			VKeyToItem				= 0x002E,
			CharToItem				= 0x002F,
			SetFont					= 0x0030,
			GetFont					= 0x0031,
			SetHotKey				= 0x0032,
			GetHotKey				= 0x0033,
			QueryDragIcon			= 0x0037,
			CompareItem				= 0x0039,

			GetObject				= 0x003D,

			Compacting				= 0x0041,
			WindowPosChanging		= 0x0046,
			WindowPosChanged		= 0x0047,

			Power					= 0x0048,

			CopyData				= 0x004A,
			CancelJournal			= 0x004B,

			Notify					= 0x004E,
			InputLangChangeRequest	= 0x0050,
			InputLangChange			= 0x0051,
			TCard					= 0x0052,
			Help					= 0x0053,
			UserChanged				= 0x0054,
			NotifyFormat			= 0x0055,

			ContextMenu				= 0x007B,
			StyleChanging			= 0x007C,
			StyleChanged			= 0x007D,
			DisplayChange			= 0x007E,
			GetIcon					= 0x007F,
			SetIcon					= 0x0080,

			NCCreate				= 0x0081,
			NCDestroy				= 0x0082,
			NCCalCSize				= 0x0083,
			NCHitTest				= 0x0084,
			NCPaint					= 0x0085,
			NCActivate				= 0x0086,
			GetDlgCode				= 0x0087,

			SyncPaint				= 0x0088,

			NCMouseMove				= 0x00A0,
			NCLButtonDown			= 0x00A1,
			NCLButtonUp				= 0x00A2,
			NCLButtonDblClk			= 0x00A3,
			NCRButtonDown			= 0x00A4,
			NCRButtonUp				= 0x00A5,
			NCRButtonDblClk			= 0x00A6,
			NCMButtonDown			= 0x00A7,
			NCMButtonUp				= 0x00A8,
			NCMButtonDblClk			= 0x00A9,

			NCXButtonDown			= 0x00AB,
			NCXButtonUp				= 0x00AC,
			NCXButtonDblClk			= 0x00AD,

			Input					= 0x00FF,

			KeyFirst				= 0x0100,
			KeyDown					= 0x0100,
			KeyUp					= 0x0101,
			Char					= 0x0102,
			DeadChar				= 0x0103,
			SysKeyDown				= 0x0104,
			SysKeyUp				= 0x0105,
			SysChar					= 0x0106,
			SysDeadChar				= 0x0107,

			UniChar					= 0x0109,
			KeyLast					= 0x0109,

			IME_StartComposition	= 0x010D,
			IME_EndComposition		= 0x010E,
			IME_Composition			= 0x010F,
			IME_KeyLast				= 0x010F,

			InitDialog				= 0x0110,
			Command					= 0x0111,
			SysCommand				= 0x0112,
			Timer					= 0x0113,
			HScroll					= 0x0114,
			VScroll					= 0x0115,
			InitMenu				= 0x0116,
			InitMenuPopup			= 0x0117,
			MenuSelect				= 0x011F,
			MenuChar				= 0x0120,
			EnterIdle				= 0x0121,

			MenuRButtonUp			= 0x0122,
			MenuDrag				= 0x0123,
			MenuGetObject			= 0x0124,
			UninitMenuPopup			= 0x0125,
			MenuCommand				= 0x0126,

			ChangeUIState			= 0x0127,
			UpdateUIState			= 0x0128,
			QueryUIState			= 0x0129,

			CTLColorMsgBox			= 0x0132,
			CTLColorEdit			= 0x0133,
			CTLColorListBox			= 0x0134,
			CTLColorBtn				= 0x0135,
			CTLColorDlg				= 0x0136,
			CTLColorScrollBar		= 0x0137,
			CTLColorStatic			= 0x0138,
			GetHMenu				= 0x01E1,

			MouseMove				= 0x0200,			MouseFirst				= 0x0200,
			LButtonDown				= 0x0201,
			LButtonUp				= 0x0202,
			LButtonDblClk			= 0x0203,
			RButtonDown				= 0x0204,
			RButtonUp				= 0x0205,
			RButtonDblClk			= 0x0206,
			MButtonDown				= 0x0207,
			MButtonUp				= 0x0208,
			MButtonDblClk			= 0x0209,
			MouseWheel				= 0x020A,
			XButtonDown				= 0x020B,
			XButtonUp				= 0x020C,
			XButtonDblClk			= 0x020D,			MouseLast				= 0x020D,

			ParentNotify			= 0x0210,
			EnterMenuLoop			= 0x0211,
			ExitMenuLoop			= 0x0212,

			NextMenu				= 0x0213,
			Sizing					= 0x0214,
			CaptureChanged			= 0x0215,
			Moving					= 0x0216,

			MouseHover				= 0x02A1,
			MouseLeave				= 0x02A3,

			NCMouseHover			= 0x02A0,
			NCMouseLeave			= 0x02A2,
		}

		#endregion WM_ Messages

		#region CCM_ Messages

		/// <summary>
		/// Common Control Shared Messages
		/// </summary>
		public enum CCM : int
		{
			First					= 0x2000,
			Last					= (First + 0x200),

			SetBkColor				= (First + 1),
			SetColorScheme			= (First + 2),
			GetColorScheme			= (First + 3),
			GetDropTarget			= (First + 4),
			SetUnicodeFormat		= (First + 5),
			GetUnicodeFormat		= (First + 6),
		}

		#endregion CCM_ Messages

		#region EN_ Notifications

		/// <summary>
		/// Edit Control Notification Codes
		/// </summary>
		public enum EN : int
		{
			SetFocus		= 0x0100,
			KillFocus		= 0x0200,
			Change			= 0x0300,
			Update			= 0x0400,
			Errspace		= 0x0500,
			Maxtext			= 0x0501,
			Hscroll			= 0x0601,
			Vscroll			= 0x0602,

			Align_LTR_EC	= 0x0700,
			Align_RTL_EC	= 0x0701
		}

		#endregion EN_ Notifications

		#region LVM_ Notfications

		/// <summary>
		/// ListView Message
		/// </summary>
		public enum LVM : int
		{
			First						= 0x1000,
			SetUnicodeFormat			= CCM.SetUnicodeFormat,
			GetUnicodeFormat			= CCM.GetUnicodeFormat,

			GetBkColor					= (First + 0),
			SetBkColor					= (First + 1),
			GetImageList				= (First + 2),

			SetImageList				= (First + 3),
			GetItemCount				= (First + 4),

			GetItemA					= (First + 5),
			GetItemW					= (First + 75),		// Unicode
			GetItem						= GetItemW,

			SetItemA					= (First + 6),
			SetItemW					= (First + 76),		// Unicode
			SetItem						= SetItemW,

			InsertItemA					= (First + 7),
			InsertItemW					= (First + 77),		// Unicode
			InsertItem					= InsertItemW,

			DeleteItem					= (First + 8),
			DeleteAllItems				= (First + 9),
			GetCallbackMask				= (First + 10),
			SetCallbackMask				= (First + 11),
			GetNextItem					= (First + 12),

			FindItemA					= (First + 13),
			FindItemW					= (First + 83),		// Unicode
			FindItem					= FindItemW,

			GetItemRect					= (First + 14),
			SetItemPosition				= (First + 15),
			GetItemPosition				= (First + 16),

			GetStringWidthA				= (First + 17),
			GetStringWidthW				= (First + 87),		// Unicode
			GetStringWidth				= GetStringWidthW,

			HitTest						= (First + 18),
			EnsureVisible				= (First + 19),
			Scroll						= (First + 20),
			RedrawItems					= (First + 21),
			Arrange						= (First + 22),

			EditLabelA					= (First + 23),
			EditLabelW					= (First + 118),	// Unicode
			EditLabel					= EditLabelW,

			GetEditControl				= (First + 24),

			GetColumnA					= (First + 25),
			GetColumnW					= (First + 95),		// Unicode
			GetColumn					= GetColumnW,

			SetColumnA					= (First + 26),
			SetColumnW					= (First + 96),		// Unicode
			SetColumn					= SetColumnW,

			InsertColumnA				= (First + 27),
			InsertColumnW				= (First + 97),		// Unicode
			InsertColumn				= InsertColumnW,

			DeleteColumn				= (First + 28),
			GetColumnWidth				= (First + 29),
			SetColumnWidth				= (First + 30),
			GetHeader					= (First + 31),
			CreateDragImage				= (First + 33),
			GetViewRect					= (First + 34),
			GetTextColor				= (First + 35),
			SetTextColor				= (First + 36),
			GetTextBkColor				= (First + 37),
			SetTextBkColor				= (First + 38),
			GetTopIndex					= (First + 39),
			GetCountPerPage				= (First + 40),
			GetOrigin					= (First + 41),
			Update						= (First + 42),
			SetItemState				= (First + 43),
			GetItemState				= (First + 44),

			GetItemTextA				= (First + 45),
			GetItemTextW				= (First + 115),	// Unicode
			GetItemText					= GetItemTextW,

			SetItemTextA				= (First + 46),
			SetItemTextW				= (First + 116),	// Unicode
			SetItemText					= SetItemTextW,

			SetItemCount				= (First + 47),
			SortItems					= (First + 48),
			SetItemPosition32			= (First + 49),
			GetSelectedCount			= (First + 50),
			GetItemSpacing				= (First + 51),

			GetISearchStringA			= (First + 52),
			GetISearchStringW			= (First + 117),	// Unicode
			GetISearchString			= GetISearchStringW,

			SetIconSpacing				= (First + 53),
			SetExtendedListViewStyle	= (First + 54),
			GetExtendedListViewStyle	= (First + 55),
			GetSubItemRect				= (First + 56),
			SubItemHitTest				= (First + 57),
			SetColumnOrderArray			= (First + 58),

			SetHotItem					= (First + 60),
			GetHotItem					= (First + 61),
			SetHotCursor				= (First + 62),
			GetHotCursor				= (First + 63),
			ApproximateViewRect			= (First + 64),

			GetColumnOrderArray			= (First + 59),
		}

		#endregion LVM_ Notfications
	}

	#endregion Message Codes

	#region Window / Control Styles

	namespace Style
	{
		#region Window Styles

		public enum WS : long
		{
			Overlapped			= 0x00000000L,
			Popup				= 0x80000000L,
			Child				= 0x40000000L,
			Minimize			= 0x20000000L,
			Visible				= 0x10000000L,
			Disabled			= 0x08000000L,
			ClipSiblings		= 0x04000000L,
			ClipChildren		= 0x02000000L,
			Maximize			= 0x01000000L,
			Caption				= 0x00C00000L,	// WS_BORDER | WS_DLGFRAME
			Border				= 0x00800000L,
			DlgFrame			= 0x00400000L,
			VScroll				= 0x00200000L,
			HScroll				= 0x00100000L,
			SysMenu				= 0x00080000L,
			ThickFrame			= 0x00040000L,
			Group				= 0x00020000L,
			TabStop				= 0x00010000L,

			MinimizeBox			= 0x00020000L,
			MaximizeBox			= 0x00010000L,


			Tiled				= Overlapped,
			Iconic				= Minimize,
			SizeBox				= ThickFrame,
			TiledWindow			= OverlappedWindow,

			// Common Window Styles
			OverlappedWindow	= (Overlapped | Caption | SysMenu | ThickFrame | MinimizeBox | MaximizeBox),
			PopupWindow			= (Popup      | Border  | SysMenu),
			ChildWindow			= (Child),
		}

		#endregion Window Styles

		#region EditBox Styles

		public enum ES : long
		{
			Left			= 0x0000L,
			Center			= 0x0001L,
			Right			= 0x0002L,
			MultiLine		= 0x0004L,
			UpperCase		= 0x0008L,
			LowerCase		= 0x0010L,
			Password		= 0x0020L,
			AutoVScroll		= 0x0040L,
			AutoHScroll		= 0x0080L,
			NoHideSel		= 0x0100L,
			OEMConvert		= 0x0400L,
			ReadOnly		= 0x0800L,
			WantReturn		= 0x1000L,
			Number			= 0x2000L
		}

		#endregion EditBox Styles

		#region ComboBox Styles

		public enum CBS : long
		{
			Simple				= 0x0001L,
			DropDown			= 0x0002L,
			DropDownList		= 0x0003L,
			OwnerDrawFixed		= 0x0010L,
			OwnerDrawVariable	= 0x0020L,
			AutoHScroll			= 0x0040L,
			OEMConvert			= 0x0080L,
			Sort				= 0x0100L,
			HasStrings			= 0x0200L,
			NoIntegralHeight	= 0x0400L,
			DisableNoScroll		= 0x0800L,
			UpperCase			= 0x2000L,
			LowerCase			= 0x4000L
		}

		#endregion ComboBox Styles

		#region Static Styles

		public enum SS : long
		{
			Left				= 0x00000000L,
			Center				= 0x00000001L,
			Right				= 0x00000002L,
			Icon				= 0x00000003L,
			BlackRect			= 0x00000004L,
			GrayRect			= 0x00000005L,
			WhiteRect			= 0x00000006L,
			BlackFrame			= 0x00000007L,
			GrayFrame			= 0x00000008L,
			WhiteFrame			= 0x00000009L,
			UserItem			= 0x0000000AL,
			Simple				= 0x0000000BL,
			LeftNoWordWrap		= 0x0000000CL,
			OwnerDraw			= 0x0000000DL,
			Bitmap				= 0x0000000EL,
			EnhMetaFile			= 0x0000000FL,
			EtchedHorz			= 0x00000010L,
			EtchedVert			= 0x00000011L,
			EtchedFrame			= 0x00000012L,
			TypeMask			= 0x0000001FL,
			RealsizeControl		= 0x00000040L,
			NoPrefix			= 0x00000080L,
			Notify				= 0x00000100L,
			CenterImage			= 0x00000200L,
			RightJust			= 0x00000400L,
			RealSizeImage		= 0x00000800L,
			Sunken				= 0x00001000L,
			EditControl			= 0x00002000L,
			EndEllipsis			= 0x00004000L,
			PathEllipsis		= 0x00008000L,
			WordEllipsis		= 0x0000C000L,
			EllipsisMask		= 0x0000C000L
		}

		#endregion Static Styles

		#region ListBox Styles

		public enum LBS : long
		{
			Notify				= 0x0001L,
			Sort				= 0x0002L,
			NoRedraw			= 0x0004L,
			MultipleSel			= 0x0008L,
			OwnerDrawFixed		= 0x0010L,
			OwnerDrawVariable	= 0x0020L,
			HasStrings			= 0x0040L,
			UseTabStops			= 0x0080L,
			NoIntegralHeight	= 0x0100L,
			MultiColumn			= 0x0200L,
			WantKeyboardInput	= 0x0400L,
			ExtendedSel			= 0x0800L,
			DisableNoScroll		= 0x1000L,
			NoData				= 0x2000L,
			NoSel				= 0x4000L,
			ComboBox			= 0x8000L,
			Standard			= (Notify | Sort | WS.VScroll | WS.Border)
		}

		#endregion ListBox Styles
	}

	#endregion Window / Control Styles
}
