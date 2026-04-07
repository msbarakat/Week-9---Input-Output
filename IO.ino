/*
 * COLOUR MATCH GAME
 *
 * A random target colour is displayed on RGB LED 1.
 * The player uses a potentiometer to dial in their guess one channel at a time (Red → Green → Blue).
 * RGB LED 2 shows the guess in real time.
 * A "Next" button cycles between channels.
 * A "Lock" button submits the final answer and scores it.
 */

// ── PIN DEFINITIONS ──
const int potPin  = A0; // Potentiometer wiper — reads 0 to 1023
const int nextBtn = 2;  // Cycles between R, G, B channels
const int lockBtn = 3;  // Submits the final guess

// RGB LED — Guess (shows player's current colour in real time)
// Long leg (GND) goes to ground. PWM pins allow analogWrite.
const int redG = 9, greenG = 10, blueG = 11;

// RGB LED — Target (shows the colour the player must match)
const int redT = 5, greenT = 6, blueT = 7;

// ── GAME VARIABLES ──
int targetR, targetG, targetB; // Randomly generated target colour
int guessR = 0, guessG = 0, guessB = 0; // Player's current guess

// Tracks which channel is being adjusted
// 0 = Red, 1 = Green, 2 = Blue, 3 = Finished (ready to lock)
int currentState = 0;

// ── SETUP ──
void setup() {
  Serial.begin(9600);

  // Buttons use INPUT_PULLUP — no resistor needed
  // Pin reads HIGH normally, LOW when pressed
  pinMode(nextBtn, INPUT_PULLUP);
  pinMode(lockBtn, INPUT_PULLUP);

  // Set all LED pins as outputs
  pinMode(redG, OUTPUT); pinMode(greenG, OUTPUT); pinMode(blueG, OUTPUT);
  pinMode(redT, OUTPUT); pinMode(greenT, OUTPUT); pinMode(blueT, OUTPUT);

  startNewRound();
}

// ── MAIN LOOP ──
void loop() {

  // Read potentiometer and scale from 0–1023 to 0–255 (LED brightness range)
  int val = map(analogRead(potPin), 0, 1023, 0, 255);

  // ── UPDATE ACTIVE CHANNEL ──
  // Only the active channel updates — the others hold their last value
  if (currentState == 0) {
    guessR = val;
  } else if (currentState == 1) {
    guessG = val;
  } else if (currentState == 2) {
    guessB = val;
  }

  // Show the current guess on the Guess LED in real time
  // analogWrite sends a PWM signal (0 = off, 255 = full brightness)
  analogWrite(redG,   guessR);
  analogWrite(greenG, guessG);
  analogWrite(blueG,  guessB);

  // ── NEXT BUTTON ──
  // Advances to the next colour channel
  // Once all three are set, pressing Next wraps back to Red
  if (digitalRead(nextBtn) == LOW) {
    delay(250); // Debounce — prevents a single press registering multiple times

    currentState++;

    if (currentState == 1) {
      Serial.println("─────────────────────────────");
      Serial.println("Adjusting GREEN — turn pot, press button when ready.");
    } else if (currentState == 2) {
      Serial.println("─────────────────────────────");
      Serial.println("Adjusting BLUE  — turn pot, press button when ready.");
    } else if (currentState == 3) {
      Serial.println("─────────────────────────────");
      Serial.println("All colours set! Press LOCK to check accuracy.");
    } else {
      // State exceeded 3 — wrap back to Red so player can refine
      currentState = 0;
      Serial.println("Adjusting RED   — turn pot, press button when ready.");
    }
  }

  // ── LOCK BUTTON ──
  // Submits the guess and scores it — only works after all 3 channels are set
  if (digitalRead(lockBtn) == LOW) {
    delay(250); // Debounce

    if (currentState >= 3) {
      checkScore();
      delay(3000); // Pause so player can read the result
      startNewRound();
    } else {
      // Player tried to lock before finishing all channels
      Serial.println("Finish all 3 colours first!");
    }
  }
}

// ── START NEW ROUND ──
// Generates a new random target colour and resets all state
void startNewRound() {
  // Seed with noise from an unconnected pin for true randomness each round
  randomSeed(analogRead(A5));

  targetR = random(0, 256);
  targetG = random(0, 256);
  targetB = random(0, 256);

  // Display the target colour on LED 1 immediately
  analogWrite(redT,   targetR);
  analogWrite(greenT, targetG);
  analogWrite(blueT,  targetB);

  // Reset guess and channel back to the start
  guessR = 0; guessG = 0; guessB = 0;
  currentState = 0;

  Serial.println("\n\n\n\n");
  Serial.println("────────────────────────────────────────");
  Serial.println("NEW ROUND — match the colour!");
  Serial.print("Target: R="); Serial.print(targetR);
  Serial.print("  G=");       Serial.print(targetG);
  Serial.print("  B=");       Serial.println(targetB);
  Serial.println("─────────────────────────────");
  Serial.println("Adjusting RED   — turn pot, press button when ready.");
}

// ── CHECK SCORE ──
// Compares the player's guess to the target and prints the result
// Uses total difference across all three channels as the score
void checkScore() {
  // Total difference: sum of how far off each channel is (max possible = 765)
  int diff = abs(targetR - guessR) + abs(targetG - guessG) + abs(targetB - guessB);

  Serial.println("─────────────────────────────");
  Serial.print("Final Guess: R="); Serial.print(guessR);
  Serial.print(" G=");             Serial.print(guessG);
  Serial.print(" B=");             Serial.println(guessB);

  // Score thresholds — adjust these to make the game harder or easier
  if      (diff < 30)  Serial.println("PERFECT MATCH!");
  else if (diff < 100) Serial.println("SO CLOSE!");
  else                 Serial.println("NOT QUITE! TRY AGAIN.");

  Serial.println("────────────────────────────────────────");
}