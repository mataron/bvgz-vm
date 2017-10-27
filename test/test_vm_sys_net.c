#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "tests.h"
#include "vm.h"


#define PRG_PATH    "/test/asm/sys/"

struct sockaddr_in google_addr;


static void test_net_connect()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH
        "net-connect.bvgzs");

    *(uint32_t*)(vm->memory + 8) = google_addr.sin_addr.s_addr;
    *(uint16_t*)(vm->memory + 12) = htons(80);

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


static void test_http_get()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH
        "http-get.bvgzs");

    *(uint32_t*)(vm->memory + 8) = google_addr.sin_addr.s_addr;
    *(uint16_t*)(vm->memory + 12) = htons(80);

    execute_vm(vm);
    print_vm_state(vm);

    printf("bytes read: %lu\n", *((uint64_t*)(vm->memory + 22)));
    printf("%s\n", (char*)(vm->memory + 167));

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->error_no == 0);
    assert(*((uint64_t*)vm->memory) == 0);
    assert(*((uint64_t*)(vm->memory + 14)) > 0); // the fd
    assert(vm->io.used_fds == 0);

    destroy_vm(vm);
}


static void* echo_srv_thread()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH
        "echo-srv.bvgzs");

    *(uint32_t*)(vm->memory + 8) = inet_addr("127.0.0.1");
    *(uint16_t*)(vm->memory + 12) = htons(22280);

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->error_no == 0);
    assert(*((uint64_t*)vm->memory) == 0);
    assert(*((uint64_t*)(vm->memory + 14)) > 0); // the fd
    assert(vm->io.used_fds == 0);

    destroy_vm(vm);
    pthread_exit(NULL);
    return NULL;
}

static void* echo_cli_thread()
{
    const char* str = "asdfajsdfh;jasdhfjkas";
    char str2[1024];

    usleep(250000);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(fd > 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(22280);

    puts("CLI: connecting");
    int c = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    assert(c == 0);

    printf("CLI: write(%lu)\n", sizeof(str));
    int w = write(fd, str, sizeof(str));
    assert(w == sizeof(str));

    puts("CLI: read");
    int r = read(fd, str2, 1024);
    // assert(r == sizeof(str));

    printf("CLI: test(%d) & close\n", r);
    assert(strncmp(str, str2, r) == 0);

    close(fd);
    return NULL;
}

static void test_echo_srv()
{
    int r;
    pthread_t srv_th, cli_th;
    void* x = NULL;

    r = pthread_create(&srv_th, NULL, &echo_srv_thread, x);
    assert(r == 0);

    r = pthread_create(&cli_th, NULL, &echo_cli_thread, x);
    assert(r == 0);

    r = pthread_join(srv_th, NULL);
    assert(r == 0);

    r = pthread_join(cli_th, NULL);
    assert(r == 0);
}


int main(int argc, char** argv)
{
    struct addrinfo* result;
    int err = getaddrinfo("www.google.com", NULL, NULL, &result);
    assert(err == 0);
    assert(result != NULL);

    google_addr = *(struct sockaddr_in*)result->ai_addr;

    freeaddrinfo(result);

    initialize_engine(NULL);

    test_net_connect();
    test_http_get();
    test_echo_srv();

    return 0;
}
