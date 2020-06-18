///////////////////////////////////////////////////////////////
// Auto audio input channel switching for Arduino
//

#include <alloca.h>

#define ANALOG_INPUT_AUDIO1 A0
#define ANALOG_INPUT_AUDIO2 A1

#define ANALOG_OUTPUT_RELAY1 13
#define ANALOG_OUTPUT_RELAY2 12
#define ANALOG_OUTPUT_LED 11

#define SAMPLE_RATE 10 // # Samples per sec
#define SAMPLE_PROCESS_COUNT 5 // Num of samples per input channel to collect for analysing average volume
#define SWITCHING_THRESHOLD 3 // Volume threshold to consider a non-silent input channel
#define SILENCE_TURN_OFF_THRESHOLD_SEC 180 // 3 min

#define LED_LOW 16
#define LED_OFF 0
#define LED_HIGH 255

//#define DEBUG // Comment out for production

#define F2A(f) (dtostrf(f, 0, 2, (char*)alloca(16))) // Inline conversion of float to string

// Prototypes, type defs
//
typedef enum State { Off, Input1, Input2 } tagState;

void setState(State state);
inline int readPin(int analogPin) __attribute__((always_inline));
void processChannelVolume(float v1, float v2);
void dbgPrint(const char *fmt, ...);

// Globals
//
uint64_t silenceInterval = 0;
int sampleCount = 0;
int vsum1, vsum2 = 0;
State currentState = State::Off;

// Main code
//

void setup() 
{    
  Serial.begin(9600);
  
  analogReference(EXTERNAL);

  pinMode(ANALOG_OUTPUT_RELAY1, OUTPUT);
  pinMode(ANALOG_OUTPUT_RELAY2, OUTPUT);
  pinMode(ANALOG_OUTPUT_LED, OUTPUT);

  setState(State::Off);  
}

void dbgPrint(const char* fmt, ...)
{
#ifdef DEBUG 
  static char buf[512] = {};

  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  Serial.println(buf);
#endif
}

void setState(State state)
{
  if(state == Off && currentState != State::Off)
  {
    dbgPrint("setState: OFF");
    digitalWrite(ANALOG_OUTPUT_RELAY1, LOW);
    digitalWrite(ANALOG_OUTPUT_RELAY2, LOW);  
  }
  else if(state == Input1 && currentState != State::Input1)
  {
    dbgPrint("setState: Input 1");
    digitalWrite(ANALOG_OUTPUT_RELAY1, LOW);
    digitalWrite(ANALOG_OUTPUT_RELAY2, HIGH);    
  }
  else if(state == Input2 && currentState != State::Input2)
  {
    dbgPrint("setState: Input 2");
    digitalWrite(ANALOG_OUTPUT_RELAY1, HIGH);
    digitalWrite(ANALOG_OUTPUT_RELAY2, HIGH); 
  }
  else
    return;

  currentState = state;
}

inline int readPin(int analogPin)
{
  int val = analogRead(analogPin) - 511;
  return abs(val);
}

void processChannelVolume(float v1, float v2)
{    
    dbgPrint("~\t%s, %s", F2A(v1), F2A(v2));

    if(v1 > SWITCHING_THRESHOLD || v2 > SWITCHING_THRESHOLD) // Input detected
    {                       
      silenceInterval = 0;      
      
      if(v1 >= v2) // Input 1 wins      
        setState(State::Input1);
      else // Input 2 wins  
        setState(State::Input2);

      analogWrite(ANALOG_OUTPUT_LED, LED_HIGH);
    }
    else // Had silence at both inputs
    {                                        
      if(silenceInterval < (SILENCE_TURN_OFF_THRESHOLD_SEC * 1000L))              
        silenceInterval += (1000 * SAMPLE_PROCESS_COUNT) / SAMPLE_RATE;
      else
        setState(State::Off);

      if(currentState == State::Off)
      {
        analogWrite(ANALOG_OUTPUT_LED, LED_LOW);
        delay(10);
        analogWrite(ANALOG_OUTPUT_LED, LED_OFF);
      }
      else
        analogWrite(ANALOG_OUTPUT_LED, LED_LOW);
    }
}

void loop() 
{ 
  int v1 = readPin(ANALOG_INPUT_AUDIO1);
  int v2 = readPin(ANALOG_INPUT_AUDIO2);

  dbgPrint("%4d, %4d", v1, v2);
  
  vsum1 += v1;
  vsum2 += v2;
  sampleCount++;
  
  if(sampleCount >= SAMPLE_PROCESS_COUNT) // Do we have enough samples
  {    
    float avg1 = (float)vsum1 / (float)SAMPLE_PROCESS_COUNT;
    float avg2 = (float)vsum2 / (float)SAMPLE_PROCESS_COUNT;
    
    processChannelVolume(avg1, avg2);   
    
    vsum1 = vsum2 = sampleCount = 0; // reset counters
  } 

  delay(1000 / SAMPLE_RATE);
}
