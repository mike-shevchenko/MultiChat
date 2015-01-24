QT += widgets network testlib

CONFIG += c++11 static

HEADERS = \
    ChatMessages.h \
    RunTests.h \
    ChatMessagesTest.h \
    Multicaster.h \
    MainDialog.h \
    ReliableTextSender.h \
    ReliableTextReceiver.h \
    ContactList.h \
    ReliableTextReceiverTest.h \
    ChatEngine.h \
    AboutDialog.h \
    WelcomeDialog.h

SOURCES = \
    main.cpp \
    ChatMessages.cpp \
    RunTests.cpp \
    Multicaster.cpp \
    MainDialog.cpp \
    ReliableTextSender.cpp \
    ReliableTextReceiver.cpp \
    ContactList.cpp \
    ChatEngine.cpp \
    AboutDialog.cpp \
    WelcomeDialog.cpp

FORMS = MainDialog.ui \
    AboutDialog.ui \
    WelcomeDialog.ui

RESOURCES =
