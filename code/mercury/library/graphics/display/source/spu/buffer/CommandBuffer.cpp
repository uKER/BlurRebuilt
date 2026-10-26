#include "Display/Spu/Display.h"
#include "Display/Spu/Buffer/CommandBuffer.h"



namespace Display
{
namespace Spu
{



// Patches a command buffer in the appropriate places for jump to self sync.
// Note: Will not correctly parse Jumps, Calls, or Returns.
U32 PatchForLocalStallSync(U32 * const pAddress, const U32 size)
{
	static const qword s_Dabc = {0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b};

	if (!size)
		return 0;
		
	qword cmdEnd = si_from_ptr(pAddress + size / 4);
    qword cmd0 = si_from_ptr(pAddress);

    qword cmd1         = (qword)(0);
    qword notPastEnd   = (qword)(0);
    qword startAligned = (qword)(0);
    qword endAligned   = startAligned;
    qword cmd2         = (qword)(0);
    qword num          = (qword)(0);
    qword get0         = (qword)(0);
    qword sizeQw = si_from_uint(size);

    const qword mask0 = (qword)spu_splats(0x1ffc);
    const qword alignMask = (qword)spu_splats(0x7F);
    const qword getMask = (qword)spu_splats(0xE003FFFF);
    const qword s_Aabc = si_selb(si_ila(0x10203), s_Dabc, si_fsm(si_from_uint(0x7)));

    qword out0 = (qword)(0);
    qword out1 = (qword)(0);
    qword out2 = (qword)(0);
    qword out3 = (qword)(0);
    qword in4  = (qword)(0);

    //note: Tabbing indicates loop iteration.
    do 
	{
            notPastEnd = si_clgt(cmdEnd, cmd1);
        qword get0Input = si_lqd(cmd0, 0x0);
            qword isAligned = si_ceq(startAligned, endAligned);

        startAligned = si_andc(cmd0, alignMask);
        if(__builtin_expect(si_to_uint(notPastEnd) == 0, false))
            break;

            cmd2 = si_ori(cmd1, 0);
        cmd1 = si_ori(cmd0, 0);
            qword get2 = si_ori(get0, 0);

            qword cmd2Added = si_ai(cmd2, 0x80);
        get0 = si_rotqby(get0Input, cmd0);
            qword cmd2Sub = si_ai(cmd2, 4);
            qword get2Rotated = si_rotqmbii(si_rotqmbyi(get2, -3), -4);
            
            qword splitAt = si_andc(cmd2Added, alignMask);
        qword get0Rotated = si_rotqmbyi(get0, -2);
            qword newNum = si_sf(cmd2Sub, splitAt);
            qword get2Anded1 = si_andi(get2Rotated, 0x4);
            qword num2 = si_sf(newNum, num);
            qword inc = si_xori(get2Anded1, 0x4);
        num = si_and(get0Rotated, mask0);
            qword newNumShifted = si_shli(newNum, 16);
        qword end = si_a(cmd0, num);
            qword incXnewNum = si_mpyu(inc, si_rotqmbii(newNum,-2));
        cmd0 = si_ai(end, 4);
        endAligned = si_andc(end, alignMask);

        if (__builtin_expect(si_to_uint(isAligned), true))
            continue;

        qword oldCmdValue = si_lqd(cmd2, 0x0);
        qword pInTmp0 = si_sf(splitAt, cmdEnd);
        qword pInTmp1 = si_ai(pInTmp0, 0x30);
        qword iterCount = si_ai(si_rotmi(pInTmp1, -6), 1);
        qword pInTmp2 = si_andi(pInTmp1, -0x40);
        qword pIn = si_a(splitAt, pInTmp2);

        qword newCmd = si_selb(newNumShifted, get2, getMask);
        qword s_mask = si_cwd(cmd2, 0x0);
        qword newCmdOut = si_shufb(newCmd, oldCmdValue, s_mask);
        si_stqd(newCmdOut, cmd2, 0x00);

        do
        {
            iterCount = si_ai(iterCount, -1);
            in4 = si_lqd(pIn, 0x00);
            qword in3 = si_lqd(pIn, -0x10);
            qword in2 = si_lqd(pIn, -0x20);
            qword in1 = si_lqd(pIn, -0x30);
            qword in0 = si_lqd(pIn, -0x40);
            si_stqd(out3, pIn, 0x40);
            si_stqd(out2, pIn, 0x30);
            si_stqd(out1, pIn, 0x20);
            si_stqd(out0, pIn, 0x10);
            out3 = si_shufb(in3, in4, s_Dabc);
            out2 = si_shufb(in2, in3, s_Dabc);
            out1 = si_shufb(in1, in2, s_Dabc);
            out0 = si_shufb(in0, in1, s_Dabc);
            pIn = si_ai(pIn, -0x40);
        } while(si_to_uint(iterCount) != 0);

        qword num2Shifted = si_shli(num2, 16);
        qword tmp0 = si_selb(num2Shifted, get2, getMask);
        qword newSplitAt = si_a(tmp0, incXnewNum);

        qword splitAtShifted = si_shufb(newSplitAt, in4, s_Aabc);
		
        si_stqd(splitAtShifted, splitAt, 0x0);

		cmd0 = splitAt;
        cmd1 = cmd0;
		cmdEnd = si_ai(cmdEnd,4);
		sizeQw = si_ai(sizeQw, 4);
        endAligned = startAligned;
	} while(true);

	return si_to_uint(sizeQw);
}



}; // namespace Spu
}; // namespace Display
