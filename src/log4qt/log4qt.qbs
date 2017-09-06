import qbs
import "../../log4qtlib.qbs" as ProductLibrary

ProductLibrary {
    id: library
    name: "log4qt"
    version: "1.4.2"
    property int versionMajor: parseInt(version.split('.')[0])
    property int versionMinor: parseInt(version.split('.')[1])
    property int versionPatch: parseInt(version.split('.')[2])

    cpp.includePaths: [".." ,"."]
    cpp.defines: ["LOG4QT_LIBRARY",
                  "NOMINMAX",
                  "QT_DEPRECATED_WARNINGS",
                  "QT_DISABLE_DEPRECATED_BEFORE=0x050600",
                  "QT_NO_CAST_FROM_BYTEARRAY",
                  "QT_USE_QSTRINGBUILDER",
                  "LOG4QT_VERSION_STR=\"" + library.version + "\"",
                  "LOG4QT_VERSION_MAJOR=" + library.versionMajor,
                  "LOG4QT_VERSION_MINOR=" + library.versionMinor,
                  "LOG4QT_VERSION_PATCH=" + library.versionPatch
                ]


    files:["appender.h",
           "appenderskeleton.h",
           "asyncappender.h",
           "basicconfigurator.h",
           "binaryfileappender.h",
           "binarylayout.h",
           "binarylogger.h",
           "binaryloggingevent.h",
           "binarylogstream.h",
           "binarytotextlayout.h",
           "binarywriterappender.h",
           "colorconsoleappender.h",
           "consoleappender.h",
           "dailyfileappender.h",
           "dailyrollingfileappender.h",
           "fileappender.h",
           "hierarchy.h",
           "layout.h",
           "level.h",
           "log4qt.h",
           "log4qtshared.h",
           "log4qtsharedptr.h",
           "logger.h",
           "loggerrepository.h",
           "loggingevent.h",
           "logmanager.h",
           "logstream.h",
           "mainthreadappender.h",
           "mdc.h",
           "ndc.h",
           "patternlayout.h",
           "propertyconfigurator.h",
           "qmllogger.h",
           "rollingbinaryfileappender.h",
           "rollingfileappender.h",
           "signalappender.h",
           "simplelayout.h",
           "simpletimelayout.h",
           "systemlogappender.h",
           "ttcclayout.h",
           "writerappender.h",
           "xmllayout.h",
           "appender.cpp",
           "appenderskeleton.cpp",
           "basicconfigurator.cpp",
           "colorconsoleappender.cpp",
           "consoleappender.cpp",
           "dailyrollingfileappender.cpp",
           "asyncappender.cpp",
           "dailyfileappender.cpp",
           "mainthreadappender.cpp",
           "fileappender.cpp",
           "hierarchy.cpp",
           "layout.cpp",
           "level.cpp",
           "logger.cpp",
           "loggerrepository.cpp",
           "loggingevent.cpp",
           "logmanager.cpp",
           "mdc.cpp",
           "ndc.cpp",
           "patternlayout.cpp",
           "propertyconfigurator.cpp",
           "rollingfileappender.cpp",
           "signalappender.cpp",
           "simplelayout.cpp",
           "simpletimelayout.cpp",
           "ttcclayout.cpp",
           "writerappender.cpp",
           "systemlogappender.cpp",
           "logstream.cpp",
           "binaryloggingevent.cpp",
           "binarylogger.cpp",
           "binarytotextlayout.cpp",
           "binarywriterappender.cpp",
           "binaryfileappender.cpp",
           "binarylogstream.cpp",
           "rollingbinaryfileappender.cpp",
           "binarylayout.cpp",
           "xmllayout.cpp",
           "qmllogger.cpp"]

    Group {
        name: "helpers"
        prefix: "helpers/"
        files:[ "appenderattachable.h",
                "binaryclasslogger.h",
                "classlogger.h",
                "configuratorhelper.h",
                "datetime.h",
                "dispatcher.h",
                "factory.h",
                "initialisationhelper.h",
                "logerror.h",
                "optionconverter.h",
                "patternformatter.h",
                "properties.h",
                "binaryclasslogger.cpp",
                "classlogger.cpp",
                "appenderattachable.cpp",
                "configuratorhelper.cpp",
                "datetime.cpp",
                "factory.cpp",
                "initialisationhelper.cpp",
                "logerror.cpp",
                "optionconverter.cpp",
                "patternformatter.cpp",
                "properties.cpp",
                "dispatcher.cpp"  ]
    }

    Group {
        name: "spi"
        prefix: "spi/"
        files:[ "filter.h",
                "filter.cpp"]
    }

    Group {
        name: "varia"
        prefix: "varia/"
        files:[ "binaryeventfilter.h",
                "debugappender.h",
                "denyallfilter.h",
                "levelmatchfilter.h",
                "levelrangefilter.h",
                "listappender.h",
                "nullappender.h",
                "stringmatchfilter.h",
                "binaryeventfilter.cpp",
                "debugappender.cpp",
                "denyallfilter.cpp",
                "levelmatchfilter.cpp",
                "levelrangefilter.cpp",
                "listappender.cpp",
                "nullappender.cpp",
                "stringmatchfilter.cpp" ]
    }

    Group {
        name: "windows"
        condition: qbs.targetOS.contains("windows")
        files: [
            "wdcappender.h",
            "wdcappender.cpp" ]
    }

    Group {
        name: "telnetappender"
        condition: Qt.network.present
        files: [
            "telnetappender.h",
            "telnetappender.cpp" ]
    }

    Group {
        name: "databasesppender"
        condition: Qt.sql.present
        files: [
            "databaseappender.h",
            "databaselayout.h",
            "databaseappender.cpp",
            "databaselayout.cpp" ]
    }
}
