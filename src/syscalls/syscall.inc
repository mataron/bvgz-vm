
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
// error = read(handle64, ptr32, size64, cb_args_ptr32,
//              callback(handle64, errno, size64))
/* 6 */ SYSCALL(read)
// error = write(handle64, ptr32, size64, cb_args_ptr32,
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

// handle64 = socket(kind)
/* 15 */ SYSCALL(socket)
// error = bind(handle64, addr_ptr)
/* 16 */ SYSCALL(bind)
// error = connect(handle64, addr_ptr, cb_args_ptr,
//                 callback(handle64, errno))
/* 17 */ SYSCALL(connect)
// error = listen(handle64)
/* 18 */ SYSCALL(listen)
// error = accept(handle64, cb_args_ptr,
//                callback(handle64, errno))
/* 19 */ SYSCALL(accept)
// error = peer_address(handle64, addr_ptr)
/* 20 */ SYSCALL(peer_address)

// error = mexpand(size64)
/* 21 */ SYSCALL(mexpand)
// error = mretract(size64)
/* 22 */ SYSCALL(mretract)
// size = msize()
/* 23 */ SYSCALL(msize)

// pid = exec(argc64, argv32)
/* 24 */ SYSCALL(exec)
// pid = run(code32, codesz64, mem32, memsz64, entry64)
/* 25 */ SYSCALL(run)
// error = kill(pid64)
/* 26 */ SYSCALL(kill)
// error = onexit(pid64, cb_args_ptr,
//                callback(pid64, exited64, exit_status64,
//                         signaled64, signal64, stopped64,
//                         continued64))
/* 27 */ SYSCALL(onexit)

// size64 = codesz()
/* 28 */ SYSCALL(codesz)
// error = codecp(begin64, size64, ptr32)
/* 29 */ SYSCALL(codecp)
