#include <RGBmatrixPanel.h>
#include <Ethernet.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>         //einbinden der entsprechenden Libraries
#define CLK  8
#define OE   9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2        //definieren der Ports

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);
                      //Aktivieren des Panels mit den Ports von oben
//---------------------------------


byte mac_addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Mac des Arduinos
IPAddress ip(192,168,178,120);   // IP des Arduinos
IPAddress server_addr(192,168,178,98);  // IP vom MySQL server 
char user[] = "root";              // MySQL Benutzer
char password[] = "";        // MySQL Passwort


char query[] = "SELECT * FROM farben.zaehlen ORDER BY ID DESC LIMIT 1";
                                //Zugriff auf Datenbank
EthernetClient client;  //etabliert Internetverbindung
MySQL_Connection conn((Client *)&client);   //stellt Verbindung zur Datenbank her
              
String Inhalt;      
String Farbe;
String Datum;
String r = "rot";
String b = "blau";
String gr = "grün";
String ge = "gelb";
String o = "orange";      //Variablen zum späteren Programmieren

#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr
                          //wie werden LEDs angesteuert
const char str[] PROGMEM = "Platzhalter";   //Platzhalter für späteren Inhalt
int16_t    textX         = matrix.width(),  //Breite des Displays
           textMin       = sizeof(str) * -12; //Wie viel größer darf der Inhalt sein als Display
                           

void setup() {
  Serial.begin(115200);   //Etablieren des serial Monitor
  while (!Serial); // warten auf den Monitor
  pinMode(12, INPUT); //Input für Knöpfe
  pinMode(7, INPUT); 
  Ethernet.begin(mac_addr,ip);     //Verbindung mit Internet
  Serial.println("Connecting...");  
  delay(5000);
  if (conn.connect(server_addr, 3306, user, password)) {
    delay(1000);   //wenn Verbinung vorhanden, dann Delay (1 sekunde Pause)
  }
  else{
    Serial.println("Connection failed, bitte neustarten");
    while(1);     //sonst Endlosschleife, weil Absturz
    }

matrix.begin();   //Anchalten der Matrix
matrix.setTextWrap(false); // wenn Text länger als Display ist, geht er in selber Zeile weiter
matrix.setTextSize(1);  //Textgröße
}


void loop() {
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);      //Befehle aus Library werden im Programm etabliert 
  cur_mem->execute(query);                //Befehle werden ausgeführt
  column_names *cols = cur_mem->get_columns();  //Spalten aus Datenbank werden ausgelesen
  for (int f = 0; f < cols->num_fields; f++) {    //for-Schleife zum ausgeben der tatsächlichen Anzahl der Spalten
    Serial.print(cols->fields[f]->name);    // s. Monitor gibt Namen der Spalten aus
    if (f < cols->num_fields-1) {       // setze nach jedenm Wort ein Komma
      Serial.print(", "); 
    }
  }
  Serial.println(); 
  row_values *row = NULL;   //reseten der Auslese
  do {
    row = cur_mem->get_next_row();    //spring in die nächste Zeile, wenn du mit einer fertig bist 
    if (row != NULL) { //wenn keine Zeile mehr da ist, hör auf
      for (int f = 0; f < cols->num_fields; f++) {
        Serial.print(row->values[f]); //solange F kleiner ist als Anzahl der Zeilen, lies die nächste aus
        if(f == 1){ Inhalt = row->values[f];}   //wenn f=1, schreib Ausgelesenes in Inhalt
        if(f == 2){ Farbe = row->values[f];}    //funktioniert analog
        if(f == 3){ Datum = row->values[f];} 
        if (f < cols->num_fields-1) {     //setze hinter jeden Wert ein Komma
          Serial.print(", ");
        }
      }
      Serial.println(); //Zeilenumbruch
    }
  } while (row != NULL);      //Ausgabe der Zeileninhalte, solange Zeilen da sind   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  delete cur_mem;   //Library, die die Zeilen und Spalten von der Datenbank asuwählt, wird gelöscht, da zu hohe Prozessor-Anforderungen
Serial.println("");   
Serial.println("Werte für das LED Display: ");
Serial.print("Farbe: ");
Serial.println(Farbe);
Serial.print("Inhalt: ");
Serial.println(Inhalt);
Serial.print("Datum: ");
Serial.println(Datum);    //s. Monitor schreibt Werte aus

//---------------------------------------------------------
  
if(Farbe == b){   //wenn Farbe = blau, Schriftfarbe blau  
matrix.setTextColor(matrix.Color333(0,0,7));  //Schriftfarbe blau
Serial.println("Der Text ist blau");      //serieller Monitor bestätigt
}
else if(Farbe == gr){     //das selbe Spiel mit grün  
matrix.setTextColor(matrix.Color333(0,7,0));  
Serial.println("Der Text ist grün");
}
else if(Farbe == ge){     //gelb
matrix.setTextColor(matrix.Color333(7,7,0));
Serial.println("Der Text ist gelb");
}
else if(Farbe == o){      //orange
matrix.setTextColor(matrix.Color333(7,3,0));  
Serial.println("Der Text ist orange");
}
else if(Farbe == r){     //rot
matrix.setTextColor(matrix.Color333(7,0,0));
Serial.println("Der Text ist rot");
}
else {          //sonst wird keine Farbe erkannt
Serial.println("Keine Farbe erkannt");
}  

matrix.setCursor(textX, 0); //gehe 1. Zeile
matrix.print(Inhalt);   //schreibe Inhalt

matrix.setCursor(textX, 8); //gehe 2. Zeile
matrix.print(Datum);      //schreibe Datum
  
  if((--textX) < textMin) textX = matrix.width();
        //wenn Text 12 Einheiten überragt, dann setze zurück ins Display
if(digitalRead(12) == HIGH){
matrix.fillScreen(0);   //screen wird zurückgesetzt
matrix.setTextColor(matrix.Color333(0,7,0)); //grün
matrix.setCursor(textX, 0);   //Curor auf erste Zeile
matrix.print("OPEN       OPEN       OPEN"); //Open
matrix.setCursor(textX, 8); //Zweite Zeile
matrix.print(Datum);}     //Datum aus Datenbank

if(digitalRead(7) == HIGH){
matrix.fillScreen(0);
matrix.setTextColor(matrix.Color333(7,0,0));
matrix.setCursor(textX, 0);
matrix.print("CLOSE       CLOSE       CLOSE"); 
matrix.setCursor(textX, 8);
matrix.print(Datum);} //das Gleiche mit rot und close

matrix.swapBuffers(false);  //Library
matrix.fillScreen(0);   //Scrren zurück setzten
}
