#ifndef XT_EDITOR_PARTICLEEVENT_RECORD_H
#define XT_EDITOR_PARTICLEEVENT_RECORD_H

#include "GenericRecord.h"

class wxAuiNotebook;

class wxTreeItemId;

namespace Xt
{
	struct DomElement;
}

namespace RecordGraph
{
	class C_Graph;
}

class C_ParticleEventRecord : public C_GenericRecord
{
public:
	C_ParticleEventRecord(Xt::DomRecord* pDomRecord, DynamicForm::C_Form* pForm) : C_GenericRecord(pDomRecord, pForm) {}
	~C_ParticleEventRecord() {};

	void						Select();

};

#endif // XT_EDITOR_PARTICLEEVENT_RECORD_H