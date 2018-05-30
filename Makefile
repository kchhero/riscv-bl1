#
# Copyright (C) 2012 Nexell Co., All Rights Reserved
# Nexell Co. Proprietary & Confidential
#
# NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
# AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
# BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR
# FITNESS FOR A PARTICULAR PURPOSE.
#
# Moudle	: bl0
# File		: Makefile
# Description	:
# Author	: Firware Team
# History	: 2017.07.21 hans first draft
#

include config.mak

CFLAGS		+= \
		-march=$(ARCH) \
		-mabi=$(ABI) \
		-mcmodel=medany \
		-O2 -std=gnu11 \
		-Wall -nostartfiles \
		-fno-common -DENTROPY=0 \
		-DNONSMP_HART=0 \
		-I ./src -I . -I ./include

LDFLAGS	=	-static -nostdlib -L ./ -T$(LDS_NAME) \
		-Wl,-Map=$(DIR_TARGETOUTPUT)/$(PROJECT_NAME).map,--cref	\
		-L$(DIR_OBJOUTPUT) \
		-march=$(ARCH) \
                -mabi=$(ABI)


SYS_OBJS	=	head.o
SYS_OBJS	+=	bl1_main.o \
			iSDBOOT.o \
			iSPIBOOT.o \
			nx_gpt.o \
			nx_lib.o \
			nx_gpio.o \
			nx_cpuif_regmap.o

emul = no
ifeq ($(QEMU), y)
emul = yes
endif
ifeq ($(SIM), y)
emul = yes
CFLAGS			+=	-DSOC_SIM
endif

ifeq ($(emul), yes)
	SYS_OBJS	+=	nx_qemu_sim_printf.o
else
	SYS_OBJS	+=	nx_swallow_printf.o nx_debug.o nx_lib.o
endif

ifeq ($(MEMTEST),y)
CFLAGS			+=	-DMEMTEST -DSIMPLE_MEMTEST
SYS_OBJS		+=	memtester.o
endif

SYS_OBJS_LIST	= 	$(addprefix $(DIR_OBJOUTPUT)/,$(SYS_OBJS))

SYS_INCLUDES 	=	-I. \
			-I ./src			\
			-I ./src/test			\
			-I ./include


################################################################################
$(DIR_OBJOUTPUT)/%.o: src/%.c
	$(Q)echo [compile....$<]
	$(Q)$(CC) $< -c -o $@ $(CFLAGS) $(SYS_INCLUDES)
################################################################################
$(DIR_OBJOUTPUT)/%.o: src/test/%.c
	$(Q)echo [compile....$<]
	$(Q)$(CC) $< -c -o $@ $(CFLAGS) $(SYS_INCLUDES)
################################################################################
$(DIR_OBJOUTPUT)/%.o: src/%.S
	$(Q)echo [compile....$<]
	$(Q)$(CC) $< -c -o $@ $(ASFLAG) $(CFLAGS) $(SYS_INCLUDES)
################################################################################


#all: libs mkobjdir $(SYS_OBJS_LIST) link bin
all: mkobjdir $(SYS_OBJS_LIST) link bin

mkobjdir:
ifeq ($(OS),Windows_NT)
	$(Q)if not exist $(DIR_OBJOUTPUT)		\
		$(Q)$(MKDIR) $(DIR_OBJOUTPUT)
	$(Q)if not exist $(DIR_TARGETOUTPUT)		\
		$(Q)$(MKDIR) $(DIR_TARGETOUTPUT)
else
	$(Q)if [ ! -e $(DIR_OBJOUTPUT) ]; then		\
		$(MKDIR) $(DIR_OBJOUTPUT);		\
	fi;
	$(Q)if [ ! -e $(DIR_TARGETOUTPUT) ]; then	\
		$(MKDIR) $(DIR_TARGETOUTPUT);		\
	fi;
endif

#libs:
#	$(MAKE) -C src

link:
	$(Q)echo [link.... $(PROJECT_NAME).elf]
	$(Q)$(CC) $(SYS_OBJS_LIST) $(LDFLAGS)		\
	       	-o $(DIR_TARGETOUTPUT)/$(PROJECT_NAME).elf

bin:
	$(Q)echo [binary.... $(PROJECT_NAME).bin]
	$(Q)$(MAKEBIN) -O binary $(DIR_TARGETOUTPUT)/$(PROJECT_NAME).elf	\
		$(DIR_TARGETOUTPUT)/$(PROJECT_NAME).bin

ifeq ($(OS),Windows_NT)
	$(Q)if exist $(DIR_OBJOUTPUT)			\
		@$(RM) $(DIR_OBJOUTPUT)\buildinfo.o
else
	$(Q)if     [ -e $(DIR_OBJOUTPUT) ]; then	\
		$(RM) $(DIR_OBJOUTPUT)/buildinfo.o;	\
	fi;
endif

clean:
#	$(MAKE) -C src clean
ifeq ($(OS),Windows_NT)
	$(Q)if exist $(DIR_OBJOUTPUT)			\
		$(Q)$(RMDIR) $(DIR_OBJOUTPUT)
	$(Q)if exist $(DIR_TARGETOUTPUT)		\
		$(Q)$(RMDIR) $(DIR_TARGETOUTPUT)
else
	$(Q)if [ -e $(DIR_OBJOUTPUT) ]; then		\
		$(RMDIR) $(DIR_OBJOUTPUT);		\
	fi;
	$(Q)if [ -e $(DIR_TARGETOUTPUT) ]; then		\
		$(RMDIR) $(DIR_TARGETOUTPUT);		\
	fi;

endif


################################################################################
-include $(SYS_OBJS_LIST:.o=.d)

