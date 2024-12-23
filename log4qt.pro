#CONFIG += build_with_db_logging \
#          build_with_telnet_logging \
#          build_with_qml_logging

TEMPLATE = subdirs
SUBDIRS +=  src \
            tests \
            examples

tests.depends = src
examples.depends = src

OTHER_FILES += LICENSE \
               Readme.md \
               .travis.yml \
               appveyor.yml \
               ChangeLog.md
