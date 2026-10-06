/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   18/11/2024
 ******************************************************************************
**/

#include "sys_irq.h"

#include "sys_cfg.h"
#include "stm32f4xx.h"
#include "system_stm32f4xx.h"

#include "io_cfg.h"
#include "cst816t.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_console.h"
#include "task_display.h"

#include "task.h"
#include "net_rf.h"

/* nested critical section */
static int32_t nested_critical_counter = 0;
static uint32_t original_primask = 0;

/* counter of milliseconds since the application run */
volatile uint32_t millis_current;

void default_handler() {
    SYS_FATAL("SYSTEM", 0x01);
}

void nmi_handler() {
    SYS_FATAL("SYSTEM", 0x02);
}

void hardfault_handler() {
    SYS_FATAL("SYSTEM", 0x03);
}

void mem_manage_handler() {
    SYS_FATAL("SYSTEM", 0x04);
}

void bus_fault_handler() {
    SYS_FATAL("SYSTEM", 0x05);
}

void usage_fault_handler() {
    SYS_FATAL("SYSTEM", 0x06);
}

void svc_handler() {

}

void debug_mon_handler() {

}

void pendsv_handler() {
    task_os_pendsv_handler();
}

void enable_interrupts() {
    __enable_irq();
}

void disable_interrupts() {
    __disable_irq();
}

void enter_critical() {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    if (nested_critical_counter == 0) {
        original_primask = primask;
    }

    nested_critical_counter++;
}

void exit_critical() {
    if (nested_critical_counter > 0) {
        nested_critical_counter--;
        
        if (nested_critical_counter == 0) {
            if (original_primask == 0) {
                __enable_irq();
            }
        }
    }
    else {
        SYS_FATAL("CRI", 0x01);
    }
}

void pendsv_trigger() {
    /* set pendSV pending */
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void system_tick_handler() {
    static uint32_t div_counter = 0;

    /* increasing millis counter */
    millis_current++;

    /* task os scheduler */
    task_os_tick(1);

    if (div_counter == 10) {
        div_counter = 0;
    }

    switch (div_counter) {
    case 0: {
        /* timer service 10ms */
        sys_irq_timer_10ms();
    }
        break;

    default:
        break;
    }

    div_counter++;
}

uint32_t sys_ctrl_millis() {
    volatile uint32_t ret;
    ENTRY_CRITICAL();
    ret = millis_current;
    EXIT_CRITICAL();
    return ret;
}

void usart1_irq_handler() {
    volatile uint8_t c = 0;
    c = usart1_get_char();
    sys_console_getc(c);
}

void exti9_5_handler() {
    if (EXTI_GetITStatus(NRF24L01_IRQ_LINE) != RESET) {
        /* nrf24 irq handler */
        nrf_phy_irq();

        /* clear interrupt pending bit */
        EXTI_ClearITPendingBit(NRF24L01_IRQ_LINE);
    }
}

void exti_15_10_handler() {
    if (EXTI_GetITStatus(TOUCH_IRQ_LINE) != RESET) {
        /* touch irq handler */
        cst816t_touch_isr();

        /* clear interrupt pending bit */
        EXTI_ClearITPendingBit(TOUCH_IRQ_LINE);
    }
}

void dma2_stream3_irq_handler() {
    if (DMA_GetITStatus(DMA2_Stream3, DMA_IT_TCIF3)) {

        DMA_ClearITPendingBit(DMA2_Stream3, DMA_IT_TCIF3);
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

        /* lvgl flush ready */
        st7789_dma_irq();
    }
}
