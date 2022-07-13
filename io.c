/**
 *   Copyright 2022 Ray Cole
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

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
