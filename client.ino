#include "BLEDevice.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <WiFi.h>
#define NUM_CH 3

const char* ssid     = "Wifi_test-Gaetano";
const char* password = "PassDiTest1998";
const char* host = "192.168.137.1";

const char PASSWORD[]={'#','*','0','*','#'};
const byte ROWS = 4; //righe del keypad
const byte COLS = 4; //colonne del keypad
//definiamo i simboli sui bottoni della keypad
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {13, 12, 14, 27}; //pin delle righe della keypad
byte colPins[COLS] = {26, 25, 33, 32}; //pin delle colonne dela keypad

//inizializziamo un'istanza della classe keypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

//SDA->D21  SCL->D22
LiquidCrystal_I2C lcd(0x27,16,2); // inizializiamo lcd con: indirizzo I2C, numero di colonne e numero di righe

static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b"); // Il servizio BLE a cui vogliamo connetterci
static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8"); // La caratteristica del servizio BLE a cui siamo interessati

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

WiFiClient client;

static String CANALE[NUM_CH] = {"A1","B2","C3"};


class Prodotto{
  public:
  String id;
  int quantita;
  float prezzo;
  String canale;
  
  Prodotto(String n, int x,float y,String ch){
    id=n;
    quantita=x;
    prezzo=y;
    canale=ch;
  }
  void venduto(){
    quantita--;
  }
};

//la classe di callback associata al client BLE
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

//funzione per connettere il client al server
bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    //creiamo il client
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");
    
    //associamo al client creato un'istanza di callback
    pClient->setClientCallbacks(new MyClientCallback());
    
    // Connessione al server BLE.
    pClient->connect(myDevice);
    Serial.println(" - Connected to server");
    
    // Otteniamo una reference al servizio cercto nel server
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");
    
    //Otteniamo una reference alla caratteristica del servizio remoto
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Leggiamo il valore della caratteristica
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }
    connected = true;
}

//Dichiariamo la callback che viene richiamata quando viene trovato un server bluetooth
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    //Vediamo se il server trovato offre il servizio che ci interessa
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    }
  }
};


int stringStart = 0;
int stringStop = 16;
int previous = 0;

void stampaScroll(String line){


  if((millis()/800)% 1 == 0 && previous != (millis()/800)){
    previous =  (millis()/800);
    lcd.setCursor(0,0);
    lcd.print(line.substring(stringStart,stringStop));
  
  if(stringStart == 0 || stringStop == line.length()){
    int a = millis() + 500;
          while(a != millis()){
          }
   } 
     
     if (stringStop == line.length()) {
        stringStart = 0;
        stringStop = 16;
      } else {
        stringStart++;
        stringStop++;
      }
  }
}


void initProdotti(Prodotto*p[NUM_CH]){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Init product: ");
  for(int i=0;i<NUM_CH;i++){
    lcd.setCursor(14,0);
    lcd.print(CANALE[i]);
    String id="";
    int qnt=0;
    float prz=0;
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print("id: ");
    for(int j=0;j<4;j++){
      lcd.setCursor(4+j,1);
      char customKey;
      do{
        customKey=customKeypad.getKey();
      }while(!(customKey>='0'&&customKey<='9'));
      lcd.write(customKey);
      id=id+customKey;
    }
    if(id!="0000"){
        delay(500);
        lcd.setCursor(0,1);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print("Quantita': ");
        for(int j=0;j<2;j++){
          lcd.setCursor(11+j,1);
          char customKey;
          do{
            customKey=customKeypad.getKey();
          }while(!(customKey>='0'&&customKey<='9'));
          lcd.write(customKey);
          qnt=qnt+(customKey-'0')*pow(10,1-j);
        }
    }else qnt=0;
    
    if (client.connect(host, 80)) {
      String url="GET /init.php?id="+id+"&quantitaDisp="+qnt;
      client.print(url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");
      unsigned long timeout = millis();
      while (client.available() == 0) {
          if (millis() - timeout > 10000) {
              Serial.println(">>> Client Timeout !");
              client.stop();
              return;
          }
      }
      String prezzo="";
      while(client.available()){
          String line=client.readStringUntil('\r');
          Serial.print(line);
          prezzo+=line;
      }
      unsigned int a= prezzo.indexOf("Prezzo=");
      if(a!=-1){
        prezzo=prezzo.substring(a+7);
        prz=prezzo.toFloat();
      }
    }else Serial.println("connection failed");
    p[i]= new Prodotto(id,qnt,prz,CANALE[i]);
  }
  
}

Prodotto* prodotto[NUM_CH];

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("Client");

  //Impostiamo e attiviamo lo scan bluetooth
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  
  //LCD CODE
  lcd.init();
  lcd.backlight();

  //WIFI
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  if (client.connect(host, 80)) {
    String url="GET /reset.php";
    client.print(url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 10000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
        }
    }
  }
  

  //PRODOTTI
  initProdotti(prodotto);
  for(int i=0;i<NUM_CH;i++) Serial.println("Id: "+prodotto[i]->id+"\tQuantità: "+prodotto[i]->quantita+"\tPrezzo: "+prodotto[i]->prezzo);
  lcd.clear();
}

