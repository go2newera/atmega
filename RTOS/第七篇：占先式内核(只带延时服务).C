/*第七篇：占先式内核(只带延时服务)   
 
                                           Preemptive Multitasking  
    当大家理解时间片轮番调度法的任务调度方式后，占先式的内核的原理，已经伸手可及
了。  
    先想想，占先式内核是在什么地方实现任务调度的呢？对了，它在可以在任务中进行调度，
这个在协作式的内核中已经做到了；同时，它也可以在中断结束后进行调度，这个问题，已
经在时间片轮番调度法中已经做到了。  
      
    由于中断是可以嵌套的，只有当各层嵌套中要求调度，并且中断嵌套返回到最初进入的中
断的那一层时，才能进行任务调度。  
*/    
#include <avr/io.h>  
#include <avr/Interrupt.h>  
#include <avr/signal.h>  
unsigned char Stack[400];  
 
register unsigned char OSRdyTbl          /*asm("r2")*/;    //任务运行就绪表  
register unsigned char OSTaskRunningPrio /*asm("r3")*/;    //正在运行的任务  
register unsigned char IntNum            /*asm("r4")*/;  //中断嵌套计数器  
//只有当中断嵌套数为0，并且有中断要求时，才能在退出中断时，进行任务调度  
register unsigned char OSCoreState       /*asm("r16")*/; // 系统核心标志位 ,R16 编译器没有使用  
//只有大于R15的寄存器才能直接赋值 例LDI R16,0x01  
//0x01 正在任务 切换  0x02 有中断要求切换  
 
#define OS_TASKS 3                    //设定运行任务的数量  
struct TaskCtrBlock  
{  
  unsigned int OSTaskStackTop;  //保存任务的堆栈顶  
  unsigned int OSWaitTick;      //任务延时时钟  
} TCB[OS_TASKS+1];  
 
//防止被编译器占用  
//register unsigned char tempR4  asm("r4");  
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
//register unsigned char tempR16 asm("r16");  
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
  __asm__ __volatile__(    "reti"       "  \t"  );   
}  
 
//进行任务调度  
void OSSched(void)  
{   
 
  __asm__ __volatile__("LDI  R16,0x01               \t");    
  //清除中断要求任务切换的标志位,设置正在任务切换标志位  
  __asm__ __volatile__("SEI                         \t");        
  //开中断,因为如果因中断在任务调度中进行,要重新进行调度时，已经关中断  
  //根据中断时保存寄存器的次序入栈，模拟一次中断后，入栈的情况    
  __asm__ __volatile__("PUSH __zero_reg__\t");  //R1  
  __asm__ __volatile__("PUSH __tmp_reg__\t");  //R0   
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
      
  __asm__ __volatile__("Int_OSSched: \t");  //当中断要求调度，直接进入这里  
  __asm__ __volatile__("SEI\t");   
//开中断,因为如果因中断在任务调度中进行，已经关中断   
  __asm__ __volatile__("PUSH R28\t");  //R28与R29用于建立在堆栈上的指针  
  __asm__ __volatile__("PUSH R29\t");  //入栈完成  
      
  TCB[OSTaskRunningPrio].OSTaskStackTop=SP;           //将正在运行的任务的堆栈底保存  
 
  unsigned char OSNextTaskPrio;                            //在现有堆栈上开设新的空间   
  for (OSNextTaskPrio = 0;                                 //进行任务调度  
    OSNextTaskPrio < OS_TASKS && !(OSRdyTbl & (0x01<<OSNextTaskPrio));   
    OSNextTaskPrio++);  
    OSTaskRunningPrio = OSNextTaskPrio ;  
 
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
  __asm__ __volatile__("POP  __tmp_reg__\t");      //SERG 出栈并恢复  
  __asm__ __volatile__("OUT  __SREG__,__tmp_reg__ \t");      //  
  __asm__ __volatile__("POP  __tmp_reg__\t");      //R0 出栈  
  __asm__ __volatile__("POP  __zero_reg__\t");      //R1 出栈  
  //中断时出栈完成  
  __asm__ __volatile__("CLI\t");  //关中断      
  __asm__ __volatile__("SBRC R16,1\t");  //SBRC当寄存器位为0则跳过下一条指令  
  //检查是在调度时，是否有中断要求任务调度 0x02是中断要求调度的标志位  
  __asm__ __volatile__("RJMP OSSched\t");  //重新调度  
  __asm__ __volatile__("LDI  R16,0x00\t");    
  //清除中断要求任务切换的标志位,清除正在任务切换标志位  
  __asm__ __volatile__("RETI\t");     //返回并开中断  
}  
 
 
//从中断退出并进行调度  
void IntSwitch(void)  
{      
  //当中断无嵌套，并且没有在切换任务的过程中，直接进行任务切换  
  if(OSCoreState == 0x02 && IntNum==0)   
  {  
    //进入中断时，已经保存了SREG和R0,R1,R18~R27,R30,R31  
    __asm__ __volatile__("POP  R31\t");  //去除因调用子程序而入栈的PC  
    __asm__ __volatile__("POP  R31\t");  
    __asm__ __volatile__("LDI  R16,0x01\t");    
    //清除中断要求任务切换的标志位,设置正在任务切换标志位  
    __asm__ __volatile__("RJMP Int_OSSched\t");  //重新调度  
  }  
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
 
void TCN0Init(void)    // 计时器0  
{  
  TCCR0 = 0;  
  TCCR0 |= (1<<CS02);  // 256预分频  
  TIMSK |= (1<<TOIE0); // T0溢出中断允许                    
  TCNT0 = 100;         // 置计数起始值  
      
}  
 
SIGNAL(SIG_OVERFLOW0)  
{  
  //中断嵌套+1  	
  IntNum++; 
  //在中断中，重开中断  
  sei();  
      
  unsigned char i,j=0; 
   //任务时钟  
  for(i=0;i<OS_TASKS;i++)       
  {  
    if(TCB[i].OSWaitTick)   
    {  
      TCB[i].OSWaitTick--;  
	  //当任务时钟到时,必须是由定时器减时的才行 
      if(TCB[i].OSWaitTick==0)          
      {    
	  	//使任务可以重新运行  
        OSRdyTbl |= (0x01<<i);  
		//要求任务切换的标志位  
        OSCoreState|=0x02;              
      }  
    }  
  }  
  TCNT0=100;  
  cli(); 
  //中断嵌套-1  
  IntNum--;   
  //进行任务调度  
  IntSwitch();         
}  
 
void Task0()  
{  
  unsigned int j=0;  
  while(1)  
  {              
    PORTB=j++;  
    OSTimeDly(50);  
  }  
}  
 
void Task1()  
{  
  unsigned int j=0;  
  while(1)  
  {  
    PORTC=j++;  
    OSTimeDly(20);  
  }  
}  
 
void Task2()  
{  
  unsigned int j=0;  
  while(1)  
  {  
    PORTD=j++;   
    OSTimeDly(5);    
  }  
}  
 
 
 
void TaskScheduler()  
{   
  OSSched();   
  while(1)  
  {          
     //OSSched();      //反复进行调度  
  }  
}  
 
 
int main(void)  
{      
  TCN0Init();  
  OSRdyTbl=0;  
  IntNum=0;  
  OSTaskCreate(Task0,&Stack[99],0);  
  OSTaskCreate(Task1,&Stack[199],1);  
  OSTaskCreate(Task2,&Stack[299],2);  
  OSTaskCreate(TaskScheduler,&Stack[399],OS_TASKS);  
  OSStartTask();  
} 