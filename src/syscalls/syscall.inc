
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