String stringa="";

void loop() {
  char customKey = customKeypad.getKey();

  stampaScroll("SELEZIONARE UN PRODOTTO:");
  
  if(stringa.length()==0){
  lcd.setCursor(0,1);
  } else{
    lcd.setCursor(1,1);
  }
  
  if (customKey!=NO_KEY){
    stringa=stringa+customKey;
    Serial.println(customKey);
    lcd.write(customKey);
    lcd.setCursor(1,1);
  }

  if(stringa.length()>=2){
    delay(1000);

    if(stringa=="A1"||stringa=="B2"||stringa=="C3"){
      Prodotto* p = nullptr;
      int i=0;
      do{
        if(prodotto[i]->canale==stringa) p=prodotto[i];
        i++;
      }while(i<NUM_CH&&p==nullptr);
      if(p!=nullptr){
        if(p->quantita>0){
          lcd.setCursor(6,1);
          lcd.print(String(p->prezzo));
          delay(2000);
          lcd.clear();
          lcd.setCursor(0,0);
          if (doConnect == true){
              if (connectToServer()) {
                Serial.println("We are now connected to the BLE Server.");
                lcd.print("IN EROGAZIONE...");
                stringStart = 0;
                stringStop = 16;
              } else {
                Serial.println("We have failed to connect to the server; there is nothin more we will do.");
                lcd.print("ERRORE");
                stringStart = 0;
                stringStop = 16;
                delay(3000);
              }
            }
        
            if (connected) {
              pRemoteCharacteristic->writeValue(stringa.c_str(), stringa.length());
              p->venduto();
              if (!client.connect(host, 80)) {
                  Serial.println("connection failed");
                  return;
                }
                String url="GET /vendita.php?id="+(p->id);
                client.print(url + " HTTP/1.1\r\n" +
                             "Host: " + host + "\r\n" +
                             "Connection: close\r\n\r\n");
                unsigned long timeout = millis();
                while (client.available() == 0) {
                    if (millis() - timeout > 5000) {
                        Serial.println(">>> Client Timeout !");
                        client.stop();
                        return;
                    }
                }
  
            }else if(doScan){
              BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
            }
        }else {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("ESAURITO");
          stringStart = 0;
          stringStop = 16;
          delay(3000);
        }
        
      }else {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("ERRORE");
        stringStart = 0;
        stringStop = 16;
        delay(3000);
      }
      
    }else if(stringa=="00"){
      lcd.clear();
      lcd.print("PASSWORD RESET:");
      boolean c=true;
      for(int i=0;i<(sizeof(PASSWORD)/sizeof(char));i++){
        lcd.setCursor(i,1);
        do{
          customKey=customKeypad.getKey();
        }while(customKey==NO_KEY);
        lcd.print("*");
        if(customKey!=PASSWORD[i]) c=false;
      }
      if(c) {
        delay(1000);
        ESP.restart();
      }
      else {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("PASSWORD ERRATA");
        stringStart = 0;
        stringStop = 16;
        delay(3000);
      } 
    }else{
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("ERRORE CANALE");
      stringStart = 0;
      stringStop = 16;
      delay(3000);
    }

    stringa="";
    lcd.clear();
  }
}
