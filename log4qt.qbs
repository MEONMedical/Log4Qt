import qbs

Project {
    minimumQbsVersion: "1.7.0"

    references: [
        "src/log4qt/log4qt.qbs",
        "examples/examples.qbs",
        "tests/tests.qbs",
        "others.qbs"
    ]
}

