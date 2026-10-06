/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   07/09/2025
 * @brief:  app log debug
 ******************************************************************************
**/

#ifndef __APP_DBG_H__
#define __APP_DBG_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#if defined (APP_RELEASE)
    #define APP_PRINT(fmt, ...)                 printf(fmt, ##__VA_ARGS__)
    #define APP_DBG(fmt, ...)               
#else
    #define APP_PRINT(fmt, ...)                 printf(fmt, ##__VA_ARGS__)
    #define APP_DBG(fmt, ...)                   printf("[DBG] " fmt, ##__VA_ARGS__)
#endif

#endif /* __APP_DBG_H__ */
