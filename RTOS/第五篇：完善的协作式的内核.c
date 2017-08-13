/*第五篇：完善的协作式的内核   
 
   现在为上面的协作式内核添加一些OS中所必须的服务：  
   1  挂起和重新运行任务  
   2  信号量(在必要时候，可以扩展成邮箱和信息队列)  
   3  延时 
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
  __asm__ __volatile__("PUSH __zero_reg__\t");  //R1  
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
  __asm__ __volatile__("POP  __tmp_reg__\t");      //R0 出栈  
  __asm__ __volatile__("POP  __zero_reg__ \t");      //R1 出栈  
  //中断时出栈完成  
}  
 
 
////////////////////////////////////////////任务处理  
//挂起任务  
void OSTaskSuspend(unsigned char prio)   
{  
  TCB[prio].OSWaitTick=0;  
  OSRdyTbl &= ~(0x01<<prio); //从任务就绪表上去除标志位  
  if(OSTaskRunningPrio==prio)  //当要挂起的任务为当前任务  
    OSSched();               //从新调度  
}  
 
//恢复任务 可以让被OSTaskSuspend或 OSTimeDly暂停的任务恢复  
void OSTaskResume(unsigned char prio)  
{  
  OSRdyTbl |= 0x01<<prio;    //从任务就绪表上重置标志位  
    TCB[prio].OSWaitTick=0;        //将时间计时设为0,到时  
  if(OSTaskRunningPrio>prio)   //当要当前任务的优先级低于重置位的任务的优先级  
    OSSched();               //从新调度              //从新调度  
}  
 
// 任务延时  
void OSTimeDly(unsigned int ticks)  
{  
  if(ticks)                             //当延时有效  
  {  
    OSRdyTbl &= ~(0x01<<OSTaskRunningPrio);           
    TCB[OSTaskRunningPrio].OSWaitTick=ticks;  
    OSSched();                          //从新调度  
  }  
}  
 
 
//信号量  
struct SemBlk  
{  
  unsigned char OSEventType;     //型号 0,信号量独占型；1信号量共享型   
  unsigned char OSEventState;    //状态 0,不可用;1,可用  
  unsigned char OSTaskPendTbl;   //等待信号量的任务列表  
} Sem[10];  
 
//初始化信号量  
void OSSemCreat(unsigned char Index,unsigned char Type)  
{  
  Sem[Index].OSEventType=Type;  //型号 0,信号量独占型；1信号量共享型   
  Sem[Index].OSTaskPendTbl=0;  
  Sem[Index].OSEventState=0;  
}  
 
//任务等待信号量,挂起  
unsigned char OSTaskSemPend(unsigned char Index,unsigned int Timeout)  
{  
 
  //unsigned char i=0;  
  if(Sem[Index].OSEventState)               //信号量有效  
  {   
    if(Sem[Index].OSEventType==0)          //如果为独占型  
    Sem[Index].OSEventState = 0x00;       //信号量被独占，不可用  
  }  
  else  
  {                                         //加入信号的任务等待表  
    Sem[Index].OSTaskPendTbl |= 0x01<<OSTaskRunningPrio;   
    OSRdyTbl &= ~(0x01<<OSTaskRunningPrio);  //从任务就绪表中去除      
    TCB[OSTaskRunningPrio].OSWaitTick=Timeout;    //如延时为0，刚无限等待  
    OSSched();   //从新调度  
    if(TCB[OSTaskRunningPrio].OSWaitTick==0) return 0;      
  }  
  return 1;  
}  
 
//发送一个信号量，可以从任务或中断发送  
void OSSemPost(unsigned char Index)  
{  
if(Sem[Index].OSEventType)                //当要求的信号量是共享型  
  {  
    Sem[Index].OSEventState=0x01;           //使信号量有效  
    OSRdyTbl |=Sem [Index].OSTaskPendTbl;   //使在等待该信号的所有任务就绪  
    Sem[Index].OSTaskPendTbl=0;             //清空所有等待该信号的等待任务  
  }    
  else                                       //当要求的信号量为独占型  
  {        
    unsigned char i;  
    for (i = 0; i < OS_TASKS && !(Sem[Index].OSTaskPendTbl & (0x01<<i));  i++);  
    if(i < OS_TASKS)                       //如果有任务需要  
    {  
      Sem[Index].OSTaskPendTbl &= ~(0x01<<i); //从等待表中去除  
      OSRdyTbl |= 0x01<<i;                     //任务就绪  
    }  
    else  
    {  
      Sem[Index].OSEventState =1;        //使信号量有效  
    }  
  }  
}  
 
//从任务发送一个信号量，并进行调度  
void OSTaskSemPost(unsigned char Index)   
{  
  OSSemPost(Index);  
  OSSched();     
}  
 
//清除一个信号量,只对共享型的有用。  
//对于独占型的信号量，在任务占用后，就交得不可以用了。   
 
void OSSemClean(unsigned char Index)  
{  
  Sem[Index].OSEventState =0;          //要求的信号量无效  
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
    OSTaskSuspend(1);    //挂起任务1   
    OSTaskSemPost(0);  
    OSTimeDly(50);  
    OSTaskResume(1);     //恢复任务1  
    OSSemClean(0);  
    OSTimeDly(50);  
  }  
}  
 
void Task1()  
{  
  unsigned int j=0;  
  while(1)  
  {  
    PORTC=j++;  
    OSTimeDly(5);  
  }  
}  
 
void Task2()  
{  
  unsigned int j=0;  
  while(1)  
  {  
      OSTaskSemPend(0,10);  
    PORTD=j++;   
    OSTimeDly(5);    
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
  OSSemCreat(0,1);  //将信号量设为共享型  
  OSTaskCreate(Task0,&Stack[99],0);  
  OSTaskCreate(Task1,&Stack[199],1);  
  OSTaskCreate(Task2,&Stack[299],2);  
  OSTaskCreate(TaskScheduler,&Stack[399],OS_TASKS);  
  OSStartTask();  
}