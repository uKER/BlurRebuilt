#ifndef GENERATE_ID_H
#define GENERATE_ID_H

#pragma once

namespace Xt
{
	struct DomBlock;
}

struct LarvaField;

void GenerateIdForBlock(Xt::DomBlock* block, LarvaField* field);

#endif // GENERATE_ID_H
