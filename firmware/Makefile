# Project Name
TARGET = Blink
# Sources
CPP_SOURCES = Blink.cpp 
# Library Locations
LIBDAISY_DIR = ../../libDaisy
DAISYSP_DIR = ../../DaisySP

C_INCLUDES += -I$(LIBDAISY_DIR)/Drivers/CMSIS/DSP/Include
C_SOURCES += $(LIBDAISY_DIR)/Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_f32.c \
             $(LIBDAISY_DIR)/Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_init_f32.c

C_SOURCES +=$(LIBDAISY_DIR)/Drivers/CMSIS/DSP/Source/CommonTables/arm_common_tables.c
C_SOURCES += $(LIBDAISY_DIR)/Drivers/CMSIS/DSP/Source/ControllerFunctions/arm_sin_cos_f32.c
CFLAGS += -DARM_MATH_CM7
# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
