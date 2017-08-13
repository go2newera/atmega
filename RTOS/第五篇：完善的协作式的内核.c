/*����ƪ�����Ƶ�Э��ʽ���ں�   
 
   ����Ϊ�����Э��ʽ�ں����һЩOS��������ķ���  
   1  �����������������  
   2  �ź���(�ڱ�Ҫʱ�򣬿�����չ���������Ϣ����)  
   3  ��ʱ 
*/

#include <avr/io.h>  
#include <avr/Interrupt.h>  
#include <avr/signal.h>  
unsigned char Stack[400];  
 
register unsigned char OSRdyTbl          /*asm("r2")*/;    //�������о�����  
register unsigned char OSTaskRunningPrio /*asm("r3")*/;    //�������е�����  
 
#define OS_TASKS 3                    //�趨�������������  
struct TaskCtrBlock  
{  
  unsigned int OSTaskStackTop;  //��������Ķ�ջ��  
  unsigned int OSWaitTick;      //������ʱʱ��  
} TCB[OS_TASKS+1];  
 
//��ֹ��������ռ��  
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
 
 
//��������  
void OSTaskCreate(void (*Task)(void),unsigned char *Stack,unsigned char TaskID)  
{  
  unsigned char i;                       
  *Stack--=(unsigned int)Task>>8;    //������ĵ�ַ��λѹ���ջ��  
  *Stack--=(unsigned int)Task;         //������ĵ�ַ��λѹ���ջ��  
      
  *Stack--=0x00;                     //R1 __zero_reg__              
  *Stack--=0x00;                     //R0 __tmp_reg__  
  *Stack--=0x80;                                          
 
//SREG �������У�����ȫ���ж�          
  for(i=0;i<14;i++)    //�� avr-libc �е� FAQ�е� What registers are used by the C compiler?  
    *Stack--=i;                    //�����˼Ĵ���������      
  TCB[TaskID].OSTaskStackTop=(unsigned int)Stack;    //���˹���ջ��ջ�������浽��ջ��������  
  OSRdyTbl|=0x01<<TaskID;      //����������Ѿ�׼����  
}  
 
//��ʼ�������,��������ȼ�������Ŀ�ʼ  
void OSStartTask()          
{  
  OSTaskRunningPrio=OS_TASKS;  
  SP=TCB[OS_TASKS].OSTaskStackTop+17;  
  __asm__ __volatile__(    "reti"       " \t"  );   
}  
 
//�����������  
void OSSched(void)  
{   
   //  �����ж�ʱ����Ĵ����Ĵ�����ջ��ģ��һ���жϺ���ջ�����    
  __asm__ __volatile__("PUSH __zero_reg__\t");  //R1  
  __asm__ __volatile__("PUSH __tmp_reg__ \t");  //R0   
  __asm__ __volatile__("IN   __tmp_reg__,__SREG__ \t");  //����״̬�Ĵ���SREG  
  __asm__ __volatile__("PUSH __tmp_reg__\t");  
  __asm__ __volatile__("CLR  __zero_reg__\t");  //R0��������  
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
  __asm__ __volatile__("PUSH R28\t");  //R28��R29���ڽ����ڶ�ջ�ϵ�ָ��  
  __asm__ __volatile__("PUSH R29\t");  //��ջ���  
      
  TCB[OSTaskRunningPrio].OSTaskStackTop=SP;           //���������е�����Ķ�ջ�ױ���  
 
  unsigned char OSNextTaskID;                             //�����ж�ջ�Ͽ����µĿռ�   
  for (OSNextTaskID = 0;                                  //�����������  
    OSNextTaskID < OS_TASKS && !(OSRdyTbl & (0x01<<OSNextTaskID));   
    OSNextTaskID++);  
    OSTaskRunningPrio = OSNextTaskID ;  
 
  cli();  //������ջת��  
  SP=TCB[OSTaskRunningPrio].OSTaskStackTop;  
  sei();  
      
    //�����ж�ʱ�ĳ�ջ����      
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
  __asm__ __volatile__("POP  __tmp_reg__ \t");      //SERG ��ջ���ָ�  
  __asm__ __volatile__("OUT  __SREG__,__tmp_reg__ \t");      //  
  __asm__ __volatile__("POP  __tmp_reg__\t");      //R0 ��ջ  
  __asm__ __volatile__("POP  __zero_reg__ \t");      //R1 ��ջ  
  //�ж�ʱ��ջ���  
}  
 
 
////////////////////////////////////////////������  
//��������  
void OSTaskSuspend(unsigned char prio)   
{  
  TCB[prio].OSWaitTick=0;  
  OSRdyTbl &= ~(0x01<<prio); //�������������ȥ����־λ  
  if(OSTaskRunningPrio==prio)  //��Ҫ���������Ϊ��ǰ����  
    OSSched();               //���µ���  
}  
 
//�ָ����� �����ñ�OSTaskSuspend�� OSTimeDly��ͣ������ָ�  
void OSTaskResume(unsigned char prio)  
{  
  OSRdyTbl |= 0x01<<prio;    //����������������ñ�־λ  
    TCB[prio].OSWaitTick=0;        //��ʱ���ʱ��Ϊ0,��ʱ  
  if(OSTaskRunningPrio>prio)   //��Ҫ��ǰ��������ȼ���������λ����������ȼ�  
    OSSched();               //���µ���              //���µ���  
}  
 
