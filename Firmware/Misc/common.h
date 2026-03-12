/**
 * @file       common.h
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Common definitions
 *
 * @addtogroup grSerial
 * @{
 */

#ifndef _COMMON_H_
#define _COMMON_H_

/* Includes ------------------------------------------------------------------*/

#include <string.h>
#include <stdint.h>

/* Definitions----------------------------------------------------------------*/

/**
 * Success status
 */
#define STATUS_OK        0
/**
 * Error or fail status
 */
#define STATUS_ERROR     1

/**
 * Error or fail status
 */
#define STATUS_TIMEOUT   2

/**
 * Busy status
 */
#define STATUS_BUSY      3

/* Macros ------------------------------------------------------------------*/

/**
 * Minimum of two arguments
 */
#ifndef MIN
#define MIN(a, b)   (((a)>(b))?(b):(a))
#endif

/**
 * Maximum of two arguments
 */
#ifndef MAX
#define MAX(a, b)   (((a)<(b))?(b):(a))
#endif

/**
 * Saturate the X to the given upper bound VAL
 */
#define SAT_UP(x, val)      ((x) = ((x)>(val))?(val):(x))

/**
 * Saturate the X to the given lower bound VAL
 */
#define SAT_DOWN(x, val)    ((x) = ((x)<(val))?(val):(x))

/**
 * General printf-like definition
 */
#define PRINTF(...)
//#define PRINTF(...) printf(__VA_ARGS__)

/**
 * Parameters assertion. If the expr is false, the warning message is printed.
 */
#define ASSERT_PARAM(expr)

/**
 * Common Error message
 */
#define ERR_PRINT(ret, code)

/**
 * Error checking macro. If ret_value is non-zero, the error message is printed with given error_code and code continues
 */
#define CHECK_ERROR(ret_value, error_code)\
do{\
  if ((ret_value) == STATUS_ERROR)\
  {\
    ret_value = 0;\
  }\
}while(0)

#ifndef __weak
    #define __weak   __attribute__((weak))
#endif /* __weak */

/**
 * Backward compatibility of HAL error handlers
 */
//#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

/**
 * Get byte macros
 */
#define GET_BYTE_0(a)       ( (uint8_t) ((a) & 0xff))
#define GET_BYTE_1(a)       ( (uint8_t) (((a) >> 8) & 0xff))
#define GET_BYTE_2(a)       ( (uint8_t) (((a) >> 16) & 0xff))
#define GET_BYTE_3(a)       ( (uint8_t) (((a) >> 24) & 0xff))

/* Typedefs-------------------------------------------------------------------*/

/**
 * General system pointer to function type
 */
typedef void (*System_Callback_t)(void);

/**
 * General status return type
 */
typedef int16_t Status_t;

/* Functions -----------------------------------------------------------------*/

void _Error_Handler(char *file, int line);

void Error_Handler(void);

/* ---------------------------------------------------------------------------*/

#endif /* _COMMON_H_ */

/** @} */
