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
        name: "qmake project files"
        Group {
            name: "files"
            files: ["**/*.pr[io]"]
        }
    }
}

