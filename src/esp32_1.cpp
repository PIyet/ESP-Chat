
#include <Arduino.h>
#include <WiFi.h>
#include "credentials.h"
#include <map>

#define SERIAL_BAUDRATE 115200

#define I2C_SDA 23
#define I2C_SCL 22

#define LCD_I2C_ADDR 0x3f
#define LCD_WIDTH 20
#define LCD_HEIGHT 4

#define KEYBOARD_DATA 19
#define KEYBOARD_CLK  18

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <PS2Keyboard.h>
#include <Streaming.h>


IPAddress server(192,168,42,113);
WiFiClient client;
int port = 8888;
bool opakovani_pripojeni_portu = true; 
String typ_zpravy;
std:: map <IPAddress, String> Ip_to_user;
String text;
String zprava;
char pismeno;
String stringOne = "<\r><\n>";

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_WIDTH, LCD_HEIGHT);
PS2Keyboard keyboard;

void setup(){      
    Serial.begin(115200);
    Serial.begin(SERIAL_BAUDRATE);
    
    Wire.begin(I2C_SDA, I2C_SCL);
    WiFi.mode(WIFI_STA);
    
    lcd.begin(LCD_WIDTH, LCD_HEIGHT);
    lcd.backlight();

    keyboard.begin(KEYBOARD_DATA, KEYBOARD_CLK);

    Serial.println("\n\n\nESP32chat");
    Serial.println("pripojovani... ");

    while (WiFi.status() !=WL_CONNECTED){
        if(WiFi.status()==WL_CONNECT_FAILED||WiFi.status()==WL_CONNECTION_LOST||WiFi.status()==WL_DISCONNECTED)
        {
            WiFi.begin(ssid,password);
            delay(500);
            Serial.print("nelze se pripojit snaha o opakovani... ");
            Serial.println(" ");
        }
        }
        Serial.println("");
        Serial.print("pripojil jsem se k siti ");
        Serial.print(ssid);
        Serial.print(" s IP adresou ");
        Serial.println(WiFi.localIP());
    } //Connecting to wifi


String readStringUntil(char delimiter){   
        String res;
        lcd.clear();
        lcd.setCursor(0, 3);
        while (1){        
            if(Serial.available()){
                char c = Serial.read();
                if(c == delimiter){
                    break;
                } 
                if(c == '\b'){
                 lcd.rightToLeft();
                 lcd.print(' ');
                 Serial.print("\b \b");   
                }
                else{
                    lcd.leftToRight();
                    Serial.print(c);             
                    lcd.print(c);
                    res += c;
                }
            }
        }
        return res;
    }//Writing

String readclient(int n){
    String raw_txt;
    for(int i = 0; i < n; i++){
        while(!client.available()){
        }
        char z = client.read();
        raw_txt += z;            
    }
    return raw_txt;
}
void loop(){ 

        while (!client.connected()){ 
            Serial.print("pripojovani k portu ");
            
            if (client.connect(server,port)){
                Serial.print(port);
                Serial.println("...");
                Serial.print("pripojeno k portu ");
                Serial.print(port);
                Serial.println("");
                delay(500);
                String wmessage = "w";
                for(int w = 0; w < 4; w++){
                    wmessage += '\0';
                }
                client.println(wmessage + "Pavel" + '\0');
                Serial.println("Welcome massage sended");
            }
        }  //Conecting to port
        Serial.setTimeout(99999);
        
        if (Serial.available()){
          String res = readStringUntil('\n');
          Serial.print(res);
          lcd.print(res);
          String message = "m";
          for ( int i = 0; i != 4; i++)
            message += '\0';
          message += res;
          message += '\0';
          client.write(message.c_str(), message.length());
        }
        if (client.available()){
            String raw_txt = readclient(5);
            if(raw_txt[0] == 'k'){
                readclient(1);
            }
            while (raw_txt[0] == 'a'){
                String t = readclient(1);
                if(t[0] == '\0'){
                    break;
                }
                t += readclient(3);
                String name = client.readStringUntil(10);
                IPAddress ip(t[0], t[1], t[2], t[3]);
                Ip_to_user[ip] = name;
                Serial.print("vidim uzivatele ");
                Serial.print(name);
                Serial.print(" s ip addressou ");
                Serial.println(ip); 
            }
            if(raw_txt[0] == 'c'){
                String user = client.readStringUntil('\0');
                Serial.println("Uzivatel " + user + " se pripojil");
            }
            if(raw_txt[0] == 'd'){
                String user = client.readStringUntil('\0');
                Serial.println("Uzivatel " + user + " se odpojil");
            }
            if(raw_txt[0] == 'm'){
                String text = client.readStringUntil('\0');
                IPAddress ip(raw_txt[1], raw_txt[2], raw_txt[3], raw_txt[4]);                
                Serial.print("Uzivatel ");
                Serial.print(Ip_to_user[ip]);
                Serial.print(" poslal zpravu: ");
                Serial.println(text);
            }
            if(raw_txt[0] == 't'){
                String user = client.readStringUntil('\0');
                Serial.println("Uzivatel" + user + "pise...");
            }
        }   
        if(WiFi.status()==WL_CONNECT_FAILED||WiFi.status()==WL_CONNECTION_LOST||WiFi.status()==WL_DISCONNECTED){
            WiFi.begin(ssid,password);
            Serial.print("odpojeno snaha o opetovne pripojeni");
            Serial.println(" ");
        } //reconect
}