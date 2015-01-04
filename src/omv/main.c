/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * main function.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stm32f4xx_hal.h>
#include "mpconfig.h"
#include "misc.h"
#include "systick.h"
#include "pendsv.h"
#include "qstr.h"
#include "misc.h"
#include "nlr.h"
#include "lexer.h"
#include "parse.h"
#include "obj.h"
#include "objmodule.h"
#include "runtime.h"
#include "gc.h"
#include "stackctrl.h"
#include "gccollect.h"
#include "readline.h"
#include "pyexec.h"
#include "timer.h"
#include "pin.h"
#include "usb.h"
#include "rtc.h"
#include "storage.h"
#include "sdcard.h"
#include "ff.h"
#include "mdefs.h"

#include "rng.h"
#include "sensor.h"
#include "usbdbg.h"
#include "sdram.h"
#include "xalloc.h"

#include "py_led.h"
#include "py_time.h"
#include "py_sensor.h"
#include "py_image.h"
#include "py_file.h"
#include "py_lwip.h"
#include "py_socket.h"
#include "py_select.h"
#include "py_gpio.h"
#include "py_spi.h"
#include "py/uart.h"

#include "mlx90620.h"

int errno;
static FATFS fatfs;
extern char _stack_size;

static const char fresh_main_py[] =
"# main.py -- put your code here!\n"
"import led, time\n"
"while (vcp_is_connected()==False):\n"
"   led.on(led.BLUE)\n"
"   time.sleep(150)\n"
"   led.off(led.BLUE)\n"
"   time.sleep(100)\n"
"   led.on(led.BLUE)\n"
"   time.sleep(150)\n"
"   led.off(led.BLUE)\n"
"   time.sleep(600)\n"
;

static const char fresh_pybcdc_inf[] =
#include "genhdr/pybcdc_inf.h"
;

static const char fresh_readme_txt[] =
"This is a Micro Python board\r\n"
"\r\n"
"You can get started right away by writing your Python code in 'main.py'.\r\n"
"\r\n"
"For a serial prompt:\r\n"
" - Windows: you need to go to 'Device manager', right click on the unknown device,\r\n"
"   then update the driver software, using the 'pybcdc.inf' file found on this drive.\r\n"
"   Then use a terminal program like Hyperterminal or putty.\r\n"
" - Mac OS X: use the command: screen /dev/tty.usbmodem*\r\n"
" - Linux: use the command: screen /dev/ttyACM0\r\n"
"\r\n"
"Please visit http://micropython.org/help/ for further help.\r\n"
;

typedef struct {
    const char *name;
    const mp_obj_module_t *(*init)(void);
} module_t;

static const module_t init_modules[] ={
    {"sensor",  py_sensor_init},
    {"led",     py_led_init},
    {"time",    py_time_init},
	{"lwip",    py_lwip_init},
//  {"wlan",    py_wlan_init},
//  {"socket",  py_socket_init},
//  {"select",  py_select_init},
//    {"spi",     py_spi_init},
    {"gpio",    py_gpio_init},
//    {"uart",    py_uart_init},
#ifdef OPENMV2
//    {"mlx90620", py_mlx90620_init},
#endif
    {NULL}
};

void flash_error(int n) {
    for (int i = 0; i < n; i++) {
        led_state(LED_RED, 0);
        HAL_Delay(100);
        led_state(LED_RED, 1);
        HAL_Delay(100);
    }
    led_state(LED_RED, 0);
}

void __fatal_error(const char *msg) {
    printf("\nFATAL ERROR:\n");
    printf(msg);
    for (uint i = 0;;) {
        led_toggle(((i++) & 3));
        for (volatile uint delay = 0; delay < 500000; delay++) {
        }
    }
}

void nlr_jump_fail(void *val) {
    printf("FATAL: uncaught exception %p\n", val);
    __fatal_error("");
}

#ifndef NDEBUG
void __attribute__((weak))
    __assert_func(const char *file, int line, const char *func, const char *expr) {
    (void)func;
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("");

    while (1);
}
#endif

