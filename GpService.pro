# ----------- Config -----------
TEMPLATE        = lib
QMAKE_CXXFLAGS += -DGP_REFLECTION_STATIC_ADD_TO_MANAGER
QMAKE_CXXFLAGS += -DGP_MODULE_UUID=4556d9d1-0c4b-41e1-a6b9-27a518d896dc
PACKET_NAME     = GpService
DEFINES        += GP_SERVICE_LIBRARY
_VER_MAJ        = 2
_VER_MIN        = 1
_VER_PAT        = 6
DIR_LEVEL       = ./..

include($$DIR_LEVEL/../QtGlobalPro.pri)

equals(var_link, "static") {
	CONFIG += staticlib
}

# ----------- Libraries -----------
equals(var_os, "windows") {
	LIBS += -lGpLogCore$$TARGET_POSTFIX
	LIBS += -lGpJson$$TARGET_POSTFIX
	LIBS += -lGpTasks$$TARGET_POSTFIX
	LIBS += -lGpReflection$$TARGET_POSTFIX
	LIBS += -lGpUtils$$TARGET_POSTFIX

	QMAKE_LFLAGS += /NODEFAULTLIB:libboost_context-vc143-mt-x64-1_86
	QMAKE_LFLAGS += /NODEFAULTLIB:libboost_context-vc143-mt-gd-x64-1_86
	QMAKE_LFLAGS += /NODEFAULTLIB:boost_context-vc143-mt-gd-x64-1_86
	LIBS += -lboost_context-vc143-mt-x64-1_86
}

equals(var_os, "linux") {
	LIBS += -lGpLogCore$$TARGET_POSTFIX
	LIBS += -lGpJson$$TARGET_POSTFIX
	LIBS += -lGpTasks$$TARGET_POSTFIX
	LIBS += -lGpReflection$$TARGET_POSTFIX
	LIBS += -lGpUtils$$TARGET_POSTFIX

	LIBS += -lboost_context
	LIBS += -lfmt
}

# ----------- Sources and headers -----------
SOURCES += \
	ArgParser/GpServiceArgBaseDesc.cpp \
	ArgParser/GpServiceArgBaseDescFactory.cpp \
	ArgParser/GpServiceArgParser.cpp \
	Config/GpServiceCfgBaseDesc.cpp \
	Config/GpServiceCfgBaseDescFactory.cpp \
	Config/GpServiceCfgTaskManagerDesc.cpp \
	GpService.cpp \
	GpServiceLib.cpp \
	GpServiceMainTask.cpp

HEADERS += \
    ArgParser/GpServiceArgBaseDesc.hpp \
    ArgParser/GpServiceArgBaseDescFactory.hpp \
    ArgParser/GpServiceArgParser.hpp \
    Config/GpServiceCfgBaseDesc.hpp \
    Config/GpServiceCfgBaseDescFactory.hpp \
    Config/GpServiceCfgTaskManagerDesc.hpp \
    GpService.hpp \
    GpServiceLib.hpp \
    GpServiceMainTask.hpp \
    GpServiceMainTaskFactory.hpp \
    GpService_global.hpp
