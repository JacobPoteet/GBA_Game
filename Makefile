#---------------------------------------------------------------------------------------------------------------------
# GBA_Game - Butano/devkitARM makefile.
#
# See third_party/butano/template/Makefile for the full list of supported variables.
#
# Build the ROM:      make -j$(nproc)
# Clean:              make clean
#---------------------------------------------------------------------------------------------------------------------

#---------------------------------------------------------------------------------------------------------------------
# Python interpreter used by Butano's asset pipeline.
# devkitPro's MSYS2 shell ships "python", most Linux images only ship "python3", so detect rather than hardcode.
# Override explicitly with: make PYTHON=python
#---------------------------------------------------------------------------------------------------------------------
ifndef PYTHON
	PYTHON	:=	$(shell command -v python3 >/dev/null 2>&1 && echo python3 || echo python)
endif

TARGET      	:=  gba_game
BUILD       	:=  build
LIBBUTANO   	:=  third_party/butano/butano
SOURCES     	:=  src
INCLUDES    	:=  include third_party/butano/common/include
DATA        	:=
GRAPHICS    	:=  graphics third_party/butano/common/graphics
AUDIO       	:=  audio
AUDIOBACKEND	:=  maxmod
AUDIOTOOL   	:=
DMGAUDIO    	:=
DMGAUDIOBACKEND	:=  default
ROMTITLE    	:=  GBA PUZZLE
ROMCODE     	:=  PZLE
USERFLAGS   	:=
USERCXXFLAGS	:=
USERASFLAGS 	:=
USERLDFLAGS 	:=
USERLIBDIRS 	:=
USERLIBS    	:=
DEFAULTLIBS 	:=
STACKTRACE  	:=
USERBUILD   	:=
EXTTOOL     	:=

#---------------------------------------------------------------------------------------------------------------------
# Export absolute butano path:
#---------------------------------------------------------------------------------------------------------------------
ifndef LIBBUTANOABS
	export LIBBUTANOABS	:=	$(realpath $(LIBBUTANO))
endif

#---------------------------------------------------------------------------------------------------------------------
# Include main makefile:
#---------------------------------------------------------------------------------------------------------------------
include $(LIBBUTANOABS)/butano.mak