STATIC mp_obj_t pyb_config_source_dir = MP_OBJ_NULL;
STATIC mp_obj_t pyb_config_main = MP_OBJ_NULL;
STATIC mp_obj_t pyb_config_usb_mode = MP_OBJ_NULL;

STATIC mp_obj_t pyb_source_dir(mp_obj_t source_dir) {
    if (MP_OBJ_IS_STR(source_dir)) {
        pyb_config_source_dir = source_dir;
    }
    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_1(pyb_source_dir_obj, pyb_source_dir);

STATIC mp_obj_t pyb_main(mp_obj_t main) {
    if (MP_OBJ_IS_STR(main)) {
        pyb_config_main = main;
    }
    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_1(pyb_main_obj, pyb_main);

STATIC mp_obj_t pyb_usb_mode(mp_obj_t usb_mode) {
    if (MP_OBJ_IS_STR(usb_mode)) {
        pyb_config_usb_mode = usb_mode;
    }
    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_1(pyb_usb_mode_obj, pyb_usb_mode);

STATIC mp_obj_t py_vcp_is_connected(void ) {
    return MP_BOOL(usb_vcp_is_connected());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_vcp_is_connected_obj, py_vcp_is_connected);

STATIC mp_obj_t py_random(mp_obj_t min, mp_obj_t max) {
    return mp_obj_new_int(rng_randint(mp_obj_get_int(min), mp_obj_get_int(max)));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_random_obj, py_random);

extern uint32_t SystemCoreClock;
STATIC mp_obj_t py_cpu_freq(void ) {
    return mp_obj_new_int(SystemCoreClock);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_cpu_freq_obj, py_cpu_freq);

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_load_image_obj, py_image_load_image);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_load_cascade_obj, py_image_load_cascade);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_load_descriptor_obj, py_image_load_descriptor);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_load_lbp_obj, py_image_load_lbp);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_save_descriptor_obj, py_image_save_descriptor);

static void make_flash_fs()
{
    FIL fp;
    UINT n;

    led_state(LED_RED, 1);

    if (f_mkfs("0:", 0, 0) != FR_OK) {
        __fatal_error("could not create LFS");
    }

    // create default main.py
    f_open(&fp, "main.py", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fp, fresh_main_py, sizeof(fresh_main_py) - 1 /* don't count null terminator */, &n);
    f_close(&fp);

    // create .inf driver file
    f_open(&fp, "pybcdc.inf", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fp, fresh_pybcdc_inf, sizeof(fresh_pybcdc_inf) - 1 /* don't count null terminator */, &n);
    f_close(&fp);

    // create readme file
    f_open(&fp, "README.txt", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fp, fresh_readme_txt, sizeof(fresh_readme_txt) - 1 /* don't count null terminator */, &n);
    f_close(&fp);

    led_state(LED_RED, 0);
}

