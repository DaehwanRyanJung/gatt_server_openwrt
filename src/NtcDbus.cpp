/*
 * Dbus interface
 */

#include "NtcDbus.h"

#include <dbus/dbus.h>
#include <sys/ioctl.h>

#include "NtcLogger.h"

namespace fw
{
// Flag for "Disabling Pairing timeout" option(-n) on commandline
bool niceMode=false;

// Initialize static member
NtcDbus::BtSocket NtcDbus::_btSocket;

// In the situation where a connected device generates Authentication Denied errors, it is likely that the user forgot the pairing
// from the phone.  We still see it as paired but it generates EAuthenticationFailedEvent messages.  So, let's tidy up our end.
void NtcDbus::deletePairingRecord(const std::string &device, const std::string &interface)
{
    // call path /org/bluez/hci0 interface org.bluez.Adapter1 function RemoveDevice with arg of device path
    DBusConnection *connection = nullptr;
    DBusMessage *msgQuery = nullptr;
    DBusError error;

    std::string path = "/org/bluez/" + interface;
    std::string parameter = "/org/bluez/" + interface + "/" + device;

    fw::utils::ScopeDeletor connDeletor([&connection] {
        dbus_connection_unref(connection);
    });

    fw::utils::ScopeDeletor msgDeletor([&msgQuery] {
            dbus_message_unref(msgQuery);
    });

    dbus_error_init(&error);
    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connection == nullptr)
    {
        log(LOG_ERR, "DBUS ERROR: Failed to open connection to system bus: (%s)", error.message);
        dbus_error_free (&error);
        return;
    }

    // From API document - Returns NULL if memory can't be allocated for the message.
    msgQuery = dbus_message_new_method_call("org.bluez", path.c_str(), "org.bluez.Adapter1", "RemoveDevice");
    if (!msgQuery) {
        log(LOG_ERR, "DBUS EXCEPTION: Memory allocation failed dbus_message_new_method_call()");
        return;
    }

    const char* pptr = parameter.c_str(); // we have to do it this way - see the warning at
                                          // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga591f3aab5dd2c87e56e05423c2a671d9

    dbus_message_append_args (msgQuery, DBUS_TYPE_OBJECT_PATH, &pptr, DBUS_TYPE_INVALID);
    if (!dbus_connection_send (connection, msgQuery, NULL))
    {
        log(LOG_ERR, "DBUS EXCEPTION: Failed dbus_connection_send()");
        return;
    }

    /* ggkSetDasBootFlag(); // force a disconnect */ // TODO:: check
    return;
}


// Check there is a connection which is not paired, yet.
//
// depairCurrentDevice if true, remove connected device info from the action list(/var/lib/bluetooth/)
//
// return true, if there is a connection whici is not paired, yet.
bool NtcDbus::checkConnectionsForPairing(bool depairCurrentDevice)
{
    struct hci_conn_list_req *cl = nullptr;
    struct hci_conn_info *ci = nullptr;
    int sk = _btSocket.getFd();

    fw::utils::ScopeDeletor clDeletor([&cl] { ::delete cl; });

    if (sk < 0)
        return false;

    if ((cl =  reinterpret_cast<hci_conn_list_req*>(::operator new(sizeof(*cl) + sizeof(*ci), std::nothrow))) == nullptr) {
        return false;
    }

    cl->dev_id = 0; // hci0
    cl->conn_num = 1; // single connection
    ci = cl->conn_info;

    if (ioctl(sk, HCIGETCONNLIST, (void *) cl)) {
        return false;
    }

    if (cl->conn_num != 1) {
        return false;
    }

    /* ci->type:  SCO_LINK(0x00), ACL_LINK(0x01), ESCO_LINK(0x02), LE_LINK(0x80) */
    /* ci->state: BT_CONNECTED(1), BT_OPEN(2), BT_BOUND(3), BT_LISTEN(4), BT_CONNECT(5), */
    /*            BT_CONNECT2(6), BT_CONFIG(7), BT_DISCONN(8), BT_CLOSED(9) */
    /* ci->link_mode(mask): */
    /*    #define HCI_LM_ACCEPT   0x8000 */
    /*    #define HCI_LM_MASTER   0x0001 */
    /*    #define HCI_LM_AUTH     0x0002 */
    /*    #define HCI_LM_ENCRYPT  0x0004 */
    /*    #define HCI_LM_TRUSTED  0x0008 */
    /*    #define HCI_LM_RELIABLE 0x0010 */
    /*    #define HCI_LM_SECURE   0x0020 */
    bool connected = ci->type == 0x80 && ci->state == 1;
    bool paired = (ci->link_mode & 0x0006) == 0x0006;
    if (connected && depairCurrentDevice) {
        std::string dev = fw::utils::format("dev_%2.2X_%2.2X_%2.2X_%2.2X_%2.2X_%2.2X", ci->bdaddr.b[5], ci->bdaddr.b[4], ci->bdaddr.b[3], ci->bdaddr.b[2], ci->bdaddr.b[1], ci->bdaddr.b[0]);
        deletePairingRecord(dev, "hci0");
        return false;
    }

    return connected && !paired;
}

}; // namespace fw
