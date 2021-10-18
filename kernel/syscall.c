#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

// Fetch the uint64 at addr from the current process.
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz)
    return -1;
  if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  int err = copyinstr(p->pagetable, buf, addr, max);
  if(err < 0)
    return err;
  return strlen(buf);
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  *ip = argraw(n);
  return 0;
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
int
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
  return 0;
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  if(argaddr(n, &addr) < 0)
    return -1;
  return fetchstr(addr, buf, max);
}

extern uint64 sys_chdir(void);
extern uint64 sys_close(void);
extern uint64 sys_dup(void);
extern uint64 sys_exec(void);
extern uint64 sys_exit(void);
extern uint64 sys_fork(void);
extern uint64 sys_fstat(void);
extern uint64 sys_getpid(void);
extern uint64 sys_kill(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_mknod(void);
extern uint64 sys_open(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_unlink(void);
extern uint64 sys_wait(void);
extern uint64 sys_write(void);
extern uint64 sys_uptime(void);
extern uint64 sys_trace(void);
extern uint64 sys_sysinfo(void);

static uint64 (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
[SYS_trace]   sys_trace,
[SYS_sysinfo]   sys_sysinfo,
};

void
syscall(void)
{
  int num;
  uint64 ret;
  int arg_first;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    argint(0, &arg_first);
    ret = syscalls[num]();
    p->trapframe->a0 = ret;
    if (p->trace >> num & 1) {
      acquire(&p->lock);
      printf("%d: ", p->pid);
      release(&p->lock);
      switch (num) {
        case SYS_fork:
          printf("sys_fork(%d)", arg_first);
          break;
        case SYS_exit:
          printf("sys_exit(%d)", arg_first);
          break;
        case SYS_wait:
          printf("sys_wait(%d)", arg_first);
          break;
        case SYS_pipe:
          printf("sys_pipe(%d)", arg_first);
          break;
        case SYS_read:
          printf("sys_read(%d)", arg_first);
          break;
        case SYS_kill:
          printf("sys_kill(%d)", arg_first);
          break;
        case SYS_exec:
          printf("sys_exec(%d)", arg_first);
          break;
        case SYS_fstat:
          printf("sys_fstat(%d)", arg_first);
          break;
        case SYS_chdir:
          printf("sys_chdir(%d)", arg_first);
          break;
        case SYS_dup:
          printf("sys_dup(%d)", arg_first);
          break;
        case SYS_getpid:
          printf("sys_getpid(%d)", arg_first);
          break;
        case SYS_sbrk:
          printf("sys_sbrk(%d)", arg_first);
          break;
        case SYS_sleep:
          printf("sys_sleep(%d)", arg_first);
          break;
        case SYS_uptime:
          printf("sys_uptime(%d)", arg_first);
          break;
        case SYS_open:
          printf("sys_open(%d)", arg_first);
          break;
        case SYS_write:
          printf("sys_write(%d)", arg_first);
          break;
        case SYS_mknod:
          printf("sys_mknod(%d)", arg_first);
          break;
        case SYS_unlink:
          printf("sys_unlink(%d)", arg_first);
          break;
        case SYS_link:
          printf("sys_link(%d)", arg_first);
          break;
        case SYS_mkdir:
          printf("sys_mkdir(%d)", arg_first);
          break;
        case SYS_close:
          printf("sys_close(%d)", arg_first);
          break;
        case SYS_trace:
          printf("sys_trace(%d)", arg_first);
          break;
      }
      printf(" -> %d\n", ret);
    }
  } else {
    printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
