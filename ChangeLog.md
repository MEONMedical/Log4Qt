# Change Log
All notable changes to this project will be documented in this file.
----

## [v1.3.0] -

### Added
- Wrapper logger object for logging from Qml (QmlLogger)
- Appender for windows debug console (OutputDebugString)
- continuous integration windows - appveyor ci config files
- Changelog

### Improvements
- Replaced old-c-style casts

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
