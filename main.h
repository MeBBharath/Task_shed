#ifndef MAIN_H_
#define MAIN_H_


#define Task_size 1024U
#define SCED_Task_size 1024U

#define SRAM_start 0x20000000U
#define SRAM_size (128 *1024)
#define SRAM_end  ((SRAM_start) +(SRAM_size))

#define Task1_start SRAM_end
#define Task2_start ((Task1_start) - (1*Task_size))
#define Task3_start ((Task1_start) - (2*Task_size))
#define Task4_start ((Task1_start) - (3*Task_size))
#define IDLE_start  ((Task1_start) - (4*Task_size))
#define SCHED_Task_start ((Task1_start) - (5*Task_size))

#define SYSTICK_TIM_clk 16000000U
#define TICK_HZ 1000U

#define MAX_Tasks 5
#define DUMMY_XPSR 0x01000000U

#define TASK_READY_STATE 0x00
#define TASK_BLOCK_STATE 0xFF

#define INTERRUPT_DISABLE() do{__asm volatile("MOV R0,0x1 ");__asm volatile("MSR PRIMASK,R0 ");}while(0)
#define INTERRUPT_ENABLE()  do{__asm volatile("MOV R0,0x1 ");__asm volatile("MSR PRIMASK,R0");}while(0)

#endif /* MAIN_H_ */
