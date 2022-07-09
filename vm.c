
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/mman.h>

#include "vm.h"
#include "logging.h"

static int init_ports(PortTable *ports);

int init_hypervisor(Hypervisor *hv)
{
    int kvm = open("/dev/kvm", O_RDWR | O_CLOEXEC);
    if (kvm == -1)
    {
        logerr("Could not open kvm");
        return -EPERM;
    }

    int ret = ioctl(kvm, KVM_GET_API_VERSION, NULL);
    if (ret != 12)
    {
        log("Bad kvm api version, expected 12 got %d", ret);
        return -EBADF;
    }

    ret = ioctl(kvm, KVM_CHECK_EXTENSION, KVM_CAP_USER_MEMORY);
    if (ret == -1)
    {
        logerr("kvm check extension failed");
        return -ENODEV;
    }
    if (!ret)
    {
        logerr("Requires KVM_CAP_USER_MEMORY");
        return -ENOMEM;
    }

    hv->kvm = kvm;
    return 0;
}

static int init_ports(PortTable *ports)
{
    ports->port = 0;
    memset(ports->next, 0, sizeof(PortTable*) * PORT_TABLE_SIZE);
    return 0;
}

int create_machine(Hypervisor *hv, Machine *m, const uint8_t *code, size_t code_size)
{
    int vmfd = ioctl(hv->kvm, KVM_CREATE_VM, (unsigned long)0);
    if (vmfd == -1)
    {
        logerr("Could not create VM");
        return -EINTR;
    }

    uint8_t * mem = (uint8_t*) mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mem == NULL)
    {
        logerr("Could not map memory");
        return -ENOMEM;
    }

    memcpy(mem, code, code_size);

    struct kvm_userspace_memory_region region = {
        .slot = 0,
        .guest_phys_addr = 0x1000,
        .memory_size = 0x1000,
        .userspace_addr = (uint64_t)mem,
    };

    ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region);

    int vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, (unsigned long)0);
    if (vcpufd < 0)
    {
        logerr("Could not create vCPU");
        return -EBUSY;
    }

    int mmap_size = ioctl(hv->kvm, KVM_GET_VCPU_MMAP_SIZE, NULL);
    struct kvm_run * run = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpufd, 0);
    if (run == NULL)
    {
        logerr("Could not allocate vCPU memory region");
        return -EFAULT;
    }

    m->vm = vmfd;
    m->vcpu = vcpufd;
    m->memory = mem;
    m->run = run;

    init_ports(&m->io);

    return 0;
}

int set_registers(Machine *m)
{
    // Set special registers
    if (ioctl(m->vcpu, KVM_GET_SREGS, &(m->sregs)) < 0)
        return -1;
    m->sregs.cs.base = 0;
    m->sregs.cs.selector = 0;

    if (ioctl(m->vcpu, KVM_SET_SREGS, &(m->sregs)) < 0)
        return -1;

    // Set normal registers
    m->regs.rip = 0x1000;
    m->regs.rax = 2;
    m->regs.rbx = 2;
    m->regs.rflags = 0x2;

    if (ioctl(m->vcpu, KVM_SET_REGS, &(m->regs)) < 0)
        return -1;

    return 0;
}

struct kvm_run * run_machine(Machine *m)
{
    if (ioctl(m->vcpu, KVM_RUN, NULL) < 0)
    {
        logerr("Could not run vCPU");
        return NULL;
    }
    return m->run;
}

void dump_state(Machine *m)
{
    printf("m:exit=%d\n",m->run->exit_reason);
    if (ioctl(m->vcpu, KVM_GET_REGS, &(m->regs)) >= 0)
        printf("  rip=>0x%llx\n", (unsigned long long) m->regs.rip);
}
