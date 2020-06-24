#
_XDCBUILDCOUNT = 
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = /home/raffaele-bithiatec/opt/ti/simplelink_cc2640r2_sdk_4_10_00_10/source;/home/raffaele-bithiatec/opt/ti/simplelink_cc2640r2_sdk_4_10_00_10/kernel/tirtos/packages;/home/raffaele-bithiatec/opt/ti/simplelink_cc2640r2_sdk_4_10_00_10/source/ti/blestack
override XDCROOT = /home/raffaele-bithiatec/opt/ti/ccs910/xdctools_3_51_03_28_core
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = /home/raffaele-bithiatec/opt/ti/simplelink_cc2640r2_sdk_4_10_00_10/source;/home/raffaele-bithiatec/opt/ti/simplelink_cc2640r2_sdk_4_10_00_10/kernel/tirtos/packages;/home/raffaele-bithiatec/opt/ti/simplelink_cc2640r2_sdk_4_10_00_10/source/ti/blestack;/home/raffaele-bithiatec/opt/ti/ccs910/xdctools_3_51_03_28_core/packages;..
HOSTOS = Linux
endif
