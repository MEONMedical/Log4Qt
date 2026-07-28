set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(log4qt_SOURCES
    abstractstringlayout.cpp
    appender.cpp
    appenderskeleton.cpp
    asyncappender.cpp
    basicconfigurator.cpp
    consoleappender.cpp
    dailyrollingfileappender.cpp
    fileappender.cpp
    helpers/appenderattachable.cpp
    helpers/classlogger.cpp
    helpers/configuratorhelper.cpp
    helpers/cronexpression.cpp
    helpers/datetime.cpp
    helpers/asyncworker.cpp

    helpers/factory.cpp
    helpers/initialisationhelper.cpp
    helpers/logerror.cpp
    helpers/optionconverter.cpp
    helpers/patternformatter.cpp
    helpers/properties.cpp
    hierarchy.cpp
    jsonconfigurator.cpp
    jsonlayout.cpp
    abstractlayout.cpp
    level.cpp
    logger.cpp
    loggerrepository.cpp
    loggingevent.cpp                                                                                                                                                  
    logmanager.cpp                                                                                                                                                    
    logstream.cpp                                                                                                                                                     
    mainthreadappender.cpp                                                                                                                                            
    mdc.cpp                                                                                                                                                           
    ndc.cpp                                                                                                                                                           
    patternlayout.cpp                                                                                                                                                 
    propertyconfigurator.cpp                                                                                                                                          
    randomaccessfileappender.cpp
    rollingfileappender.cpp                                              
    signalappender.cpp                                                                                                                                                
    simplelayout.cpp                                                                                                                                                  
    simpletimelayout.cpp                                                                                                                                                                                                                       
    spi/compositetriggeringpolicy.cpp
    spi/crontriggeringpolicy.cpp
    spi/headerfooterprovider.cpp
    spi/daterolloverstrategy.cpp
    spi/defaultrolloverstrategy.cpp
    spi/filter.cpp
    spi/onstartuptriggeringpolicy.cpp
    spi/rolloverstrategy.cpp
    spi/sizebasedtriggeringpolicy.cpp
    spi/timebasedtriggeringpolicy.cpp
    spi/triggeringpolicy.cpp                                                                                                                                                                                                                             
    systemlogappender.cpp                                                                                                                                                                                                                      
    ttcclayout.cpp                                                                                                                                                                                                                             
    varia/debugappender.cpp                                                                                                                                                                                                                    
    varia/denyallfilter.cpp                                                                                                                                                                                                                    
    varia/levelmatchfilter.cpp                                                                                                                                                                                                                 
    varia/levelrangefilter.cpp                                                                                                                                                                                                                 
    varia/listappender.cpp                                                                                                                                                                                                                     
    varia/nullappender.cpp
    varia/stringmatchfilter.cpp
    writerappender.cpp
    xmlconfigurator.cpp
    xmllayout.cpp
)
set(log4qt_HEADERS
    abstractstringlayout.h
    appender.h
    appenderskeleton.h
    asyncappender.h
    basicconfigurator.h
    consoleappender.h
    dailyrollingfileappender.h
    fileappender.h
    hierarchy.h
    jsonconfigurator.h
    jsonlayout.h
    abstractlayout.h
    level.h
    log4qtdefs.h
    log4qt.h
    log4qtshared.h
    log4qtsharedptr.h
    logger.h
    loggerrepository.h
    loggingevent.h
    logmanager.h
    logstream.h
    mainthreadappender.h
    mdc.h
    ndc.h
    patternlayout.h
    propertyconfigurator.h
    randomaccessfileappender.h
    rollingfileappender.h
    signalappender.h
    simplelayout.h
    simpletimelayout.h
    systemlogappender.h
    ttcclayout.h
    writerappender.h
    xmlconfigurator.h
    xmllayout.h
)
set(log4qt_HEADERS_helpers
    helpers/appenderattachable.h
    helpers/asyncworker.h
    helpers/boundedblockingqueue.h
    helpers/classlogger.h
    helpers/configuratorhelper.h
    helpers/cronexpression.h
    helpers/datetime.h

    helpers/factory.h
    helpers/initialisationhelper.h
    helpers/logerror.h
    helpers/optionconverter.h
    helpers/patternformatter.h
    helpers/properties.h
)
set(log4qt_HEADERS_spi
    spi/compositetriggeringpolicy.h
    spi/crontriggeringpolicy.h
    spi/headerfooterprovider.h
    spi/daterolloverstrategy.h
    spi/defaultrolloverstrategy.h
    spi/filter.h
    spi/onstartuptriggeringpolicy.h
    spi/rolloverstrategy.h
    spi/sizebasedtriggeringpolicy.h
    spi/timebasedtriggeringpolicy.h
    spi/triggeringpolicy.h
)
set(log4qt_HEADERS_varia
    varia/debugappender.h
    varia/denyallfilter.h
    varia/levelmatchfilter.h
    varia/levelrangefilter.h
    varia/listappender.h
    varia/nullappender.h
    varia/stringmatchfilter.h
)
if(WIN32)
    list(APPEND log4qt_SOURCES
         wdcappender.cpp
         colorconsoleappender.cpp
    )
    list(APPEND log4qt_HEADERS
         wdcappender.h
         colorconsoleappender.h
    )
endif()

# Explicit library type: without it (and with QTP0003 unset, which needs
# Qt >= 6.7) qt_add_library would ignore BUILD_SHARED_LIBS and follow how
# Qt itself was built.
if(BUILD_SHARED_LIBS)
    set(log4qt_LIB_TYPE SHARED)
