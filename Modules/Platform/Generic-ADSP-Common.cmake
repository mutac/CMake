# support for the Analog Devices toolchain for their DSPs
# Raphael Cotty" <raphael.cotty (AT) googlemail.com>
#
# it supports three architectures:
# Blackfin
# TS (TigerShark)
# 21k (Sharc 21xxx)

IF(NOT ADSP)

  SET(ADSP TRUE)

  SET(CMAKE_STATIC_LIBRARY_SUFFIX ".dlb")
  SET(CMAKE_SHARED_LIBRARY_SUFFIX "")
  SET(CMAKE_EXECUTABLE_SUFFIX ".dxe")

  # if ADSP_PROCESSOR has not been set, but CMAKE_SYSTEM_PROCESSOR has,
  # assume that this is the processor name to use for the compiler
  IF(CMAKE_SYSTEM_PROCESSOR AND NOT ADSP_PROCESSOR)
    SET(ADSP_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR})
  ENDIF(CMAKE_SYSTEM_PROCESSOR AND NOT ADSP_PROCESSOR)

  # if ADSP_PROCESSOR_SILICIUM_REVISION has not been set, use "none"
  IF(NOT ADSP_PROCESSOR_SILICIUM_REVISION)
    SET(ADSP_PROCESSOR_SILICIUM_REVISION "none")
  ENDIF(NOT ADSP_PROCESSOR_SILICIUM_REVISION)

  # this file is included from the C and CXX files, so handle both here

  GET_FILENAME_COMPONENT(_ADSP_DIR "${CMAKE_C_COMPILER}" PATH)
  IF(NOT _ADSP_DIR)
    GET_FILENAME_COMPONENT(_ADSP_DIR "${CMAKE_CXX_COMPILER}" PATH)
  ENDIF(NOT _ADSP_DIR)
  IF(NOT _ADSP_DIR)
    GET_FILENAME_COMPONENT(_ADSP_DIR "${CMAKE_ASM_COMPILER}" PATH)
  ENDIF(NOT _ADSP_DIR)

  # detect architecture

  IF(CMAKE_C_COMPILER MATCHES ccblkfn OR CMAKE_CXX_COMPILER MATCHES ccblkfn OR CMAKE_ASM_COMPILER MATCHES easmBLKFN)
    IF(NOT ADSP_PROCESSOR)
      SET(ADSP_PROCESSOR "ADSP-BF561")
    ENDIF(NOT ADSP_PROCESSOR)
    SET(ADSP_BLACKFIN TRUE)
    SET(_ADSP_FAMILY_DIR "${_ADSP_DIR}/Blackfin")
  ENDIF(CMAKE_C_COMPILER MATCHES ccblkfn OR CMAKE_CXX_COMPILER MATCHES ccblkfn OR CMAKE_ASM_COMPILER MATCHES easmBLKFN)

  IF(CMAKE_C_COMPILER MATCHES ccts OR CMAKE_CXX_COMPILER MATCHES ccts OR CMAKE_ASM_COMPILER MATCHES easmTS)
    IF(NOT ADSP_PROCESSOR)
      SET(ADSP_PROCESSOR "ADSP-TS101")
    ENDIF(NOT ADSP_PROCESSOR)
    SET(ADSP_TS TRUE)
    SET(_ADSP_FAMILY_DIR "${_ADSP_DIR}/TS")
  ENDIF(CMAKE_C_COMPILER MATCHES ccts OR CMAKE_CXX_COMPILER MATCHES ccts OR CMAKE_ASM_COMPILER MATCHES easmTS)

  IF(CMAKE_C_COMPILER MATCHES cc21k OR CMAKE_CXX_COMPILER MATCHES cc21k OR CMAKE_ASM_COMPILER MATCHES easm21k)
    IF(NOT ADSP_PROCESSOR)
      SET(ADSP_PROCESSOR "ADSP-21060")
    ENDIF(NOT ADSP_PROCESSOR)
    SET(ADSP_21K TRUE)

    SET(_ADSP_FAMILY_DIR "${_ADSP_DIR}/21k")  # default if nothing matches
    IF   (ADSP_PROCESSOR MATCHES "210..$")
      SET(_ADSP_FAMILY_DIR "${_ADSP_DIR}/21k")
    ENDIF(ADSP_PROCESSOR MATCHES "210..$")

    IF   (ADSP_PROCESSOR MATCHES "211..$")
      SET(_ADSP_FAMILY_DIR "${_ADSP_DIR}/211k")
    ENDIF(ADSP_PROCESSOR MATCHES "211..$")

    IF   (ADSP_PROCESSOR MATCHES "212..$")
      SET(_ADSP_FAMILY_DIR "${_ADSP_DIR}/212k")
    ENDIF(ADSP_PROCESSOR MATCHES "212..$")

    IF   (ADSP_PROCESSOR MATCHES "213..$")
      SET(_ADSP_FAMILY_DIR "${_ADSP_DIR}/213k")
    ENDIF(ADSP_PROCESSOR MATCHES "213..$")

    SET(_ADSP_FAMILY_DIR "${_ADSP_DIR}/21k")
  ENDIF(CMAKE_C_COMPILER MATCHES cc21k OR CMAKE_CXX_COMPILER MATCHES cc21k OR CMAKE_ASM_COMPILER MATCHES easm21k)


  LINK_DIRECTORIES("${_ADSP_FAMILY_DIR}/lib")

  # vdk support
  FIND_PROGRAM( ADSP_VDKGEN_EXECUTABLE vdkgen "${_ADSP_FAMILY_DIR}/vdk" )

  MACRO(ADSP_GENERATE_VDK VDK_GENERATED_HEADER VDK_GENERATED_SOURCE VDK_KERNEL_SUPPORT_FILE)
    ADD_CUSTOM_COMMAND(
      OUTPUT ${VDK_GENERATED_HEADER} ${VDK_GENERATED_SOURCE}
      COMMAND ${ADSP_VDKGEN_EXECUTABLE} ${VDK_KERNEL_SUPPORT_FILE} -proc ${ADSP_PROCESSOR} -si-revision ${ADSP_PROCESSOR_SILICIUM_REVISION} -MM
      DEPENDS ${VDK_KERNEL_SUPPORT_FILE}
      )
  ENDMACRO(ADSP_GENERATE_VDK VDK_GENERATED_HEADER VDK_GENERATED_SOURCE VDK_KERNEL_SUPPORT_FILE)

  # loader support
  FIND_PROGRAM( ADSP_ELFLOADER_EXECUTABLE elfloader "${_ADSP_FAMILY_DIR}" )

  # BOOT_MODE: prom, flash, spi, spislave, UART, TWI, FIFO
  # FORMAT: hex, ASCII, binary, include
  # WIDTH: 8, 16
  MACRO(ADSP_CREATE_LOADER_FILE TARGET_NAME BOOT_MODE FORMAT WIDTH)
    ADD_CUSTOM_COMMAND(
      TARGET ${TARGET_NAME}
      POST_BUILD
      COMMAND ${ADSP_ELFLOADER_EXECUTABLE} ${EXECUTABLE_OUTPUT_PATH}/${TARGET_NAME}.dxe -proc ${ADSP_PROCESSOR} -si-revision ${ADSP_PROCESSOR_SILICIUM_REVISION} -b ${BOOT_MODE} -f ${FORMAT} -width ${WIDTH} -o ${EXECUTABLE_OUTPUT_PATH}/${TARGET_NAME}.ldr
      COMMENT "Building the loader file"
      )
  ENDMACRO(ADSP_CREATE_LOADER_FILE TARGET_NAME BOOT_MODE FORMAT WIDTH)

  MACRO(ADSP_CREATE_LOADER_FILE_INIT TARGET_NAME BOOT_MODE FORMAT WIDTH INITIALIZATION_FILE)
    ADD_CUSTOM_COMMAND(
      TARGET ${TARGET_NAME}
      POST_BUILD
      COMMAND ${ADSP_ELFLOADER_EXECUTABLE} ${EXECUTABLE_OUTPUT_PATH}/${TARGET_NAME}.dxe -proc ${ADSP_PROCESSOR} -si-revision ${ADSP_PROCESSOR_SILICIUM_REVISION} -b ${BOOT_MODE} -f ${FORMAT} -width ${WIDTH} -o ${EXECUTABLE_OUTPUT_PATH}/${TARGET_NAME}.ldr -init ${INITIALIZATION_FILE}
      COMMENT "Building the loader file"
      )
  ENDMACRO(ADSP_CREATE_LOADER_FILE_INIT TARGET_NAME BOOT_MODE FORMAT WIDTH INITIALIZATION_FILE)

ENDIF(NOT ADSP)
