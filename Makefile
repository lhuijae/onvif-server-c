
# make -DTARGET_ARM7 to compile for ARM

DAEMON_NAME           = onvif_srvd
DAEMON_MAJOR_VERSION  = 1
DAEMON_MINOR_VERSION  = 1
DAEMON_PATCH_VERSION  = 0
DAEMON_PID_FILE_NAME  = $(DAEMON_NAME).pid
DAEMON_LOG_FILE_NAME  = $(DAEMON_NAME).log
DAEMON_NO_CHDIR       = 1
DAEMON_NO_CLOSE_STDIO = 0

GSOAP_DIR         = ./gsoap-2.8/gsoap
GSOAP_CUSTOM_DIR  = $(GSOAP_DIR)/custom
GSOAP_PLUGIN_DIR  = $(GSOAP_DIR)/plugin
GSOAP_IMPORT_DIR  = $(GSOAP_DIR)/import

SOAPCPP2          = $(GSOAP_DIR)/src/soapcpp2
WSDL2H            = $(GSOAP_DIR)/wsdl/wsdl2h

ifdef TARGET_ARM7
	SOAPCPP2 = /home/flemieux/gsoap-2.8/gsoap/src/soapcpp2
	WSDL2H = /home/flemieux/gsoap-2.8/gsoap/wsdl/wsdl2h
endif

COMMON_DIR        = ./src
GENERATED_DIR     = ./generated

CFLAGS            = -DDAEMON_NAME='"$(DAEMON_NAME)"'
CFLAGS           += -DDAEMON_MAJOR_VERSION=$(DAEMON_MAJOR_VERSION)
CFLAGS           += -DDAEMON_MINOR_VERSION=$(DAEMON_MINOR_VERSION)
CFLAGS           += -DDAEMON_PATCH_VERSION=$(DAEMON_PATCH_VERSION)
CFLAGS           += -DDAEMON_PID_FILE_NAME='"$(DAEMON_PID_FILE_NAME)"'
CFLAGS           += -DDAEMON_LOG_FILE_NAME='"$(DAEMON_LOG_FILE_NAME)"'
CFLAGS           += -DDAEMON_NO_CHDIR=$(DAEMON_NO_CHDIR)
CFLAGS           += -DDAEMON_NO_CLOSE_STDIO=$(DAEMON_NO_CLOSE_STDIO)

CFLAGS           += -I$(COMMON_DIR)
CFLAGS           += -I$(COMMON_DIR)/toolbox
CFLAGS           += -I$(GENERATED_DIR)
CFLAGS           += -I$(MAKITO_INCLUDE)
CFLAGS           += -I$(GSOAP_DIR) -I$(GSOAP_CUSTOM_DIR) -I$(GSOAP_PLUGIN_DIR) -I$(GSOAP_IMPORT_DIR)
CFLAGS           +=  -Wall -pipe -Wfatal-errors -pthread

GCC              ?=  gcc
GSOAP_CONFIGURE  ?= 

ifdef TARGET_ARM7
	GCC       ?=  arm-none-linux-gnueabi-gcc
	GSOAP_CONFIGURE ?= --prefix=/usr --program-prefix= --target=arm-none-linux-gnueabi --build=i686-pc-linux-gnu56 --host=arm-none-linux-gnueabi --with-gnu-ld --disable-lfs --with-openssl --with-zlib
	MAKITO_INCLUDE = /home/flemieux/makito2_enc_2.2.1/components/include/
else
	MAKITO_INCLUDE = $(COMMON_DIR)/sessmgr_alt/
endif

CFLAGS += -I$(MAKITO_INCLUDE)



ifdef TARGET_ARM7
	
	#LDFLAGS=" $LDFLAGS -L/usr/local/haivision/makito2fs/usr/lib"
	CFLAGS  += -L/home/flemieux/makito2_enc_2.2.1/build/OSCAR/output/libs
	CFLAGS  += -lmxsessmgr -lmxtools -lmxcfgfile -lcrypt -lhailogin -lmxclock -lmxfpga -lrt

