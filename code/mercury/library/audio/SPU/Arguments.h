// 
#include "system/spu/ArgBase.h"

// Arguments that will be passed to every task in the task set
class cAudioTaskSetArgs : public cArgBase
{
public:
	uint64_t	placeholder;
};

// arguments that will be passed to individual tasks
class cAudioTaskArgs : public cArgBase
{
public:
	// Add some Status flags
	volatile	uint32_t	ppu_status_flags;
	// Add some input data pointers
	uint32_t	pointer_to_my_input_data1;
	// Add some output data pointers
	uint32_t	pointer_to_my_output_data1;
};


extern void TestSPUTemplate();
