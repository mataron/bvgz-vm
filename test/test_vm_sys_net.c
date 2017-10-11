#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "tests.h"
#include "vm.h"


#define PRG_PATH    "/test/asm/sys/"

struct sockaddr_in google_addr;


static void test_net_connect()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH
        "net-connect.s");
    
    *(uint32_t*)(vm->memory + 8) = google_addr.sin_addr.s_addr;
    *(uint16_t*)(vm->memory + 12) = google_addr.sin_port;

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->error_no == 0);
    assert(vm->io.used_fds == 0);
    assert(*((uint64_t*)vm->memory) == 0);
    assert(*((uint64_t*)(vm->memory + 14)) > 0); // the fd

    destroy_vm(vm);
}


int main(int argc, char** argv)
{
    struct addrinfo* result;
    int err = getaddrinfo("www.google.com", NULL, NULL, &result);
    assert(err == 0);
    assert(result != NULL);

    google_addr = *(struct sockaddr_in*)result->ai_addr;

    freeaddrinfo(result);

    initialize_engine();

    test_net_connect();

    return 0;
}
