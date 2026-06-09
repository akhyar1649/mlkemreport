set(MLKEM_KEM_COMPONENT pqclean)
if(CONFIG_MLKEM_PIE_OPT)
    set(MLKEM_KEM_COMPONENT mlkem_pie)
endif()

idf_component_register(
    SRCS
        "app_main.c"
        "wifi_manager.c"
        "scenario_mqtt.c"
        "scenario_local.c"
        "microbench.c"
        "metrics.c"
        "ina219.c"
        "energy_sampler.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi nvs_flash esp_event esp_netif mqtt driver esp_timer esp_pm ${MLKEM_KEM_COMPONENT}
)
