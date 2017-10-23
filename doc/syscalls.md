# System call list

All return values are 64bits long. Argument sizes are encoded to the argument names.

## Error check

### ID=1 _getlasterr()_

Returns the last error set in the VM. It will also clear it. The last error is set by other system calls, or the VM itself, each time an error occurs. Possible values for the last error are set from those that can be set to system's _errno_.

### ID=2 _setlasterr(errno64)_

Set the last error set in the VM.

## Time & Procedure priorities

> TBD

## Generic I/O

> TBD

## Filesystem I/O

> TBD

## Network I/O

> TBD

## Memory management

> TBD

## Process management

> TBD

## Code management

> TBD