endif

# To build a daemon with WS-Security support, 
# call make with the WSSE_ON=1 parameter
# example:
# make WSSE_ON=1 all
ifdef WSSE_ON
CFLAGS       += -DWITH_OPENSSL -lssl -lcrypto -lz

WSSE_SOURCES  = $(GSOAP_PLUGIN_DIR)/wsseapi.c \
                $(GSOAP_PLUGIN_DIR)/mecevp.c  \
                $(GSOAP_PLUGIN_DIR)/smdevp.c  \
                $(GSOAP_PLUGIN_DIR)/wsaapi.c

WSSE_IMPORT   = echo '\#import "wsse.h" ' >> $@
endif



SOAP_SRC = $(GSOAP_DIR)/stdsoap2.cpp        \
           $(GSOAP_DIR)/dom.cpp             \
           $(GSOAP_CUSTOM_DIR)/duration.c


# We can't use wildcard func, this files will be generated
SOAP_SERVICE_SRC = $(GENERATED_DIR)/soapDeviceBindingService.cpp \
                   $(GENERATED_DIR)/soapMediaBindingService.cpp


# Add your source files to the list.
# Supported *.c  *.cpp  *.S files.
# For other file types write a template rule for build, see below.
SOURCES  = $(COMMON_DIR)/$(DAEMON_NAME).c         \
           $(COMMON_DIR)/daemon.c                 \
           $(COMMON_DIR)/eth_dev_param.c          \
           $(COMMON_DIR)/ServiceContext.c         \
           $(COMMON_DIR)/ServiceDevice.c          \
           $(COMMON_DIR)/ServiceMedia.c           \
           $(COMMON_DIR)/hai_soap.c               \
           $(COMMON_DIR)/toolbox/toolbox-linkedlist.c     \
           $(COMMON_DIR)/toolbox/toolbox-char-array.c     \
           $(COMMON_DIR)/toolbox/toolbox-line-parser.c    \
           $(COMMON_DIR)/toolbox/config_ini_manager.c    \
           $(COMMON_DIR)/toolbox/toolbox-text-buffer-reader.c    \
           $(COMMON_DIR)/toolbox/toolbox-text-file-reader.c    \
           $(COMMON_DIR)/toolbox/toolbox-flexstring.c        \
           $(COMMON_DIR)/toolbox/toolbox-mutex.c        \
           $(COMMON_DIR)/toolbox/toolbox-network.c        \
           $(COMMON_DIR)/toolbox/toolbox.c        \
           $(GENERATED_DIR)/soapC.c               \
           $(GENERATED_DIR)/soapServer.c          \
           $(SOAP_SRC)                            \
           $(WSSE_SOURCES)





OBJECTS  := $(patsubst %.c,  %.o, $(SOURCES) )
OBJECTS  := $(patsubst %.cpp,%.o, $(OBJECTS) )
OBJECTS  := $(patsubst %.S,  %.o, $(OBJECTS) )


DEBUG_SUFFIX   = debug

DEBUG_OBJECTS := $(patsubst %.o, %_$(DEBUG_SUFFIX).o, $(OBJECTS) )



