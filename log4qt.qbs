import qbs

Project {
    minimumQbsVersion: "1.9.0"
    references: [
        "examples/examples.qbs",
        "others.qbs",
        "src/log4qt/log4qt.qbs",
        "tests/tests.qbs",
    ]

    Product {
        name: "qmake project files(log4qt)"
        Group {
            name: "files"
            files: ["**/*.pr[io]"]
        }
    }

    Product {
        name: "cmake project files(log4qt)"
        Group {
            name: "files"
            files: ["**/CMakeLists.txt", "cmake/*"]
        }
    }
}

