// Copyright 2017-2019 Paul Nettle
//
// This file is part of Gobbledegook.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file in the root of the source tree.

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// >>
// >>>  INSIDE THIS FILE
// >>
//
// This is an example single-file stand-alone application that runs a Gobbledegook server.
//
// >>
// >>>  DISCUSSION
// >>
//
// Very little is required ("MUST") by a stand-alone application to instantiate a valid Gobbledegook server. There are also some
// things that are reocommended ("SHOULD").
//
// * A stand-alone application MUST:
//
//     * Start the server via a call to `ggkStart()`.
//
//         Once started the server will run on its own thread.
//
//         Two of the parameters to `ggkStart()` are delegates responsible for providing data accessors for the server, a
//         `GGKServerDataGetter` delegate and a 'GGKServerDataSetter' delegate. The getter method simply receives a string name (for
//         example, "battery/level") and returns a void pointer to that data (for example: `(void *)&batteryLevel`). The setter does
//         the same only in reverse.
//
//         While the server is running, you will likely need to update the data being served. This is done by calling
//         `ggkNofifyUpdatedCharacteristic()` or `ggkNofifyUpdatedDescriptor()` with the full path to the characteristic or delegate
//         whose data has been updated. This will trigger your server's `onUpdatedValue()` method, which can perform whatever
//         actions are needed such as sending out a change notification (or in BlueZ parlance, a "PropertiesChanged" signal.)
//
// * A stand-alone application SHOULD:
//
//     * Shutdown the server before termination
//
//         Triggering the server to begin shutting down is done via a call to `ggkTriggerShutdown()`. This is a non-blocking method
//         that begins the asynchronous shutdown process.
//
//         Before your application terminates, it should wait for the server to be completely stopped. This is done via a call to
//         `ggkWait()`. If the server has not yet reached the `EStopped` state when `ggkWait()` is called, it will block until the
//         server has done so.
//
//         To avoid the blocking behavior of `ggkWait()`, ensure that the server has stopped before calling it. This can be done
//         by ensuring `ggkGetServerRunState() == EStopped`. Even if the server has stopped, it is recommended to call `ggkWait()`
//         to ensure the server has cleaned up all threads and other internals.
//
//         If you want to keep things simple, there is a method `ggkShutdownAndWait()` which will trigger the shutdown and then
//         block until the server has stopped.
//
//     * Implement signal handling to provide a clean shut-down
//
//         This is done by calling `ggkTriggerShutdown()` from any signal received that can terminate your application. For an
//         example of this, search for all occurrences of the string "signalHandler" in the code below.
//
//     * Register a custom logging mechanism with the server
//
//         This is done by calling each of the log registeration methods:
//
//             `ggkLogRegisterDebug()`
//             `ggkLogRegisterInfo()`
//             `ggkLogRegisterStatus()`
//             `ggkLogRegisterWarn()`
//             `ggkLogRegisterError()`
//             `ggkLogRegisterFatal()`
//             `ggkLogRegisterAlways()`
//             `ggkLogRegisterTrace()`
//
//         Each registration method manages a different log level. For a full description of these levels, see the header comment
//         in Logger.cpp.
//
//         The code below includes a simple logging mechanism that logs to stdout and filters logs based on a few command-line
//         options to specify the level of verbosity.
//
// >>
// >>>  Building with GOBBLEDEGOOK
// >>
//
// The Gobbledegook distribution includes this file as part of the Gobbledegook files with everything compiling to a single, stand-
// alone binary. It is built this way because Gobbledegook is not intended to be a generic library. You will need to make your
// custom modifications to it. Don't worry, a lot of work went into Gobbledegook to make it almost trivial to customize
// (see Server.cpp).
//
// If it is important to you or your build process that Gobbledegook exist as a library, you are welcome to do so. Just configure
// your build process to build the Gobbledegook files (minus this file) as a library and link against that instead. All that is
// required by applications linking to a Gobbledegook library is to include `include/Gobbledegook.h`.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <signal.h>
#include <iostream>
#include <thread>
#include <sstream>
#include <unistd.h>

#include "../include/Gobbledegook.h"
#include "NtcLogger.h"
#include "NtcUci.h"
#include "NtcDbus.h"

//
// Constants
//

#define GATT_SHORT_ADV_NAME_LEN 10 // Length of short advertising name
#define PAIRING_GRACE_TIME  600  // When bonding window is open, how many seconds to allow unpaired connections (waiting for user to click "Paired")
#define BONDING_WINDOW_TIME 600 // How long after startup to keep bonding window open - production value should be 600 seconds

#define MIN_BONDING_WINDOW_TIME 100
#define MAX_BONDING_WINDOW_TIME 86400

