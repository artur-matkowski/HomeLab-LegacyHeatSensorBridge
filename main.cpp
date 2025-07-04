#include <csignal>
#include <iostream>
#include <ostream>
#include <streambuf>
#include "Udp.hpp"
#include "Message.hpp"
#include "MessagePsqlStorage.hpp"

// 1. A buffer that swallows all output.
class NullBuffer final : public std::streambuf {
protected:
    // Called for each character that would be put to the buffer.
    int_type overflow(int_type c) noexcept override {
        return traits_type::not_eof(c);          // signal “success”
    }
    // Speed-up for block writes (e.g. std::string).
    std::streamsize xsputn(const char*, std::streamsize n) noexcept override {
        return n;                                // report “all written”
    }
};

// 2. A global null stream you can pass anywhere an ostream& is required.
inline std::ostream& null_stream()
{
    static NullBuffer   sink;
    static std::ostream out(&sink);
    return out;
}



bool inloop = true;

void signalHandler(int signum)
{
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    // Cleanup and close up stuff here
    // Terminate program
    inloop = false;
}

int main(int argc, char* argv[])
{
    // Register signal handler for CTRL+C
    signal(SIGINT, signalHandler);

    // Example usage of Udp class
    const char* env_dbPasswd = std::getenv("DB_PASSWD");
    if(env_dbPasswd == nullptr || *env_dbPasswd == '\0')
    {
        std::cerr << "No DB_PASSWD environment variable set." << std::endl;
        return 1;
    }

    const char* env_host = std::getenv("DB_HOST");
    if(env_host == nullptr || *env_host == '\0')
    {
        std::cerr << "No DB_HOST environment variable set." << std::endl;
        return 1;
    }

    const char* env_name = std::getenv("DB_NAME");
    if(env_name == nullptr || *env_name == '\0')
    {
        std::cerr << "No DB_NAME environment variable set." << std::endl;
        return 1;
    }

    
    const char* logLevel = std::getenv("LOG_LEVEL");
    bool debugLog = (logLevel && std::string(logLevel) == "DEBUG");

    std::cout << "Log level specified: " << (debugLog ? "DEBUG" : "INFO") << std::endl;


    Udp udp(5005, nullptr, std::cerr, std::cout, std::cout, null_stream());
    MessagePsqlStorage storage(env_name, "admin", env_dbPasswd, env_host, 5432);

    // Previous values for change detection
    int prevMCUt_i = 0, prevCWU_i = 0, prevCWUh_i = 0, prevCWUc_i = 0, prevCOh_i = 0, prevCOc_i = 0, prevF1 = 0, prevF2 = 0;

    while(inloop)
    {
        char msgBuff[4096];
        int msgSize = sizeof(msgBuff);
        char remoteHost[16];
        uint16_t remotePort;
        int result = udp.Read(msgBuff, &msgSize, remoteHost, sizeof(remoteHost), true, &remotePort);

        if(result > 0)
        {
            double MCUt, CWU, CWUh, CWUc, COh, COc;
            char API[32];
            int f1, f2;

            sscanf(msgBuff,
                "{MCUt:%lf}{CWU:%lf}{CWUh:%lf}{CWUc:%lf}{COh:%lf}{COc:%lf}{API:%31[^}]}{f1:%d}{f2:%d}",
                &MCUt, &CWU, &CWUh, &CWUc, &COh, &COc, API, &f1, &f2);

            // Multiply by 1000 and store as int
            int MCUt_i = (int)(MCUt * 100);
            int CWU_i = (int)(CWU * 100);
            int CWUh_i = (int)(CWUh * 100);
            int CWUc_i = (int)(CWUc * 100);
            int COh_i = (int)(COh * 100);
            int COc_i = (int)(COc * 100);

            const uint8_t SENDER_ID = 2;
            const uint8_t RECEIVER_ID = 0;

            // Only store if value changed
            if (MCUt_i != prevMCUt_i) {
                if (debugLog) {
                    std::cout << "[DEBUG] MCUt changed: " << prevMCUt_i << " -> " << MCUt_i << std::endl;
                }
                Message msgMCUt(Message::MCU_temp, MCUt_i);
                msgMCUt.idSender = SENDER_ID;
                msgMCUt.idTarget = RECEIVER_ID;
                storage.StoreMessage(msgMCUt);
                prevMCUt_i = MCUt_i;
            }
            if (CWU_i != prevCWU_i) {
                if (debugLog) {
                    std::cout << "[DEBUG] CWU changed: " << prevCWU_i << " -> " << CWU_i << std::endl;
                }
                Message msgCWU(Message::CWU_temp, CWU_i);
                msgCWU.idSender = SENDER_ID;
                msgCWU.idTarget = RECEIVER_ID;
                storage.StoreMessage(msgCWU);
                prevCWU_i = CWU_i;
            }
            if (CWUh_i != prevCWUh_i) {
                if (debugLog) {
                    std::cout << "[DEBUG] CWUh changed: " << prevCWUh_i << " -> " << CWUh_i << std::endl;
                }
                Message msgCWUh(Message::CWUh_temp, CWUh_i);
                msgCWUh.idSender = SENDER_ID;
                msgCWUh.idTarget = RECEIVER_ID;
                storage.StoreMessage(msgCWUh);
                prevCWUh_i = CWUh_i;
            }
            if (CWUc_i != prevCWUc_i) {
                if (debugLog) {
                    std::cout << "[DEBUG] CWUc changed: " << prevCWUc_i << " -> " << CWUc_i << std::endl;
                }
                Message msgCWUc(Message::CWUc_temp, CWUc_i);
                msgCWUc.idSender = SENDER_ID;
                msgCWUc.idTarget = RECEIVER_ID;
                storage.StoreMessage(msgCWUc);
                prevCWUc_i = CWUc_i;
            }
            if (COh_i != prevCOh_i) {
                if (debugLog) {
                    std::cout << "[DEBUG] COh changed: " << prevCOh_i << " -> " << COh_i << std::endl;
                }
                Message msgCOh(Message::COh_temp, COh_i);
                msgCOh.idSender = SENDER_ID;
                msgCOh.idTarget = RECEIVER_ID;
                storage.StoreMessage(msgCOh);
                prevCOh_i = COh_i;
            }
            if (COc_i != prevCOc_i) {
                if (debugLog) {
                    std::cout << "[DEBUG] COc changed: " << prevCOc_i << " -> " << COc_i << std::endl;
                }
                Message msgCOc(Message::COc_temp, COc_i);
                msgCOc.idSender = SENDER_ID;
                msgCOc.idTarget = RECEIVER_ID;
                storage.StoreMessage(msgCOc);
                prevCOc_i = COc_i;
            }
            if (f1 != prevF1) {
                if (debugLog) {
                    std::cout << "[DEBUG] f1 changed: " << prevF1 << " -> " << f1 << std::endl;
                }
                Message msgF1(Message::flow1, f1);
                msgF1.idSender = SENDER_ID;
                msgF1.idTarget = RECEIVER_ID;
                storage.StoreMessage(msgF1);
                prevF1 = f1;
            }
            if (f2 != prevF2) {
                if (debugLog) {
                    std::cout << "[DEBUG] f2 changed: " << prevF2 << " -> " << f2 << std::endl;
                }
                Message msgF2(Message::flow2, f2);
                msgF2.idSender = SENDER_ID;
                msgF2.idTarget = RECEIVER_ID;
                storage.StoreMessage(msgF2);
                prevF2 = f2;
            }
            // Print values from Message objects instead of temporary variables
            //printf("MCUt: %d\nCWU: %d\nCWUh: %d\nCWUc: %d\nCOh: %d\nCOc: %d\nAPI: %s\nf1: %d\nf2: %d\n",
            //    MCUt_i, CWU_i, CWUh_i, CWUc_i, COh_i, COc_i, API, f1, f2);
        }
        else if (result < 0)
        {
            std::cerr << "Error reading message." << std::endl;
        }
    }
}