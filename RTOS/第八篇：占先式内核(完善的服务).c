/*�ڰ�ƪ��ռ��ʽ�ں�(���Ƶķ���)   
 
    �����ǰ�����ᵽ��ռ��ʽ�ں˺�Э��ʽ�ں������һ�𣬺����׾Ϳ��Եõ�һ�����ܽ�
Ϊ���Ƶ�ռ��ʽ�ںˣ����Ĺ����У�  
    1,����ͻָ�����  
    2,������ʱ  
    3,�ź���(���������ͺͶ�ռ��)  
    ���⣬�ڱ����У��ڸ��������м����˴Ӵ��ڷ�������״̬�Ĺ��ܡ�  
*/      
 
#include <avr/io.h>  
#include <avr/Interrupt.h>  
#include <avr/signal.h>  
unsigned char Stack[400];  
 
register unsigned char OSRdyTbl          /*asm("r2")*/;    //�������о�����  
register unsigned char OSTaskRunningPrio /*asm("r3")*/;    //�������е�����  
register unsigned char IntNum            /*asm("r4")*/;     //�ж�Ƕ�׼�����  
//ֻ�е��ж�Ƕ����Ϊ0���������ж�Ҫ��ʱ���������˳��ж�ʱ�������������  
register unsigned char OSCoreState       /*asm("r16")*/; // ϵͳ���ı�־λ ,R16 ������û��ʹ��  
//ֻ�д���R15�ļĴ�������ֱ�Ӹ�ֵ ��LDI R16,0x01  
//0x01 �������� �л�  0x02 ���ж�Ҫ���л�  
 
#define OS_TASKS 3                    //�趨�������������  
struct TaskCtrBlock  
{  
  unsigned int OSTaskStackTop;  //��������Ķ�ջ��  
  unsigned int OSWaitTick;      //������ʱʱ��  
} TCB[OS_TASKS+1];  
 
//��ֹ��������ռ��  
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
  __asm__ __volatile__(    "reti"       "\t"  );   
}  
 
