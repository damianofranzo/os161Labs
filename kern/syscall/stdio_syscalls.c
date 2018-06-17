#include <types.h>
#include <clock.h>
#include <copyinout.h>
#include <syscall.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <synch.h>
#include <uio.h>
#include <vfs.h>
#include <proc.h>
#include <vnode.h>

//definiamo staticamente il numero massimo di file
#define MAXF 100

//qua ci andrebbe un lock o una primitiva di protezione
//in questa versione non si riciclano vecchi file descriptor
//esistono tanti modi per gestire questa ventualità, tipo vettori di
//boolean oppure mediante lista o allocazione dinamica

struct vnode *fdTable[MAXF];
int currFid = 3;

//versione minimale, creiamo un file con le protezioni standard
int sys_open(const char *filename, int flags){
  struct vnode *v;
  int fd, result;
  result = vfs_open((char *) filename, flags, 0 , &v);

  if(result) return -1;
  if(currFid>=MAXF) return -1;

  fd = currFid++;
  fdTable[fd] = v;
  return fd;

}

int sys_close(int fd){
  if(fd <3 || fd > MAXF) return 1;
  if(fdTable[fd] == NULL) return 1;
  vfs_close(fdTable[fd]);
  fdTable[fd] = NULL;
  return 0;
}


/*
//versione semplificata
int sys_write(int fd, const void *buf, size_t count)
{
	int result;
	unsigned int i;	  
	char *mybuf;
	mybuf = (char *)buf;
	
	if(fd==1)	//stampo solo su stdout
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
*/


int sys_write(int fd, const char* buf, size_t nbytes){

	int i, retval=0, result=0;
	switch(fd){
	   case 0:
		kprintf("Error - cannot print on STDIN\n");
		retval=0;
		break;

	   case 1:
	   case 2:
		
		for(i=0;i<(int) nbytes;i++){
			putch(buf[i]);
		}
		retval=nbytes;//nbytes scritti
		break;	
	   default:
	   {
		struct iovec iov;
		struct uio u;
		struct vnode *v=/*curproc->*/fdTable[fd];/*da file descriptor al vnode*/
		KASSERT(v!=NULL);
		/*settiamo struct per realizzare VOP_WRITE in spazio user*/
		iov.iov_ubase=(userptr_t) buf;//puntatore
		iov.iov_len=nbytes;//dimensione
		u.uio_iov=&iov;
		u.uio_iovcnt=1;
		u.uio_offset=0;//all'interno del buf
		u.uio_resid=nbytes;//residual id ->#byte residui che devo ancora scrivere. se alla fine dell i/o è >0 per qualche motivo non è stato terminato l'i/o 
		u.uio_segflg=UIO_USERSPACE;
		u.uio_rw=UIO_WRITE;
		
		u.uio_space=curproc->p_addrspace;

		result=VOP_WRITE(v,&u);
		if(result<0) retval=0;
		else retval=nbytes - u.uio_resid; //u.uio_resid se ok, vale 0
	   }
		break;
	}
	
	return retval;
}


/*
//versione semplice
int sys_read(int fd, void *buf, size_t count)
{
	int result;	
	
	if(fd==0)	//leggo solo da stdin
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
	
}*/


int sys_read(int fd, char* buf, size_t nbytes){

	int i, result=0,retval=0;
	char c;
	switch(fd){
	   case 1:
	   case 2:
		kprintf("Error - cannot read from STDOUT or ERR\n");
		retval=0;
		break;

	   case 0:
		for(i=0;i<(int) nbytes;i++){
			c=getch();
			*(buf+i)=c;
		}
		retval=nbytes;
		break;	
	   default:
	   {
		struct iovec iov;
		struct uio u;
		struct vnode *v=/*curproc->*/fdTable[fd];/*da file descriptor al vnode*/
		KASSERT(v!=NULL);
		/*settiamo struct per realizzare VOP_READ*/
		iov.iov_ubase=(userptr_t) buf;//puntatore
		iov.iov_len=nbytes;//dimensione
		u.uio_iov=&iov;
		u.uio_iovcnt=1;
		u.uio_offset=0;//all'interno del buf
		u.uio_resid=nbytes;//residual id ->#byte residui che devo ancora scrivere. se alla fine dell i/o è >0 per qualche motivo non è stato terminato l'i/o 
		u.uio_segflg=UIO_USERSPACE;
		u.uio_rw=UIO_READ;
		
		u.uio_space=curproc->p_addrspace;

		result=VOP_READ(v,&u);
		if(result<0 ) retval=0;
		else retval=nbytes - u.uio_resid;
	   }//u.uio_resid se ok, vale 0
		break;
	}
	
	return retval;
}




/*
int sys_exit(int code)
{
	curproc->exit_code = code;
	as_destroy(curproc->p_addrspace);	//only when wait IS NOT implemented
	//V(curproc->s);	//only when wait IS implemented
	thread_exit();
}*/
/*
int sys_waitpid(int pid)
{
	struct proc *p;
	p = get_process(pid);
	return proc_wait(p);
}*/

//sys remove riceve il nome del file, per testbin/filetest bisogna 
//implementarla, altrimenti il test fallisce.
