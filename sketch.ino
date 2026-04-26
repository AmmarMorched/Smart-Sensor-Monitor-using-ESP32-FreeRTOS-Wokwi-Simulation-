#include <Arduino.h>
#include <DHTesp.h>

// ============================
// Pin Definitions
// ============================
#define DHT_PIN     15
#define LED_PIN      2
#define BUZZER_PIN   4
#define LOAD_PIN    34   // Analog input from potentiometer

// ============================
// Objects
// ============================
DHTesp dht;

// Structure to store sensor data
struct SensorData {
  float temperature;
  float humidity;
  int loadValue;
};

// Queue handle
QueueHandle_t sensorQueue;

// =====================================================
// TASK 1: Read sensors every 500ms
// =====================================================
void SensorTask(void *parameter) {

  SensorData data;

  while (true) {

    // Read DHT22
    TempAndHumidity values = dht.getTempAndHumidity();

    data.temperature = values.temperature;
    data.humidity    = values.humidity;

    // Read potentiometer (0 to 4095)
    data.loadValue = analogRead(LOAD_PIN);

    // Send data to queue
    xQueueSend(sensorQueue, &data, portMAX_DELAY);

    Serial.println("SensorTask: Data sent to queue");

    // Wait 500 ms
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// =====================================================
// TASK 2: Monitor thresholds and activate alarms
// =====================================================
void AlertTask(void *parameter) {

  SensorData receivedData;

  while (true) {

    // Wait until data arrives
    if (xQueueReceive(sensorQueue, &receivedData, portMAX_DELAY)) {

      bool alert = false;

      // Temperature threshold
      if (receivedData.temperature > 30) {
        alert = true;
      }

      // Load threshold
      if (receivedData.loadValue > 3000) {
        alert = true;
      }

      // Activate outputs
      if (alert) {
        digitalWrite(LED_PIN, HIGH);
        digitalWrite(BUZZER_PIN, HIGH);
        Serial.println("ALERT: Threshold exceeded!");
      } else {
        digitalWrite(LED_PIN, LOW);
        digitalWrite(BUZZER_PIN, LOW);
      }

      // Print values
      Serial.print("Temp: ");
      Serial.print(receivedData.temperature);

      Serial.print(" | Humidity: ");
      Serial.print(receivedData.humidity);

      Serial.print(" | Load: ");
      Serial.println(receivedData.loadValue);
    }
  }
}

// =====================================================
// TASK 3: Simulated cloud transmission
// =====================================================
void CloudTask(void *parameter) {

  while (true) {

    Serial.println("CloudTask: Sending data to IoT platform...");

    // Simulate network delay
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

// =====================================================
// Setup
// =====================================================
void setup() {

  Serial.begin(115200);

  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize DHT sensor
  dht.setup(DHT_PIN, DHTesp::DHT22);

  // Create queue (5 elements)
  sensorQueue = xQueueCreate(5, sizeof(SensorData));

  // Create tasks
  xTaskCreate(
    SensorTask,
    "Sensor Task",
    2048,
    NULL,
    3,
    NULL
  );

  xTaskCreate(
    AlertTask,
    "Alert Task",
    2048,
    NULL,
    2,
    NULL
  );

  xTaskCreate(
    CloudTask,
    "Cloud Task",
    2048,
    NULL,
    1,
    NULL
  );
}

void loop() {
  // Empty because FreeRTOS handles everything
}