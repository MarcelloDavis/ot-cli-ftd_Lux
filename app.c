/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "sl_i2cspm_instances.h"
#include "sl_si1133.h"
#include "sl_app_log.h"

#include <assert.h>
#include <openthread-core-config.h>
#include <openthread/config.h>

#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>

#include "openthread-system.h"
#include "app.h"

#include "sl_component_catalog.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif

#if (defined(SL_CATALOG_BTN0_PRESENT) || defined(SL_CATALOG_BTN1_PRESENT))
#include "sl_button.h"
#include "sl_simple_button.h"
#endif

#if OPENTHREAD_EXAMPLES_SIMULATION
#include <setjmp.h>
#include <unistd.h>

jmp_buf gResetJump;

void __gcov_flush();
#endif

#ifndef OPENTHREAD_ENABLE_COVERAGE
#define OPENTHREAD_ENABLE_COVERAGE 0
#endif

#define sl_i2cspm_t *sl_i2cspm_sensor;

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
void *otPlatCAlloc(size_t aNum, size_t aSize)
{
    return calloc(aNum, aSize);
}

void otPlatFree(void *aPtr)
{
    free(aPtr);
}
#endif

static otInstance *    sInstance       = NULL;
static bool            sButtonPressed  = false;
static bool            sStayAwake      = true;

otInstance *otGetInstance(void)
{
    return sInstance;
}

#if (defined(SL_CATALOG_BTN0_PRESENT) || defined(SL_CATALOG_BTN1_PRESENT))
void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    sButtonPressed = true;
    otSysEventSignalPending();
  }
}
#endif

void sl_ot_rtos_application_tick(void)
{
  if (sButtonPressed) {
    sButtonPressed = false;
    sStayAwake = !sStayAwake;
    if (sStayAwake) {
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
      sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif
    } else {
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
      sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif
    }
  }
}

/*
 * Provide, if required an "otPlatLog()" function
 */
#if OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_APP
void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, ...)
{
    OT_UNUSED_VARIABLE(aLogLevel);
    OT_UNUSED_VARIABLE(aLogRegion);
    OT_UNUSED_VARIABLE(aFormat);

    va_list ap;
    va_start(ap, aFormat);
    otCliPlatLogv(aLogLevel, aLogRegion, aFormat, ap);
    va_end(ap);
}
#endif

void sl_ot_create_instance(void)
{
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    size_t   otInstanceBufferLength = 0;
    uint8_t *otInstanceBuffer       = NULL;

    // Call to query the buffer size
    (void)otInstanceInit(NULL, &otInstanceBufferLength);

    // Call to allocate the buffer
    otInstanceBuffer = (uint8_t *)malloc(otInstanceBufferLength);
    assert(otInstanceBuffer);

    // Initialize OpenThread with the buffer
    sInstance = otInstanceInit(otInstanceBuffer, &otInstanceBufferLength);
#else
    sInstance = otInstanceInitSingle();
#endif
    assert(sInstance);
}

void sl_ot_cli_init(void)
{
    otCliUartInit(sInstance);
}

// Private function declarations

/*static void shutdown_start_timer(void);
static void shutdown_stop_timer(void);
static void shutdown(sl_simple_timer_t *timer, void *data);
*/
static void sensor_init(void);
static void sensor_deinit(void);
/**************************************************************************//**
 * Application Init.
 *****************************************************************************/

void app_init(void)
{

  sl_app_log("Thuderboard demo initialised\n");
  sensor_init();
#if OPENTHREAD_EXAMPLES_SIMULATION
    if (setjmp(gResetJump))
    {
        alarm(0);
#if OPENTHREAD_ENABLE_COVERAGE
        __gcov_flush();
#endif
        execvp(argv[0], argv); // TO DO: argc, argv?
    }
#endif
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void)
{
    //Help Befehle nicht verfügbar, falls:
    //     alles ausgeklammert
    //    nur ot-Befehle ausgeklammert
    //    alles NICHT ausgeklammert
    //    ot Befehle und app_log ausgeklammert
    //    alles außer sl_app_log ausgeklammert

    otTaskletsProcess(sInstance);
    otSysProcessDrivers(sInstance);

    sl_status_t sc;
     float lux=0.0;
     float uvi=0.0;
     sc = sl_sensor_light_get(&lux, &uvi);
     if (sc != SL_STATUS_OK) {
         sl_app_log("Warning! Invalid \n");
     }
     sl_app_log("Lux: %3.2f, UVI: %1.2f \n", lux, uvi);

}

/**************************************************************************//**
 * Application Exit.
 *****************************************************************************/
void app_exit(void)
{
    otInstanceFinalize(sInstance);
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    free(otInstanceBuffer);
#endif
    // TO DO : pseudo reset?
}

// Sensor init / deinit

static void sensor_init(void)
{
#ifdef SL_CATALOG_SENSOR_LIGHT_PRESENT
  sl_sensor_light_init();
#endif // SL_CATALOG_SENSOR_LIGHT_PRESENT
#ifdef SL_CATALOG_SENSOR_LUX_PRESENT
  sl_sensor_lux_init();
#endif // SL_CATALOG_SENSOR_LUX_PRESENT
}

static void sensor_deinit(void)
{
#ifdef SL_CATALOG_SENSOR_LIGHT_PRESENT
  sl_sensor_light_deinit();
#endif // SL_CATALOG_SENSOR_LIGHT_PRESENT
#ifdef SL_CATALOG_SENSOR_LUX_PRESENT
  sl_sensor_lux_deinit();
#endif // SL_CATALOG_SENSOR_LUX_PRESENT
}
