
// errno := getlasterr()
/* 0 */ SYSCALL(getlasterr)
// setlasterr(errno)
/* 1 */ SYSCALL(setlasterr)

// error = time(uint64_t[2] {seconds, nanoseconds})
/* 2 */ SYSCALL(time)
// error = timeout(function_ptr, timeout_milliseconds)
/* 3 */ SYSCALL(timeout)

// void yield()
/* 4 */ SYSCALL(yield)

// error = close(handle64)
/* 5 */ SYSCALL(close)
// error = read(handle64, ptr64, size64, cb_args_ptr,
//              callback(handle64, errno, size64))
/* 6 */ SYSCALL(read)
// error = write(handle64, ptr64, size64, cb_args_ptr,
//               callback(handle64, errno, size64))
/* 7 */ SYSCALL(write)

// handle64 = fs_open(path, options64)
// handle64 is zero on error
/* 8 */ SYSCALL(fs_open)
// error = fs_stat(path, stat_ptr)
/* 9 */ SYSCALL(fs_stat)
// error = fs_unlink(path)
/* 10 */ SYSCALL(fs_unlink)
// error = fs_seek(handle64, offset64, whence64)
/* 11 */ SYSCALL(fs_seek)

// error = fs_mkdir(path)
/* 12 */ SYSCALL(fs_mkdir)
// error = fs_rmdir(path)
/* 13 */ SYSCALL(fs_rmdir)
// error = fs_readdir(path, buf32, buflen64,
//                    *n_saved_entries, *n_total_entries,
//                    *min_buf_sz)
/* 14 */ SYSCALL(fs_readdir)
