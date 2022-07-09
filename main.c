
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/mman.h>

#include "vm.h"
#include "io.h"
#include "logging.h"

int run_mmio(struct kvm_run * run)
{
    if (run->mmio.is_write)
    {
        printf("mmio(IN): write([addr(0x%llx), data(%d, ", (unsigned long long)run->mmio.phys_addr, run->mmio.len);
        for (int i=0; i<run->mmio.len; i++)
        {
            printf("%x", run->mmio.data[i]);
            if (i > 0 && i % 4 == 0 && i < run->mmio.len-1)
            {
                printf(":");
            }
        }
        printf(")\n");
    }
    else
    {
        printf("mmio_in: read[addr(0x%llx), len(%d)]\n", (unsigned long long)run->mmio.phys_addr, run->mmio.len);
    }

    return 0;
}

int main(int argc, char **argv)
{
    const uint8_t code[] = {
        0xba, 0xf8, 0x03, /* mov $0x3f8, %dx */
        0x00, 0xd8,       /* add %bl, %al */
        0x04, '0',        /* add $'0', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, '\n',       /* mov $'\n', %al */
        0xee,             /* out %al, (%dx) */
        0xf4,             /* hlt */
    };

    Hypervisor hv;
    if (init_hypervisor(&hv) < 0)
        return -1;

    Machine machine;
    if (create_machine(&hv, &machine, code, sizeof(code)) < 0)
        return -1;

    if (set_registers(&machine) < 0)
        return -1;

    while(1)
    {
        if (run_machine(&machine) == NULL)
            return -1;

        switch(machine.run->exit_reason)
        {
            case KVM_EXIT_HLT:
                log("KVM_EXIT_HALT");
                return 0;

            case KVM_EXIT_IO:
                if (run_io(machine.run))
                {
                    log("failed IO");
                    return -1;
                }
                break;

            case KVM_EXIT_MMIO:
                if (run_mmio(machine.run))
                {
                    log("failed MMIO");
                    return -1;
                }
                break;

            case KVM_EXIT_FAIL_ENTRY:
                log("KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_reason = 0x%llx",
                            (unsigned long long)machine.run->fail_entry.hardware_entry_failure_reason);
                return -1;

            case KVM_EXIT_INTERNAL_ERROR:
                log("KVM_EXIT_INTERNAL_ERROR: suberror = 0x%x", machine.run->internal.suberror);
                return -1;

            case KVM_EXIT_UNKNOWN:
                log("KVM_EXIT_UNKNOWN: hardware_exit_reason = 0x%llx",
                            (unsigned long long)machine.run->hw.hardware_exit_reason);
                return -1;

            case KVM_EXIT_DEBUG:
                break;

            default:
                break;
        }
    }
}

