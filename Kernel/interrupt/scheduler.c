#include <utils/types.h>
#include <memory/gdt.h>
#include <core/process.h>

#include <video/Screen.h>

void switch_to_task(int n, int mode)
{
	u32 kesp, eflags;
	u16 kss, ss, cs;
	
	current = &p_list[n];
	
	//charger TSS
	default_tss.ss0 = current->kstack.ss0;
	default_tss.esp0 = current->kstack.esp0;
	
	//empile les registres necessaires à la commut
	
	ss = current->regs.ss;
	cs = current->regs.cs;
    eflags = (current->regs.eflags | 0x200) & 0xFFFFBFFF;
	
	if(mode == USERMODE)
	{
		kss = current->kstack.ss0;
		kesp = current->kstack.esp0;
	}
	else
	{
		kss = current->regs.ss;
		kesp = current->regs.esp;
	}
	
	asm("	mov %0, %%ss; \
			mov %1, %%esp; \
			cmp %[KMODE], %[mode]; \
			je next; \
			push %2; \
			push %3; \
			next: \
			push %4; \
			push %5; \
			push %6; \
			push %7; \
            ljmp $0x08, $do_switch"
			:: \
			"m"(kss), \
			"m"(kesp), \
			"m"(ss), \
			"m"(current->regs.esp), \
			"m"(eflags), \
			"m"(cs), \
			"m"(current->regs.eip), \
			"m"(current), \
            [KMODE] "i"(KERNELMODE), \
			[mode] "g"(mode)
		);
}

void schedule()
{
	struct process *p;
	u32 *stack_ptr;
    int newpid;
	
	//Récup ds stack_ptr le poiteur vers registres sauvegardes
	asm("mov (%%ebp), %%eax; mov %%eax, %0" : "=m" (stack_ptr) : );
	
    if(!n_proc)
    {
		return;
    }
    else if(n_proc == 1 && current->pid != 0)
    {
		return;
    }
	else
	{
		current->regs.eflags = stack_ptr[16];
		current->regs.cs = stack_ptr[15];
		current->regs.eip = stack_ptr[14];
		current->regs.eax = stack_ptr[13];
		current->regs.ecx = stack_ptr[12];
		current->regs.edx = stack_ptr[11];
		current->regs.ebx = stack_ptr[10];
		current->regs.ebp = stack_ptr[8];
		current->regs.esi = stack_ptr[7];
		current->regs.edi = stack_ptr[6];
		current->regs.ds = stack_ptr[5];
		current->regs.es = stack_ptr[4];
		current->regs.fs = stack_ptr[3];
		current->regs.gs = stack_ptr[2];
		
		if(current->regs.cs != 0x08)
		{
			current->regs.esp = stack_ptr[17];
			current->regs.ss = stack_ptr[18];
		}
		else
		{
			current->regs.esp = stack_ptr[9] + 12;
			current->regs.ss = default_tss.ss0;
        }
		
		//sauve le tss
		current->kstack.ss0 = default_tss.ss0;
		current->kstack.esp0 = default_tss.esp0;
    }
		
    newpid = 0;
	
    for(unsigned int i = current->pid + 1; i < MAXPID && newpid == 0; i++)
		if(p_list[i].state == 1)
			newpid = i;
	
	if(!newpid)
        for(unsigned int i = 1; i < current->pid && newpid == 0; i++)
			if(p_list[i].state == 1)
				newpid = i;

	p = &p_list[newpid];
	//Commutation
	if(p->regs.cs != 0x08)
		switch_to_task(p->pid, USERMODE);
	else
		switch_to_task(p->pid, KERNELMODE);
}
