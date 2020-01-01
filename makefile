#-------------------------------------------------------------------------------
# $RCSfile: makefile,v $
# $Revision: 1.9 $
#-------------------------------------------------------------------------------
# README:
# This is the project specific makefile.
# It imports all common make variables from makefile.defines and redefines some
# of them if necessary. Most importantly it builds all project specific targets.
#
#-------------------------------------------------------------------------------
include make/makefile.defines
#
#-------------------------------------------------------------------------------
# Project specific definitions that differ from default in makefile.defines .
ADMIN_FILES      = notes.txt tbd.txt ideas.txt
BACKUP_NETWORK   = /mnt/hgfs/Seagate_500/Backup/${PROJECT}
CC               = g++
PROJECT          = Condor
PROJECTs         = condor
#
#-------------------------------------------------------------------------------
# Additional compile flags and directives needed here.
#
# For profiling set PROF, recompile, execute ${TARGET} then do: gprof ${TARGET}
#PROF       = -pg
DEF        =  NoDef
FLAGS      = -D${DEF} -D${PROJECT} -D${PLATFORM} ${PROF}
MAIN_FLAGS = -DRELEASE=${RELEASE} -DPATCHES='${PATCHES}'
#
#-------------------------------------------------------------------------------
# Include and Libraries:
INCLUDE   = -I${INC} -I${SRC}
LIBS      =
#
#-------------------------------------------------------------------------------
# Object List:
OBJS = \
	$(OBJ)/Attribute.o \
	$(OBJ)/Aggregate.o \
	$(OBJ)/Bit.o       \
	$(OBJ)/Condition.o \
	$(OBJ)/Clock.o     \
	$(OBJ)/Database.o  \
	$(OBJ)/Event.o     \
	$(OBJ)/EventFromTrigger.o      \
	$(OBJ)/DBTables.o  \
	$(OBJ)/DBTransaction.o         \
	$(OBJ)/DBTransactionRec.o      \
	$(OBJ)/DBTransactionRecValue.o \
	$(OBJ)/Enum.o      \
	$(OBJ)/Interface.o \
	$(OBJ)/InterfaceDesc_AXI.o \
	$(OBJ)/InterfaceDesc_Clock.o \
	$(OBJ)/InterfaceDesc_RW.o \
	$(OBJ)/MailboxBase.o\
	$(OBJ)/Module.o\
	$(OBJ)/ModuleTop.o \
	$(OBJ)/PRNG.o      \
	$(OBJ)/Process.o   \
	$(OBJ)/ProcessExecuteReason.o  \
	$(OBJ)/Process_AXI_Master.o    \
	$(OBJ)/Process_RW.o            \
	$(OBJ)/Query.o     \
	$(OBJ)/Record.o    \
	$(OBJ)/Scheduler.o \
	$(OBJ)/Semaphore.o \
	$(OBJ)/Signal.o    \
	$(OBJ)/SimulationUtilities.o \
	$(OBJ)/Simulator.o \
	$(OBJ)/SimulatorTime.o \
	$(OBJ)/String.o    \
	$(OBJ)/Table.o     \
	$(OBJ)/TableRecord.o           \
	$(OBJ)/Timer.o     \
	$(OBJ)/Trigger.o   \
	$(OBJ)/Value.o     \
	$(OBJ)/Vcd.o       \
	$(OBJ)/ViewJoin.o  \
	$(OBJ)/View.o

#	$(OBJ)/Utilities.o \

#
#-------------------------------------------------------------------------------
# Convenience targets. First is ${TARGET} which 'make' will build by default.
${TARGET}: ${BIN}/${TARGET}
#
#-------------------------------------------------------------------------------
# Make Executable.
${BIN}/${TARGET}: ${OBJS} ${TARGET_SRC}/${TARGET}.cc ${SRC}/MemoryUsage.cc
	$(CC) $(DBG) ${FLAGS} ${MAIN_FLAGS} -o ${BIN}/${TARGET} ${TARGET_SRC}/${TARGET}.cc \
	${OBJS} $(INCLUDE) $(LIBS)
#
#-------------------------------------------------------------------------------
# Other useful targets:
#
# Make objects up to date, do not create executable:
objs: ${OBJS}
#
# Build a library useful for other projects import this projects functionality.
Lib: ${BIN}/libvr.a
#
${BIN}/libvr.a: ${OBJS}
	ar -rc ${BIN}/libvr.a ${OBJS}
	ranlib ${BIN}/libvr.a
#
# Recreate the dependencies:
depend:
	@ echo "Creating dependencies file ${MAKE_DIR}/${MAKEFILE_DEPEND}";
	@ \rm -f ${MAKE_DIR}/${MAKEFILE_DEPEND}
	@ echo "# This file is created automatically with: make depend" > ${MAKE_DIR}/${MAKEFILE_DEPEND}
	@ for file in `ls ${SRC}/*.cc`; \
    do \
      file_base=`echo $$file | sed -e "s/^.*\///" | sed -e "s/\..*//"`; \
      echo gcc ${INCLUDE} -MT ${OBJ}/$$file_base.o -MM $$file; \
      gcc ${INCLUDE} -MT ${OBJ}/$$file_base.o -MM $$file >> ${MAKE_DIR}/${MAKEFILE_DEPEND}; \
    done
	@ echo "Created ${MAKE_DIR}/${MAKEFILE_DEPEND}"
#
#-------------------------------------------------------------------------------
# Rules for creating the .o targets.
#
$(OBJS): $(OBJ)/%.o: $(SRC)/%.cc
	@echo "$< $@"
	$(CC) $(DBG) ${FLAGS} -c $< -o $@ $(INCLUDE)
#
#-------------------------------------------------------------------------------
.PHONY: ${TARGET}
#-------------------------------------------------------------------------------
include ${MAKE_DIR}/${MAKEFILE_DEPEND}
include ${MAKE_DIR}/${MAKEFILE_TOOLS}
include tests/makefile.tests
#
#-------------------------------------------------------------------------------
# End
