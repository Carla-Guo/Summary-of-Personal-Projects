#include <Seeed_Arduino_SSCMA.h>
#include "time.h"

SSCMA AI;

#define  LED_PIN  D0
#define USER_BUZZER D6
#define PWM_PERIOD 255

volatile uint16_t buzzerValue = 0;  
hw_timer_t * time_buzzer = NULL;
hw_timer_t * time_recognition = NULL;
bool past_recog = false;
bool current_recog = false;
volatile bool recognition_flag = false;

void IRAM_ATTR set_recognition_flag() {
    recognition_flag = true;
}

void func_buzzer(){
  static uint8_t buzzerCount = 0;
  if (++buzzerCount < buzzerValue) {
    digitalWrite(USER_BUZZER, LOW); 
  }
  if (buzzerCount >= PWM_PERIOD) {
    buzzerCount = 0;
    digitalWrite(USER_BUZZER, HIGH);  
  }
}

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    pinMode(USER_BUZZER, OUTPUT);
    Serial.begin(9600);
    while(!Serial)
    {
        delay(1000);
    } 
    AI.begin();
    while(!AI.begin())
    {
        delay(1000);
    }
    Serial.println("wire is ready");
    Serial.println("initial finished");
    delay(1000);
    
    time_recognition = timerBegin(0, 80, true);
    timerAttachInterrupt(time_recognition, &set_recognition_flag, true);
    timerAlarmWrite(time_recognition, 1000000, true);  
    timerAlarmEnable(time_recognition);
    time_buzzer = timerBegin(2, 80, true);
    timerAttachInterrupt(time_buzzer, &func_buzzer, true);
    timerAlarmWrite(time_buzzer, 10, true);
    timerAlarmEnable(time_buzzer);
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

        past_recog = current_recog;
        if (!past_recog)
        {
            buzzerValue = 0;
            digitalWrite(LED_PIN, LOW);
            Serial.println("nobody here");
        }
        else
        {
            buzzerValue = 128;
            digitalWrite(LED_PIN, HIGH);
            Serial.println("there is someone here");
        }
  
      
    }

}


