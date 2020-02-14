# Change Log

All notable changes to this project will be documented in this file.
----
## [v1.5.1] - 14.02.2020
- Automatically delete files written by DailyFileAppender after a configurable period of time.

## [v1.5.0] - 25.06.2018
### Improvements
- RollingFileAppender: Changed the behavior on application restart and if
                       appendFile is set to false to avoid data loss. The
                       existing log files are rolled instead of overwritten.
                       This behavior is different to the log4/log4cpp implementation.
- Modernize c++ and qt usage wit clang-tidy and clazy

### Fixed
- Fixed "DATE" format string, it will be formatted as MMM YYYY HH:mm:ss.zzz
- Minimum required Qt version is 5.7
- Move color console appender to windows only (is only implemented for windows),
  unified windows os preprocessor switches

## [v1.4.2] - 2017-09-06
### Improvements
- Build: Use moc file includes in order to improve build performance
- Allow java - loggernames in the form 'Foo.bar' within Hierarchy::createLogger()

### Fixed
- Work around for the QFileSystemWatcher shortcome handling changes in files which
  are modified/written into a temp file, on commit the orig file is deleted and
  the temp file renamed/moved to orig file name


## [v1.4.1] - 2017-06-17
### Improvements
- Added qbs support

### Fixed
- Fixed cmake build
- Fix Patternformatter maxLength behaviour to match the log4j documentation -
  if the data item is longer than the maximum field, then the extra characters are removed from the beginning
  of the data item and not from the end.

## [v1.4.0] - 2017-05-04

### Added
- Environment variables starting with LOG4QT_ can now be used within a log4qt.properties file ${LOG4QT_...}
- Added cmake support
- Added suport for location information (%F, %L, %M, %l)
- Added support for QLoggingCategories

### Improvements
- Support static builds
- detect minimum required compiler version (log4qt depends on magic statics c++11)
- Cleanup includes in headers
- Make database and telnet appender optional dependig on Qt module sql and network

### Fixed
- Make DatabaseAppender compile

## [v1.3.0] - 2016-09-13

### Added
- Changed directory layout
- Wrapper logger object for logging from Qml (QmlLogger)
- Appender for windows debug console (OutputDebugString)
- continuous integration windows - appveyor ci config files
- Changelog
- New global boolean property for *.properties files
    log4j.watchThisFile=[true]|[false]
    If set to true an file system watcher is installed for the *.properties file
    and the loggers are automatically reconfigured if this file changes.

### Improvements
- Replaced old-c-style casts
- Searching for *.properties files are extended:
  1. If <application binaryname.exe.log4qt.properties exists this file is used (Windows only)
  2. If <application binaryname>.log4qt.properties exists this file is used
  3. If "log4qt.properties" exists in the executables path this file is used
  4. If "log4qt.properties" exists in the current working directory this file is used (the default before)

### Fixed
- Minimum required Qt version is 5.3
- Reenabled an fixed unit test suite

## [v1.2.0] - 2016-05-03

### Added
- continuous integration linux - travis ci config files

### Improvment
- Replaced object ownership via LogObject and LogObjectPtr with QSharedPointer
- Removed pre Qt5 code
- Code improvements (use c++11 feature, e.g. nulptr, range base for loop, override, ...)
- Replaced msecSinceEpoch methods of DateTime with QDateTime methodes which exists since Qt4.7
- Replaced custom DataTime Formater with QDateTime::toString() - lost additional expression week

### Fixed
- converted inlcude file from utf-16 to ascii
- DatePatternConverter: fixed scope of toString() call

## [v1.1.0] - 2015-12-22

### Added
- Binary logger
- XMLLayout to support apache chainsaw
