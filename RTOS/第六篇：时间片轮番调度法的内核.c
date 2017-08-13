/*����ƪ��ʱ��Ƭ�ַ����ȷ����ں�   
 
                                 Round-Robin Sheduling  
 
    ʱ��Ƭ�ֵ����Ƿǳ���Ȥ�ġ���ƪ�е����ӣ�������3����������û�����ȼ�����ʱ��
�жϵĵ����£�ÿ����������������ͬ��ʱ�䡣������ں���û�м����������񣬸о���
�ͺ�������������ѭ����ͬʱ���С�  
 
    ����ֻ���ṩ��һ����ʱ���жϽ��е��ȵ��ںˣ���ҿ��Ը����Լ�����Ҫ�������Ӧ��
����  
    Ҫע�⵽:   
    1,������ʱ���ж��ڵ����������л���������Ϊ�ڽ����ж�ʱ���Ѿ���һϵ�еļĴ�����
ջ��  
    2,���ж��ڽ��е��ȣ���ֱ��ͨ��"RJMP Int_OSSched"���������л��͵��ȵģ�����
GCC AVR��һ���ص㣬Ϊ��C��д�ں��ṩ�˼���ķ��㡣  
    3,���Ķ������ͬʱ��������Ķ������������� *.lst�ļ����������������кܴ�İ�����  
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
  __asm__ __volatile__("PUSH __zero_reg__ \t");  //R1  
  __asm__ __volatile__("PUSH __tmp_reg__\t");  //R0   
  __asm__ __volatile__("IN   __tmp_reg__,__SREG__\t");  //����״̬�Ĵ���SREG  
  __asm__ __volatile__("PUSH __tmp_reg__\t");  
  __asm__ __volatile__("CLR  __zero_reg__ \t");  //R0��������  
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
      
  __asm__ __volatile__("Int_OSSched: \t");  //���ж�Ҫ����ȣ�ֱ�ӽ�������  
  __asm__ __volatile__("PUSH R28\t");  //R28��R29���ڽ����ڶ�ջ�ϵ�ָ��  
  __asm__ __volatile__("PUSH R29\t");  //��ջ���  
      
  TCB[OSTaskRunningPrio].OSTaskStackTop=SP;           //���������е�����Ķ�ջ�ױ���  
 
  if(++OSTaskRunningPrio>=OS_TASKS) //�������и�������û�����ȼ�  
      OSTaskRunningPrio=0;  
 
  //cli();  //������ջת��  
  SP=TCB[OSTaskRunningPrio].OSTaskStackTop;  
  //sei();  
      
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
  __asm__ __volatile__("POP  __tmp_reg__\t");      //SERG ��ջ���ָ�  
  __asm__ __volatile__("OUT  __SREG__,__tmp_reg__ \t");      //  
  __asm__ __volatile__("POP  __tmp_reg__ \t");      //R0 ��ջ  
  __asm__ __volatile__("POP  __zero_reg__ \t");      //R1 ��ջ  
  __asm__ __volatile__("RETI\t");     //���ز����ж�  
  //�ж�ʱ��ջ���  
}  
 
 
void IntSwitch(void)  
{      
  __asm__ __volatile__("POP  R31\t");  //ȥ��������ӳ������ջ��PC  
  __asm__ __volatile__("POP  R31\t");  
  __asm__ __volatile__("RJMP Int_OSSched\t");  //���µ���  
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
  TCNT0=100;  
  IntSwitch();        //�������  
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
     OSSched();      //�������е���  
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