TEMPLATE        = lib
#CONFIG         += staticlib
VER_MAJ		    = 0
VER_MIN		    = 1
VER_PAT		    = 0
QMAKE_CXXFLAGS += -DGP_MODULE_UUID=4556d9d1-0c4b-41e1-a6b9-27a518d896dc
QMAKE_CXXFLAGS += -DGP_REFLECTION_STATIC_ADD_TO_MANAGER
DEFINES		   += GP_SERVICE_LIBRARY
PACKET_NAME     = GpService
DIR_LEVEL       = ./..

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
