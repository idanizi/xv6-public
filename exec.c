#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

/*
 * DONE: 2. Exec – should start running on a single thread of the new process.
 */
int
exec(char *path, char **argv)
{
  char *s, *last;
  int i, off;
  uint argc, sz, sp, ustack[3+MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;

  begin_op();
  if((ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  ilock(ip);
  pgdir = 0;

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) < sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  if((pgdir = setupkvm()) == 0)
    goto bad;

  // Load program into memory.
  sz = 0;
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
      goto bad;
    if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }
  iunlockput(ip);
  end_op();
  ip = 0;

  // Allocate two pages at the next page boundary.
  // Make the first inaccessible.  Use the second as the user stack.
  sz = PGROUNDUP(sz);
  if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
    goto bad;
  clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
  sp = sz;

  // Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(proc->name, last, sizeof(proc->name));

  // Commit to the user image.
  // changed #task1.1
  // critical section for threads
  /*
   * Note that in a multi-threaded environment a thread might be running while another thread of the same process is attempting to perform exec.
   * The thread performing exec should “tell” other threads of the same process to
   * destroy themselves and only then complete the exec task.
   * Hint: You can review the code that is performed when the proc->killed field is set and write your implementation
   * similarly.
   */
  struct thread *t;
  acquire(thread->parent->threadTable.lock);
  for (t = thread->parent->threadTable.threads; t < &thread->parent->threadTable.threads[NTHREAD]; t++) {
    if (t->tid != thread->tid && t->state != T_UNUSED) {
      t->killed = 1;
      // wake threads from sleep to finish their run
      if (t->state == T_SLEEPING) t->state = T_RUNNABLE;
    }
  }
  release(proc->threadTable.lock);

  // look for killed threads and join them (wait for them to finish their run)
  for (t = thread->parent->threadTable.threads; t < &thread->parent->threadTable.threads[NTHREAD]; t++) {
    if(t->killed)
      kthread_join(t->tid);
  }

  //only the thread preforming exec doing this piece of code
  oldpgdir = thread->parent->pgdir;
  thread->parent->pgdir = pgdir;
  thread->parent->sz = sz;
  thread->tf->eip = elf.entry;  // main
  thread->tf->esp = sp;
  switchuvm(thread);
  freevm(oldpgdir);
  // end of critical section
  // changed #end
  return 0;

 bad:
  if(pgdir)
    freevm(pgdir);
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}
