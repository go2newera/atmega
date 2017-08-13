/*第六篇：时间片轮番调度法的内核   
 
                                 Round-Robin Sheduling  
 
    时间片轮调法是非常有趣的。本篇中的例子，建立了3个任务，任务没有优先级，在时间
中断的调度下，每个任务都轮流运行相同的时间。如果在内核中没有加入其它服务，感觉上
就好像是有三个大循环在同时运行。  
 
    本例只是提供了一个用时间中断进行调度的内核，大家可以根据自己的需要，添加相应的
服务。  
    要注意到:   
    1,由于在时间中断内调用了任务切换函数，因为在进入中断时，已经将一系列的寄存器入
栈。  
    2,在中断内进行调度，是直接通过"RJMP Int_OSSched"进入任务切换和调度的，这是
GCC AVR的一个特点，为用C编写内核提供了极大的方便。  
    3,在阅读代码的同时，请对照阅读编译器产生的 *.lst文件，会对你理解例子有很大的帮助。  
*/     
#include <avr/io.h>  
#include <avr/Interrupt.h>  
#include <avr/signal.h>  
unsigned char Stack[400];  
 
register unsigned char OSRdyTbl          /*asm("r2")*/;    //任务运行就绪表  
register unsigned char OSTaskRunningPrio /*asm("r3")*/;    //正在运行的任务  
 
#define OS_TASKS 3                    //设定运行任务的数量  
struct TaskCtrBlock  
{  
  unsigned int OSTaskStackTop;  //保存任务的堆栈顶  
  unsigned int OSWaitTick;      //任务延时时钟  
} TCB[OS_TASKS+1];  
 
//防止被编译器占用  
register unsigned char tempR4  asm("r4");  
register unsigned char tempR5  asm("r5");  
register unsigned char tempR6  asm("r6");  
register unsigned char tempR7  asm("r7");  
register unsigned char tempR8  asm("r8");  
register unsigned char tempR9  asm("r9");  
register unsigned char tempR10 asm("r10");  
register unsigned char tempR11 asm("r11");  
register unsigned char tempR12 asm("r12");  
register unsigned char tempR13 asm("r13");  
register unsigned char tempR14 asm("r14");  
register unsigned char tempR15 asm("r15");  
register unsigned char tempR16 asm("r16");  
register unsigned char tempR16 asm("r17");  
 
 
//建立任务  
void OSTaskCreate(void (*Task)(void),unsigned char *Stack,unsigned char TaskID)  
{  
  unsigned char i;                       
  *Stack--=(unsigned int)Task>>8;    //将任务的地址高位压入堆栈，  
  *Stack--=(unsigned int)Task;         //将任务的地址低位压入堆栈，  
      
  *Stack--=0x00;                     //R1 __zero_reg__              
  *Stack--=0x00;                     //R0 __tmp_reg__  
  *Stack--=0x80;                                          
 
//SREG 在任务中，开启全局中断          
  for(i=0;i<14;i++)    //在 avr-libc 中的 FAQ中的 What registers are used by the C compiler?  
    *Stack--=i;                    //描述了寄存器的作用      
  TCB[TaskID].OSTaskStackTop=(unsigned int)Stack;    //将人工堆栈的栈顶，保存到堆栈的数组中  
  OSRdyTbl|=0x01<<TaskID;      //任务就绪表已经准备好  
}  
 
//开始任务调度,从最低优先级的任务的开始  
void OSStartTask()          
{  
  OSTaskRunningPrio=OS_TASKS;  
  SP=TCB[OS_TASKS].OSTaskStackTop+17;  
  __asm__ __volatile__(    "reti"       " \t"  );   
}  
 
