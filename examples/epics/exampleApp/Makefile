TOP=..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

PROD_HOST += example
USR_INCLUDES += -I$(TOP)/../../
USR_INCLUDES += -I$(PVXS)/include/
example_SRCS += example.cpp
example_LIBS += pvxs $(EPICS_BASE_HOST_LIBS)

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
