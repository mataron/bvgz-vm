#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "tests.h"
#include "vm.h"


#define PRG_PATH    "/test/asm/sys/"


int main(int argc, char** argv)
{
    initialize_engine();

    return 0;
}