//�����������  
void OSSched(void)  
{   
 
  __asm__ __volatile__("LDI  R16,0x01\t");    
  //����ж�Ҫ�������л��ı�־λ,�������������л���־λ  
  __asm__ __volatile__("SEI\t");        
  //���ж�,��Ϊ������ж�����������н���,Ҫ���½��е���ʱ���Ѿ����ж�  
   //  �����ж�ʱ����Ĵ����Ĵ�����ջ��ģ��һ���жϺ���ջ�����    
  __asm__ __volatile__("PUSH __zero_reg__\t");  //R1  
  __asm__ __volatile__("PUSH __tmp_reg__\t");  //R0   
  __asm__ __volatile__("IN   __tmp_reg__,__SREG__\t");  //����״̬�Ĵ���SREG  
  __asm__ __volatile__("PUSH __tmp_reg__ \t");  
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
      
  __asm__ __volatile__("Int_OSSched: \t");  //���ж�Ҫ����ȣ�ֱ�ӽ�������  
  __asm__ __volatile__("SEI\t");   
    //���ж�,��Ϊ������ж�����������н��У��Ѿ����ж�   
  __asm__ __volatile__("PUSH R28\t");  //R28��R29���ڽ����ڶ�ջ�ϵ�ָ��  
  __asm__ __volatile__("PUSH R29\t");  //��ջ���  
      
  TCB[OSTaskRunningPrio].OSTaskStackTop=SP;           //���������е�����Ķ�ջ�ױ���  
 
  unsigned char OSNextTaskPrio;                            //�����ж�ջ�Ͽ����µĿռ�   
  for (OSNextTaskPrio = 0;                                 //�����������  
    OSNextTaskPrio < OS_TASKS && !(OSRdyTbl & (0x01<<OSNextTaskPrio));   
    OSNextTaskPrio++);  
    OSTaskRunningPrio = OSNextTaskPrio ;  
 
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
  __asm__ __volatile__("POP  __tmp_reg__\t");      //SERG ��ջ���ָ�  
  __asm__ __volatile__("OUT  __SREG__,__tmp_reg__ \t");      //  
  __asm__ __volatile__("POP  __tmp_reg__\t");      //R0 ��ջ  
  __asm__ __volatile__("POP  __zero_reg__\t");      //R1 ��ջ  
  //�ж�ʱ��ջ���  
  __asm__ __volatile__("CLI\t");  //���ж�      
  __asm__ __volatile__("SBRC R16,1\t");  //SBRC���Ĵ���λΪ0��������һ��ָ��  
  //������ڵ���ʱ���Ƿ����ж�Ҫ��������� 0x02���ж�Ҫ����ȵı�־λ  
  __asm__ __volatile__("RJMP OSSched\t");  //���µ���  
  __asm__ __volatile__("LDI  R16,0x00\t");    
  //����ж�Ҫ�������л��ı�־λ,������������л���־λ  
  __asm__ __volatile__("RETI\t");     //���ز����ж�  
}  
 
 
//���ж��˳������е���  
void IntSwitch(void)  
{      
  //���ж���Ƕ�ף�����û�����л�����Ĺ����У�ֱ�ӽ��������л�  
  if(OSCoreState == 0x02 && IntNum==0)   
  {  
    //�����ж�ʱ���Ѿ�������SREG��R0,R1,R18~R27,R30,R31  
    __asm__ __volatile__("POP  R31\t");  //ȥ��������ӳ������ջ��PC  
    __asm__ __volatile__("POP  R31\t");  
    __asm__ __volatile__("LDI  R16,0x01\t");    
    //����ж�Ҫ�������л��ı�־λ,�������������л���־λ  
    __asm__ __volatile__("RJMP Int_OSSched \t");  //���µ���  
  }  
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
//��Timeout==0xffffʱ��Ϊ������ʱ  
unsigned char OSTaskSemPend(unsigned char Index,unsigned int Timeout)  
{  
 
  //unsigned char i=0;  
  if(Sem[Index].OSEventState)                      //�ź�����Ч  
  {   
    if(Sem[Index].OSEventType==0)                  //���Ϊ��ռ��  
    Sem[Index].OSEventState = 0x00;                //�ź�������ռ��������  
  }  
  else  
  {                                                //�����źŵ�����ȴ���  
    Sem[Index].OSTaskPendTbl |= 0x01<<OSTaskRunningPrio;   
    TCB[OSTaskRunningPrio].OSWaitTick=Timeout;    //����ʱΪ0�������޵ȴ�  
    OSRdyTbl &= ~(0x01<<OSTaskRunningPrio);       //�������������ȥ��      
    OSSched();   //���µ���  
    if(TCB[OSTaskRunningPrio].OSWaitTick==0 )     //��ʱ��δ���õ���Դ  
          return 0;          
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
  IntNum++;     //�ж�Ƕ��+1  
  sei();  //���ж��У��ؿ��ж�  
      
  unsigned char i;  
  for(i=0;i<OS_TASKS;i++)        //����ʱ��  
  {  
    if(TCB[i].OSWaitTick && TCB[i].OSWaitTick!=0xffff)   
    {  
      TCB[i].OSWaitTick--;  
      if(TCB[i].OSWaitTick==0)         //������ʱ�ӵ�ʱ,�������ɶ�ʱ����ʱ�Ĳ���  
      {    
        OSRdyTbl |= (0x01<<i);         //ʹ���������������  
        OSCoreState|=0x02;         //Ҫ�������л��ı�־λ  
      }  
    }  
  }  
  TCNT0=100;  
  cli();  
  IntNum--;               //�ж�Ƕ��-1  
  IntSwitch();         //�����������  
}  
 
 
unsigned char __attribute__ ((progmem)) proStrA[]="Task                       ";  
 
unsigned char strA[20];  
 
SIGNAL(SIG_UART_RECV)        //���ڽ����ж�  
{  
  strA[0]=UDR;  
}  
 
 
/////////////////////////////////////���ڷ���  
 
unsigned char *pstr_UART_Send;  
unsigned int  nUART_Sending=0;  
 
void UART_Send(unsigned char *Res,unsigned int Len)    //�����ַ�������  
{  
  if(Len>0)  
  {  
    pstr_UART_Send=Res;    //�����ִ���ָ��  
    nUART_Sending=Len;    //�����ִ��ĳ���  
    UCSRB=0xB8;                    //�����ж�ʹ��  
  }  
}  
 
 
//SIGNAL ���ж��ڼ䣬�����жϽ�ֹ  
 
SIGNAL(SIG_UART_DATA)       //���ڷ��������ж�  
{  
 
  IntNum++;     //�ж�Ƕ��+1,�������ж�  
 
  if(nUART_Sending)                    //���δ����  
  {  
    UDR=*pstr_UART_Send;        //�����ֽ�  
    pstr_UART_Send++;                //�����ִ���ָ���1  
    nUART_Sending--;                //�ȴ����͵��ִ�����1  
  }  
  if(nUART_Sending==0)            //���Ѿ�������  
  {      
    OSSemPost(0);  
    OSCoreState|=0x02;      //Ҫ�������л��ı�־λ  
    UCSRB=0x98;      
  }  
  cli();                        //�ط����ж�  
  IntNum--;      
  IntSwitch(); //�����������  
}  
 
 
void UARTInit()    //��ʼ������  
{  
#define fosc 8000000 //����8  MHZ UBRRL=(fosc/16/(baud+1))%256;  
#define baud 9600     //������  
  OSCCAL=0x97;          //���ڲ�����У��ֵ���ӱ�����ж���  
  //UCSRB=(1<<RXEN)|(1<<TXEN);//�����ͺͽ���  
  UCSRB=0x98;  
  //UCSRB=0x08;  
  UBRRL=(fosc/16/(baud+1))%256;  
  UBRRH=(fosc/16/(baud+1))/256;  
  UCSRC=(1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);//8λ����+1λSTOPλ  
  UCSRB=0xB8;  
  UDR=0;  
}  
 
 
//��ӡunsigned int ���ַ����� 00000  
void strPUT_uInt(unsigned char *Des,unsigned int i)  
{  
  unsigned char j;  
  Des=Des+4;  
  for(j=0;j<5;j++)  
  {  
    *Des=i%10+'0';  
    i=i/10;  
    Des--;  
  }  
}  
 
void strPUT_Star(unsigned char *Des,unsigned char i)  
{  
  unsigned char j;  
  for(j=0;j<i;j++)  
  {  
    *Des++='*';  
  }  
  *Des++=13;  
}  
 
unsigned int strPUT_TaskState(unsigned char *Des,  unsigned char TaskID,  unsigned char Num)  
{  
  //unsigned int i=0;  
  *(Des+4)='0'+TaskID;  
  strPUT_uInt(Des+6,Num);  
  strPUT_Star(Des+12,TaskID);  
  return 12+TaskID+1;  
}  
 
void Task0()  
{  
  unsigned int j=0;  
  while(1)  
  {              
    PORTB=j++;              
    if(OSTaskSemPend(0,0xffff))  
    {  
      unsigned int m;  
      m=strPUT_TaskState(strA,OSTaskRunningPrio,j);  
      UART_Send(strA,m);  
    }  
    OSTimeDly(200);  
  }  
}  
 
void Task1()  
{  
  unsigned int j=0;  
  while(1)  
  {  
    PORTC=j++;  
    if(OSTaskSemPend(0,0xffff))  
    {  
      unsigned int m;  
      m=strPUT_TaskState(strA,OSTaskRunningPrio,j);  
      UART_Send(strA,m);  
    }  
    OSTimeDly(100);  
  }  
}  
 
void Task2()  
{  
  unsigned int j=0;  
  while(1)  
  {  
    if(OSTaskSemPend(0,0xffff))  
    {  
      unsigned int m;  
      m=strPUT_TaskState(strA,OSTaskRunningPrio,j);  
      UART_Send(strA,m);  
    }  
    PORTD=j++;   
    OSTimeDly(50);    
  }  
}  
 
void TaskScheduler()  
{   
  OSSched();   
  while(1)  
  {          
  }  
}  
 
int main(void)  
{          
  strlcpy_P(strA,proStrA,20);  
  UARTInit();  
  TCN0Init();  
 
  OSRdyTbl=0;  
  IntNum=0;  
  OSTaskCreate(Task0,&Stack[99],0);  
  OSTaskCreate(Task1,&Stack[199],1);  
  OSTaskCreate(Task2,&Stack[299],2);  
  OSTaskCreate(TaskScheduler,&Stack[399],OS_TASKS);  
  OSStartTask();  
}