
#ifndef __IO_H__
#define __IO_H__

#include <linux/kvm.h>

int run_io(struct kvm_run * run);

#endif // __IO_H__