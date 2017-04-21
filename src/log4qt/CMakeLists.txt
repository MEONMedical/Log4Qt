set(log4qt_SRCS
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
    colorconsoleappender.cpp
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
    qmllogger.cpp                                                                                                                                                     
    rollingbinaryfileappender.cpp                                                                                                                                     
    rollingfileappender.cpp                                                                                                                                           
    signalappender.cpp                                                                                                                                                
    simplelayout.cpp                                                                                                                                                  
    simpletimelayout.cpp                                                                                                                                                                                                                       
    spi/filter.cpp                                                                                                                                                                                                                             
    systemlogappender.cpp                                                                                                                                                                                                                      
    telnetappender.cpp                                                                                                                                                                                                                         
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
set(log4qt_HDRS
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
    colorconsoleappender.h
    consoleappender.h
    dailyfileappender.h
    dailyrollingfileappender.h
    fileappender.h
    hierarchy.h
    layout.h
    level.h
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
    qmllogger.h
    rollingbinaryfileappender.h
    rollingfileappender.h
    signalappender.h
    simplelayout.h
    simpletimelayout.h
    systemlogappender.h
    telnetappender.h
    ttcclayout.h
    writerappender.h
    xmllayout.h
)
set(log4qt_HDRS_helpers
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
set(log4qt_HDRS_spi
    spi/filter.h
)
set(log4qt_HDRS_varia
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
    list(APPEND log4qt_SRCS
        wdcappender.cpp
    )
    list(APPEND log4qt_HDRS
        wdcappender.h
    )
endif()
# TODO: add cmake option?
if(TARGET Qt5::Sql)
    list(APPEND log4qt_SRCS
        databaseappender.cpp
        databaselayout.cpp
    )
    list(APPEND log4qt_HDRS
        databaseappender.h
        databaselayout.h
    )
    message(STATUS "Adding sql database logging support")
else()
    message(STATUS "Not adding sql database logging support - Qt5::Sql not available")
endif()
add_library(log4qt SHARED ${log4qt_SRCS} ${log4qt_HDRS} ${log4qt_HDRS_helpers} ${log4qt_HDRS_spi} ${log4qt_HDRS_varia})
target_link_libraries(log4qt
    PUBLIC
        Qt5::Core
        Qt5::Network
)
if(TARGET Qt5::Sql)
    target_link_libraries(log4qt
        PUBLIC
            Qt5::Sql
    )
endif()
target_include_directories(log4qt
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
set_target_properties(log4qt PROPERTIES VERSION ${Log4Qt_VERSION_MAJOR}.${Log4Qt_VERSION_MINOR}.${Log4Qt_VERSION_PATCH})
set_target_properties(log4qt PROPERTIES SOVERSION ${Log4Qt_VERSION_MAJOR}.${Log4Qt_VERSION_MINOR})
install(TARGETS log4qt RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                       LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                       ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${log4qt_HDRS}         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log4qt)
install(FILES ${log4qt_HDRS_helpers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log4qt/helpers)
install(FILES ${log4qt_HDRS_spi}     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log4qt/spi)
install(FILES ${log4qt_HDRS_varia}   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log4qt/varia)
