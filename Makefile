#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	$(shell basename $(CURDIR))
export TOPDIR		:=	$(CURDIR)


.PHONY: arm7/$(TARGET).elf arm9/$(TARGET).elf

#---------------------------------------------------------------------------------
TEXT1 		:= Attorney Online DS
TEXT2 		:= Headshotnoby
#TEXT3 		:= 
ICON 		:= icon.bmp
NITRODIR	:= -d nitrofiles
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: $(TARGET).nds

#---------------------------------------------------------------------------------
$(TARGET).nds	:	arm7/$(TARGET).elf arm9/$(TARGET).elf
	@echo Compiling ARM7 and ARM9
	ndstool -c $(TARGET).nds -7 arm7/$(TARGET).elf -9 arm9/$(TARGET).elf -b $(ICON) "$(TEXT1);$(TEXT2)" $(NITRODIR)

#---------------------------------------------------------------------------------
arm7/$(TARGET).elf:
	@echo Compiling ARM7
	@$(MAKE) -C arm7

#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	@echo Compiling ARM9
	@$(MAKE) -C arm9

#---------------------------------------------------------------------------------
cleanall: clean
clean:
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C arm7 clean
	@rm -f $(TARGET).nds
