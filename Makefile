# SPDX-License-Identifier: GPL-2.0
#/*------------------------------------------------------------------------*/
#/*  Copyright (c) 2015-2018 by Nuvoton Technology Corporation             */
#/*  All Rights Reserved                                                   */
#/*                                                                        */
#/*<<<---------------------------------------------------------------------*/
#/* File Contents:                                                         */
#/*   makefile                                                             */
#/*            This file contains Bingo makefile                           */
#/* Project:                                                               */
#/*            BINGO                                                       */
#/*------------------------------------------------------------------------*/

#----------------------------------------------------------------------------
# tools used in makefile
#----------------------------------------------------------------------------


#----------------------------------------------------------------------------
# Suffixes
#----------------------------------------------------------------------------

.SUFFIXES:
.SUFFIXES: .h .c .cpp .o

#----------------------------------------------------------------------------
# Directories
#----------------------------------------------------------------------------
DELIV_LOC       = ./deliverables/linux
OUT_NAME        = Release
SRC_DIR		= ./src
OUTPUT_DIR      = $(DELIV_LOC)/$(OUT_NAME)


#----------------------------------------------------------------------------
# Files
#----------------------------------------------------------------------------

BINGO_SRC    =    \
                $(SRC_DIR)/pugiXML/pugixml.cpp     \
		$(SRC_DIR)/errors.cpp              \
		$(SRC_DIR)/error_correction.cpp    \
		$(SRC_DIR)/fields.cpp              \
		$(SRC_DIR)/file_maker.cpp          \
		$(SRC_DIR)/main.cpp                \
		$(SRC_DIR)/utilities.cpp

#----------------------------------------------------------------------------
# C compilation flags
#----------------------------------------------------------------------------

CC		= g++
MAKEDIR		= mkdir -p
INCLUDE 	= -I $(SRC_DIR) -I ../src/pugiXML 
TARGET  	= bingo
CFLAGS  	= -std=c++0x -D__LINUX_APP__


bingo:
	@echo Creating \"$(TARGET)\" in directory \"$(OUTPUT_DIR)\" ...
	@$(MAKEDIR)	$(OUTPUT_DIR)
	@echo $(CC) $(CFLAGS) $(INCLUDE) $(BINGO_SRC) -o $(OUTPUT_DIR)/$(TARGET)
	@$(CC) $(CFLAGS) $(INCLUDE) $(BINGO_SRC) -o $(OUTPUT_DIR)/$(TARGET)
	
all:
	@echo Creating \"$(TARGET)\" in directory \"$(OUTPUT_DIR)\" ...
	@$(MAKEDIR)	$(OUTPUT_DIR)
	@echo $(CC) $(CFLAGS) $(INCLUDE) $(BINGO_SRC) -o $(OUTPUT_DIR)/$(TARGET)
	@$(CC) $(CFLAGS) $(INCLUDE) $(BINGO_SRC) -o $(OUTPUT_DIR)/$(TARGET)


#----------------------------------------------------------------------------
# Clean
#----------------------------------------------------------------------------
RMDIR		= rm
RMFLAGS		= -rf

clean:
	@echo Removing Directory \"$(OUTPUT_DIR)\" ...
	@$(RMDIR) $(RMFLAGS) $(OUTPUT_DIR)
	


