# System call list

All return values are 64bits long. Argument sizes are encoded to the argument names. Unless specified otherwise, all functions return a 64bit result, which is zero on success and non-zero otehrwise.

Unless specified otherwise, invoking any system call will cause all pending events to fire, but won't change which the current procedure.

## Error check

### ID=0 `getlasterr()`

Returns the last error set in the VM. It will also clear it. The last error is set by other system calls, or the VM itself, each time an error occurs. Possible values for the last error are set from those that can be set to system's `errno`.

### ID=1 `setlasterr(errno64)`

Set the last error set in the VM.

## Time & Procedure priorities

### ID=2 `time(seconds64, nanoseconds64)`

Read the current system time into the arguments. 

### ID=3 `timeout(function_ptr32, timeout_milliseconds64)`

Set a timeout for the given number of milliseconds and call the function at the specified pointer upon the timer's expiration.

### ID=4 `yield()`

Moves the current procedure to end of the procedure's list. This allows for all other runnable procedure to execute before the current one. This system call returns nothing.

## Generic I/O

### ID=5 `close(handle64)`

Close an i/o handle. Handles are obtained from `fs_open()` or `socket()`.

### ID=6 `read(handle64, ptr64, size64, cb_args_ptr64, callback_ptr64)`

Read from the specified i/o handle. At most, the specified amount of bytes will be read into the memory location pointed to by the given `ptr`. Upon completion the callback will be invoked in its own procedure. The argument pointer used is the one passed to `read()`.

The callback prototype is: `callback(handle64, size64, errno64)`

The first parameter is the one passed to the `read()` call. The size specifies the actual number of bytes writen to the buffer. After a successful read operation the errno parameter is zero. In case an error occurs the size is zero and the errno argument indicates the error kind.

### ID=7 `write(handle64, ptr64, size64, cb_args_ptr64, callback_ptr64)`

Write to the specified i/o handle. At most, the specified amount of bytes from the memory location pointed to by the given `ptr` will be written. Upon completion the callback will be invoked in its own procedure. The argument pointer used is the one passed to `write()`.

The callback prototype is: `callback(handle64, size64, errno64)`

The first parameter is the one passed to the `write()` call. The size specifies the actual number of bytes read from the buffer. After a successful write operation the errno parameter is zero. In case an error occurs the size is zero and the errno argument indicates the error kind.

## Filesystem I/O

### ID=8 `fs_open(path_ptr32, options64)`

Open the file specified by the null-terminated string pointed to by the first argument.

The second argument specifies the open mode. The least significant 3 bits specified read write mode:
> 0x0: read-only<br>
> 0x1: write-only<br>
> 0x2: read-write<br>

Additionally, options can be ORed with the following:
> 0x040: append-mode (write offset is set to the end-of-file initially)<br>
> 0x400: create-mode (creates the file if it doesn't already exist)<br>

The `fs_open()` call returns a 64bit i/o handle. Any non-zero value is a valid i/o handle. In case there is an error it returns zero and the errno is set to the VM internal error number, which can be obtained with `getlasterr()`.

### ID=9 `fs_stat(path_ptr32, stat_ptr32)`

Retrieves information about the file specified by the null-terminated string pointed to by the first argument. The second argument should point to the location where the stat structure should be written to. The stat structure written is the one provided by the underlying OS, as-is.

### ID=10 `fs_unlink(path_ptr32)`

Unlinks the file specified by the null-terminated string pointed to by the first argument from the filesystem.

### ID=11 `fs_seek(handle64, offset64, whence64)`

Change at which offset the next read/write operation will happen in the i/o handler provided. The handler must have been obtained uusing the `fs_open()` system call.

### ID=12 `fs_mkdir(path_ptr32)`

Create the directory specified by the null-terminated string pointed to by the first argument.

### ID=13 `fs_rmdir(path_ptr32)`

Remove the directory specified by the null-terminated string pointed to by the first argument. The directory must be empty.

### ID=14 `fs_readdir(path_ptr32, buf32, size64, n_saved_entries_ptr32, n_total_entries_ptr32, min_buf_sz_ptr32)`

Read the contents of the directory specified by the null-terminated string pointed to by the first argument into the buffer pointed to by the second argument, which is assumed to be of maximum length equal to the size given as the third argument. Note that the special entries . & .. are included in the results.

Upon completion of this call, assuming no errors occured, the buffer will contain an array of 32bit pointers into strings that are also stored into itself. The array is null-terminated, as well as the strings. In addition, assuming they are not null, the locations pointed to by the last three arguments are populated with the following information respectively:
> number of entries saved into the buffer<br>
> number of total entries in the directory (includes . & .. entries)<br>
> minimum size required for the buffer to be able to contain all of the results<br>

## Network I/O

> TBD

## Memory management

> TBD

## Process management

> TBD

## Code management

> TBD
