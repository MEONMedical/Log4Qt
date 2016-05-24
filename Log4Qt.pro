TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS +=  src \
            tests \
            examples


OTHER_FILES += LICENSE \
               Readme.md \
               .travis.yml \
               appveyor.yml \
               ChangeLog.md
