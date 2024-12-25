set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(log4qt_SOURCES
    appender.cpp
    appenderskeleton.cpp
    asyncappender.cpp
    basicconfigurator.cpp
    binaryfileappender.cpp
    binarylayout.cpp
    binarylogger.cpp
    binaryloggingevent.cpp
    binarylogstream.cpp
    binarytotextlayout.cpp
    binarywriterappender.cpp
    consoleappender.cpp
    dailyfileappender.cpp
    dailyrollingfileappender.cpp
    fileappender.cpp
    helpers/appenderattachable.cpp
    helpers/binaryclasslogger.cpp
    helpers/classlogger.cpp
    helpers/configuratorhelper.cpp
    helpers/datetime.cpp
    helpers/dispatcher.cpp
    helpers/factory.cpp
    helpers/initialisationhelper.cpp
    helpers/logerror.cpp
    helpers/optionconverter.cpp
    helpers/patternformatter.cpp
    helpers/properties.cpp
    hierarchy.cpp
    layout.cpp
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
    rollingbinaryfileappender.cpp                                                                                                                                     
    rollingfileappender.cpp                                                                                                                                           
    signalappender.cpp                                                                                                                                                
    simplelayout.cpp                                                                                                                                                  
    simpletimelayout.cpp                                                                                                                                                                                                                       
    spi/filter.cpp                                                                                                                                                                                                                             
    systemlogappender.cpp                                                                                                                                                                                                                      
    ttcclayout.cpp                                                                                                                                                                                                                             
    varia/binaryeventfilter.cpp                                                                                                                                                                                                                
    varia/debugappender.cpp                                                                                                                                                                                                                    
    varia/denyallfilter.cpp                                                                                                                                                                                                                    
    varia/levelmatchfilter.cpp                                                                                                                                                                                                                 
    varia/levelrangefilter.cpp                                                                                                                                                                                                                 
    varia/listappender.cpp                                                                                                                                                                                                                     
    varia/nullappender.cpp
    varia/stringmatchfilter.cpp
    writerappender.cpp
    xmllayout.cpp
)
set(log4qt_HEADERS
    appender.h
    appenderskeleton.h
    asyncappender.h
    basicconfigurator.h
    binaryfileappender.h
    binarylayout.h
    binarylogger.h
    binaryloggingevent.h
    binarylogstream.h
    binarytotextlayout.h
    binarywriterappender.h
    consoleappender.h
    dailyfileappender.h
    dailyrollingfileappender.h
    fileappender.h
    hierarchy.h
    layout.h
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
    rollingbinaryfileappender.h
    rollingfileappender.h
    signalappender.h
    simplelayout.h
    simpletimelayout.h
    systemlogappender.h
    ttcclayout.h
    writerappender.h
    xmllayout.h
)
set(log4qt_HEADERS_helpers
    helpers/appenderattachable.h
    helpers/binaryclasslogger.h
    helpers/classlogger.h
    helpers/configuratorhelper.h
    helpers/datetime.h
    helpers/dispatcher.h
    helpers/factory.h
    helpers/initialisationhelper.h
    helpers/logerror.h
    helpers/optionconverter.h
    helpers/patternformatter.h
    helpers/properties.h
)
set(log4qt_HEADERS_spi
    spi/filter.h
)
set(log4qt_HEADERS_varia
    varia/binaryeventfilter.h
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

add_library(log4qt ${log4qt_SOURCES} ${log4qt_HEADERS} ${log4qt_HEADERS_helpers} ${log4qt_HEADERS_spi} ${log4qt_HEADERS_varia})

target_link_libraries(log4qt
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Concurrent
)

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

    target_link_libraries(log4qt PRIVATE Qt${QT_VERSION_MAJOR}::Qml)
    target_include_directories(log4qt
          PUBLIC
              $<TARGET_PROPERTY:Qt::Qml,INTERFACE_INCLUDE_DIRECTORIES>
    )
    target_compile_definitions(log4qt
        PRIVATE
            LOG4QT_QML_LOGGING_SUPPORT
    )
    if (QT_VERSION_MAJOR GREATER 6 OR (QT_VERSION_MAJOR EQUAL 6 AND QT_VERSION_MINOR GREATER 1))
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
        )
        message(STATUS "Log4Qt: Registered logging support as qml module (org.log4qt)")
    endif()
    message(STATUS "Log4Qt: Enabling qml logging support")
else()
    message(STATUS "Log4Qt: Disabling qml logging support - disabled by cmake option")
endif()

target_include_directories(log4qt PUBLIC
    $<BUILD_INTERFACE:${LOG4QT_BUILD_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>)
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
    target_link_libraries(log4qt PRIVATE Qt${QT_VERSION_MAJOR}::Sql)
    target_include_directories(log4qt
          PUBLIC
              $<TARGET_PROPERTY:Qt::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    )
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
    target_link_libraries(log4qt PRIVATE Qt${QT_VERSION_MAJOR}::Network)
    target_include_directories(log4qt
          PUBLIC
              $<TARGET_PROPERTY:Qt::Network,INTERFACE_INCLUDE_DIRECTORIES>
    )
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
set_target_properties(log4qt PROPERTIES VERSION ${Log4Qt_VERSION_MAJOR}.${Log4Qt_VERSION_MINOR}.${Log4Qt_VERSION_PATCH})
set_target_properties(log4qt PROPERTIES SOVERSION ${Log4Qt_VERSION_MAJOR}.${Log4Qt_VERSION_MINOR})
#
# Install the whole stuff
#
install(TARGETS log4qt EXPORT log4qt_targets
                       COMPONENT Library
                       RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                       LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                       ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                       INCLUDES DESTINATION include
                       )

install(FILES ${log4qt_HEADERS}         COMPONENT Devel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log4qt)
install(FILES ${log4qt_HEADERS_helpers} COMPONENT Devel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log4qt/helpers)
install(FILES ${log4qt_HEADERS_spi}     COMPONENT Devel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log4qt/spi)
install(FILES ${log4qt_HEADERS_varia}   COMPONENT Devel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log4qt/varia)
