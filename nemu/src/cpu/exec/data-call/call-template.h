#include "cpu/exec/template-start.h"

#define instr call

make_helper (concat(call_i_, SUFFIX))
{
	int len = concat(decode_i_, SUFFIX) (eip + 1);
	reg_l (R_ESP) -= DATA_BYTE;
	swaddr_write (reg_l (R_ESP) , 4 , cpu.eip + len);
	DATA_TYPE_S offset = op_src->val;
	print_asm("call %x",cpu.eip + 1 + len + offset);
	cpu.eip += offset;
	return len + 1;
}

#include "cpu/exec/template-end.h"
