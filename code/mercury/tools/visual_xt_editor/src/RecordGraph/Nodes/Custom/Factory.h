#ifndef RECORD_GRAPH_CUSTOM_STRUCT_FACTORY_H
#define RECORD_GRAPH_CUSTOM_STRUCT_FACTORY_H

#pragma once

#include <core\utility\Singleton.h>
#include "../Struct.h"

namespace RecordGraph
{

class C_CustomNodeFactory : public Ut::Singleton<C_CustomNodeFactory, Ut::CreateStatic>
{
public:

	C_CustomNodeFactory() {};
	~C_CustomNodeFactory() {};

	C_StructNode*	GetNode(struct LfStruct* field);
};

}

#endif // RECORD_GRAPH_CUSTOM_STRUCT_FACTORY_H
