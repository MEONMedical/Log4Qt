import qbs

Project {
    minimumQbsVersion: "1.9.0"
    references: [
        "examples/examples.qbs",
        "others.qbs",
        "src/log4qt/log4qt.qbs",
        "tests/tests.qbs",
    ]
}