// Maximum time to wait for any single async process to timeout during initialization
static const int kMaxAsyncInitTimeoutMS = 30 * 1000;

//
// Logging
//

enum LogLevel
{
    Debug,
    Verbose,
    Normal,
    ErrorsOnly
};

// Our log level - defaulted to 'Normal' but can be modified via command-line options
LogLevel logLevel = Normal;

// Our full set of logging methods (we just log to stdout)
//
// NOTE: Some methods will only log if the appropriate `logLevel` is set
void LogDebug(const char *pText) { if (logLevel <= Debug) { std::cout << "  DEBUG: " << pText << std::endl; } }
void LogInfo(const char *pText) { if (logLevel <= Verbose) { std::cout << "   INFO: " << pText << std::endl; } }
void LogStatus(const char *pText) { if (logLevel <= Normal) { std::cout << " STATUS: " << pText << std::endl; } }
void LogWarn(const char *pText) { std::cout << "WARNING: " << pText << std::endl; }
void LogError(const char *pText) { std::cout << "!!ERROR: " << pText << std::endl; }
void LogFatal(const char *pText) { std::cout << "**FATAL: " << pText << std::endl; }
void LogAlways(const char *pText) { std::cout << "..Log..: " << pText << std::endl; }
void LogTrace(const char *pText) { std::cout << "-Trace-: " << pText << std::endl; }

//
// Signal handling
//

// We setup a couple Unix signals to perform graceful shutdown in the case of SIGTERM or get an SIGING (CTRL-C)
void signalHandler(int signum)
{
    switch (signum)
    {
        case SIGINT:
            LogStatus("SIGINT recieved, shutting down");
            ggkTriggerShutdown();
            break;
        case SIGTERM:
            LogStatus("SIGTERM recieved, shutting down");
            ggkTriggerShutdown();
            break;
    }
}

//
// Server data management
//

// Called by the server when it wants to retrieve a named value
//
// This method conforms to `GGKServerDataGetter` and is passed to the server via our call to `ggkStart()`.
//
// The server calls this method from its own thread, so we must ensure our implementation is thread-safe. In our case, we're simply
// sending over stored values, so we don't need to take any additional steps to ensure thread-safety.
const void *dataGetter(const char *pName)
{
    (void) pName; //UNUSED
    return nullptr;
}

// Called by the server when it wants to update a named value
//
// This method conforms to `GGKServerDataSetter` and is passed to the server via our call to `ggkStart()`.
//
// The server calls this method from its own thread, so we must ensure our implementation is thread-safe. In our case, we're simply
// sending over stored values, so we don't need to take any additional steps to ensure thread-safety.
int dataSetter(const char *pName, const void *pData)
{
    (void) pName; //UNUSED
    (void) pData; //UNUSED
    return 0;
}

//
// Entry point
//

