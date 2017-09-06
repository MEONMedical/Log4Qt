import qbs

Product {
    type: ["application", "autotest"]
    consoleApplication: true
    name : "log4qttest"
    files : 
      [  "log4qttest.cpp",
         "log4qttest.h"       
      ]
    Depends { name: "cpp" }
    Depends { name: "log4qt" }
    Depends { name: "Qt"; submodules: ["core", "xml", "network", "sql", "testlib"] }
    destinationDirectory: "../bin"

    cpp.includePaths: ["../../src"]
    cpp.cxxLanguageVersion: "c++11"
}
