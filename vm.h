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

#ifndef __VM_H__
#define __VM_H__

#include <stdint.h>
#include <linux/kvm.h>

#define PORT_TABLE_SIZE 0x100

typedef struct _PortTable {
    int port;
    struct _PortTable *next[PORT_TABLE_SIZE];
} PortTable;

typedef struct _Machine {
    int vm;
    int vcpu;
    uint8_t *memory;
    struct kvm_run *run;
    struct kvm_sregs sregs;
    struct kvm_regs regs;
    PortTable io;
} Machine;

typedef struct _Hypervisor {
    int kvm;
} Hypervisor;

int init_hypervisor(Hypervisor *hv);
int create_machine(Hypervisor *hv, Machine *m, const uint8_t *code, size_t code_size);
int set_registers(Machine *m);
struct kvm_run * run_machine(Machine *m);
void dump_state(Machine *m);

#endif // __VM_H__
