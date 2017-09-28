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

Call instruction:  
> `call` _addr_ _ret_ _args_  
> * _addr_ is the location of the function code
> * _ret_ is the ref where the result of the function will go
> * _args_ points to the first argument (all args are 64 bit long)

The vm stack ONLY holds the return addresses

## Procedure model

* vm has a list of procedures
* each procedure is represented by its instruction pointer in the code segment
* the head of the list is the currently executing procedure
* as long as a procedure runs out of instns or `ret` is called when the stack is empty, the procedure gets removed (w/o any error being raised)
* if the last procedure of the list exits, then the vm stops
* if a procedure executes `yield`, it pushes itself at the end of the runnable list
* i/o, timers and such exec-later stuff are implemented by having the vm insert callback codes at the end of the proc list
	* NOTE: memory is shared & all ptrs refer to the same memory segments. Thus, no data may be passed from VM owned memory to procedure memory, cause the later cannot address the former. Therefore, for each listener that needs data, the location must be provided at listener registration time.

## Assembler

### Assembly syntax
`asm` := `line`*  
`line` := `label_ln` | `instn_ln`  
`label_ln` := `word` ':'  
`instn_ln` := `word` `arg`*  
`arg` := `imm` | `ref`  
`ref` := `word` | '@' `num`  
`imm` := '-'? `num`  
`num` := `hex` | `dec`  
`hex` := 0x[0-9a-fA-F]+  
`dec` := [0-9]+  
`word` := [a-zA-Z_][a-zA-Z_0-9]*

### Preprocessor directives

Memory gaps or empty tail area are initialy set to zero.

`%include` "`filename`"  
> 	include the contents of the file `filename`

`%data` `what` ( ':' (`offset`)? (':' `size`)? )?  
> `what` := @`filename` | "string" | `hex`  
> `offset` := `num`  
> `size` := `num`

`%datasz` `num`
> specify the total amount of memory to allocate to `num`
