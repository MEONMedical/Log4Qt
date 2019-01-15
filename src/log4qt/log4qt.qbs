import qbs
import qbs.FileInfo

import "../../log4qtlib.qbs" as ProductLibrary

ProductLibrary {
    id: library
    name: "log4qt"
    version: "1.5.1"
    property int versionMajor: parseInt(version.split('.')[0])
    property int versionMinor: parseInt(version.split('.')[1])
    property int versionPatch: parseInt(version.split('.')[2])

    cpp.includePaths: [".." ,"."]
    cpp.defines: ["LOG4QT_LIBRARY",
                  "NOMINMAX",
                  "QT_DEPRECATED_WARNINGS",
                  "QT_DISABLE_DEPRECATED_BEFORE=0x050700",
                  "QT_NO_CAST_FROM_BYTEARRAY",
                  "QT_USE_QSTRINGBUILDER",
                  "LOG4QT_VERSION_STR=\"" + library.version + "\"",
                  "LOG4QT_VERSION_MAJOR=" + library.versionMajor,
                  "LOG4QT_VERSION_MINOR=" + library.versionMinor,
                  "LOG4QT_VERSION_PATCH=" + library.versionPatch
                ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: FileInfo.cleanPath(product.sourceDirectory + "/..")
    }

    Depends {
        name: "Qt"
        submodules: [ "concurrent" ]
    }

    files: [
        "appender.cpp",
        "appender.h",
        "appenderskeleton.cpp",
        "appenderskeleton.h",
        "asyncappender.cpp",
        "asyncappender.h",
        "basicconfigurator.cpp",
        "basicconfigurator.h",
        "binaryfileappender.cpp",
        "binaryfileappender.h",
        "binarylayout.cpp",
        "binarylayout.h",
        "binarylogger.cpp",
        "binarylogger.h",
        "binaryloggingevent.cpp",
        "binaryloggingevent.h",
        "binarylogstream.cpp",
        "binarylogstream.h",
        "binarytotextlayout.cpp",
        "binarytotextlayout.h",
        "binarywriterappender.cpp",
        "binarywriterappender.h",
        "consoleappender.cpp",
        "consoleappender.h",
        "dailyfileappender.cpp",
        "dailyfileappender.h",
        "dailyrollingfileappender.cpp",
        "dailyrollingfileappender.h",
        "fileappender.cpp",
        "fileappender.h",
        "hierarchy.cpp",
        "hierarchy.h",
        "layout.cpp",
        "layout.h",
        "level.cpp",
        "level.h",
        "log4qt.h",
        "log4qtshared.h",
        "log4qtsharedptr.h",
        "logger.cpp",
        "logger.h",
        "loggerrepository.cpp",
        "loggerrepository.h",
        "loggingevent.cpp",
        "loggingevent.h",
        "logmanager.cpp",
        "logmanager.h",
        "logstream.cpp",
        "logstream.h",
        "mainthreadappender.cpp",
        "mainthreadappender.h",
        "mdc.cpp",
        "mdc.h",
        "ndc.cpp",
        "ndc.h",
        "patternlayout.cpp",
        "patternlayout.h",
        "propertyconfigurator.cpp",
        "propertyconfigurator.h",
        "qmllogger.cpp",
        "qmllogger.h",
        "rollingbinaryfileappender.cpp",
        "rollingbinaryfileappender.h",
        "rollingfileappender.cpp",
        "rollingfileappender.h",
        "signalappender.cpp",
        "signalappender.h",
        "simplelayout.cpp",
        "simplelayout.h",
        "simpletimelayout.cpp",
        "simpletimelayout.h",
        "systemlogappender.cpp",
        "systemlogappender.h",
        "ttcclayout.cpp",
        "ttcclayout.h",
        "writerappender.cpp",
        "writerappender.h",
        "xmllayout.cpp",
        "xmllayout.h",
    ]

    Group {
        name: "helpers"
        prefix: "helpers/"
        files: [
            "appenderattachable.cpp",
            "appenderattachable.h",
            "binaryclasslogger.cpp",
            "binaryclasslogger.h",
            "classlogger.cpp",
            "classlogger.h",
            "configuratorhelper.cpp",
            "configuratorhelper.h",
            "datetime.cpp",
            "datetime.h",
            "dispatcher.cpp",
            "dispatcher.h",
            "factory.cpp",
            "factory.h",
            "initialisationhelper.cpp",
            "initialisationhelper.h",
            "logerror.cpp",
            "logerror.h",
            "optionconverter.cpp",
            "optionconverter.h",
            "patternformatter.cpp",
            "patternformatter.h",
            "properties.cpp",
            "properties.h",
        ]
    }

    Group {
        name: "spi"
        prefix: "spi/"
        files:[ "filter.h",
                "filter.cpp" ]
    }

    Group {
        name: "varia"
        prefix: "varia/"
        files: [
            "binaryeventfilter.cpp",
            "binaryeventfilter.h",
            "debugappender.cpp",
            "debugappender.h",
            "denyallfilter.cpp",
            "denyallfilter.h",
            "levelmatchfilter.cpp",
            "levelmatchfilter.h",
            "levelrangefilter.cpp",
            "levelrangefilter.h",
            "listappender.cpp",
            "listappender.h",
            "nullappender.cpp",
            "nullappender.h",
            "stringmatchfilter.cpp",
            "stringmatchfilter.h",
        ]
    }

    Group {
        name: "windows"
        condition: qbs.targetOS.contains("windows")
        files: [
            "wdcappender.h",
            "wdcappender.cpp",
            "colorconsoleappender.cpp",
            "colorconsoleappender.h"]
    }

    Group {
        name: "telnetappender"
        condition: Qt.network.present
        files: [
            "telnetappender.h",
            "telnetappender.cpp"
        ]
    }

    Group {
        name: "databasesppender"
        condition: Qt.sql.present
        files: [
            "databaseappender.h",
            "databaselayout.h",
            "databaseappender.cpp",
            "databaselayout.cpp"
        ]
    }
}