int main(void)
{
    // Stack limit should be less than real stack size, so we
    // had chance to recover from limit hit.
    mp_stack_set_limit((mp_uint_t) (&_stack_size - 1024));

    /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
    */
    HAL_Init();

#ifdef OPENMV2
    if (sdram_init() == false) {
        __fatal_error("could not init sdram");
    }
#if 0   //SDRAM test
    if (sdram_test() == false) {
        __fatal_error("sdram test1 failed");
    }
#endif
#endif

    // basic sub-system init
    pendsv_init();
    timer_tim3_init();

soft_reset:
    // check if user switch held to select the reset mode
    led_state(LED_RED, 1);
    led_state(LED_GREEN, 1);
    led_state(LED_BLUE, 1);
    uint reset_mode = 1;

#if MICROPY_HW_ENABLE_RTC
    rtc_init();
#endif

    // GC init
    gc_init(&_heap_start, &_heap_end);

    // Micro Python init
    mp_init();
    mp_obj_list_init(mp_sys_path, 0);
    mp_obj_list_init(mp_sys_argv, 0);

    readline_init0();
    pin_init0();
    timer_init0();
    pyb_usb_init0();

    xalloc_init();
    rng_init();
    usbdbg_init();

    /* init built-in modules */
    for (const module_t *p = init_modules; p->init; p++) {
        const mp_obj_module_t *module = p->init();
        if (module == NULL) {
            char buf[256];
            snprintf(buf, sizeof(buf), "failed to init %s module", p->name);
            __fatal_error(buf);
        }
    }

    /* Export functions to the global python namespace */
    mp_store_global(qstr_from_str("random"),            (mp_obj_t)&py_random_obj);
    mp_store_global(qstr_from_str("cpu_freq"),          (mp_obj_t)&py_cpu_freq_obj);
    mp_store_global(qstr_from_str("Image"),             (mp_obj_t)&py_image_load_image_obj);
    mp_store_global(qstr_from_str("HaarCascade"),       (mp_obj_t)&py_image_load_cascade_obj);
    mp_store_global(qstr_from_str("FreakDesc"),         (mp_obj_t)&py_image_load_descriptor_obj);
    mp_store_global(qstr_from_str("FreakDescSave"),     (mp_obj_t)&py_image_save_descriptor_obj);
    mp_store_global(qstr_from_str("LBPDesc"),           (mp_obj_t)&py_image_load_lbp_obj);
    mp_store_global(qstr_from_str("vcp_is_connected"),  (mp_obj_t)&py_vcp_is_connected_obj);

    usb_storage_medium_t usb_medium;

    if (sdcard_is_present()) {
        sdcard_init();
        FRESULT res = f_mount(&fatfs, "1:", 1);
        if (res != FR_OK) {
            __fatal_error("could not mount SD\n");
        }
        // Set CWD and USB medium to SD
        f_chdrive("1:");
        usb_medium = USB_STORAGE_MEDIUM_SDCARD;

        // add sdcard to sys path
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_1_colon__slash_));
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_1_colon__slash_lib));
    } else {
        storage_init();
        // try to mount the flash
        FRESULT res = f_mount(&fatfs, "0:", 1);
        if (reset_mode == 3 || res == FR_NO_FILESYSTEM) {
            // create a fresh fs
            make_flash_fs();
        } else if (res != FR_OK) {
            __fatal_error("could not access LFS\n");
        }

        // Set CWD and USB medium to flash
        f_chdrive("0:");
        usb_medium = USB_STORAGE_MEDIUM_FLASH;

        // add sdcard to sys path
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_0_colon__slash_));
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_0_colon__slash_lib));
    }

    // turn boot-up LEDs off
    led_state(LED_RED, 0);
    led_state(LED_GREEN, 0);
    led_state(LED_BLUE, 0);

    // USB device
    if (reset_mode == 1) {
        usb_device_mode_t usb_mode = USB_DEVICE_MODE_CDC_MSC;
        if (pyb_config_usb_mode != MP_OBJ_NULL) {
            if (strcmp(mp_obj_str_get_str(pyb_config_usb_mode), "CDC+HID") == 0) {
                usb_mode = USB_DEVICE_MODE_CDC_HID;
            }
        }
        pyb_usb_dev_init(usb_mode, usb_medium);
    } else {
        pyb_usb_dev_init(USB_DEVICE_MODE_CDC_MSC, usb_medium);
    }

#if 0
    // Run the main script from the current directory.
    if (reset_mode == 1 && pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL) {
        FRESULT res = f_stat("main.py", NULL);
        if (res == FR_OK) {
            if (!pyexec_file("main.py")) {
                nlr_buf_t nlr;
                if (nlr_push(&nlr) == 0) {
                    flash_error(3);
                    nlr_pop();
                }
            }
        }
    }
#endif /* 0 */

    // Enter REPL
    nlr_buf_t nlr;
    for (;;) {
        if (nlr_push(&nlr) == 0) {
            if (usbdbg_script_ready()) {
                pyexec_push_scope();
                pyexec_str(usbdbg_get_script());
                pyexec_pop_scope();
            }
            usbdbg_clr_script();

            // no script run repl
            pyexec_friendly_repl();
            nlr_pop();
        }
    }

    printf("PYB: sync filesystems\n");
    storage_flush();

    printf("PYB: soft reboot\n");

    goto soft_reset;
}
