
ifeq ($(OS),Windows_NT)
BLOCKSDS            ?= C:/msys64/opt/wonderful/thirdparty/blocksds/core
BLOCKSDSEXT         ?= C:/msys64/opt/wonderful/thirdparty/blocksds/external
WONDERFUL_TOOLCHAIN ?= C:/msys64/opt/wonderful
else
BLOCKSDS            ?= /opt/blocksds/core
endif

# User config

NAME            := ao-nds
GAME_TITLE      := Attorney Online DS
GAME_AUTHOR     := Headshotnoby
#GAME_SUBTITLE   :=
GAME_ICON       := icon.png
#NITRODIR        := -d nitrofiles

include $(BLOCKSDS)/sys/default_makefiles/rom_arm9arm7/Makefile
