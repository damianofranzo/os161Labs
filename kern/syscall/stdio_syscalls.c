#include <types.h>
#include <clock.h>
#include <copyinout.h>
#include <syscall.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <synch.h>
/*
int sys_open(char *path, int openflags)
{
	kprintf("opening file %s\n", path);	
	return proc_file_open(path, openflags);
}

int sys_close(int fd)
{
	kprintf("closing file\n");	
	return proc_file_close(fd);
}*/

int sys_write(int fd, const void *buf, size_t count)
{
	int result;
	unsigned int i;
	char *mybuf;
	
	mybuf = (char *)buf;
	
	if(fd==1)	/*stampo solo su stdout*/
	{
		for(i=0;i<count;i++)
		{
			kprintf("%c",mybuf[i]);
			result = 0;
		}
	}
	else
	{
		kprintf("only support stdout\n");
		result = -1;
	}
	
	return result;
	
}


int sys_read(int fd, void *buf, size_t count)
{
	int result;	
	
	if(fd==0)	/*leggo solo da stdin*/
	{
		kgets((char *)buf, count);
		result = 0;
	}
	else
	{
		kprintf("only support stdin\n");
		result = -1;
	}
	
	return result;
	
}

int sys_exit(int code)
{
	curproc->exit_code = code;
	as_destroy(curproc->p_addrspace);	//only when wait IS NOT implemented
	//V(curproc->s);	//only when wait IS implemented
	thread_exit();
}
/*
int sys_waitpid(int pid)
{
	struct proc *p;
	p = get_process(pid);
	return proc_wait(p);
}*/
