TEMPLATE        = lib
#CONFIG         += staticlib
VER_MAJ		    = 2
VER_MIN		    = 1
VER_PAT		    = 4
QMAKE_CXXFLAGS += -DGP_MODULE_UUID=4556d9d1-0c4b-41e1-a6b9-27a518d896dc
QMAKE_CXXFLAGS += -DGP_REFLECTION_STATIC_ADD_TO_MANAGER
PACKET_NAME     = GpService
DIR_LEVEL       = ./..

DEFINES		   += GP_SERVICE_LIBRARY
DEFINES        += "GP_CURRENT_LIB_VER_MAJ=\\\"$$VER_MAJ\\\""
DEFINES        += "GP_CURRENT_LIB_VER_MIN=\\\"$$VER_MIN\\\""
DEFINES        += "GP_CURRENT_LIB_VER_PAT=\\\"$$VER_PAT\\\""
DEFINES        += "GP_CURRENT_LIB_PACKET_NAME=\\\"$$PACKET_NAME\\\""

include(../../QtGlobalPro.pri)

#------------------------------ LIBS BEGIN ---------------------------------
os_windows{
}

os_linux{
}
#------------------------------- LIBS END ----------------------------------

SOURCES += \
	ArgParser/GpServiceArgBaseDesc.cpp \
	ArgParser/GpServiceArgBaseDescFactory.cpp \
	ArgParser/GpServiceArgParser.cpp \
	Config/GpServiceCfgBaseDesc.cpp \
	Config/GpServiceCfgBaseDescFactory.cpp \
	Config/GpServiceCfgTaskManagerDesc.cpp \
	GpService.cpp \
	GpServiceMainTask.cpp

HEADERS += \
    ArgParser/GpServiceArgBaseDesc.hpp \
    ArgParser/GpServiceArgBaseDescFactory.hpp \
    ArgParser/GpServiceArgParser.hpp \
    Config/GpServiceCfgBaseDesc.hpp \
    Config/GpServiceCfgBaseDescFactory.hpp \
    Config/GpServiceCfgTaskManagerDesc.hpp \
    GpService.hpp \
    GpServiceMainTask.hpp \
    GpServiceMainTaskFactory.hpp \
    GpService_global.hpp
