// ==============================================================
// Lick Port System - Test Code
// ==============================================================

/*
* Detects licks and delivers water as reward : Tactile stimulation → Opens the valve for 2 seconds
* This is a prototype; the code should be adapted according to the specific task and desired data.
*/

const int waterValvePin = 8;     // Digital pin controlling water valve
const int lickSensorPin = 13;    // Input pin for lick sensor

byte LickState = LOW;            // State of lick sensor without touch - can be HIGH depending on capacitive sensor type

// Time parameters
const unsigned long waterDuration = 2000;   // Duration to open water valve (ms) => adjust based on desired water amount

unsigned long lastLickTime = 0;          // Last time a lick was detected
unsigned long timerStart = 0;            // Start time for water delivery
unsigned long currentStateChangeTime = 0; // Time when state last changed
unsigned long lastActivityTime = 0;      // Last time any activity occurred

unsigned long waterStartTime = 0;        // Start time for water valve opening
unsigned long waterEndTime = 0;          // End time for water valve opening

// State machine variables
enum class WaterState : uint8_t {WAITING, DISPENSING}; // Define possible water supply states
WaterState currentWaterState = WaterState::WAITING;

unsigned long lickTime[100];  // Array to store lick timestamps (fixed size)
unsigned long waterTime[100]; // Array to store water delivery timestamps

int lickEventCount = 0;       // Counter for number of licks
int waterEventCount = 0;      // Counter for number of water deliveries

// Setup: initialize pins and arrays
void setup() {
    Serial.begin(9600);

    pinMode(waterValvePin, OUTPUT);
    pinMode(lickSensorPin, INPUT_PULLUP);

    // Initialize event counters
    memset(lickTime, 0, sizeof(lickTime));
    memset(waterTime, 0, sizeof(waterTime));

    // Reset state machine
    currentWaterState = WaterState::WAITING;
    lastActivityTime = millis();
}

// Main loop: handle lick detection and water delivery
void loop() {
    unsigned long currentTime = millis();

    switch(currentWaterState) {
        case WaterState::WAITING:
            // Check if a lick occurred
            if(digitalRead(lickSensorPin) == HIGH && ((currentTime - lastLickTime >= 50) || lickEventCount == 0)) { 
                // HIGH = lick detected
                lastLickTime = currentTime;

                lickTime[lickEventCount] = currentTime;
                lickEventCount++;

                Serial.println("Lick detected!");

                currentStateChangeTime = currentTime; 
                currentWaterState = WaterState::DISPENSING; // Change state to DISPENSING
                
                waterStartTime = currentTime;
                digitalWrite(waterValvePin, HIGH); // Open valve
                waterTime[waterEventCount] = currentTime;
                waterEventCount++;
                Serial.println("Water dispensing started");
            }
            break;

        case WaterState::DISPENSING:
            // Close the valve after the set duration
            if(currentTime - waterStartTime >= waterDuration) {
                digitalWrite(waterValvePin, LOW); // Close valve
                waterEndTime = currentTime;
                Serial.println("Water delivery ended");
                currentWaterState = WaterState::WAITING; // Go back to waiting
            }
            break;
    }
}
