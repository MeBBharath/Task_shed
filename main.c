#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "led.h"
void task1_handler(void);
void task2_handler(void);
void task3_handler(void);
void task4_handler(void);
__attribute ((naked)) void init_scheduler_stack(uint32_t Sched_Task_start);
uint32_t get_psp(void);
__attribute ((naked)) void ch_sp_psp(void);

void update_global_tick_count(void);
void unblock_tasks(void);

void schedule(void);
void update_next_task(void);


void init_systick_timer(uint32_t tick_hz);
void init_tasks_stack(void);
void en_fault_excep(void);
void task_delay(uint32_t tick_count);
void IDLE_TASK(void);

uint32_t current_task=0;

uint32_t tick_count=1;
uint32_t g_tick_count=0;
typedef struct
{
  uint32_t psp_value;
  uint32_t block_count;
  uint8_t current_state;
  void (*task_handler)(void);
}TCB_t;
TCB_t user_tasks[MAX_Tasks];
int main(void)
{
    /* Loop forever */
	/* enable fault excep */
	en_fault_excep();
	/*for MSP*/
	init_scheduler_stack(SCHED_Task_start);

	/*for task handler loc and for dummy task initialization*/
	init_tasks_stack();

	/*for enabling systick config*/
	init_systick_timer(TICK_HZ);

	led_init_all();

	ch_sp_psp();

	task1_handler();
	for(;;);
}
void task1_handler(void)
{
	while(1)
	{
		led_on(LED_GREEN);
		task_delay(1000);
		led_off(LED_GREEN);
		task_delay(1000);
	}

}

void task2_handler(void)
{
	while(1)
	{
		led_on(LED_ORANGE);
		task_delay(500);
		led_off(LED_ORANGE);
		task_delay(500);
	}

}

void task3_handler(void)
{
	while(1)
	{
		led_on(LED_BLUE);
		task_delay(250);
		led_off(LED_BLUE);
		task_delay(250);
	}

}

void task4_handler(void)

{
	while(1)
	{
		led_on(LED_RED);
		task_delay(125);
		led_off(LED_RED);
		task_delay(125);
	}


}
void IDLE_TASK(void)
{
	while(1);
}
void init_systick_timer(uint32_t tick_hz)
{
	uint32_t count_value = (SYSTICK_TIM_clk/tick_hz)-1;

	uint32_t *SYST_CSR =(uint32_t *)0xE000E010;
	uint32_t *SYST_RVR =(uint32_t *)0xE000E014;

	*SYST_RVR&=~(0x00FFFFFF);
	*SYST_RVR|=count_value;

	*SYST_CSR|=(1<<1);
	*SYST_CSR|=(1<<2);
	*SYST_CSR|=(1<<0);

}
__attribute ((naked)) void init_scheduler_stack(uint32_t Sched_Task_start)
{
	__asm volatile("MSR MSP,R0");
	__asm volatile("BX LR");
}


