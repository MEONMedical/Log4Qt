import qbs

Product {
    type: ["application", "autotest"]
    consoleApplication: true
    name : "binaryloggertest"
    files: [
        "binaryloggertest.cpp",
         "testappender.cpp",
         "testappender.h",
         "elementsinarray.h",
         "atscopeexit.h"
    ]

    Depends { name: "cpp" }
    Depends { name: "log4qt" }
    Depends { name: "Qt"; submodules: ["core", "xml", "network", "sql", "testlib"] }
    destinationDirectory: "../bin"

    cpp.cxxLanguageVersion: "c++11"

    Properties {
        condition: qbs.targetOS.contains("macos")
        cpp.sonamePrefix: "@rpath"
        cpp.useRPaths: true
        cpp.rpaths: ["@loader_path", "@executable_path"]
    }

    Properties {
        condition: qbs.targetOS.contains("linux")
        cpp.useRPaths: true
        cpp.rpaths: ["$ORIGIN"]
    }     
}
