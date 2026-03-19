// ==============================================================
// Lick Port System - Calibration Code
// ==============================================================

/*
 * Command format via Serial: START X
 * X = duration in seconds for opening the valve
 * System delivers 100 drops with 2s interval between each
 */

const int waterValvePin = 8;       // Digital pin controlling water valve

// Time parameters
const unsigned long closedDuration = 2000; // 2-second valve closed
const int totalPulses = 100;              // Total number of pulses to deliver

unsigned long openDuration = 2000; // default 2 seconds (ms)
unsigned long stateStartTime = 0;
int pulseCount = 0;

// State machine variables
enum class ValveState : uint8_t {IDLE, OPEN, CLOSED, FINISHED}; // Define valve control states
ValveState currentState = ValveState::IDLE;

String inputCommand = "";  

void setup() {
    Serial.begin(9600);
    pinMode(waterValvePin, OUTPUT);
    digitalWrite(waterValvePin, LOW);   // Ensure valve is closed at startup
    Serial.println("System ready.");
    Serial.println("Type: START X  (X = seconds open)");
}

void loop() {
    unsigned long currentTime = millis();
    
    // ----- SERIAL COMMAND -----
    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\n') {                   // End of command
            inputCommand.trim();            // Remove whitespace

            if (inputCommand.startsWith("START")) { // Command starts with START
                int spaceIndex = inputCommand.indexOf(' '); 
                if (spaceIndex > 0) {
                    int seconds = inputCommand.substring(spaceIndex + 1).toInt(); // Extract number of seconds
                    if (seconds > 0) {
                        openDuration = (unsigned long)seconds * 1000; // Convert to ms
                        pulseCount = 0;                               // Reset pulse count
                        stateStartTime = currentTime;
                        currentState = ValveState::OPEN;
                        Serial.print("Sequence started with ");
                        Serial.print(seconds);
                        Serial.println(" seconds open duration.");
                    }
                }
            }
            inputCommand = "";  // Clear buffer
        } else {
            inputCommand += c;  // Build command string
        }
    }

    // ----- STATE MACHINE -----
    switch (currentState) {
        case ValveState::IDLE: // Waiting for command
            break;

        case ValveState::OPEN:
            digitalWrite(waterValvePin, HIGH); // Open valve
            if(currentTime - stateStartTime >= openDuration) { 
                digitalWrite(waterValvePin, LOW); // Close valve
                stateStartTime = currentTime;
                pulseCount++;
                Serial.print("Pulse ");
                Serial.print(pulseCount);
                Serial.println(" completed");

                if (pulseCount >= totalPulses) {
                    currentState = ValveState::FINISHED;
                    Serial.println("Sequence finished.");
                } else {
                    currentState = ValveState::CLOSED; // Go to closed state for next pulse
                }
            }
            break;

        case ValveState::CLOSED:
            if(currentTime - stateStartTime >= closedDuration) {
                stateStartTime = currentTime;
                currentState = ValveState::OPEN; // Next pulse
            }
            break;

        case ValveState::FINISHED:
            digitalWrite(waterValvePin, LOW); // Ensure valve is closed
            break;
    }
}
