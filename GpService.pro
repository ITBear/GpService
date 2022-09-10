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
	ArgParser/GpArgBaseDesc.cpp \
	ArgParser/GpArgParser.cpp \
	GpService.cpp \
	GpServiceCfgBaseDesc.cpp \
	GpServiceCfgTaskManagerDesc.cpp \
	GpServiceMainTask.cpp \
	GpServiceStartFlag.cpp

HEADERS += \
    ArgParser/GpArgBaseDesc.hpp \
    ArgParser/GpArgParser.hpp \
    GpService.hpp \
    GpServiceCfgBaseDesc.hpp \
    GpServiceCfgTaskManagerDesc.hpp \
    GpServiceFactory.hpp \
    GpServiceMainTask.hpp \
    GpServiceStartFlag.hpp \
    GpService_global.hpp
