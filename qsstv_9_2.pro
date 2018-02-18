TEMPLATE = subdirs

CONFIG += ordered

CONFIG(debug ,debug|release){
SUBDIRS += \
    qwt \
    qsstv
}


CONFIG(release ,debug|release){
SUBDIRS += \
    qsstv
}
