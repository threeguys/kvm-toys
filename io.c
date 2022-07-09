
#include <stdint.h>
#include <stdio.h>

#include "io.h"

static char *printable(char c, char *buffer, int size);

static char *printable(char c, char *buffer, int size)
{
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
        snprintf(buffer, size, "%c", c);
    else
        switch(c)
        {
            case '\n':
                snprintf(buffer, size, "NL");
                break;
            case '\r':
                snprintf(buffer, size, "CR");
                break;
            default:
                snprintf(buffer, size, ".");
                break;
        }
    
    return buffer;
}

int run_io(struct kvm_run * run)
{
    char *data = ((char *) run) + run->io.data_offset;
    char *end = data + run->io.size;

    printf("IO(");
    if (run->io.direction == KVM_EXIT_IO_OUT)
        printf("out");
    else
        printf("in");
    printf(")[p:%x,s:%d,c:%d]) - [", run->io.port, run->io.size, run->io.count);

    char buffer[16];
    for (; data<end; data++)
        printf("%s", printable(*data, buffer, 16));

    printf("]\n");

    switch(run->io.direction)
    {
        case KVM_EXIT_IO_OUT:
            break;
        case KVM_EXIT_IO_IN:
            break;
        default:
            break;
    }

    return 0;
}
