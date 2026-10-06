/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   18/11/2024
 ******************************************************************************
**/

#ifndef __SYS_IRQ_H__
#define __SYS_IRQ_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>

#define ENTRY_CRITICAL()            enter_critical()
#define EXIT_CRITICAL()             exit_critical()

/* system interrupts */
extern void default_handler();
extern void nmi_handler();
extern void hardfault_handler();
extern void mem_manage_handler();
extern void bus_fault_handler();
extern void usage_fault_handler();
extern void svc_handler();
extern void debug_mon_handler();
extern void pendsv_handler();
extern void system_tick_handler();

extern void enable_interrupts();
extern void disable_interrupts();
extern void enter_critical();
extern void exit_critical();

/* external interrupts */
extern void usart1_irq_handler();
extern void usart2_irq_handler();
extern void exti9_5_handler();

#ifdef __cplusplus
}
#endif

#endif /* __SYS_IRQ_H__ */
