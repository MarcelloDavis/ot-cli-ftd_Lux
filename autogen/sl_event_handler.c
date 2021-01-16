#include "sl_event_handler.h"

#include "em_chip.h"
#include "sl_device_init_nvic.h"
#include "sl_board_init.h"
#include "sl_device_init_dcdc.h"
#include "sl_device_init_hfxo.h"
#include "sl_device_init_lfxo.h"
#include "sl_device_init_clocks.h"
#include "sl_device_init_emu.h"
#include "pa_conversions_efr32.h"
#include "sl_rail_util_pti.h"
#include "platform-efr32.h"
#include "sl_ot_internal_app_init.h"
#include "sl_sleeptimer.h"
#include "sl_debug_swo.h"
#include "sl_i2cspm_instances.h"
#include "sl_iostream_init_instances.h"
#include "sl_iostream_debug.h"
#include "sl_iostream_rtt.h"
#include "sl_iostream_swo.h"
#include "sl_iostream_vuart.h"
#include "sl_mbedtls.h"
#include "sl_mpu.h"
#include "nvm3_default.h"
#include "sl_simple_button_instances.h"
#include "sl_uartdrv_instances.h"

void sl_iostream_init_instances(void)
{
  sl_iostream_debug_init();
  sl_iostream_rtt_init();
  sl_iostream_swo_init();
  sl_iostream_vuart_init();
}

void sl_platform_init(void)
{
  CHIP_Init();
  sl_device_init_nvic();
  sl_board_preinit();
  sl_device_init_dcdc();
  sl_device_init_hfxo();
  sl_device_init_lfxo();
  sl_device_init_clocks();
  sl_device_init_emu();
  sl_board_init();
  nvm3_initDefault();
}

void sl_driver_init(void)
{
  sl_debug_swo_init();
  sl_i2cspm_init_instances();
  sl_simple_button_init_instances();
  sl_uartdrv_init_instances();
}

void sl_service_init(void)
{
  sl_sleeptimer_init();
  sl_iostream_init_instances();
  sl_mbedtls_init();
  sl_mpu_disable_execute_from_ram();
}

void sl_stack_init(void)
{
  sl_rail_util_pa_init();
  sl_rail_util_pti_init();
  sl_ot_sys_init();
}

void sl_internal_app_init(void)
{
  sl_ot_create_instance();
  sl_ot_cli_init();
  sl_ot_custom_cli_init();
}

void sl_platform_process_action(void)
{
}

void sl_service_process_action(void)
{
}

void sl_stack_process_action(void)
{
}

void sl_internal_app_process_action(void)
{
}

