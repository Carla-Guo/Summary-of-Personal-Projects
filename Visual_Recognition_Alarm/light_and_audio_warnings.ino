//Note that the “WT2605C_Player.h” header file seems to have some errors, please be patient, we will fix it as soon as possible


#include <Seeed_Arduino_SSCMA.h>
#include "WT2605C_Player.h"
#include "time.h"
HardwareSerial SerialPort(1);
WT2605C<HardwareSerial> Mp3Player;
SSCMA AI;
const int LED_PIN = D2;
hw_timer_t * time_recognition = NULL;
bool past_recog = false;
bool current_recog = false;
volatile bool recognition_flag = false;
volatile bool lock_flag = false;
unsigned long previousMillis = 0;
const long interval = 5000;  // 5 seconds interval
void IRAM_ATTR set_recognition_flag() {
    recognition_flag = true;
}

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    Serial.begin(9600);
    while(!Serial)
    {
        delay(1000);
    } 
    SerialPort.begin(9600, SERIAL_8N1, 7, 6);
    while(!SerialPort){
        delay(1000);
    }
    AI.begin();
    while(!AI.begin())
    {
        delay(1000);
    }
    Serial.println("wire is ready");
    Mp3Player.init(SerialPort);
    Serial.println("initial finished");
    delay(1000);
    //enable the recognition flag every 1 second
    time_recognition = timerBegin(0, 80, true);
    timerAttachInterrupt(time_recognition, &set_recognition_flag, true);
    timerAlarmWrite(time_recognition, 1000000, true);  // 1 second
    timerAlarmEnable(time_recognition);
}

void loop()
{
    // Handle recognition logic
    if (recognition_flag) {
        recognition_flag = false;
        if (!AI.invoke())
        {
            Serial.println("invoke success");
            Serial.print("perf: prepocess=");
            Serial.print(AI.perf().prepocess);
            Serial.print(", inference=");
            Serial.print(AI.perf().inference);
            Serial.print(", postpocess=");
            Serial.println(AI.perf().postprocess);
            for (int i = 0; i < AI.boxes().size(); i++)
            {
                Serial.print("Box[");
                Serial.print(i);
                Serial.print("] target=");
                Serial.print(AI.boxes()[i].target);
                Serial.print(", score=");
                Serial.print(AI.boxes()[i].score);
                Serial.print(", x=");
                Serial.print(AI.boxes()[i].x);
                Serial.print(", y=");
                Serial.print(AI.boxes()[i].y);
                Serial.print(", w=");
                Serial.print(AI.boxes()[i].w);
                Serial.print(", h=");
                Serial.println(AI.boxes()[i].h);
            }
            current_recog = (AI.boxes().size() > 0);
        }
        else 
        {
            Serial.println("invoke failed");
            current_recog = false;
        }
    }

    // Check for recognition state changes
    if (past_recog != current_recog )
    {
      if(!lock_flag)
      {
        past_recog = current_recog;
          //if no target is detected
        if (!past_recog)
        {
            Mp3Player.stop();
            digitalWrite(LED_PIN, LOW);
            Serial.println("music stop");
        }
            //if the target is detected
        else
        {
            //put the audio you want to play on your SD card and name it "test1.mp3"
            playSong("test1.mp3");
            digitalWrite(LED_PIN, HIGH);
        }
        lock_flag = true;  
      }
        //For less frequent interruptions of the recognition results, the setting is set to read the recognition status every 5 seconds
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval)
      {
          previousMillis = currentMillis;
          lock_flag = false;
      }
    }

}

int playSong(const char* fileName){
     if (index == 0){
         return 0;
     }
    Mp3Player.playSDSong(fileName);
    Serial.println("Play music: " + String(fileName));
    return 1;
}


