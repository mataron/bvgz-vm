# bvgz-vm instruction set

## Arithmetic

All arithmetic instructions operate on 64bit operands.

`add` _dest_ _src_
> _dest_ += _src_

`sub` _dest_ _src_
> _dest_ -= _src_

`mul` _dest_ _src_
> _dest_ *= _src_

`div` _dest_ _src_
> _dest_ /= _src_

`mod` _dest_ _src_
> _dest_ %= _src_

`add` _dest_ _src0_ _src1_
> _dest_ = _src0_ + _src1_

`sub` _dest_ _src0_ _src1_
> _dest_ = _src0_ - _src1_

`mul` _dest_ _src0_ _src1_
> _dest_ = _src0_ * _src1_

`div` _dest_ _src0_ _src1_
> _dest_ = _src0_ / _src1_

`mod` _dest_ _src0_ _src1_
> _dest_ = _src0_ % _src1_

## Bitwise

All bitwise instructions operate on 64bit operands.

`and` _dest_ _src_
> _dest_ &= _src_

`or` _dest_ _src_
> _dest_ |= _src_

`xor` _dest_ _src_
> _dest_ ^= _src_

`not` _dest_
> _dest_ = ~_dest_

`shl` _dest_ _src_
> _dest_ <<= _src_

`shr` _dest_ _src_
> _dest_ >>= _src_

`and` _dest_ _src0_ _src1_
> _dest_ = _src0_ & _src1_

`or` _dest_ _src0_ _src1_
> _dest_ = _src0_ | _src1_

`xor` _dest_ _src0_ _src1_
> _dest_ = _src0_ ^ _src1_

`not` _dest_ _src_
> _dest_ = ~_src_

`shl` _dest_ _src0_ _src1_
> _dest_ = _src0_ << _src1_

`shr` _dest_ _src0_ _src1_
> _dest_ = _src0_ >> _src1_

## Relational

All relational instructions operate on 64bit operands.

All relational instructions set their destination operand to either 0 or 1 (the remaining 63 bits are always zero).

`eq` _dest_ _src0_ _src1_
> _dest_ = _src0_ == _src1_

`ne` _dest_ _src0_ _src1_
> _dest_ = _src0_ != _src1_

`gt` _dest_ _src0_ _src1_
> _dest_ = _src0_ > _src1_

`lt` _dest_ _src0_ _src1_
> _dest_ = _src0_ < _src1_

`ge` _dest_ _src0_ _src1_
> _dest_ = _src0_ >= _src1_

`le` _dest_ _src0_ _src1_
> _dest_ = _src0_ <= _src1_

## Logical

All logical instructions operate on 64bit operands.

All logical instructions set their destination operand to either 0 or 1 (the remaining 63 bits are always zero).

`l_and` _dest_ _src0_ _src1_
> _dest_ = _src0_ && _src1_

`l_or` _dest_ _src0_ _src1_
> _dest_ = _src0_ || _src1_

`l_not` _dest_ _src_
> _dest_ = !_src_

`l_bool` _dest_ _src_
> _dest_ = !!_src_

`l_and` _dest_ _src_
> _dest_ = _dest_ && _src_

`l_or` _dest_ _src_
> _dest_ = _dest_ || _src_

`l_not` _dest_
> _dest_ = !_dest_

`l_bool` _dest_
> _dest_ = !!_dest_

## Memory Access

### copy

`cp` _dest_ _src_ _n_
> copy _n_ bytes from _src_, the address of those bytes, into the location pointed to by _dest_

### read

`read8` _dest_ _src_ _offset_
> copy 1 byte (into _dest_) from the location pointed to by the address stored in _src_ plus _offset_ bytes.<br>
> Equivalent to: _dest_ = *(_src_ + _offset_)

`read16` _dest_ _src_ _offset_
> copy 2 bytes (into _dest_) from the location pointed to by the address stored in _src_ plus _offset_ bytes.<br>
> Equivalent to: _dest_ = *(_src_ + _offset_)

`read32` _dest_ _src_ _offset_
> copy 2 bytes (into _dest_) from the location pointed to by the address stored in _src_ plus _offset_ bytes.<br>
> Equivalent to: _dest_ = *(_src_ + _offset_)

`read64` _dest_ _src_ _offset_
> copy 2 bytes (into _dest_) from the location pointed to by the address stored in _src_ plus _offset_ bytes.<br>
> Equivalent to: _dest_ = *(_src_ + _offset_)

### write

`write8` _dest_ _src_ _offset_
> copy 1 byte (from _src_) to the location pointed to by the address stored in _dest_ plus _offset_ bytes.<br>
> Equivalent to: *(_dest_ + _offset_) = _src_

`write16` _dest_ _src_ _offset_
> copy 2 bytes (from _src_) to the location pointed to by the address stored in _dest_ plus _offset_ bytes.<br>
> Equivalent to: *(_dest_ + _offset_) = _src_

`write32` _dest_ _src_ _offset_
> copy 2 bytes (from _src_) to the location pointed to by the address stored in _dest_ plus _offset_ bytes.<br>
> Equivalent to: *(_dest_ + _offset_) = _src_

`write64` _dest_ _src_ _offset_
> copy 2 bytes (from _src_) to the location pointed to by the address stored in _dest_ plus _offset_ bytes.<br>
> Equivalent to: *(_dest_ + _offset_) = _src_

## Flow Control

`jmp` _addr_
> transfer program flow to the given address

`jtrue` _addr_ _condition_
> transfer program flow to the given address, iff the value stored in _condtion_ evaluates to true.

`jfalse` _addr_ _condition_
> transfer program flow to the given address, iff the value stored in _condtion_ evaluates to false.

`call` _addr_ _ret_ _args_
> _addr_ is the location of the function code<br>
> _ret_ is the ref where the result of the function will go<br>
> _args_ points to the first argument (all args are 64 bit long)

`ret`
> transfer control back to right after the last `call` instruction. If no such address exists, the currently executing procedure will terminate (w/o any errors).

`argv` _addr_
> copy the address of the argument list to _addr_.

`retv` _value_
> copy the _value_ to the return value location.

## Other

`nop`
> :)

`assert` _id_ _condition_
> do nothing if the _condition_ is _true_. Otherwise it will exit the program and set a VM exception and its _id_ to the failed assertion indicator.

`syscall` _id_ _argv_ _retv_
> invoke the system call indentified by the given _id_. The system call will receive its arguments from _argv_ and write its result to _retv_. Both _argv_ and _retv_ are not dereferenced when they system call invoked doesn't need them (so they can be anything). The system call list can be found [here](syscalls.md)
