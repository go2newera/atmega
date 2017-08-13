/*第四篇：只有延时服务的协作式的内核   
 
                                      Cooperative Multitasking  
   前后台系统，协作式内核系统，与占先式内核系统，有什么不同呢？  
   记得在21IC上看过这样的比喻,“你(小工)在用厕所，经理在外面排第一，老板在外面排第
二。如果是前后台，不管是谁，都必须按排队的次序使用厕所；如果是协作式，那么可以等
你用完厕所，老板就要比经理先进入；如果是占先式，只要有更高级的人在外面等，那么厕
所里无论是谁，都要第一时间让出来，让最高级别的人先用。”  
*/

#include <avr/io.h>  
#include <avr/Interrupt.h>  
#include <avr/signal.h>  
unsigned char Stack[200];  
 
register unsigned char OSRdyTbl          /*asm("r2")*/;    //任务运行就绪表  
register unsigned char OSTaskRunningPrio /*asm("r3")*/;    //正在运行的任务  
 
#define OS_TASKS 3                    //设定运行任务的数量  
struct TaskCtrBlock           //任务控制块  
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
  *Stack--=0x80;                                        //SREG 在任务中，开启全局中断          
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
  __asm__ __volatile__("PUSH __tmp_reg__ \t");  //R0   
  __asm__ __volatile__("IN   __tmp_reg__,__SREG__ \t");  //保存状态寄存器SREG  
  __asm__ __volatile__("PUSH __tmp_reg__\t");  
  __asm__ __volatile__("CLR  __zero_reg__\t");  //R0重新清零  
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
  __asm__ __volatile__("PUSH R28\t");  //R28与R29用于建立在堆栈上的指针  
  __asm__ __volatile__("PUSH R29\t");  //入栈完成  
      
  TCB[OSTaskRunningPrio].OSTaskStackTop=SP;           //将正在运行的任务的堆栈底保存  
      
     unsigned char OSNextTaskID;                             //在现有堆栈上开设新的空间   
  for (OSNextTaskID = 0;                                  //进行任务调度  
    OSNextTaskID < OS_TASKS && !(OSRdyTbl & (0x01<<OSNextTaskID));   
    OSNextTaskID++);  
    OSTaskRunningPrio = OSNextTaskID ;  
 
  cli();  //保护堆栈转换  
  SP=TCB[OSTaskRunningPrio].OSTaskStackTop;  
  sei();  
      
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
  __asm__ __volatile__("POP  __tmp_reg__ \t");      //SERG 出栈并恢复  
  __asm__ __volatile__("OUT  __SREG__,__tmp_reg__ \t");      //  
  __asm__ __volatile__("POP  __tmp_reg__ \t");      //R0 出栈  
  __asm__ __volatile__("POP  __zero_reg__ \t");      //R1 出栈  
  //中断时出栈完成  
}  
 
void OSTimeDly(unsigned int ticks)  
{  
  if(ticks)                             //当延时有效  
  {  
    OSRdyTbl &= ~(0x01<<OSTaskRunningPrio);  
    TCB[OSTaskRunningPrio].OSWaitTick=ticks;  
    OSSched();                          //从新调度  
  }  
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
  unsigned char i;  
  for(i=0;i<OS_TASKS;i++)       //任务时钟  
  {  
    if(TCB[i].OSWaitTick)   
    {  
      TCB[i].OSWaitTick--;  
      if(TCB[i].OSWaitTick==0)     //当任务时钟到时,必须是由定时器减时的才行  
      {    
        OSRdyTbl |= (0x01<<i);     //使任务在就绪表中置位     
      }  
    }  
  }  
  TCNT0=100;  
}  
 
void Task0()  
{  
  unsigned int j=0;  
  while(1)  
  {              
    PORTB=j++;  
    OSTimeDly(2);  
  }  
}  
 
void Task1()  
{  
  unsigned int j=0;  
  while(1)  
  {  
    PORTC=j++;  
    OSTimeDly(4);  
  }  
}  
 
void Task2()  
{  
  unsigned int j=0;  
  while(1)  
  {  
    PORTD=j++;     
    OSTimeDly(8);  
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
  OSTaskRunningPrio=0;  
  OSTaskCreate(Task0,&Stack[49],0);  
  OSTaskCreate(Task1,&Stack[99],1);  
  OSTaskCreate(Task2,&Stack[149],2);  
  OSTaskCreate(TaskScheduler,&Stack[199],OS_TASKS);  
  OSStartTask();  
}  
 
/*   在上面的例子中，一切变得很简单，三个正在运行的主任务，都通过延时服务，主动放弃
对CPU的控制权。  
   在时间中断中，对各个任务的的延时进行计时，如果某个任务的延时结束，将任务重新在
就绪表中置位。  
   最低级的系统任务TaskScheduler()，在三个主任务在放弃对CPU的控制权后开始不断地进
行调度。如果某个任务在就绪表中置位，通过调度，进入最高级别的任务中继续运行。*/