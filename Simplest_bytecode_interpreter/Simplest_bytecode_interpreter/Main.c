#include <stdint.h> // uint8_t
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef struct{
	uint8_t* instruction_pointer;
	uint64_t accumulator;
} vm;

typedef enum opcode{
	OP_INC, // increment the register
	OP_DEC, // decrement the register
	OP_ADDI, // add the immediate argument to the register
	OP_SUBI, // substract the immediate argument from the register
	OP_DONE // stop execution
} opcode;

typedef enum interpret_result {
	SUCCESS, 
	ERROR_UNKNOWN_RESULT
} interpret_result;

vm* vm_reset(void)
{
	puts("Reset vm state");
	vm* VM;
	memset(&VM, 0, sizeof(vm));
	return VM;
}

interpret_result vm_interpret(uint8_t* bytecode, vm* ada)
{
	puts("Start interpreting");
	ada->instruction_pointer = bytecode;
	for (;;)
	{
		uint8_t	instruction = *ada->instruction_pointer++;
		switch (instruction)
		{
			case OP_INC: 
			{
				ada->accumulator++;
				break;
			}
			case OP_DEC:
			{
				ada->accumulator--;
				break;
			}
			case OP_ADDI:
			{
				uint8_t arg = *ada->instruction_pointer++; // get the argument
				ada->accumulator += arg;
				break;
			}
			case OP_SUBI:
			{
				uint8_t arg = *ada->instruction_pointer++;
				ada->accumulator -= arg;
				break;
			}
			case OP_DONE:
			{
				return SUCCESS;
			}
			default:
			{
				return ERROR_UNKNOWN_RESULT;
			}
		}
	}
	return SUCCESS;
}

int main()
{
	{
		vm* ada = vm_reset();

		uint8_t code[] = { OP_INC, OP_INC, OP_DEC, OP_DONE };
		interpret_result result = vm_interpret(code, ada);
		printf("vm state: %" PRIu64 "\n", ada->accumulator);

		assert(result == SUCCESS);
		assert(ada->accumulator == 1);
		free(ada);
	}

	{
		vm* earl = vm_reset();

		uint8_t code[] = { OP_INC, OP_DEC, OP_DEC, OP_DONE };
		interpret_result result = vm_interpret(code, earl);
		printf("vm state: %" PRIu64 "\n", earl->accumulator);

		assert(result == SUCCESS);
		assert(earl->accumulator == UINT64_MAX);
		free(earl);
	}

	return 0;
}