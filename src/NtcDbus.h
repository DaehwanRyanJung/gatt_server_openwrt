#ifndef NTCDBUS_H_16035801032024
#define NTCDBUS_H_16035801032024
/*
 * Dbus interface
 */

#include <string>

#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <sys/socket.h>

namespace fw
{
extern bool niceMode;

class NtcDbus
{
  public:
    bool checkConnectionsForPairing(bool depairCurrentDevice);

    class BtSocket
    {
      public:
        BtSocket() : _btSocketFd(-1) { connect(); }
        ~BtSocket() { disconnect(); }
        int getFd() {
            if (_btSocketFd < 0) {
                connect();
            }
            return _btSocketFd;
        }

      private:
        int _btSocketFd;

        void connect() {
            _btSocketFd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
        }
        void disconnect() {
            if (_btSocketFd > 0) {
                close(_btSocketFd);
                _btSocketFd = -1;
            }
        }
    };

  private:
    static BtSocket _btSocket;

    void deletePairingRecord(const std::string &device, const std::string &interface);
};
}; // namespace fw

#endif // NTCDBUS_H_16035801032024
