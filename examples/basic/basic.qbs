import qbs

Product {
    type: "application"
    consoleApplication: true
    name : "basic"
    files :
          [  "main.cpp",
             "loggerobject.cpp",
             "loggerobject.h",
             "loggerobjectprio.cpp",
             "loggerobjectprio.h",
             "loggerstatic.cpp",
             "loggerstatic.h"
          ]
    destinationDirectory: "../bin"
    Depends { name: "cpp" }
    Depends { name: "log4qt" }
    Depends { name: "Qt"; submodules: ["core", "xml", "network", "sql"] }

    cpp.includePaths: ["../../src"]
    cpp.cxxLanguageVersion: "c++11"
}
