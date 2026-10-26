#ifndef __ICOMPONENT_H
#define __ICOMPONENT_H


#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/ComponentEnums.h>
#include <ComponentSystem/MessageEnums.h>
#include <ComponentSystem/CObjectId.h>


namespace airacingtech
{
	// Forward declarations
	class CParameterNode;
	class CMessage;
	class CObjectManager;


	/// lets define a macro here
	#define BEGIN_INTERFACE_DEC(name)\
	class name : public IComponent { \
	public: \
	name(){}\
	virtual ~name(){}
		
	#define END_INTERFACE_DEC protected: static void RegisterInterface(EComponentTypeId); };


	#define BEGIN_COMPONENT_DEC(name, iname)\
	class name : public iname {\
	public:\
	name();\
	virtual ~name();\
	static void	RegisterComponentType(void);\
	static IComponent *CreateMe();\
	static bool DestroyMe(IComponent *);\
	virtual bool Init(CObjectId, CParameterNode &);\
	virtual void Deinit(void);\
	virtual EMessageResult HandleMessage(const CMessage &);\
	virtual EComponentTypeId GetComponentTypeId(void);

	#define END_COMPONENT_DEC };



	// The IComponent class. All our component classes inherit from this
	class IComponent
	{
	public:
		IComponent();
		virtual ~IComponent();

		virtual bool        Init(CObjectId, CParameterNode &) = 0;
		virtual void        Deinit(void) = 0;
		const CObjectId&    GetObjectId(void) const     { return mObjectId; }

		virtual EMessageResult		HandleMessage(const CMessage &);
		virtual EComponentTypeId	GetComponentTypeId(void) = 0;

		
	private:
		void            SetObjectId(CObjectId oId)  { mObjectId = oId; }
		CObjectId       mObjectId;

		friend class CObjectManager;
	};
}

#endif //__ICOMPONENT_H
