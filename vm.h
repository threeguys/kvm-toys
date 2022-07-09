
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
