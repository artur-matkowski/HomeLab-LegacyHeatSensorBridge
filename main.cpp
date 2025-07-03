#include <csignal>
#include <iostream>
#include "Udp.hpp"
#include "Message.hpp"
#include "MessagePsqlStorage.hpp"



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
    const char* env_p = std::getenv("DB_PASSWD");
    if(env_p == nullptr || *env_p == '\0')
    {
        std::cerr << "No DB_PASSWD environment variable set." << std::endl;
        return 1;
    }

    Udp udp(5005);
    MessagePsqlStorage storage("test_db", "admin", env_p, "192.168.74.209", 5432);

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
            int MCUt_i = (int)(MCUt * 1000);
            int CWU_i = (int)(CWU * 1000);
            int CWUh_i = (int)(CWUh * 1000);
            int CWUc_i = (int)(CWUc * 1000);
            int COh_i = (int)(COh * 1000);
            int COc_i = (int)(COc * 1000);

            const uint8_t SENDER_ID = 2;
            const uint8_t RECEIVER_ID = 0;

            // After parsing and converting values:
            Message msgMCUt(Message::MCU_temp, MCUt_i);
            msgMCUt.idSender = SENDER_ID;
            msgMCUt.idTarget = RECEIVER_ID;

            Message msgCWU(Message::CWU_temp, CWU_i);
            msgCWU.idSender = SENDER_ID;
            msgCWU.idTarget = RECEIVER_ID;

            Message msgCWUh(Message::CWUh_temp, CWUh_i);
            msgCWUh.idSender = SENDER_ID;
            msgCWUh.idTarget = RECEIVER_ID;

            Message msgCWUc(Message::CWUc_temp, CWUc_i);
            msgCWUc.idSender = SENDER_ID;
            msgCWUc.idTarget = RECEIVER_ID;

            Message msgCOh(Message::COh_temp, COh_i);
            msgCOh.idSender = SENDER_ID;
            msgCOh.idTarget = RECEIVER_ID;

            Message msgCOc(Message::COc_temp, COc_i);
            msgCOc.idSender = SENDER_ID;
            msgCOc.idTarget = RECEIVER_ID;

            Message msgF1(Message::flow1, f1);
            msgF1.idSender = SENDER_ID;
            msgF1.idTarget = RECEIVER_ID;

            Message msgF2(Message::flow2, f2);
            msgF2.idSender = SENDER_ID;
            msgF2.idTarget = RECEIVER_ID;

            storage.StoreMessage(msgMCUt);
            storage.StoreMessage(msgCWU);
            storage.StoreMessage(msgCWUh);
            storage.StoreMessage(msgCWUc);
            storage.StoreMessage(msgCOh);
            storage.StoreMessage(msgCOc);
            storage.StoreMessage(msgF1);
            storage.StoreMessage(msgF2);



            // Now you have all values as integers
            //printf("MCUt: %d\nCWU: %d\nCWUh: %d\nCWUc: %d\nCOh: %d\nCOc: %d\nAPI: %s\nf1: %d\nf2: %d\n",
            //    MCUt_i, CWU_i, CWUh_i, CWUc_i, COh_i, COc_i, API, f1, f2);
        }
        else if (result < 0)
        {
            std::cerr << "Error reading message." << std::endl;
        }
    }
}