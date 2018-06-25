import qbs

Product {
    type: "dynamiclibrary"

    destinationDirectory: "../bin"
    Depends { name: 'cpp' }
    Depends { name: "Qt"; submodules: ["core", "xml"]; versionAtLeast: "5.7.0" }
    Depends { name: "Qt.sql"; required: false }
    Depends { name: "Qt.network"; required: false }

    cpp.cxxLanguageVersion: "c++14"
    cpp.visibility: "hidden"

    Properties {
        condition: qbs.toolchain.contains('gcc')
        cpp.cFlags: [ "-Wno-long-long", "-ansi" ]
        cpp.cxxFlags: [ "-Wnon-virtual-dtor", "-Wundef",  "-Wcast-align",
                        "-Wchar-subscripts", "-Wpointer-arith",
                        "-Wwrite-strings", "-Wpacked", "-Wformat-security",
                        "-Wmissing-format-attribute", "-Woverloaded-virtual",
                        "-pedantic", "-Wold-style-cast",
                        "-Wunreachable-code"]
    }

    Properties {
        condition: qbs.toolchain.contains('msvc')
        cpp.cFlags: [ "-W4" ]
        cpp.cxxFlags: [ "-W4",
                        "-wd4127", "-wd4512", "-wd4267", "-we4265"]

        //disable common warnings in Qt/stdlib
        //disable 4127: conditional expression is constant
        //        4512: assignment operator could not be generated
        //         4267: conversion from 'size_t' to 'type', possible loss of data

        // /WX ... treat all warnings as errors
        // /we<n> ... Treat warning <n> as error
    }

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

