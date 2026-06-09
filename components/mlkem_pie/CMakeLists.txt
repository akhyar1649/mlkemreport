set(PQCLEAN_DIR ${CMAKE_CURRENT_LIST_DIR}/../pqclean)
set(KEM_BASE_DIR ${PQCLEAN_DIR}/crypto_kem)

if(NOT DEFINED CONFIG_MLKEM_512 AND NOT DEFINED CONFIG_MLKEM_768 AND NOT DEFINED CONFIG_MLKEM_1024)
    set(CONFIG_MLKEM_512 1)
endif()

if(CONFIG_MLKEM_512)
    set(KEM_DIR ${KEM_BASE_DIR}/ml-kem-512/clean)
    set(PIE_NTT ${CMAKE_CURRENT_LIST_DIR}/ml-kem-512/ntt_pie.c)
elseif(CONFIG_MLKEM_768)
    set(KEM_DIR ${KEM_BASE_DIR}/ml-kem-768/clean)
    set(PIE_NTT ${CMAKE_CURRENT_LIST_DIR}/ml-kem-768/ntt_pie.c)
elseif(CONFIG_MLKEM_1024)
    set(KEM_DIR ${KEM_BASE_DIR}/ml-kem-1024/clean)
    set(PIE_NTT ${CMAKE_CURRENT_LIST_DIR}/ml-kem-1024/ntt_pie.c)
else()
    message(FATAL_ERROR "Select an ML-KEM variant in menuconfig")
endif()

set(KEM_SRCS
    ${KEM_DIR}/cbd.c
    ${KEM_DIR}/indcpa.c
    ${KEM_DIR}/kem.c
    ${PIE_NTT}
    ${KEM_DIR}/poly.c
    ${KEM_DIR}/polyvec.c
    ${KEM_DIR}/reduce.c
    ${KEM_DIR}/symmetric-shake.c
    ${KEM_DIR}/verify.c
)

set(COMMON_SRCS
    ${PQCLEAN_DIR}/common/fips202.c
)

idf_component_register(
    SRCS ${KEM_SRCS} ${COMMON_SRCS} ${PQCLEAN_DIR}/esp_randombytes.c
    INCLUDE_DIRS ${KEM_DIR} ${PQCLEAN_DIR}/common ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE MLKEM_PIE=1)

if(CONFIG_MLKEM_PIE_AGGRESSIVE)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE MLKEM_PIE_AGGRESSIVE=1)
endif()
