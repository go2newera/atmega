/*����ƪ��ֻ����ʱ�����Э��ʽ���ں�   
 
                                      Cooperative Multitasking  
   ǰ��̨ϵͳ��Э��ʽ�ں�ϵͳ����ռ��ʽ�ں�ϵͳ����ʲô��ͬ�أ�  
   �ǵ���21IC�Ͽ��������ı���,����(С��)���ò����������������ŵ�һ���ϰ��������ŵ�
���������ǰ��̨��������˭�������밴�ŶӵĴ���ʹ�ò����������Э��ʽ����ô���Ե�
������������ϰ��Ҫ�Ⱦ����Ƚ��룻�����ռ��ʽ��ֻҪ�и��߼�����������ȣ���ô��
����������˭����Ҫ��һʱ���ó���������߼���������á���  
*/

#include <avr/io.h>  
#include <avr/Interrupt.h>  
#include <avr/signal.h>  
unsigned char Stack[200];  
 
register unsigned char OSRdyTbl          /*asm("r2")*/;    //�������о�����  
register unsigned char OSTaskRunningPrio /*asm("r3")*/;    //�������е�����  
 
#define OS_TASKS 3                    //�趨�������������  
struct TaskCtrBlock           //������ƿ�  
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
  *Stack--=0x80;                                        //SREG �������У�����ȫ���ж�          
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
  __asm__ __volatile__("POP  __tmp_reg__ \t");      //R0 ��ջ  
  __asm__ __volatile__("POP  __zero_reg__ \t");      //R1 ��ջ  
  //�ж�ʱ��ջ���  
}  
 
void OSTimeDly(unsigned int ticks)  
{  
  if(ticks)                             //����ʱ��Ч  
  {  
    OSRdyTbl &= ~(0x01<<OSTaskRunningPrio);  
    TCB[OSTaskRunningPrio].OSWaitTick=ticks;  
    OSSched();                          //���µ���  
  }  
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
     OSSched();      //�������е���  
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
 
/*   ������������У�һ�б�úܼ򵥣������������е������񣬶�ͨ����ʱ������������
��CPU�Ŀ���Ȩ��  
   ��ʱ���ж��У��Ը�������ĵ���ʱ���м�ʱ�����ĳ���������ʱ������������������
����������λ��  
   ��ͼ���ϵͳ����TaskScheduler()���������������ڷ�����CPU�Ŀ���Ȩ��ʼ���ϵؽ�
�е��ȡ����ĳ�������ھ���������λ��ͨ�����ȣ�������߼���������м������С�*/