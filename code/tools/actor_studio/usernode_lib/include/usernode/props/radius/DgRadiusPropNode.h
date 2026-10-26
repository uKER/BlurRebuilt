#ifndef EDITOR_LIB_RADIUS_PROP_H
#define EDITOR_LIB_RADIUS_PROP_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_DgRadiusPropNode : public C_DgXtPropNodeBase
	{
	//From C_XtObject
		virtual C_Status XtInitialise(
			C_XtLevel* pLevel, 
			LarvaType* pLarvaType, 
			Xt::DomElement* pData);

	// From I_DgObject
	public:
		virtual E_ObjType GetType() const;

		virtual const char*		GetTypeName() const;

		virtual bool IsA(E_ObjType type) const;

	//From I_DgNode
	public:
		virtual bool Update(C_UpdateHintCollection* pHc = 0);

		virtual void GetPlugs(Btl::Vector<I_Plug*>& plugs);

	// From C_DgXtPropNodeBase
	public:
		virtual bool IsVisible() const;


	//From C_DgRadiusPropNode
	public:

		static const char* GetStaticTypeName();

		static const char* GetStaticDisplayName();

		static E_ObjType GetStaticType();

		static void* Create();

		U32 GetNumRadii() const;

		C_StaticPlug<F32>& GetRadiusPlug(U32 index);

		const C_StaticPlug<F32>& GetRadiusPlug(U32 index) const;

		C_StaticPlug<Maths::C_RGBA_U8>& GetColourPlug(U32 index);

		const C_StaticPlug<Maths::C_RGBA_U8>& GetColourPlug(U32 index) const;

	// From C_XtObject
	protected:
		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);


	//From C_DgRadiusPropNode
	protected:

		struct RadiusPlug
		{
			C_StaticPlug<F32>				radiusPlug;
			C_StaticPlug<Maths::C_RGBA_U8>	colourPlug;
		};

		C_DgRadiusPropNode();
		virtual ~C_DgRadiusPropNode();

		C_DgRadiusPropNode(const C_DgRadiusPropNode& rhs);
		const C_DgRadiusPropNode& operator=(const C_DgRadiusPropNode& rhs);

		static void OnPlugChanged(I_Plug* pPlug);

		Btl::Vector<RadiusPlug> m_radiusPlugs;
	};
}

#endif
