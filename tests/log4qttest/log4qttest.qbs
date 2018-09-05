import qbs

Product {
    type: ["application", "autotest"]
    consoleApplication: true
    name : "log4qttest"
    files: [
        "log4qttest.cpp",
         "log4qttest.h"
    ]

    Depends { name: "cpp" }
    Depends { name: "log4qt" }
    Depends { name: "Qt"; submodules: ["core", "xml", "network", "sql", "testlib"] }
    destinationDirectory: "../tests/bin"

    cpp.cxxLanguageVersion: "c++11"

    Properties {
        condition: qbs.targetOS.contains("macos")
        cpp.sonamePrefix: "@rpath"
        cpp.useRPaths: true
        cpp.rpaths: ["@loader_path", "@loader_path/../../bin", "@executable_path", "@executable_path/../../bin"]
    }

    Properties {
        condition: qbs.targetOS.contains("linux")
        cpp.useRPaths: true
        cpp.rpaths: ["$ORIGIN", "$ORIGIN/../../bin"]
    }

}
