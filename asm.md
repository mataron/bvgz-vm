# Assembler

## Assembly syntax
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

## Preprocessor directives

Memory gaps or empty tail area are initialy set to zero.

`%include` `filename`
> 	include the contents of the file `filename`

`%data` '@'`label` `what` ( (`offset`)? ( `size`)? )?
> `what` := '='`filename` | "string" | `hex`
>
> `offset` := `num`
>
> `size` := `num`