else()
    set(log4qt_LIB_TYPE STATIC)
endif()
qt_add_library(log4qt ${log4qt_LIB_TYPE} ${log4qt_SOURCES} ${log4qt_HEADERS} ${log4qt_HEADERS_helpers} ${log4qt_HEADERS_spi} ${log4qt_HEADERS_varia})
add_library(Log4Qt::log4qt ALIAS log4qt)

# Set target properties
set_target_properties(log4qt PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    EXPORT_NAME log4qt
)

# Link dependencies
target_link_libraries(log4qt
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Concurrent
)

# Apply compile definitions from parent
target_compile_definitions(log4qt
    PRIVATE
        ${LOG4QT_QT_DEFINITIONS}
        ${LOG4QT_VERSION_DEFINITIONS}
        ${LOG4QT_COMPILE_DEFINITIONS}
)

# Apply compile options from parent
if(LOG4QT_COMPILE_OPTIONS)
    target_compile_options(log4qt PRIVATE ${LOG4QT_COMPILE_OPTIONS})
endif()

if(BUILD_WITH_QML_LOGGING)
    target_sources(log4qt
        PRIVATE
         qmllogger.cpp
         qmllogger.h
    )
    # append to log4qt_HEADERS or proper install
    list(APPEND log4qt_HEADERS
         qmllogger.h
    )

    # PUBLIC: the installed qmllogger.h includes QtQml headers
    target_link_libraries(log4qt PUBLIC Qt${QT_VERSION_MAJOR}::Qml)
    target_compile_definitions(log4qt
        PRIVATE
            LOG4QT_QML_LOGGING_SUPPORT
    )
    if (TRUE) # Qt >= 6.5 always supports qt_add_qml_module
        qt_add_qml_module(
            log4qt
            URI org.log4qt
            VERSION 1.0
            OUTPUT_DIRECTORY
            ${CMAKE_BINARY_DIR}/bin/org/log4qt
            SOURCES
                qmllogger.cpp qmllogger.h
            NO_PLUGIN
            RESOURCE_PREFIX
                "/qt/qml"
            OUTPUT_TARGETS log4qt_qml_extra_targets
        )
        # In static builds qt_add_qml_module creates internal resource
        # targets that the backing library's link interface references;
        # they must be in the same export set or install(EXPORT) fails.
        if(log4qt_qml_extra_targets)
            install(TARGETS ${log4qt_qml_extra_targets}
                EXPORT log4qt_targets
                COMPONENT Devel
                DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
        endif()
        message(STATUS "Log4Qt: Registered logging support as qml module (org.log4qt)")
    endif()
    message(STATUS "Log4Qt: Enabling qml logging support")
else()
    message(STATUS "Log4Qt: Disabling qml logging support - disabled by cmake option")
endif()

target_include_directories(log4qt
    PUBLIC
        $<BUILD_INTERFACE:${LOG4QT_BUILD_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
)
#
# add databaseappender if database logging support was enabled
#
if(BUILD_WITH_DB_LOGGING)
    target_sources(log4qt
        PRIVATE
            databaseappender.cpp
            databaselayout.cpp
            databaseappender.h
            databaselayout.h
    )
    # append to log4qt_HEADERS or proper install
    list(APPEND log4qt_HEADERS
         databaseappender.h
         databaselayout.h
    )
    target_compile_definitions(log4qt
        PRIVATE
            LOG4QT_DB_LOGGING_SUPPORT
    )
    # PUBLIC: the installed databaseappender.h/databaselayout.h include QtSql headers
    target_link_libraries(log4qt PUBLIC Qt${QT_VERSION_MAJOR}::Sql)
    message(STATUS "Log4Qt: Enabling database logging support")
else()
    message(STATUS "Log4Qt: Disabling database logging support - disabled by cmake option")
endif()

#
# add telnetappender if telnet logging support was enabled
#
if(BUILD_WITH_TELNET_LOGGING)
    target_sources(log4qt
        PRIVATE
            telnetappender.cpp
            telnetappender.h
    )
    # append to log4qt_HEADERS or proper install
    list(APPEND log4qt_HEADERS
         telnetappender.h
    )
    target_compile_definitions(log4qt
        PRIVATE
            LOG4QT_TELNET_LOGGING_SUPPORT
    )
    # PUBLIC: the installed telnetappender.h includes QtNetwork headers
    target_link_libraries(log4qt PUBLIC Qt${QT_VERSION_MAJOR}::Network)
    message(STATUS "Log4Qt: Enabling telnet logging support")
else()
    message(STATUS "Log4Qt: Disabling telnet logging support - disabled by cmake option")
endif()

if(NOT BUILD_SHARED_LIBS)
    # LOG4QT_STATIC must also be set when linking against the lib
    target_compile_definitions(log4qt
        PUBLIC
            LOG4QT_STATIC
    )
else()
    # only set LOG4QT_LIBRARY when building the lib, not when linking against
    target_compile_definitions(log4qt
        PRIVATE
            LOG4QT_LIBRARY
    )
endif()
# Install the library and export targets
install(TARGETS log4qt
    EXPORT log4qt_targets
    COMPONENT Library
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION include
)

# Install headers
install(FILES ${log4qt_HEADERS}
    COMPONENT Devel
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log4qt
)
install(FILES ${log4qt_HEADERS_helpers}
    COMPONENT Devel
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log4qt/helpers
)
install(FILES ${log4qt_HEADERS_spi}
    COMPONENT Devel
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log4qt/spi
)
install(FILES ${log4qt_HEADERS_varia}
    COMPONENT Devel
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log4qt/varia
)
