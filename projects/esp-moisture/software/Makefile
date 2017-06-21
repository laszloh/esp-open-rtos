PROGRAM=moist-esp

EXTRA_COMPONENTS = extras/paho_mqtt_c extras/httpd

XTRA_CFLAGS=-DLWIP_HTTPD_CGI=1 -DLWIP_HTTPD_SSI=1 -I./fsdata

include ../../common.mk

html:
	@echo "Generating fsdata.."
	cd fsdata && ./makefsdata
