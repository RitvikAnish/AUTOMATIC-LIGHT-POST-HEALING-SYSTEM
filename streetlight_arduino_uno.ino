/*
  Self-Healing Street Light System — Arduino UNO
  ================================================
  Pins:
    A0 → Current Sensor A (Light A)
    A1 → Current Sensor B (Light B)
    D8 → Relay IN (active HIGH)

  Serial Monitor Commands (9600 baud):
    timer <seconds>   — set repair timer (e.g. "timer 300")
    on                — force relay ON  (manual override)
    off               — force relay OFF (clears timer & override)
    status            — print current status
    help              — list commands

  Page auto-prints every 2 seconds.
*/

// ─── Pin Definitions ────────────────────────────────────────────────────────
#define SENSOR_A_PIN  A0
#define SENSOR_B_PIN  A1
#define RELAY_PIN     8

// ─── ACS712 Calibration ─────────────────────────────────────────────────────
// Arduino UNO: 10-bit ADC (0–1023), 5V reference → 4.887 mV/bit
// ACS712-5A  : 185 mV/A, midpoint = 2.5V = ~512 counts
// ACS712-20A : change MV_PER_AMP to 100.0
// ACS712-30A : change MV_PER_AMP to 66.0
const int   ADC_MIDPOINT   = 512;    // Zero-current ADC count (2.5V / 4.887mV)
const float MV_PER_AMP     = 185.0;  // mV per Amp — change for your module variant
const float ADC_MV_PER_BIT = 4.887;  // 5000mV / 1023 bits

// ─── Fault Threshold ────────────────────────────────────────────────────────
// Set this to ~half your measured normal current after calibration.
// With LEDs drawing ~50mA normally, 0.02A is a safe fault floor.
const float FAULT_THRESHOLD = 0.02; // Amps — same threshold used for both A and B

// ─── Sampling ───────────────────────────────────────────────────────────────
const int SAMPLES         = 50;    // ADC samples averaged per reading
const int READ_INTERVAL   = 500;   // ms between sensor reads
const int PRINT_INTERVAL  = 2000;  // ms between status prints

// ─── State Variables ────────────────────────────────────────────────────────
float currentA    = 0.0;
float currentB    = 0.0;
bool  faultA      = false;
bool  faultB      = false;
bool  relayOn     = false;

unsigned long repairDurationMs  = 0;
unsigned long relayActivatedAt  = 0;
bool          timerActive       = false;
bool          manualOverride    = false;

unsigned long lastReadTime      = 0;
unsigned long lastPrintTime     = 0;

String serialBuffer = "";  // Accumulates incoming serial characters

// ─── Current Reading (RMS) ──────────────────────────────────────────────────
float readVoltage(int pin) {
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogRead(pin);
    delayMicroseconds(200);
  }
  float avg = (float)sum / SAMPLES;
  return (avg / 1023.0) * 5.0;  // Convert to volts
}

// ─── Relay Control ──────────────────────────────────────────────────────────
void activateRelay(unsigned long durationMs, bool manual) {
  digitalWrite(RELAY_PIN, LOW);
  relayOn          = true;
  relayActivatedAt = millis();
  repairDurationMs = durationMs;
  timerActive      = (durationMs > 0);
  manualOverride   = manual;

  Serial.println(F(">>> RELAY ON"));
  if (manual)       Serial.println(F("    Mode: Manual override"));
  else if (timerActive) {
    Serial.print(F("    Mode: Fault-triggered, timer = "));
    Serial.print(durationMs / 1000);
    Serial.println(F("s"));
  } else            Serial.println(F("    Mode: Fault-triggered (no timer)"));
}

void deactivateRelay() {
  digitalWrite(RELAY_PIN, HIGH);
  relayOn          = false;
  timerActive      = false;
  manualOverride   = false;
  Serial.println(F(">>> RELAY OFF"));
}

// ─── Fault Evaluation ───────────────────────────────────────────────────────
void evaluateFaults() {
  bool prevA = faultA;
  bool prevB = faultB;

  faultA = (currentA < FAULT_THRESHOLD);
  faultB = (currentB < FAULT_THRESHOLD);

  // Print transitions
  if (faultA && !prevA) Serial.println(F("!!! FAULT DETECTED — Light A (current below threshold)"));
  if (faultB && !prevB) Serial.println(F("!!! FAULT DETECTED — Light B (current below threshold)"));
  if (!faultA && prevA) Serial.println(F("    RECOVERY — Light A current restored"));
  if (!faultB && prevB) Serial.println(F("    RECOVERY — Light B current restored"));

  bool anyFault = faultA || faultB;

  // Auto-activate relay on new fault (skip if manual override holds it ON already)
  if (anyFault && !relayOn) {
    activateRelay(repairDurationMs, false);
  }

  // Timer expiry check
  if (timerActive && !manualOverride) {
    unsigned long elapsed = millis() - relayActivatedAt;
    if (elapsed >= repairDurationMs) {
      if (!anyFault) {
        Serial.println(F("    Timer elapsed + fault resolved → Relay OFF"));
        deactivateRelay();
      } else {
        // Fault persists — restart timer for another cycle
        relayActivatedAt = millis();
        Serial.println(F("    Timer elapsed but fault persists → Timer reset"));
      }
    }
  }

  // Auto-off if all faults cleared with no active timer or manual hold
  if (!anyFault && relayOn && !timerActive && !manualOverride) {
    Serial.println(F("    All faults cleared → Relay OFF"));
    deactivateRelay();
  }
}

