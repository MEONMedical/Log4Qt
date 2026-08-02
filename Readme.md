Log4Qt - Logging for the Qt cross-platform application framework
================================================================

Continuous Integration
----------------------
[![CMake Ubuntu build and test](https://github.com/MEONMedical/Log4Qt/actions/workflows/cmake-ubuntu.yml/badge.svg)](https://github.com/MEONMedical/Log4Qt/actions/workflows/cmake-ubuntu.yml)
[![CMake Windows build and test](https://github.com/MEONMedical/Log4Qt/actions/workflows/cmake-windows.yml/badge.svg)](https://github.com/MEONMedical/Log4Qt/actions/workflows/cmake-windows.yml)
[![CMake macOS build and test](https://github.com/MEONMedical/Log4Qt/actions/workflows/cmake-macos.yml/badge.svg)](https://github.com/MEONMedical/Log4Qt/actions/workflows/cmake-macos.yml)


Description
-----------
Log4Qt is a C++ port of the Apache Software Foundation Log4j package using the Qt Framework.
It is intended to be used by open source and commercial Qt projects.

Original project
----------------
http://log4qt.sourceforge.net/

This Log4Qt repository is a clone of the Log4Qt repository from https://gitorious.org/log4qt
which was itself a clone of the original Log4Qt project on SourceForge http://sourceforge.net/projects/log4qt/ (http://log4qt.sourceforge.net/).

Additional features
-------------------
* JSON configuration support (`log4qt.json`) alongside the classic `.properties` format
* XML configuration support (`log4qt.xml`) alongside `.properties` and `.json` formats
* Pluggable TriggeringPolicy / RolloverStrategy for RollingFileAppender (log4j2-style):
  size-based, time-based, cron-based, and on-startup triggering policies with
  composable OR-combination and a fixed-window numbered rollover strategy
* SimpleTimeLayout (“dd.MM.yyyy hh:mm [Thread] Level Logger Message”)
* ColorConsoleAppender (render colorized message by escape sequence and put it to console)
* SignalAppender (emit signal when log event happens)
* DatabaseAppender (append log event into sql table)
* DatabaseLayout (put log event into sql table columns)
* Telnet appender (append log event to telnet clients)
* LogStream (qDebug()-style log messages appending)
* MainThreadAppender (Proxy appender for sending log messages through event loop)
* XMLLayout to support apache chainsaw
* DailyRollingFileAppender which generates a logfile for each day (add current date formatted to the filename)
* Binary logger
* Windows Debug Console Appender

JSON Configuration
------------------
As an alternative to `log4qt.properties`, you can use a `log4qt.json` file:

```json
{
    "log4j": {
        "rootLogger": "DEBUG, console",
        "appender": {
            "console": {
                "type": "org.apache.log4j.ConsoleAppender",
                "target": "STDOUT_TARGET",
                "layout": {
                    "type": "org.apache.log4j.TTCCLayout",
                    "dateFormat": "ISO8601"
                }
            }
        },
        "logger": {
            "MyApp": "ERROR, console"
        },
        "additivity": {
            "MyApp": "false"
        }
    }
}
```

Nested objects produce dot-separated property keys, and the `type` key sets the class
(appender, layout, filter, …) for the parent object — **not** `class` or `@class`. The
value accepts the legacy `org.apache.log4j.*` names, the `Log4Qt::*` names, or the short
aliases (e.g. `Console`, `TTCCLayout`). Place the file next to your executable or in the
working directory. When both `log4qt.properties` and `log4qt.json` exist, the `.properties`
file takes priority.

You can also configure programmatically:

```cpp
#include <log4qt/jsonconfigurator.h>

Log4Qt::JsonConfigurator::configure("path/to/config.json");
// or with file watching:
Log4Qt::JsonConfigurator::configureAndWatch("path/to/config.json");
```

XML Configuration
------------------
As an alternative to `.properties` and `.json`, you can use a `log4qt.xml` file:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<log4j>
    <rootLogger>DEBUG, console</rootLogger>
    <appender>
        <console>
            <type>org.apache.log4j.ConsoleAppender</type>
            <target>STDOUT_TARGET</target>
            <layout>
                <type>org.apache.log4j.TTCCLayout</type>
                <dateFormat>ISO8601</dateFormat>
            </layout>
        </console>
    </appender>
    <logger>
        <MyApp>ERROR, console</MyApp>
    </logger>
    <additivity>
        <MyApp>false</MyApp>
    </additivity>
</log4j>
```

Nested elements produce dot-separated property keys, and a `<type>` element sets the class
for the parent element — **not** a `class` attribute as in Log4j's XML. (Attributes flatten
to child properties, so `type="..."` as an attribute also works, but `class="..."` does
not.) Priority order: `.properties` > `.json` > `.xml`.

You can also configure programmatically:

```cpp
#include <log4qt/xmlconfigurator.h>

Log4Qt::XmlConfigurator::configure("path/to/config.xml");
// or with file watching:
Log4Qt::XmlConfigurator::configureAndWatch("path/to/config.xml");
```

Requirements
------------
* Minimum Qt version required Qt 6.5 (for support of Qt versions down to 5.3 use branch 1.4 or the lates 1.4.x release
  or for support of Qt versions down to 5.7 use branch 1.6 or the lates 1.5.x release)
  Note: Qt 6.5 needs Visual Studio 2022 on Windows/MSVC — Qt 6.5.3, the last open source 6.5 patch, does not
  compile with the Visual Studio 2026 toolchain (its headers use the `stdext` array iterators that MSVC removed).
* C++20 required (minimum compiler version MSVC 19.29 (Visual Studio 2019 16.11), GCC 10, or Clang 10)

License
-------
Apache License 2, Version 2.0

Clone
-----
    git clone https://github.com/MEONMedical/Log4Qt.git

Build and install
-----------------
### cmake
cmake is used to build Log4Qt. An out-of-source build is required:
    <unpack/fetch to Log4Qt directory>
    mkdir Log4Qt-build
    cd Log4Qt-build
    cmake ../Log4Qt
    make/mingw32-make/msbuild Log4Qt.sln
    make/mingw32-make install


    Addition cmake options are
        * '-DBUILD_SHARED_LIBS=OFF' to build static log4qt lib (default: ON)
        * '-DBUILD_WITH_DB_LOGGING=ON|OFF to build with database logging support (default: OFF)
        * '-DBUILD_WITH_TELNET_LOGGING=ON|OFF to build with telnet appender support (default: ON)
        * '-DBUILD_WITH_QML_LOGGING=ON|OFF to build with qml logger support (default: ON)

