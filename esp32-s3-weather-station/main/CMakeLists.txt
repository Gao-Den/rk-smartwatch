idf_component_register(
    INCLUDE_DIRS 
        "lite_thread/inc"
        "app"
        "common"
        "drivers/button"
        "drivers/ethernet"
        "drivers/pcf8563"
        "drivers/rf/hal"
        "sys"
        "server"
        "libraries/buzzer"

    SRCS 
        "lite_thread/src/lt_task.c"
        "lite_thread/src/lt_timer.c"
        "lite_thread/src/lt_fsm.c"
        "lite_thread/src/lt_message.c"
        "lite_thread/src/lt_log.c"

        "drivers/button/button.c"
        "drivers/pcf8563/pcf8563.c"
        "drivers/ethernet/ethernet_init.c"
        "drivers/rf/hal/hal_nrf_hw.c"
        "drivers/rf/hal/hal_nrf_l01.c"

        "libraries/buzzer/Buzzer.cpp"
        "common/cmd_line.c"

        "sys/io_cfg.c"
        "sys/sys_cfg.c"
        
        "server/http_server.c"

        "app/app.cpp"
        "app/app_flash.cpp"
        "app/app_network.cpp"
        "app/bsp.cpp"
        "app/shell.cpp"
        "app/task_list.cpp"
        "app/task_life.cpp"
        "app/task_console.cpp"
        "app/task_dbg.cpp"
        "app/task_gw.cpp"
        "app/task_net.cpp"
        "app/task_cloud.cpp"
        "app/task_polling.cpp"
        "app/task_if.cpp"
)

spiffs_create_partition_image(storage ../main/storage FLASH_IN_PROJECT)
target_add_binary_data(${COMPONENT_TARGET} "server/index.html" TEXT)
