#All card firmware specific options is at vnet.h

#Common Cflags
CCFLAGS = -DATMEL_WLAN -D__EVENT_INTERRUPTS

#Debug flags
CCFLAGS += -DPCMCIA_DEBUG -DDBG

EXTRA_INCVPATH=$(PROJECT_ROOT)/include
EXTRA_SRCVPATH=$(PROJECT_ROOT)/xp
LIBS=drvrS pccardS pmS
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

include $(MKFILES_ROOT)/qtargets.mk
