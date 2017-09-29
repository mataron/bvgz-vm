#include <string.h>
#include <errno.h>

#include "bytecode.h"
#include "instn.h"
#include "vm.h"


int32_t decode_instn(uint8_t* iptr, vm_t* vm, instn_t* instn)
{
    int32_t offset = 0;
    uint8_t instn_idx = *iptr >> 3;
    if (instn_idx > nInstnDefs)
    {
        return -1;
    }

    instn->code = *(uint16_t*)iptr;
    offset += 2;
    if (instn->code & 0x7)
    {
        instn->arg_sizes = *(iptr + offset);
        offset++;
    }

    for (int i = 0; i < InstnDefs[instn_idx].arg_count; i++)
    {
        if (instn->code & (1 << i))
        {
            int n_bytes = 1 << ((instn->arg_sizes >> (2 * i)) & 3);
            switch (n_bytes)
            {
            case 1:
                instn->args[i].u8 = *(iptr + offset);
                offset++;
                break;
            case 2:
                instn->args[i].u16 = *(uint16_t*)(iptr + offset);
                offset += 2;
                break;
            case 4:
                instn->args[i].u32 = *(uint32_t*)(iptr + offset);
                offset += 4;
                break;
            default:
                instn->args[i].u64 = *(uint64_t*)(iptr + offset);
                offset += 8;
                break;
            }
        }
        else
        {
            uint32_t ref = *(uint32_t*)(iptr + offset);
            instn->args[i].ptr = deref_mem_ptr(ref, vm);
            if (!instn->args[i].ptr)
            {
                return -1;
            }
            offset += 4;
        }
    }

    return offset;
}


uint8_t* deref_mem_ptr(uint32_t ref, vm_t* vm)
{
    if (ref + 8 > vm->memsz) // 8 : 64bit arg
    {
        vm->exceptions |= VM_E_MemFault;
        return NULL;
    }
    return vm->memory + ref;
}


vm_t* read_bvgz_image(FILE *fp)
{
    uint16_t magic = 0;
    if (fread(&magic, sizeof(uint16_t), 1, fp) != sizeof(uint16_t))
    {
        fprintf(stderr, "fread(magic): %s\n", strerror(errno));
        return NULL;
    }
    if (magic != BVGZ_IMG_MAGIC)
    {
        fprintf(stderr, "bad magic: expected 0x%X, found: 0x%x\n",
            BVGZ_IMG_MAGIC, magic);
        return NULL;
    }

    uint16_t flags;
    if (fread(&flags, sizeof(uint16_t), 1, fp) != sizeof(uint16_t))
    {
        fprintf(stderr, "fread(flags): %s\n", strerror(errno));
        return NULL;
    }
    if ((flags & BVGZ_IMG_F_EXEC) == 0)
    {
        fprintf(stderr, "input file is not an executable bvgz image\n");
        return NULL;
    }

    uint32_t entry_label = 0;
    if (fread(&entry_label, sizeof(uint32_t), 1, fp) !=
        sizeof(uint32_t))
    {
        fprintf(stderr, "fread(entry): %s\n", strerror(errno));
        return NULL;
    }

    uint32_t codesz;
    if (fread(&codesz, sizeof(uint32_t), 1, fp) != sizeof(uint32_t))
    {
        fprintf(stderr, "fread(codesz): %s\n", strerror(errno));
        return NULL;
    }
    if (!codesz)
    {
        fprintf(stderr, "zero code size, aborting");
        return NULL;
    }

    uint32_t memsz;
    if (fread(&memsz, sizeof(uint32_t), 1, fp) !=
        sizeof(uint32_t))
    {
        fprintf(stderr, "fread(memsz): %s\n", strerror(errno));
        return NULL;
    }

    vm_t* vm = make_vm(codesz, memsz, entry_label);

    if (fread(vm->code, codesz, 1, fp) != codesz)
    {
        fprintf(stderr, "fread(code:%u): %s\n",
            codesz, strerror(errno));
        goto error;
    }

    if (fread(vm->memory, memsz, 1, fp) != memsz)
    {
        fprintf(stderr, "fread(mem:%u): %s\n", memsz, strerror(errno));
        goto error;
    }

    return vm;
error:
    destroy_vm(vm);
    return NULL;
}