void init_tasks_stack(void)
{
	user_tasks[0].current_state=TASK_READY_STATE;
	user_tasks[1].current_state=TASK_READY_STATE;
	user_tasks[2].current_state=TASK_READY_STATE;
	user_tasks[3].current_state=TASK_READY_STATE;
	user_tasks[4].current_state=TASK_READY_STATE;

	user_tasks[0].psp_value=IDLE_start;
	user_tasks[1].psp_value=Task1_start;
    user_tasks[2].psp_value=Task2_start;
	user_tasks[3].psp_value=Task3_start;
	user_tasks[4].psp_value=Task4_start;

	user_tasks[0].task_handler=IDLE_TASK;
	user_tasks[1].task_handler=task1_handler;
	user_tasks[2].task_handler=task2_handler;
	user_tasks[3].task_handler=task3_handler;
	user_tasks[4].task_handler=task4_handler;


	uint32_t *pPSP;
	for(int i=0;i<MAX_Tasks;i++)
	{
		pPSP=(uint32_t*)user_tasks[i].psp_value;
		/*XPSR*/
		pPSP--;
	    *pPSP=DUMMY_XPSR;
	    /*PC*/
	   pPSP--;
	   *pPSP=(uint32_t)user_tasks[i].task_handler;
	   /*LR*/
	  pPSP--;
	  *pPSP=0xFFFFFFFD;
	  for(int j=0;j<13;j++)
	  {
		  pPSP--;
		  *pPSP=0;
	  }
	  user_tasks[i].psp_value=(uint32_t)pPSP;
	}
}
void en_fault_excep()
{
 uint32_t *pSHSCR=(uint32_t *)0xE000ED24;
 *pSHSCR |=(7<<16);/*mem,bus,usage*/
}
void HardFault_Handler(void)
{
	printf("Hardf");
	while(1);
}
void MemManage_Handler(void)
{
	printf("MemManf");
	while(1);
}
void BusFault_Handler(void)
{
	printf("Busf");
	while(1);
}
uint32_t get_psp(void)
{
	return user_tasks[current_task].psp_value;
}
void save_psp(uint32_t psp_add)
{
	user_tasks[current_task].psp_value=psp_add;
}
void update_next_task(void)
{
	int state=TASK_BLOCK_STATE;
	for(int i=0;i<MAX_Tasks;i++)
	{
	 current_task++;
	 current_task%=MAX_Tasks;
	 state=user_tasks[current_task].current_state;
	 if((state == TASK_READY_STATE )&& (current_task!=0))
	 {
		 break;

	 }
	}
	if(state != TASK_READY_STATE )
	{
		current_task=0;
	}


}
void task_delay(uint32_t tick_count)
{
	INTERRUPT_DISABLE();
	if(current_task)
	{
	 user_tasks[current_task].block_count=g_tick_count+tick_count;
	 user_tasks[current_task].current_state=TASK_BLOCK_STATE;
	 schedule();
	}
	INTERRUPT_ENABLE();
}
__attribute ((naked)) void ch_sp_psp(void)
{
	/*initialize psp with task1 start add*/
	__asm volatile("PUSH {LR}");
	__asm volatile("BL get_psp");
	__asm volatile("MSR PSP,R0");
	__asm volatile("PUSH {LR}");

	/*change sp to psp*/
	__asm volatile("MOV R0,#0x02");
	__asm volatile("MSR CONTROL,R0");
	__asm volatile("BX LR");

}
__attribute ((naked)) void PendSV_Handler(void)
{
	/*save task1 contents*/
	   /*get current psp of running task*/
		__asm volatile("MRS R0,PSP ");
		/*using psp store sf2 R4 to R11 */
		__asm volatile("STMDB R0!,{R4-R11}");

		__asm volatile("PUSH {LR}");

		/*save value of  psp */
		__asm volatile("BL save_psp ");

	  /*get task2 contents*/
		/*load next task content */
		__asm volatile("BL update_next_task ");
		/*get value of  psp */
		__asm volatile("BL get_psp ");
		/*using psp store sf2 R4 to R11 */
		__asm volatile("LDMIA R0!,{R4-R11}");
		/*update psp and exit*/
		__asm volatile("MSR PSP,R0 ");

		__asm volatile("POP {LR}");
		__asm volatile("BX LR");
}
void unblock_tasks(void)
{
	for(int i=1;i<MAX_Tasks;i++)
	  {
		  if(user_tasks[i].current_state!=TASK_READY_STATE)
		  {
		   if(user_tasks[i].block_count==g_tick_count)
		   {
			   user_tasks[i].current_state=TASK_READY_STATE;
		   }
		  }
	  }
}
void SysTick_Handler(void)
{
  uint32_t *pICSR=(uint32_t *)0xE000ED04;
  update_global_tick_count();
  unblock_tasks();
  /*pend the pendsv exc*/
  *pICSR|=(1<<28);
}
void schedule(void)
{
	uint32_t *pICSR=(uint32_t *)0xE000ED04;
	*pICSR|=(1<<28);
}
void update_global_tick_count()
{
	while(1);
}

