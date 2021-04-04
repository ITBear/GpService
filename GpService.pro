TEMPLATE        = lib
#CONFIG         += staticlib
VER_MAJ		    = 0
VER_MIN		    = 1
VER_PAT		    = 0
QMAKE_CXXFLAGS += -DGP_MODULE_UUID=4556d9d1-0c4b-41e1-a6b9-27a518d896dc
QMAKE_CXXFLAGS += -DGP_TYPE_SYSTEM_STATIC_ADD_TO_MANAGER
DEFINES		   += GPSERVICE_LIBRARY
PACKET_NAME     = GpService
DIR_LEVEL       = ./..

include(../../QtGlobalPro.pri)

#------------------------------ LIBS BEGIN ---------------------------------
os_windows{
	GP_CORE_LIB_V		= 2
	GP_JSON_LIB_V		= 0
	BOOST_LIB_POSTFIX	= -mgw82-mt-x64-1_75
}

os_linux{
}

LIBS += -lGpCore2$$TARGET_POSTFIX$$GP_CORE_LIB_V
LIBS += -lGpJson$$TARGET_POSTFIX$$GP_JSON_LIB_V
LIBS += -lboost_program_options
#------------------------------ LIBS END ---------------------------------

SOURCES += \
	ArgParser/GpArgBaseDesc.cpp \
	ArgParser/GpArgParser.cpp \
	GpService.cpp \
	GpServiceCfgBaseDesc.cpp \
	GpServiceCfgTaskManagerDesc.cpp \
	GpServiceMainTask.cpp

HEADERS += \
    ArgParser/GpArgBaseDesc.hpp \
    ArgParser/GpArgParser.hpp \
    GpService.hpp \
    GpServiceCfgBaseDesc.hpp \
    GpServiceCfgTaskManagerDesc.hpp \
    GpServiceFactory.hpp \
    GpServiceMainTask.hpp \
    GpService_global.hpp
