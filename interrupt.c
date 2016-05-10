#include "types.h"
#include "Screen.h"
#include "kbd.h"
#include "io.h"
#include "gdt.h"
#include "scheduler.h"
#include "interrupt.h"

void love();

void isr_default_int()
{
	//print("An interrupt has been catch !\n");
}

void isr_default_exc()
{
	Screen::getScreen().println("An Exception occurred !");
}

void isr_clock_int()
{
	static int tic = 0;
	static int sec = 0;
	
	tic++;
	
    if(tic % 50 == 0)
	{
		sec++;
		tic = 0;
		
		if(Screen::getScreen().isLoading())
			Screen::getScreen().showTic();
        else
            Screen::getScreen().putcar('.');
	}
	
	schedule();
}

void isr_GP_exc(u32 error)
{
    u32 fault_addr;
    char *opcode = (char*)fault_addr;

    //asm("movl 60(%%ebp), %%eax;"
      //  "mov %%eax, %0;" : "=m"(fault_addr):);

    Screen::getScreen().setPos(0, 0);
    Screen::getScreen().clean();
    Screen::getScreen().printError("#GP");
    Screen::getScreen().printError("Faulting address : %p", fault_addr);

    if(error != 0)
    {
        Screen::getScreen().printError("Error's origin : %s", (error & 0x1) ? "External" : "Internal");

        switch ((error >> 1) & 0x3)
        {
        case 0x0:
            Screen::getScreen().printError("GDT Selector");
            break;
        case 0x1:
            Screen::getScreen().printError("IDT Selector");
            break;
        case 0x2:
            Screen::getScreen().printError("LDT Selector");
            break;
        case 0x3:
            Screen::getScreen().printError("IDT Selector");
            break;
        default:
            break;
        }

        Screen::getScreen().printError("Selector : %x", (error >> 3) & 0x1FFF);
    }
    else
        Screen::getScreen().printError("Unknown error");
	
	asm("hlt");
}

void isr_PF_exc(u32 error)
{
	u32 faulting_addr;
	u32 eip;
    char *addr = (char*)faulting_addr;
	
	asm("	movl 60(%%ebp), %%eax;	\
			mov %%eax, %0;			\
			mov %%cr2, %%eax;		\
			mov %%eax, %1": "=m"(eip), "=m"(faulting_addr): );

    Screen::getScreen().setPos(0, 0);
    Screen::getScreen().clean();
    Screen::getScreen().printError("#PF");
    Screen::getScreen().printError("Faulting addr %p, EIP %p", faulting_addr, eip);

    Screen::getScreen().printError("Error desc :");

    Screen::getScreen().printError("%s", (error & 0x1) ? "Protection violation" : "Non-present Page");
    Screen::getScreen().printError("Caused by %s access", (error & 0x2) ? "write" : "read");
    Screen::getScreen().printError("In %s mode", (error & 0x4) ? "user" : "root");

    if(error & 0x8)
        Screen::getScreen().printError("Page entry with reserved bit(s) set");
    if(error & 0x10)
        Screen::getScreen().printError("Fault caused by instruction fetch");

	
	asm("hlt");
}

void isr_kbd_int()
{
	uchar i;
	static int lshift_enable;
	static int rshift_enable;
	static int alt_enable;
	static int ctrl_enable;
	
	do
	{
		i = inb(0x64);
	}while((i & 0x01) == 0);
	
	i = inb(0x60);
	i--;
	
	if(i < 0x80)
	{
		switch(i)
		{
			case 0x29:
				lshift_enable = 1;
				break;
			case 0x35:
				rshift_enable = 1;
				break;
			case 0x1C:
				ctrl_enable = 1;
				break;
			case 0x37:
				alt_enable = 1;
				break;
			default:
				Screen::getScreen().putcar(kbdmap[i * 4 + (lshift_enable || rshift_enable)]);
		}
	}
	else
	{
		i -= 0x80;
		
		switch(i)
		{
			case 0x29:
				lshift_enable = 0;
				break;
			case 0x35:
				rshift_enable = 0;
				break;
			case 0x1C:
				ctrl_enable = 0;
				break;
			case 0x37:
				alt_enable = 0;
				break;
		}
	}
	
    /*if(kbdmap[i * 4] == 'l')
        Screen::getScreen().showLoadScreen();*/
}

void love()
{
	Screen::getScreen().print("Coucou ! Je suis le kernel et je t'aime fort !!\n");
	Screen::getScreen().show_cursor();
}

void do_syscall(int sys_num)
{
	if(sys_num == 1)
	{
		char *u_str;
	
		asm("mov %%ebx, %0" : "=m"(u_str) :);
		for(int i = 0; i < 10000; i++); //temporisation
		cli;
		Screen::getScreen().print(u_str);
		sti;
	}
	else
		Screen::getScreen().printError("Unknown syscall !");
	return;
}