//进行任务调度  
void OSSched(void)  
{   
   //  根据中断时保存寄存器的次序入栈，模拟一次中断后，入栈的情况    
  __asm__ __volatile__("PUSH __zero_reg__ \t");  //R1  
  __asm__ __volatile__("PUSH __tmp_reg__\t");  //R0   
  __asm__ __volatile__("IN   __tmp_reg__,__SREG__\t");  //保存状态寄存器SREG  
  __asm__ __volatile__("PUSH __tmp_reg__\t");  
  __asm__ __volatile__("CLR  __zero_reg__ \t");  //R0重新清零  
  __asm__ __volatile__("PUSH R18\t");  
  __asm__ __volatile__("PUSH R19\t");  
  __asm__ __volatile__("PUSH R20\t");  
  __asm__ __volatile__("PUSH R21\t");  
  __asm__ __volatile__("PUSH R22\t");  
  __asm__ __volatile__("PUSH R23\t");  
  __asm__ __volatile__("PUSH R24\t");  
  __asm__ __volatile__("PUSH R25\t");  
  __asm__ __volatile__("PUSH R26\t");  
  __asm__ __volatile__("PUSH R27\t");  
  __asm__ __volatile__("PUSH R30\t");      
  __asm__ __volatile__("PUSH R31\t");  
      
  __asm__ __volatile__("Int_OSSched: \t");  //当中断要求调度，直接进入这里  
  __asm__ __volatile__("PUSH R28\t");  //R28与R29用于建立在堆栈上的指针  
  __asm__ __volatile__("PUSH R29\t");  //入栈完成  
      
  TCB[OSTaskRunningPrio].OSTaskStackTop=SP;           //将正在运行的任务的堆栈底保存  
 
  if(++OSTaskRunningPrio>=OS_TASKS) //轮流运行各个任务，没有优先级  
      OSTaskRunningPrio=0;  
 
  //cli();  //保护堆栈转换  
  SP=TCB[OSTaskRunningPrio].OSTaskStackTop;  
  //sei();  
      
    //根据中断时的出栈次序      
  __asm__ __volatile__("POP  R29\t");      
  __asm__ __volatile__("POP  R28\t");          
  __asm__ __volatile__("POP  R31\t");      
  __asm__ __volatile__("POP  R30\t");      
  __asm__ __volatile__("POP  R27\t");      
  __asm__ __volatile__("POP  R26\t");      
  __asm__ __volatile__("POP  R25\t");      
  __asm__ __volatile__("POP  R24\t");      
  __asm__ __volatile__("POP  R23\t");      
  __asm__ __volatile__("POP  R22\t");      
  __asm__ __volatile__("POP  R21\t");      
  __asm__ __volatile__("POP  R20\t");      
  __asm__ __volatile__("POP  R19\t");  
  __asm__ __volatile__("POP  R18\t");  
  __asm__ __volatile__("POP  __tmp_reg__\t");      //SERG 出栈并恢复  
  __asm__ __volatile__("OUT  __SREG__,__tmp_reg__ \t");      //  
  __asm__ __volatile__("POP  __tmp_reg__ \t");      //R0 出栈  
  __asm__ __volatile__("POP  __zero_reg__ \t");      //R1 出栈  
  __asm__ __volatile__("RETI\t");     //返回并开中断  
  //中断时出栈完成  
}  
 
 
void IntSwitch(void)  
{      
  __asm__ __volatile__("POP  R31\t");  //去除因调用子程序而入栈的PC  
  __asm__ __volatile__("POP  R31\t");  
  __asm__ __volatile__("RJMP Int_OSSched\t");  //重新调度  
}  
 
void TCN0Init(void)    // 计时器0  
{  
  TCCR0 = 0;  
  TCCR0 |= (1<<CS02);  // 256预分频  
  TIMSK |= (1<<TOIE0); // T0溢出中断允许                    
  TCNT0 = 100;         // 置计数起始值      
}  
 
 
SIGNAL(SIG_OVERFLOW0)  
{  
  TCNT0=100;  
  IntSwitch();        //任务调度  
}  
 
void Task0()  
{  
  unsigned int j=0;  
  while(1)  
  {              
    PORTB=j++;  
    //OSTimeDly(50);  
  }  
}  
 
void Task1()  
{  
  unsigned int j=0;  
  while(1)  
  {  
    PORTC=j++;  
    //OSTimeDly(5);  
  }  
}  
 
void Task2()  
{  
  unsigned int j=0;  
  while(1)  
  {  
    PORTD=j++;   
    //OSTimeDly(5);    
  }  
}  
 
void TaskScheduler()  
{   
  while(1)  
  {          
     OSSched();      //反复进行调度  
  }  
}  
 
 
int main(void)  
{      
  TCN0Init();  
  OSRdyTbl=0;  
  OSTaskCreate(Task0,&Stack[99],0);  
  OSTaskCreate(Task1,&Stack[199],1);  
  OSTaskCreate(Task2,&Stack[299],2);  
  OSTaskCreate(TaskScheduler,&Stack[399],OS_TASKS);  
  OSStartTask();  
} 