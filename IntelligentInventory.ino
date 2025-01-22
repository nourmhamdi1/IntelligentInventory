#include <Firebase_ESP_Client.h> 
#include "addons/TokenHelper.h" 
#include "addons/RTDBHelper.h" 
#include <SPI.h> 
#include <MFRC522.h> 
#include <WiFi.h> 
// Insert your network credentials 
#define WIFI_SSID "nomduwifi" 
#define WIFI_PASSWORD "motdepasse" 
// Insert Firebase project API Key 
#define API_KEY "saisirAPI" 
// Insert RTDB URL 
#define DATABASE_URL "https://saisirlelien/" 
// Define Firebase Data object 
FirebaseData fbdo; 
FirebaseAuth auth; 
FirebaseConfig config; 
// RFID Configuration 
#define SS_PIN 5 // SDA du RC522 connecté à GPIO 5 de l'ESP32 
#define RST_PIN 4 // RESET du RC522 connecté à GPIO 22 de l'ESP32 
MFRC522 rfid(SS_PIN, RST_PIN); // Créer un objet RFID 
bool signupOK = false; 
void writeUIDToFirebase(String uid) { 
String entryPath = "/inventory/entries/" + uid; // Chemin pour les entrées UID 
String totalPath = "/inventory/totalEntries";  // Chemin pour le total des cartes scannées 
// Ajouter ou incrémenter l'UID 
if (Firebase.RTDB.getInt(&fbdo, entryPath)) { 
    int currentCount = fbdo.intData(); // Récupère la valeur actuelle 
    if (Firebase.RTDB.setInt(&fbdo, entryPath, currentCount + 1)) { 
      Serial.println("Nombre d'entrées pour l'UID incrémenté avec succès !"); 
    } else { 
      Serial.println("Erreur lors de l'incrémentation de l'UID !"); 
      Serial.println(fbdo.errorReason()); 
    } 
  } else { 
    if (Firebase.RTDB.setInt(&fbdo, entryPath, 1)) { 
      Serial.println("UID ajouté avec succès avec une valeur initiale de 1 !"); 
    } else { 
      Serial.println("Erreur lors de l'ajout de l'UID !"); 
      Serial.println(fbdo.errorReason()); 
    } 
  } 
  // Ajouter ou incrémenter le total des cartes scannées 
  if (Firebase.RTDB.getInt(&fbdo, totalPath)) { 
    int totalEntries = fbdo.intData(); // Récupère la valeur actuelle 
    if (Firebase.RTDB.setInt(&fbdo, totalPath, totalEntries + 1)) { 
      Serial.println("Total des cartes scannées incrémenté avec succès !"); 
    } else { 
      Serial.println("Erreur lors de l'incrémentation du total des cartes scannées !"); 
      Serial.println(fbdo.errorReason()); 
    } 
  } else { 
    if (Firebase.RTDB.setInt(&fbdo, totalPath, 1)) { 
      Serial.println("Total des cartes scannées initialisé à 1 !"); 
    } else { 
      Serial.println("Erreur lors de l'initialisation du total des cartes scannées !"); 
      Serial.println(fbdo.errorReason()); 
    } 
  } 
} 
 
void setup() { 
  Serial.begin(115200); 
  SPI.begin();           // Initialisation du bus SPI 
  rfid.PCD_Init();       // Initialisation du module RFID 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
  Serial.print("Connecting to Wi-Fi"); 
  while (WiFi.status() != WL_CONNECTED) { 
    Serial.print("."); 
    delay(300); 
  } 
  Serial.println(); 
  Serial.print("Connected with IP: "); 
  Serial.println(WiFi.localIP()); 
  Serial.println(); 
 
  // Assign the API key and database URL 
  config.api_key = API_KEY; 
  config.database_url = DATABASE_URL; 
  // Sign up 
  if (Firebase.signUp(&config, &auth, "", "")) { 
    Serial.println("Firebase sign-up successful!"); 
    signupOK = true; 
  } else { 
    Serial.printf("Firebase sign-up error: %s\n", config.signer.signupError.message.c_str()); 
  } 
  // Assign token callback and initialize Firebase 
  config.token_status_callback = tokenStatusCallback; 
  Firebase.begin(&config, &auth); 
  Firebase.reconnectWiFi(true); 
  Serial.println("RFID prêt. Placez une carte..."); 
} 
 
void loop() { 
  // Vérifiez si une carte est présente 
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) { 
    return; 
  } 
 
  // Lire l'UID de la carte 
  String uid = ""; 
  for (byte i = 0; i < rfid.uid.size; i++) { 
    uid += String(rfid.uid.uidByte[i], HEX); 
  } 
  uid.toUpperCase(); // Mettre en majuscules 
  Serial.println("UID lu : " + uid); 
 
  // Écrire l'UID et le total dans Firebase 
  if (signupOK) { 
    writeUIDToFirebase(uid); 
  } 
 
  // Arrêtez la lecture de la carte pour éviter les doublons 
 // rfid.PICC_HaltA(); 
} 
 

}
