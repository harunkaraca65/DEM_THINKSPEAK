/*
===============================================================================
 Project: ESP32 ThingSpeak Smart Sender (Auto vs New Setup)
-------------------------------------------------------------------------------
 @brief
   This Arduino sketch allows the user to choose between:
   [O] Auto Connect: Using saved Wi-Fi/API credentials from Flash memory.
   [N] New Setup: Entering new credentials via Serial Monitor.

 @details
   - Uses "Preferences.h" library for persistent storage (replaces EEPROM).
   - Implements a blocking boot menu (waits indefinitely for user input).
   - Validates if saved data exists before attempting auto-connection.
   - Includes secure HTTPS connection to ThingSpeak.

 Author:  Harun Karaca 
 Date:    6-11-2025
===============================================================================
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h> // For saving data to Flash memory

// --- Objects & Constants ---
Preferences preferences; // NVS Storage Handler
const char* PREF_NAMESPACE = "my_app_data"; // Storage namespace

String ssid = "";
String password = "";
String apiKey = "";
String serverName = "https://api.thingspeak.com/update";

// Timers
unsigned long lastTime = 0;
unsigned long timerDelay = 15000; // 15 Seconds

// State Flags
bool isConfigured = false;

// ============================================================
// Helper Functions
// ============================================================

/**
 * @brief Clears the Serial buffer to ensure fresh input reading.
 */
void clearSerialBuffer() {
  while (Serial.available() > 0) {
    Serial.read();
  }
}

/**
 * @brief Reads a string from Serial (Blocking).
 * @param prompt Message to display to user.
 * @param mask If true, hides input with '****' (visual only, Arduino serial monitor echoes anyway).
 */
String readInput(String prompt, bool mask = false) {
  Serial.print(prompt);
  clearSerialBuffer();
  
  while (Serial.available() == 0) {
    delay(10); // Wait for input
  }

  String input = Serial.readStringUntil('\n');
  input.trim(); // Remove whitespace/newlines
  
  if (mask) {
    Serial.println("********");
  } else {
    Serial.println(input);
  }
  return input;
}

/**
 * @brief Validates the ThingSpeak API Key via a dummy HTTP request.
 */
bool testApiKey(String key) {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  String url = serverName + "?api_key=" + key + "&field1=0";
  http.begin(url);
  int httpCode = http.GET();
  http.end();

  if (httpCode == 200) {
    return true;
  } else {
    Serial.print("API Error Code: ");
    Serial.println(httpCode);
    return false;
  }
}

/**
 * @brief Attempts to connect to Wi-Fi with provided credentials.
 */
bool attemptConnection(String w_ssid, String w_pass) {
  Serial.print("Connecting to: ");
  Serial.println(w_ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(w_ssid.c_str(), w_pass.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) { // 10 Seconds timeout
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("Connection Failed.");
    return false;
  }
}

// ============================================================
// Setup & Menu Logic
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Init Preferences
  preferences.begin(PREF_NAMESPACE, false); // false = Read/Write mode

  bool validChoice = false;
  char choice = 0;

  // --- BOOT MENU LOOP ---
  while (!validChoice) {
    Serial.println("\n===================================");
    Serial.println("   BOOT MENU SELECTION");
    Serial.println("   [O] Auto Connect (Load Saved Data)");
    Serial.println("   [N] New Setup (Enter Credentials)");
    Serial.println("===================================");
    Serial.print("Select >> ");

    clearSerialBuffer();
    while (Serial.available() == 0) {
      delay(10); // Wait indefinitely for user input
    }
    
    String inputStr = Serial.readStringUntil('\n');
    inputStr.trim();
    if (inputStr.length() > 0) choice = inputStr.charAt(0);
    else choice = ' ';

    Serial.println(choice); // Echo selection

    // --- OPTION [O]: Auto Connect ---
    if (choice == 'O' || choice == 'o') {
      // Check if keys exist in memory
      if (preferences.isKey("ssid") && preferences.isKey("pass") && preferences.isKey("api")) {
        Serial.println(">> Loading saved credentials...");
        
        ssid = preferences.getString("ssid", "");
        password = preferences.getString("pass", "");
        apiKey = preferences.getString("api", "");

        if (attemptConnection(ssid, password)) {
           Serial.println(">> System Ready (Auto Mode).");
           validChoice = true;
           isConfigured = true;
        } else {
           Serial.println(">> Error: Saved credentials failed. Please perform New Setup [N].");
        }
      } else {
        Serial.println(">> Error: No saved data found! Please perform New Setup [N].");
      }
    }
    // --- OPTION [N]: New Setup ---
    else if (choice == 'N' || choice == 'n') {
      Serial.println("\n--- NEW SETUP WIZARD ---");
      
      bool connected = false;
      while (!connected) {
        ssid = readInput("Enter Wi-Fi SSID: ", false);
        password = readInput("Enter Wi-Fi Password: ", true);

        if (attemptConnection(ssid, password)) {
          connected = true;
        } else {
          Serial.println(">> Retrying Wi-Fi setup...");
        }
      }

      // Get API Key
      apiKey = readInput("Enter ThingSpeak API Key: ", false);
      
      // Optional: Test API Key validity
      Serial.println("Validating API Key...");
      if (testApiKey(apiKey)) {
        Serial.println("API Key Validated! Saving all data...");
        
        // SAVE TO MEMORY
        preferences.putString("ssid", ssid);
        preferences.putString("pass", password);
        preferences.putString("api", apiKey);
        
        validChoice = true;
        isConfigured = true;
      } else {
        Serial.println("Warning: API Key might be invalid, but saving anyway.");
        preferences.putString("ssid", ssid);
        preferences.putString("pass", password);
        preferences.putString("api", apiKey);
        validChoice = true;
        isConfigured = true;
      }
    } 
    // --- INVALID ---
    else {
      Serial.println(">> Invalid selection. Please enter 'O' or 'N'.");
    }
  }
}

// ============================================================
// Main Loop
// ============================================================
void loop() {
  // If setup failed (should be stuck in while loop above, but strictly safely)
  if (!isConfigured) return; 

  // 1. Connection Check (Reconnect if lost)
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi Lost! Attempting reconnect...");
    WiFi.disconnect();
    WiFi.reconnect();
    
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
      delay(500);
      Serial.print(".");
      retries++;
    }
    if (WiFi.status() == WL_CONNECTED) Serial.println("\nReconnected!");
    else Serial.println("\nReconnect failed. Waiting for next loop.");
  }

  // 2. Send Data Periodically
  if ((millis() - lastTime) > timerDelay) {
    // Check WiFi again before sending
    if (WiFi.status() == WL_CONNECTED) {
      
      int val = random(10, 60); // Generate fake sensor data
      Serial.print("Sending Data: " + String(val) + " ... ");

      HTTPClient http;
      String url = serverName + "?api_key=" + apiKey + "&field1=" + String(val);
      
      http.begin(url);
      int httpCode = http.GET();

      if (httpCode == 200) {
        Serial.println("Success (200 OK).");
      } else {
        Serial.print("Error (HTTP ");
        Serial.print(httpCode);
        Serial.println(").");
      }
      http.end();
    }
    
    lastTime = millis(); // Reset timer
  }
}