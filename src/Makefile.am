# Build a static library (libggk.a)
noinst_LIBRARIES = libggk.a
libggk_a_CXXFLAGS = -fPIC -Wall -Wextra -std=c++17 -I../include/ -I./plugins/utils/ $(GLIB_CFLAGS) $(GIO_CFLAGS) $(GOBJECT_CFLAGS) $(DBUS_CFLAGS)

libggk_a_CXXFLAGS += -Wno-unused-parameter # To disable unused variable warning.

libggk_a_LDLIBS = -lbluetooth # To use UCI interface on BleRssiServicePlugin.cpp
libggk_a_LDLIBS += -lcrypto # for DigestAuth.cpp/DigestAuth.h
libggk_a_LDLIBS += -lubus -lubox -lblobmsg_json

AUTOMAKE_OPTIONS = subdir-objects

libggk_a_SOURCES = DBusInterface.cpp \
                   DBusInterface.h \
                   DBusMethod.cpp \
                   DBusMethod.h \
                   DBusObject.cpp \
                   DBusObject.h \
                   DBusObjectPath.h \
                   GattCharacteristic.cpp \
                   GattCharacteristic.h \
                   GattDescriptor.cpp \
                   GattDescriptor.h \
                   GattInterface.cpp \
                   GattInterface.h \
                   GattProperty.cpp \
                   GattProperty.h \
                   GattService.cpp \
                   GattService.h \
                   GattUuid.h \
                   Globals.h \
                   Gobbledegook.cpp \
                   ../include/Gobbledegook.h \
                   HciAdapter.cpp \
                   HciAdapter.h \
                   HciSocket.cpp \
                   HciSocket.h \
                   Init.cpp \
                   Init.h \
                   Logger.cpp \
                   Logger.h \
                   Mgmt.cpp \
                   Mgmt.h \
                   Server.cpp \
                   Server.h \
                   ServerUtils.cpp \
                   ServerUtils.h \
                   standalone.cpp \
                   TickEvent.h \
                   Utils.cpp \
                   Utils.h

libggk_a_SOURCES += NtcUtils.cpp NtcUtils.h
libggk_a_SOURCES += NtcLogger.cpp NtcLogger.h
libggk_a_SOURCES += NtcUci.cpp NtcUci.h
libggk_a_SOURCES += NtcDbus.cpp NtcDbus.h

# List of Service Plugin utility
libggk_a_SOURCES += ./plugins/utils/Ping.cpp
libggk_a_SOURCES += ./plugins/utils/DigestAuth.cpp ./plugins/utils/DigestAuth.h

# List of Service Plugins
libggk_a_SOURCES += ./plugins/NtcServicePluginBase.cpp ./plugins/NtcServicePluginBase.h
libggk_a_SOURCES += ./plugins/mainServicePlugin.cpp
libggk_a_SOURCES += ./plugins/DeviceServicePlugin.cpp ./plugins/DeviceServicePlugin.h
libggk_a_SOURCES += ./plugins/DeviceInfoServicePlugin.cpp ./plugins/DeviceInfoServicePlugin.h
libggk_a_SOURCES += ./plugins/BleRssiServicePlugin.cpp ./plugins/BleRssiServicePlugin.h


# Build our standalone server (linking statically with libggk.a, linking dynamically with GLib)
standalone_CXXFLAGS = -fPIC -Wall -Wextra -std=c++17 -I../include/ -I./plugins/utils/
noinst_PROGRAMS = standalone
standalone_SOURCES = standalone.cpp
standalone_LDADD = libggk.a
standalone_LDADD += -lbluetooth # To use UCI interface on BleRssiServicePlugin.cpp
standalone_LDADD += -lcrypto # for DigestAuth.cpp/DigestAuth.h
standalone_LDADD += -lubus -lubox -lblobmsg_json
standalone_LDLIBS = $(GLIB_LIBS) $(GIO_LIBS) $(GOBJECT_LIBS) $(DBUS_LIBS)
