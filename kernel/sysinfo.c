#include "types.h"
#include "riscv.h"
#include "param.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_sysinfo(void)
{
    struct sysinfo info;
    uint64 addr;
    argaddr(0, &addr);

    info.freemem = freemem();
    info.nproc = numprocs();

    if (copyout(myproc()->pagetable, addr, (char *)&info, sizeof(info)) < 0)
        return -1;
    return 0;
}