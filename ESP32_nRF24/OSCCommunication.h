#ifndef OscCommunication_h
#define OscCommunication_h

#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>


class OscCommunication{
    private:
        WiFiUDP Udp;
        OSCErrorCode error;
        char ssid[15] = "AndroidAP2F88"; // Changed the SSID to the correct value
        char pass[12] = "Ramba2000!"; // Changed the password to the correct value
        unsigned int outPort;
        unsigned int localPort;
    
    public:
        static bool gameState;
        static int Ip1;
        static int Ip2;
        static int Ip3;
        static int Ip4;

    OscCommunication(){
        outPort = 4901;
        localPort = 4900;
    }

    void init(){
        WiFi.begin(ssid, pass); // Use the correct SSID and password
        while (WiFi.status() != WL_CONNECTED){
            delay(500);
        }
        Serial.println("Command Online. IP Address is: ");
        Serial.println(WiFi.localIP());
        Serial.println();

        Udp.begin(localPort);
    }

    void Update(){
        OSCMessage msg;
        int size = Udp.parsePacket();
        if (size > 0){
            while (size--){
                msg.fill(Udp.read());
            }
            if (!msg.hasError()){
                msg.dispatch("/start", start);
                msg.dispatch("/stop", stop);
                msg.dispatch("/setIP", setIpAddress);
            }
        }
    }

    static void stop(OSCMessage &msg){
        gameState = false;
    }

    static void start(OSCMessage &msg){
        gameState = true;
    }

    static void setIpAddress(OSCMessage &msg){
        Ip1 = msg.getInt(0);
        Ip2 = msg.getInt(1);
        Ip3 = msg.getInt(2); 
        Ip4 = msg.getInt(3);
    }

    void send(const char* address, int _msg){
        OSCMessage msg(address);
        msg.add(_msg);
        IPAddress outIp(Ip1, Ip2, Ip3, Ip4);
        Udp.beginPacket(outIp, outPort);
        msg.send(Udp);
        Udp.endPacket();
        msg.empty();
    }

    void sendModuleStates(int Errors, int Modules){
        send("/errors",Errors);
        send("/modules", Modules);
    }

};

int OscCommunication::Ip1 = 192;
int OscCommunication::Ip2 = 168;
int OscCommunication::Ip3 = 127;
int OscCommunication::Ip4 = 129;
bool OscCommunication::gameState = false;
#endif