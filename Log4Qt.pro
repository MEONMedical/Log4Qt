TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS +=  src \
            tests


OTHER_FILES += LICENSE \
               Readme.md \
               .travis.yml \
               appveyor.yml
