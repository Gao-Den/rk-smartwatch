/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   17/05/2024
 ******************************************************************************
**/

#include "buzzer.h"

#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"

#include "io_cfg.h"
#include "sys_cfg.h"

volatile uint32_t beep_duration;
volatile bool tone_playing;
volatile const tone_t* tone;

GPIO_InitTypeDef GPIO_InitStructure;

void buzzer_irq() {
    if (BUZZER_TIM->SR & TIM_SR_UIF) {
        /* clear the TIMx's interrupt pending bit */
        BUZZER_TIM->SR &= ~TIM_SR_UIF;

        beep_duration--;
        if (beep_duration == 0) {
            if (tone_playing) {
                /* currently playing tones, take next tone */
                tone++;
                if (tone->frequency == 0 && tone->duration == 0) {
                    /* last tone in sequence */
                    buzzer_disable();
                    tone_playing = false;
                    tone = NULL;
                }
                else {
                    if (tone->frequency == 0) {
                        /* silence period */
                        BUZZER_TIM->ARR = 100000000 / (100 * BUZZER_TIM->PSC) - 1;
                        BUZZER_TIM->CCR1 = 0; /* 0% duty cycle */
                        beep_duration = tone->duration + 1;
                    }
                    else {
                        /* play next tone in sequence */
                        buzzer_enable(tone->frequency,tone->duration);
                    }
                }
            } 
            else {
                buzzer_disable();
            }
        }
    }
}

void buzzer_init() {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    NVIC_InitTypeDef NVIC_InitStruct;

    /* enable clocks */
    RCC_AHB1PeriphClockCmd(BUZZER_IO_CLOCK, ENABLE);
    RCC_APB1PeriphClockCmd(BUZZER_TIM_PERIPH, ENABLE);

    /* gpio configuration */
    GPIO_InitStructure.GPIO_Pin = BUZZER_IO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(BUZZER_IO_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(BUZZER_IO_PORT, BUZZER_IO_SOURCE, BUZZER_IO_AF);
    GPIO_ResetBits(BUZZER_IO_PORT, BUZZER_IO_PIN);

    /* timer configuration */
    TIM_DeInit(BUZZER_TIM);
    TIM_TimeBaseStructure.TIM_Prescaler = 100000000 / 4000000;
    TIM_TimeBaseStructure.TIM_Period = 999;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(BUZZER_TIM, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = 499;
    TIM_OC1Init(BUZZER_TIM, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(BUZZER_TIM, TIM_OCPreload_Enable);

    /* buzzer interrupt congfiguration */
    NVIC_InitStruct.NVIC_IRQChannel = BUZZER_TIM_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = SYS_IRQ_PRIO_TIMER2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStruct);

    TIM_ARRPreloadConfig(BUZZER_TIM, ENABLE);
    TIM_ITConfig(BUZZER_TIM, TIM_IT_Update, ENABLE);

    /* buzzer timer enable counter */
    TIM_Cmd(BUZZER_TIM, ENABLE);

    /* buzzer disable */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(BUZZER_IO_PORT, &GPIO_InitStructure);
    sys_ctrl_delay_ms(1);
}

void buzzer_enable(uint16_t freq, uint32_t duration) {
    if (freq < 100 || freq > 8000 || duration == 0) {
        buzzer_disable();
    }
    else {
        beep_duration = (freq / 100) * duration + 1;

        /* configure buzzer pin */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_Init(BUZZER_IO_PORT, &GPIO_InitStructure);

        /* configure and enable PWM timer */
        RCC->APB1ENR |= BUZZER_TIM_PERIPH; /* enable TIMx peripheral */
        BUZZER_TIM->ARR = 100000000 / (freq * BUZZER_TIM->PSC) - 1;
        BUZZER_TIM->CCR1 = BUZZER_TIM->ARR >> 1; /* 50% duty cycle */
        BUZZER_TIM->CR1 |= TIM_CR1_CEN; /* counter enable */
    }
}

void buzzer_disable() {
    /* counter disable */
    BUZZER_TIM->CR1 &= ~TIM_CR1_CEN;

    /* disable TIMx peripheral to conserve power */
    RCC->APB1ENR &= ~BUZZER_TIM_PERIPH;

    /* configure buzzer pin as analog input without pullup to conserve power */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(BUZZER_IO_PORT, &GPIO_InitStructure);
}

void buzzer_play_tone(const tone_t * tones) {
    tone = tones;
    tone_playing = true;
    buzzer_enable(tone->frequency,tone->duration);
}
