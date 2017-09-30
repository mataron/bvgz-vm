# VM Design Notes

## Instruction format

* args are either immediate values or referrences
* imm size : 2 bits := log2(byte count)
* ref size : static (32 bits)
* number of args is encoded in opcode
* bytecode:
	* the sequence is: `{OpI/R}` `{ISz}`? `{IArg|RArg}`*
	* `{OpI/R}`: the numeric opcode value left shifted by 3 bits ORed w/ at most 3 bits that select imm or ref arg. The least significant bit refers to the 1st argument.
	* `{ISz}`: if at least one arg is imm, this contains 2 bits per arg that indicate the size of their respective arguments (log2(size-in-bytes). The least significant bits couple refers to the 1st argument.
	* `{IArg|RArg}`: the arguments themselves. 1st argument comes first.

## Function ABI

`call` _addr_ _ret_ _args_

* _addr_ is the location of the function code
* _ret_ is the ref where the result of the function will go
* _args_ points to the first argument (all args are 64 bit long)

The vm stack ONLY holds the return addresses, return value address & argument list address.

## Procedure model

* vm has a list of procedures
* each procedure is represented by its instruction pointer in the code segment
* the head of the list is the currently executing procedure
* as long as a procedure runs out of instns or `ret` is called when the stack is empty, the procedure gets removed (w/o any error being raised)
* if the last procedure of the list exits, then the vm stops
* if a procedure executes `yield`, it pushes itself at the end of the runnable list
	* `yield` is a system call.
* i/o, timers and such exec-later stuff are implemented by having the vm insert callback codes at the end of the proc list
	* NOTE: memory is shared & all ptrs refer to the same memory segments. Thus, no data may be passed from VM owned memory to procedure memory, cause the later cannot address the former. Therefore, for each listener that needs data, the location must be provided at listener registration time.
