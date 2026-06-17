TOP=..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

PROD_HOST += bind
USR_INCLUDES += -I$(TOP)/../
USR_INCLUDES += -I$(PVXS)/include/
bind_SRCS += bind.cpp
bind_LIBS += pvxs $(EPICS_BASE_HOST_LIBS)

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
