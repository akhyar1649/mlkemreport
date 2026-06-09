set(KEM_BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/crypto_kem)

if(NOT DEFINED CONFIG_MLKEM_512 AND NOT DEFINED CONFIG_MLKEM_768 AND NOT DEFINED CONFIG_MLKEM_1024)
    set(CONFIG_MLKEM_512 1)
endif()

if(CONFIG_MLKEM_512)
    set(KEM_DIR ${KEM_BASE_DIR}/ml-kem-512/clean)
elseif(CONFIG_MLKEM_768)
    set(KEM_DIR ${KEM_BASE_DIR}/ml-kem-768/clean)
elseif(CONFIG_MLKEM_1024)
    set(KEM_DIR ${KEM_BASE_DIR}/ml-kem-1024/clean)
else()
    message(FATAL_ERROR "Select an ML-KEM variant in menuconfig")
endif()

set(KEM_SRCS
    ${KEM_DIR}/cbd.c
    ${KEM_DIR}/indcpa.c
    ${KEM_DIR}/kem.c
    ${KEM_DIR}/ntt.c
    ${KEM_DIR}/poly.c
    ${KEM_DIR}/polyvec.c
    ${KEM_DIR}/reduce.c
    ${KEM_DIR}/symmetric-shake.c
    ${KEM_DIR}/verify.c
)

set(COMMON_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/common/fips202.c
)

idf_component_register(
    SRCS ${KEM_SRCS} ${COMMON_SRCS} esp_randombytes.c
    INCLUDE_DIRS ${KEM_DIR} ${CMAKE_CURRENT_LIST_DIR}/common
)