// ─── Status Print ────────────────────────────────────────────────────────────
void printStatus() {
  unsigned long upSec = millis() / 1000;

  Serial.println();
  Serial.println(F("╔══════════════════════════════════════════╗"));
  Serial.println(F("║   Self-Healing Street Light — Status     ║"));
  Serial.println(F("╠══════════════════════════════════════════╣"));

  // Light A
  Serial.print(F("║  Light A  │ "));
  Serial.print(currentA, 4);
  Serial.print(F(" A │ "));
  Serial.print(faultA ? F("FAULT  ") : F("OK     "));
  Serial.println(F("             ║"));

  // Light B
  Serial.print(F("║  Light B  │ "));
  Serial.print(currentB, 4);
  Serial.print(F(" A │ "));
  Serial.print(faultB ? F("FAULT  ") : F("OK     "));
  Serial.println(F("             ║"));

  Serial.println(F("╠══════════════════════════════════════════╣"));

  // Relay
  Serial.print(F("║  Buffer C │ Relay: "));
  Serial.print(relayOn ? F("ON  ") : F("OFF "));
  Serial.print(F(" │ "));
  if (manualOverride)   Serial.print(F("Manual override  "));
  else if (timerActive) {
    long rem = max(0L, (long)(repairDurationMs / 1000) - (long)((millis() - relayActivatedAt) / 1000));
    Serial.print(F("Timer: "));
    Serial.print(rem);
    Serial.print(F("s left   "));
  } else                Serial.print(F("Auto mode        "));
  Serial.println(F("║"));

  // System
  Serial.print(F("║  System   │ "));
  Serial.print((faultA || faultB) ? F("*** FAULT ***") : F("Normal       "));
  Serial.println(F("                    ║"));

  // Uptime
  Serial.print(F("║  Uptime   │ "));
  Serial.print(upSec / 3600); Serial.print(F("h "));
  Serial.print((upSec % 3600) / 60); Serial.print(F("m "));
  Serial.print(upSec % 60);   Serial.print(F("s"));
  Serial.println(F("                          ║"));

  Serial.println(F("╚══════════════════════════════════════════╝"));
  Serial.println(F("  Commands: timer <sec> | on | off | status | help"));
  Serial.println();
}

// ─── Serial Command Parser ───────────────────────────────────────────────────
void processCommand(String cmd) {
  cmd.trim();
  cmd.toLowerCase();

  if (cmd == "help") {
    Serial.println(F("\n--- Commands ---"));
    Serial.println(F("  timer <sec>  Set repair timer in seconds (e.g. 'timer 300')"));
    Serial.println(F("  on           Force relay ON (manual override, ignores sensors)"));
    Serial.println(F("  off          Force relay OFF (clears timer & override)"));
    Serial.println(F("  status       Print current status now"));
    Serial.println(F("  help         Show this help"));
    Serial.println();

  } else if (cmd.startsWith("timer")) {
    String arg = cmd.substring(5);
    arg.trim();
    long sec = arg.toInt();
    if (sec <= 0) {
      Serial.println(F("[ERR] Usage: timer <seconds>  e.g. 'timer 120'"));
    } else {
      repairDurationMs = (unsigned long)sec * 1000UL;
      // If relay is already ON due to a fault, restart the countdown
      if (relayOn && !manualOverride) {
        relayActivatedAt = millis();
        timerActive      = true;
      }
      Serial.print(F("[OK] Repair timer set to "));
      Serial.print(sec);
      Serial.println(F(" seconds."));
    }

  } else if (cmd == "on") {
    activateRelay(0, true);
    Serial.println(F("[OK] Relay manually forced ON."));

  } else if (cmd == "off") {
    deactivateRelay();
    Serial.println(F("[OK] Relay forced OFF. Auto-logic re-engages on next sensor cycle."));

  } else if (cmd == "status") {
    printStatus();

  } else {
    Serial.print(F("[?] Unknown command: '"));
    Serial.print(cmd);
    Serial.println(F("' — type 'help' for options."));
  }
}

// ─── Setup ──────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  Serial.println(F("\n========================================"));
  Serial.println(F("  Self-Healing Street Light System"));
  Serial.println(F("  Arduino UNO — Ready"));
  Serial.println(F("========================================"));
  Serial.println(F("  A0 = Sensor A (Light A)"));
  Serial.println(F("  A1 = Sensor B (Light B)"));
  Serial.println(F("  D8 = Relay (Buffer Light C)"));
  Serial.println(F("----------------------------------------"));
  Serial.println(F("  Type 'help' for commands"));
  Serial.println();
  // 1. Add a startup settling delay — add this at the END of setup()
  delay(3000);  // Wait 3s for sensors to stabilise before fault logic starts
  Serial.println(F("Sensors settled — fault detection active."));

  // 2. Lower the threshold (or set it based on what Serial Monitor shows)
  const float FAULT_THRESHOLD = 0.005;  // Drop from 0.02 to 0.005
}

// ─── Loop ───────────────────────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  // ── Sensor read cycle ──
  if (now - lastReadTime >= READ_INTERVAL) {
    lastReadTime = now;
    currentA = readVoltage(SENSOR_A_PIN);
    currentB = readVoltage(SENSOR_B_PIN);
    evaluateFaults();
  }

  // ── Periodic status print ──
  if (now - lastPrintTime >= PRINT_INTERVAL) {
    lastPrintTime = now;
    printStatus();
  }

  // ── Non-blocking serial read ──
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialBuffer.length() > 0) {
        processCommand(serialBuffer);
        serialBuffer = "";
      }
    } else {
      serialBuffer += c;
    }
  }
}