// ������ʱ  
void OSTimeDly(unsigned int ticks)  
{  
  if(ticks)                             //����ʱ��Ч  
  {  
    OSRdyTbl &= ~(0x01<<OSTaskRunningPrio);           
    TCB[OSTaskRunningPrio].OSWaitTick=ticks;  
    OSSched();                          //���µ���  
  }  
}  
 
 
//�ź���  
struct SemBlk  
{  
  unsigned char OSEventType;     //�ͺ� 0,�ź�����ռ�ͣ�1�ź���������   
  unsigned char OSEventState;    //״̬ 0,������;1,����  
  unsigned char OSTaskPendTbl;   //�ȴ��ź����������б�  
} Sem[10];  
 
//��ʼ���ź���  
void OSSemCreat(unsigned char Index,unsigned char Type)  
{  
  Sem[Index].OSEventType=Type;  //�ͺ� 0,�ź�����ռ�ͣ�1�ź���������   
  Sem[Index].OSTaskPendTbl=0;  
  Sem[Index].OSEventState=0;  
}  
 
//����ȴ��ź���,����  
unsigned char OSTaskSemPend(unsigned char Index,unsigned int Timeout)  
{  
 
  //unsigned char i=0;  
  if(Sem[Index].OSEventState)               //�ź�����Ч  
  {   
    if(Sem[Index].OSEventType==0)          //���Ϊ��ռ��  
    Sem[Index].OSEventState = 0x00;       //�ź�������ռ��������  
  }  
  else  
  {                                         //�����źŵ�����ȴ���  
    Sem[Index].OSTaskPendTbl |= 0x01<<OSTaskRunningPrio;   
    OSRdyTbl &= ~(0x01<<OSTaskRunningPrio);  //�������������ȥ��      
    TCB[OSTaskRunningPrio].OSWaitTick=Timeout;    //����ʱΪ0�������޵ȴ�  
    OSSched();   //���µ���  
    if(TCB[OSTaskRunningPrio].OSWaitTick==0) return 0;      
  }  
  return 1;  
}  
 
//����һ���ź��������Դ�������жϷ���  
void OSSemPost(unsigned char Index)  
{  
if(Sem[Index].OSEventType)                //��Ҫ����ź����ǹ�����  
  {  
    Sem[Index].OSEventState=0x01;           //ʹ�ź�����Ч  
    OSRdyTbl |=Sem [Index].OSTaskPendTbl;   //ʹ�ڵȴ����źŵ������������  
    Sem[Index].OSTaskPendTbl=0;             //������еȴ����źŵĵȴ�����  
  }    
  else                                       //��Ҫ����ź���Ϊ��ռ��  
  {        
    unsigned char i;  
    for (i = 0; i < OS_TASKS && !(Sem[Index].OSTaskPendTbl & (0x01<<i));  i++);  
    if(i < OS_TASKS)                       //�����������Ҫ  
    {  
      Sem[Index].OSTaskPendTbl &= ~(0x01<<i); //�ӵȴ�����ȥ��  
      OSRdyTbl |= 0x01<<i;                     //�������  
    }  
    else  
    {  
      Sem[Index].OSEventState =1;        //ʹ�ź�����Ч  
    }  
  }  
}  
 
//��������һ���ź����������е���  
void OSTaskSemPost(unsigned char Index)   
{  
  OSSemPost(Index);  
  OSSched();     
}  
 
//���һ���ź���,ֻ�Թ����͵����á�  
//���ڶ�ռ�͵��ź�����������ռ�ú󣬾ͽ��ò��������ˡ�   
 
void OSSemClean(unsigned char Index)  
{  
  Sem[Index].OSEventState =0;          //Ҫ����ź�����Ч  
}  
 
 
void TCN0Init(void)    // ��ʱ��0  
{  
  TCCR0 = 0;  
  TCCR0 |= (1<<CS02);  // 256Ԥ��Ƶ  
  TIMSK |= (1<<TOIE0); // T0����ж�����                    
  TCNT0 = 100;         // �ü�����ʼֵ  
      
}  
 
 
SIGNAL(SIG_OVERFLOW0)  
{  
  unsigned char i;  
  for(i=0;i<OS_TASKS;i++)       //����ʱ��  
  {  
    if(TCB[i].OSWaitTick)   
    {  
      TCB[i].OSWaitTick--;  
      if(TCB[i].OSWaitTick==0)     //������ʱ�ӵ�ʱ,�������ɶ�ʱ����ʱ�Ĳ���  
      {    
        OSRdyTbl |= (0x01<<i);     //ʹ�����ھ���������λ     
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
    OSTaskSuspend(1);    //��������1   
    OSTaskSemPost(0);  
    OSTimeDly(50);  
    OSTaskResume(1);     //�ָ�����1  
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
     OSSched();      //�������е���  
  }  
}  
 
 
int main(void)  
{      
  TCN0Init();  
  OSRdyTbl=0;  
  OSSemCreat(0,1);  //���ź�����Ϊ������  
  OSTaskCreate(Task0,&Stack[99],0);  
  OSTaskCreate(Task1,&Stack[199],1);  
  OSTaskCreate(Task2,&Stack[299],2);  
  OSTaskCreate(TaskScheduler,&Stack[399],OS_TASKS);  
  OSStartTask();  
}