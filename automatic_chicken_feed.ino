#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <Servo.h>

// setup firebase dan wifi
#define FIREBASE_HOST "automatic-chicken-feed-a7e2f-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "vjViOfSUM7hl2OoK9Mn2jEgGaPb6Jd2GKsrBPA8t"
#define WIFI_SSID "test"
#define WIFI_PASSWORD "1sampai0"

// deklarasi pin sensor infrared
int pin_IRbawah_1 = 14;
int pin_IRatas_1 = 12;
int isObstacle_1 = HIGH; // diberi logika high yang berarti tidak ada halangan
int isObstacle_2 = HIGH;

// deklarasi pin sensor ultrasonic
const int trigPin = 5;
const int echoPin = 4;
long duration;
float distance;

Servo myServo;//servo object  

void setup() {
  Serial.begin(9600);

  //servo setup
  myServo.attach(2);

  //  setup sensor infrared
  pinMode(pin_IRbawah_1, INPUT);
  pinMode(pin_IRatas_1, INPUT);

  //  setup sensor ultrasonic
  pinMode(trigPin, OUTPUT);    // set pin trig menjadi OUTPUT
  pinMode(echoPin, INPUT);     // set pin echo menjadi INPUT

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

//melakukan pemeriksaan pada bawah wadah 1
void bawah_wadah1() {
  isObstacle_1 = digitalRead(pin_IRbawah_1);
  if (isObstacle_1 == LOW) {
    Serial.println("Bawah Wadah 1 Terisi");
    // set bool value
    Firebase.setBool("Data_wadah/Bawah_wadah_1", true);
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /truth failed:");
      Serial.println(Firebase.error());
      return;
    }
  } else {
    Serial.println("Bawah Wadah 1 Kosong");
    // set bool value
    Firebase.setBool("Data_wadah/Bawah_wadah_1", false);
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /truth failed:");
      Serial.println(Firebase.error());
      return;
    }
  }
}

//melakukan pemeriksaan pada atas wadah 1
void atas_wadah1() {
  isObstacle_2 = digitalRead(pin_IRatas_1);
  if (isObstacle_2 == LOW) {
    Serial.println("Atas Wadah 1 Full");
    // set bool value
    Firebase.setBool("Data_wadah/Atas_wadah_1", true);
    delay(500);
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /truth failed:");
      Serial.println(Firebase.error());
      return;
    }
  } else {
    Serial.println("Atas Wadah 1 Kosong");
    // set bool value
    Firebase.setBool("Data_wadah/Atas_wadah_1", false);
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /truth failed:");
      Serial.println(Firebase.error());
      return;
    }
  }
}

//melakukan pemeriksaan pada tampungan pakan
void tampungan() {
  float jml_pakan;
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);

  //perhitungan untuk dijadikan jarak
  distance = duration * 0.034 / 2;
  if (distance > 30 || distance <= 0)
  {
    Serial.println("Tampungan Tidak Tertutup");
    Firebase.setString("Data_tampungan/Pesan_error", "Tampungan tidak tertutup/sensor error");
    // set int value
    Firebase.setInt("Data_tampungan/isi_tampungan", 0);
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /truth failed:");
      Serial.println(Firebase.error());
      return;
    }
  }
  else {
    jml_pakan = 10 - 10 * distance / 30;
    Serial.print("Jarak = ");
    Serial.println(distance);
    Serial.print("Jumlah pakan = ");
    Serial.println(jml_pakan);
    // set int value
    Firebase.setFloat("Data_tampungan/isi_tampungan", jml_pakan);
    Firebase.setString("Data_tampungan/Pesan_error", "");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /truth failed:");
      Serial.println(Firebase.error());
      return;
    }
  }
}

void katup_1() {
  bool kondisi_buka = Firebase.getBool("Data_jadwal/Jadwal_1");
  bool kondisi_wadah = Firebase.getBool("Data_wadah/Atas_wadah_1");
  if (kondisi_buka == true && kondisi_wadah == false) {
    Serial.println("Katup Terbuka");
    myServo.write(180);
  }
  else {
    Serial.println("Katup Tertutup");
    myServo.write(90);
    Firebase.setBool("Data_jadwal/Jadwal_1", false);
  }
}

void loop() {
  bawah_wadah1();
  atas_wadah1();
  katup_1();
  tampungan();
}
