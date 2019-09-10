QT -= gui
QT += core sql network widgets

CONFIG += c++11 console object_parallel_to_source
CONFIG -= app_bundle

INCLUDEPATH +=\
     $$PWD/../workspace/katana/starlight2/katana/shared/ \
     $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/xsd/ \
     $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/lexertk/ \
     $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/lapack/ \
     $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/armadillo/x86_64/include/ \
     $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/armadillo/x86_64/lib/ \
     $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/lapack/lib/x86_64/ \
     $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/avantes/ \
     $$PWD/../workspace/katana/starlight2/katana/shared/common/sqlite/ \
     $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/hinnant/ \
     $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/cryptBlowfish/ \
     $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/xsd/include/ \
     $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/xsd/include/cxx/

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS USING_QT

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    computetaylor.cpp \
    temperaturecorrection.cpp \
    $$PWD/../workspace/katana/starlight2/katana/shared/common/math/polynomials/singleVariablePolynomial.cpp \
    $$PWD/../workspace/katana/starlight2/katana/shared/common/stringUtilities.cpp \
    $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/lapack/polyfit.c \
    $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/lapack/expfit.c


LIBS += -lz
LIBS += -lpng \
        -lssl \
        -lcrypto \
        -lgstreamer-1.0 \
        -lgobject-2.0 \
        -lgmodule-2.0 \
        -lgthread-2.0 \
        -lglib-2.0 \
        -L/usr/include/xercesc/ -lxerces-c \
        #-lavs \
        -lsqlite3 \
        $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/lapack/lib/x86_64/liblapack.a \
        $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/lapack/lib/x86_64/librefblas.a \
        $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/armadillo/x86_64/lib/libarmadillo.a
        -lgfortran \

LIBS += -L/usr/include/xercesc/ -lxerces-c \
        #-lavs \
        -lsqlite3 \
        -lssl \
        -lcrypto \
        -lgfortran \
        #-larmadillo \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    multipoly.h \
    computetaylor.h \
    hinnant.h \
    temperaturecorrection.h \
    $$PWD/../workspace/katana/starlight2/katana/shared/common/math/polynomials/polynomial.h \
    $$PWD/../workspace/katana/starlight2/katana/shared/common/math/polynomials/singleVariablePolynomial.h \
    $$PWD/../workspace/katana/starlight2/katana/shared/common/optional.h \
    $$PWD/../workspace/katana/starlight2/katana/shared/common/infixOstreamIterator.h \
    $$PWD/../workspace/katana/starlight2/katana/shared/common/stringUtilities.h \
    $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/lapack/polyfit.h \
    $$PWD/../workspace/katana/starlight2/katana/shared/3rdParty/lapack/expfit.h

DISTFILES += \
    first_test_data.txt \
    samplePixelPositions.csv \
    sampleWavelengths.csv \
    YPixelBeforeCorrection.csv
