###############################################################################
#                 Root paths, paths for binary, etc..                         #
###############################################################################
BUILD_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SDK_ROOT := $(BUILD_ROOT)/ESTC-NSDK


