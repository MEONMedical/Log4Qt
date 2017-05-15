import qbs 1.0

Product {
    type: ["application", "autotest"]
    consoleApplication: true
    name : "binaryloggertest"
    files : 
      [  "binaryloggertest.cpp",
         "testappender.cpp",
         "testappender.h",         
         "elementsinarray.h",
         "atscopeexit.h"      
      ]
    Depends { name: "cpp" }
    Depends { name: "log4qt" }
    Depends { name: "Qt"; submodules: ["core", "xml", "network", "sql", "testlib"] }
    destinationDirectory: "../bin"

    cpp.includePaths: ["../../src"]
    cpp.cxxLanguageVersion: "c++11"
}
