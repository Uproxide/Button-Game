#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC)
endif

include $(DEVKITPPC)/wii_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	ButtonGame
BUILD		:=	build
SOURCES		:=	source
DATA		:=	data  
INCLUDES	:=	include -IC:/wiihbthings/cJSON

#---------------------------------------------------------------------------------
# options stuff
#---------------------------------------------------------------------------------
PKGFOLDER = dev # options are dev, or release.
RELEASE = 0 # options are 0 or 1
#---------------------------------------------------------------------------------
# version stuff
#---------------------------------------------------------------------------------
READXML := read_xml () { local IFS=\> ; read -d \< ENTITY CONTENT ; } ;
VERSION := $(shell $(READXML) meta=$(TARGET)/meta.xml; if [ "$(BUILD)" == "$(notdir $(CURDIR))" ]; then meta="../$${meta}"; fi; while read_xml; do if [[ $$ENTITY == "version" ]]; then echo $${CONTENT//[$$'\t\r\n ']}; break; fi; done < $$meta)
VERSION_MAJOR := $(shell echo $(VERSION) | cut -d. -f1)
VERSION_MINOR := $(shell echo $(VERSION) | cut -d. -f2)
VERSION_PATCH := $(shell echo $(VERSION) | cut -d. -f3)
VERSION_PRE := $(shell echo $(VERSION) | cut -d- -f2 | cut -d. -f1 | cut -d+ -f1)
VERSION_BUILD := $(shell echo $(VERSION) | cut -d+ -f2 | cut -d. -f1 )
VERSION_PRERELEASE := $(shell echo $(VERSION) | cut -d- -f2 | cut -d. -f2 | cut -d+ -f1)
VERSION_BUILDNUM := $(shell echo $(VERSION) | cut -d+ -f2 | cut -d. -f2)

# Print the version components
# @echo "Version: $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)"
# @echo "Pre-release: $(VERSION_PRE)"
# @echo "Build: $(VERSION_BUILD)"
# @echo "Pre-release number: $(VERSION_PRERELEASE)"
# @echo "Build number: $(VERSION_BUILDNUM)"
#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

CFLAGS	= -g -O2 -Wall $(MACHDEP) $(INCLUDE)
CXXFLAGS	=	$(CFLAGS)

LDFLAGS	=	-g $(MACHDEP) -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# options stuff for cflags/cxxflags
#---------------------------------------------------------------------------------
ifneq ($(RELEASE),1)
	CFLAGS += -DDEV=1
	CXXFLAGS += -DDEV=1
endif

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS    :=    -lwiiuse -lfat -lbte -logc -lm 

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES_BIN     :=	$(addsuffix .o,$(BINFILES))
export OFILES_SOURCES := 	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(sFILES:.s=.o) $(SFILES:.S=.o)
export OFILES		  :=	$(OFILES_BIN) $(OFILES_SOURCES)

export HFILES := $(addsuffix .h,$(subst .,_,$(BINFILES)))

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBOGC_INC) \
                    -I$(PORTLIBS_PATH)/wii/include \
                    -I$(PORTLIBS_PATH)/ppc/include

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					-L$(LIBOGC_LIB) \
                    -L$(PORTLIBS_PATH)/wii/lib -L$(PORTLIBS_PATH)/ppc/lib

export OUTPUT	:=	$(CURDIR)/$(TARGET)

#---------------------------------------------------------------------------------
# scripts for targets
#---------------------------------------------------------------------------------
copydol2pkgfolder = (rm -fr $(OUTPUT)/boot.dol; cp $(OUTPUT).dol $(OUTPUT)/boot.dol)
#---------------------------------------------------------------------------------
# targets
#---------------------------------------------------------------------------------

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@echo "Version: $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)"
	@echo "Pre-release: $(VERSION_PRE)"
	@echo "Build: $(VERSION_BUILD)"
	@echo "Pre-release number: $(VERSION_PRERELEASE)"
	@echo "Build number: $(VERSION_BUILDNUM)"
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo cleaning build files ...
	@rm -fr $(BUILD)
	@echo cleaning dol and elf files in root project directory ...
	@rm -fr $(OUTPUT).elf $(OUTPUT).dol
	@echo cleaning dol and elf files in package directory ...
	@rm -fr $(OUTPUT)/boot.elf $(OUTPUT)/boot.dol $(OUTPUT)/$(TARGET).elf $(OUTPUT)/$(TARGET).dol apps appsgohere
	@echo cleaning packaged zip ...
	@rm -fr $(OUTPUT)*.zip
	@echo cleaning done!

#---------------------------------------------------------------------------------
run:
	wiiload $(TARGET).dol

#---------------------------------------------------------------------------------
runpkg:
	@if [ ! -f $(OUTPUT).zip ]; then $(MAKE) package; fi; wiiload $(OUTPUT).zip;

#---------------------------------------------------------------------------------
package:
	@$(call copydol2pkgfolder)
	@echo zipping up the package...
	@zip -r $(TARGET).zip $(TARGET)/*

#---------------------------------------------------------------------------------
pkgreleasefolder: # mainly for release stuffs
	@$(call copydol2pkgfolder)
	@echo pkg folder is $(PKGFOLDER)
	@if [[ "$(PKGFOLDER)" == "release" ]]; then mkdir -p appsgohere; fi
	@if [[ "$(PKGFOLDER)" == "release" ]]; then mkdir -p appsgohere/apps; else mkdir -p apps; fi
	@if [[ "$(PKGFOLDER)" == "release" ]]; then cp -r $(OUTPUT) appsgohere/apps/$(TARGET); else cp -r $(OUTPUT) apps/$(TARGET); fi
	@echo zipping up the package...
	@if [[ "$(PKGFOLDER)" == "release" ]]; then cd appsgohere; zip -r "../$(TARGET) - with apps folder.zip" apps/*; cd ..; else zip -r "$(TARGET) - with apps folder.zip" apps/*; fi
	@if [[ "$(PKGFOLDER)" != "release" ]]; then rm -fr apps; fi

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)


$(OFILES_SOURCES) : $(HFILES)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .bin extension
#---------------------------------------------------------------------------------
%.bin.o %_bin.h  :	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .mod extension
#---------------------------------------------------------------------------------
%.mod.o %_mod.h	 :	%.mod
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .mp3 extension
#---------------------------------------------------------------------------------
%.mp3.o	%_mp3.h  :	%.mp3
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .ogg extension
#---------------------------------------------------------------------------------
%.ogg.o	%_ogg.h  :	%.ogg
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .jpg extension
#---------------------------------------------------------------------------------
%.jpg.o	%_jpg.h  :	%.jpg
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .pee en gee extension
#---------------------------------------------------------------------------------
%.png.o	%_png.h  :	%.png
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)


-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