WSDL_FILES = $(wildcard wsdl/*.wsdl wsdl/*.xsd)



.PHONY: all
all: debug release



.PHONY: release
release: CFLAGS := -s  $(CFLAGS)
release: $(DAEMON_NAME)



.PHONY: debug
debug: DAEMON_NO_CLOSE_STDIO = 1
debug: CFLAGS := -DDEBUG  -g  $(CFLAGS)
debug: $(DAEMON_NAME)_$(DEBUG_SUFFIX)



# release
$(DAEMON_NAME): .depend $(OBJECTS)
	$(call build_bin, $(OBJECTS))


# debug
$(DAEMON_NAME)_$(DEBUG_SUFFIX): .depend $(DEBUG_OBJECTS)
	$(call build_bin, $(DEBUG_OBJECTS))



# Build release objects
%.o: %.c
	$(build_object)


%.o: %.cpp
	$(build_object)


%.o: %.S
	$(build_object)



# Build debug objects
%_$(DEBUG_SUFFIX).o: %.c
	$(build_object)


%_$(DEBUG_SUFFIX).o: %.cpp
	$(build_object)


%_$(DEBUG_SUFFIX).o: %.S
	$(build_object)



.PHONY: clean
clean:
	-@rm -f $(DAEMON_NAME)
	-@rm -f $(DAEMON_NAME)_$(DEBUG_SUFFIX)
	-@rm -f $(OBJECTS)
	-@rm -f $(DEBUG_OBJECTS)
	-@rm -f .depend
	-@rm -f -d -R $(GENERATED_DIR)
	-@rm -f *.*~



.PHONY: distclean
distclean: clean
	-@rm -f -d -R SDK
	-@rm -f -d -R gsoap-2.8
	-@rm -f RECV.log SENT.log TEST.log




.depend: $(GENERATED_DIR)/soapC.c
	-@rm -f .depend
	@echo "Generating dependencies..."
	@for src in $(SOURCES) ; do \
        echo "  [depend]  $$src" ; \
        $(GCC) $(CFLAGS) -MT ".depend $${src%.*}.o $${src%.*}_$(DEBUG_SUFFIX).o" -MM $$src >> .depend ; \
    done



ifeq "$(findstring $(MAKECMDGOALS),clean distclean)"  ""
    include $(wildcard .depend)
endif





# ---- gSOAP ----

$(GENERATED_DIR)/onvif.h:
	@$(build_gsoap)
	@mkdir -p $(GENERATED_DIR)
	$(WSDL2H) -d -c -t ./wsdl/typemap.dat  -o $@  $(WSDL_FILES)
	$(WSSE_IMPORT)


# this will also generate soapServer.c
$(GENERATED_DIR)/soapC.c: $(GENERATED_DIR)/onvif.h
	$(SOAPCPP2) -c -L -x -S -d $(GENERATED_DIR) -I$(GSOAP_DIR):$(GSOAP_IMPORT_DIR) $<



# This targets is needed for parallel work of make
$(OBJECTS) $(DEBUG_OBJECTS) $(SOAP_SRC) $(WSSE_SOURCES): $(GENERATED_DIR)/soapC.c




# Common commands
BUILD_ECHO = echo "\n  [build]  $@:"


define build_object
    @$(BUILD_ECHO)
    $(GCC) -c $< -o $@  $(CFLAGS)
endef



define build_bin
    @$(BUILD_ECHO)
    $(GCC)  $1 -o $@  $(CFLAGS)
    @echo "\n---- Compiled $@ ver $(DAEMON_MAJOR_VERSION).$(DAEMON_MINOR_VERSION).$(DAEMON_PATCH_VERSION) ----\n"
endef



define build_gsoap

    # get archive
    if [ ! -f SDK/gsoap.zip ]; then \
        mkdir -p SDK; \
        wget -O ./SDK/gsoap.zip.tmp "https://versaweb.dl.sourceforge.net/project/gsoap2/gsoap-2.8/gsoap_2.8.77.zip" && \
        mv ./SDK/gsoap.zip.tmp ./SDK/gsoap.zip; \
    fi

    # unzip
    if [ ! -f gsoap-2.8/README.txt ]; then \
         unzip ./SDK/gsoap.zip; \
    fi

    # build
    if [ ! -f $(SOAPCPP2) ] || [ ! -f $(WSDL2H) ]; then \
         cd gsoap-2.8; \
         ./configure $(GSOAP_CONFIGURE) && \
         make -j1; \
         cd ..;\
    fi
endef




.PHONY: help
help:
	@echo "make [command]"
	@echo "command is:"
	@echo "   all       -  build daemon in release and debug mode"
	@echo "   debug     -  build in debug mode (#define DEBUG 1)"
	@echo "   release   -  build in release mode (strip)"
	@echo "   clean     -  remove all generated files"
	@echo "   distclean -  clean + remove all SDK files"
	@echo "   help      -  this help"