int main(int argc, char **ppArgv)
{
    int optc;
    int syslogLevel = LOG_ERR;
    bool isBondable = true;

    std::chrono::steady_clock::time_point now, timeoutBondingWindow, timeoutGraceTime;
    unsigned int bondingWindowDur;

    uci::UciHandle uciHdl;
    fw::NtcDbus dbusHdl;

    while (((optc = getopt(argc, ppArgv, "gvdng:l:"))) != -1) {
        switch(optc) {
            case 'q':
                logLevel = ErrorsOnly;
                break;
            case 'v':
                logLevel = Verbose;
                break;
            case 'd':
                logLevel = Debug;
                break;

            case 'n':
                LogWarn("Disabling Pairing timeout");
                fw::niceMode = true;
                break;

            case 'l': // syslog level
                /* LOG_ERR(0), LOG_WARNING(1), LOG_NOTICE(2), LOG_INFO(3), LOG_DEBUG(4) */
                try {
                    syslogLevel = std::stoi(optarg);
                    if (syslogLevel < 0) {
                        syslogLevel = 0;
                    } else if (syslogLevel > 4) {
                        syslogLevel = 4;
                    }
                }
                catch(std::exception &ex){
                    syslogLevel = 0;
                }
                syslogLevel += LOG_ERR;
                break;

            default:
                LogFatal("");
                LogFatal("Usage: ggk-standalone [-q | -v | -d | -n | -l level]");
                LogFatal("\t-q: Error only log level");
                LogFatal("\t-v: Verbose log level");
                LogFatal("\t-d: Debug log level");
                LogFatal("\t-n: Disable Pairing timeout");
                LogFatal("\t-l level: Set syslog verbosity[0-4]");
                return -1;
        }
    }

    // Setup our signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Register our loggers
    ggkLogRegisterDebug(LogDebug);
    ggkLogRegisterInfo(LogInfo);
    ggkLogRegisterStatus(LogStatus);
    ggkLogRegisterWarn(LogWarn);
    ggkLogRegisterError(LogError);
    ggkLogRegisterFatal(LogFatal);
    ggkLogRegisterAlways(LogAlways);
    ggkLogRegisterTrace(LogTrace);

    fw::Logger::getInstance().setup(LOG_INFO);

    const char *advName;
    std::string uciAdvName = uciHdl.get({"gattserver", "config", "adv_name"}).toStdString();

    if (uciAdvName.empty()) {
        advName = "Aurus-XXXX";
    } else {
        uciAdvName.resize(GATT_SHORT_ADV_NAME_LEN);
        advName = uciAdvName.data();
    }

    bondingWindowDur = uciHdl.get({"gattserver", "config", "bonding_window_dur"}).toInt<unsigned int>(BONDING_WINDOW_TIME);
    if (bondingWindowDur < MIN_BONDING_WINDOW_TIME)
        bondingWindowDur = MIN_BONDING_WINDOW_TIME;
    if (bondingWindowDur > MAX_BONDING_WINDOW_TIME)
        bondingWindowDur = MAX_BONDING_WINDOW_TIME;

    // Start the server's ascync processing
    //
    // This starts the server on a thread and begins the initialization process
    //
    // !!!IMPORTANT!!!
    //
    //     This first parameter (the service name) must match tha name configured in the D-Bus permissions. See the Readme.md file
    //     for more information.
    //
    if (!ggkStart("gobbledegook", advName, advName, dataGetter, dataSetter, kMaxAsyncInitTimeoutMS))
    {
        return -1;
    }

    now = std::chrono::steady_clock::now();
    timeoutBondingWindow = now + std::chrono::seconds(bondingWindowDur);
    timeoutGraceTime = now + std::chrono::seconds(PAIRING_GRACE_TIME);

#ifdef V_GATT_SERVER_AUTH_y
    log(LOG_ERR, "BLE Authentication is enabled");
#else
    log(LOG_ERR, "BLE Authentication is disabled");
#endif

    // Wait for the server to start the shutdown process
    while (ggkGetServerRunState() < EStopping)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // This is a check to prevent accidental/nuisance connections from tying up the peripheral.
        if (!fw::niceMode) ggkCheckDasBoot();

        now = std::chrono::steady_clock::now();

        if (now < timeoutBondingWindow)
        {
            if (!isBondable) {
                log(LOG_NOTICE, "Enable BT Connectable/Bonding");
                ggkSetBonding(true);
                ggkSetConnectable(true);
                ggkSetDiscoverable(true);
                ggkSetAdvertising(true);
                isBondable = true;
            }
            if (!ggkGetActiveConnections())
            {
                timeoutGraceTime = now + std::chrono::seconds(PAIRING_GRACE_TIME);
                LogInfo("No active connections, bonding open, resetting grace timer");
            }
        }
        else
        {
            if (isBondable) {
                log(LOG_ERR, "BONDING_WINDOW_TIMER(%d seconds) expired, device binding is not allowed.", bondingWindowDur);
                log(LOG_NOTICE, "Disable BT Connectable/Bonding");
                ggkSetAdvertising(false);
                ggkSetDiscoverable(false);
                ggkSetConnectable(false); // Disable connectable. Otherwise, a peer keeps retrying to connect a GATT server.
                ggkSetBonding(false); // good idea to keep disabling it, in case someone else fiddles with the knob
                isBondable = false;
            }
            // Bonding Window is expired, so any unpaired connections should be disconnected, if exist.
            timeoutGraceTime = now;
        }

        // A case that a peer is connected but not paired, yet.
        if (!fw::niceMode && dbusHdl.checkConnectionsForPairing(false))
        {
            if (!ggkGetActiveConnections())
            {
                LogWarn("unpaired connection detected but no connections..  this is weird, power cycle time!");
                ggkSetDasBootFlag();
            }
            if (now >= timeoutGraceTime)
            {
                if (now < timeoutBondingWindow) {
                    log(LOG_ERR, "PAIRING_GRACE_TIMER(%d seconds) expired, terminated a session.", PAIRING_GRACE_TIME);
                } else {
                    log(LOG_ERR, "BONDING_WINDOW_TIMER(%d seconds) expired, connection request is refused.", bondingWindowDur);
                }
                ggkSetDasBootFlag();
            }
        }
        else
        {
            timeoutGraceTime = now + std::chrono::seconds(PAIRING_GRACE_TIME);
        }

        uciHdl.pollSubscription();
    }

    // Wait for the server to come to a complete stop (CTRL-C from the command line)
    if (!ggkWait())
    {
        return -1;
    }

    // Return the final server health status as a success (0) or error (-1)
    return ggkGetServerHealth() == EOk ? 0 : 1;
}
