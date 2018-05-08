Log4Qt - Logging for the Qt cross-platform application framework
================================================================

Continuous Integration
----------------------

Travis CI (Linux) [![Build Status](https://travis-ci.org/MEONMedical/Log4Qt.svg?branch=master)](https://travis-ci.org/MEONMedical/Log4Qt)

Appveyor CI (Windows) [![Build status](https://ci.appveyor.com/api/projects/status/yhlqvbqeooy7ds2l?svg=true)](https://ci.appveyor.com/project/MeonMedical/log4qt)


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
* SimpleTimeLayout (“dd.MM.yyyy hh:mm [Thread] Level Logger Message”)
* ColorConsoleAppender (render colorized message by escape sequence and put it to console)
* SignalAppender (emit signal when log event happens)
* DatabaseAppender (append log event into sql table)
* DatabaseLayout (put log event into sql table columns)
* Telnet appender (append log event to telnet clients)
* LogStream (qDebug()-style log messages appending)
* MainThreadAppender (Proxy appender for sending log messages through event loop)
* XMLLayout to support apache chainsaw
* DailyFileAppender which generates a logfile for each day (add current date formatted to the filename)
* Binary logger
* Windows Debug Console Appender

Requirements
------------
* Minimum Qt version required Qt5.7 (for support of Qt versions down to 5.3 use branch 1.4 or the lates 1.4.x release)
* C++11 feature required (minimum compiler version MSVC14, GCC 4.8 or CLANG 3.3)

License
-------
Apache License 2, Version 2.0

Clone
-----
    git clone https://github.com/MEONMedical/Log4Qt.git

Build and install
-----------------
### qmake
use qmake to build the project

    *NIX
        qmake
        make
        make install

    WIN* (mingw)
        qmake
        mingw32-make
        mingw32-make install

    WIN* (msvc)
        qmake
        msbuild Log4Qt.sln
        msbuild /t:INSTALL Log4Qt.sln

    For static build call qmake with
    qmake "DEFINES+=LOG4QT_STATIC" or uncommend LOG4QT_STATIC in the build.pri file
    Don't forget to define LOG4QT_STATIC also in your project.

    Logging to a database via databaseappender can be enabled with qmake "QT += sql"

### include in your project
Can also be used by adding the log4qt source directly to your Qt project file by adding the following line:
include(<unpackdir>/src/log4qt/log4qt.pri)

### cmake
cmake is the second option to build Log4Qt. An out-of-source build is required:
    <unpack/fetch to Log4Qt directory>
    mkdir Log4Qt-build
    cd Log4Qt-build
    cmake ../Log4Qt
    make/mingw32-make/msbuild Log4Qt.sln (same as with qmake)
    make/mingw32-make install
    or:
    msbuild /t:INSTALL Log4Qt.sln

    Addition cmake options are
        * '-DBUILD_STATIC_LOG4CXX_LIB=ON|OFF' to build static log4qt lib (default: OFF)
        * '-DBUILD_WITH_DB_LOGGING=ON|OFF' to build with database logging support
