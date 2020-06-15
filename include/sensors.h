// Setup sensors
typedef struct
  {
      String name;
      String path;
      String unit;
      double value;
  } Sensors;

#define NUM_SENSORS 7

#define FRESHWATER 0
#define BLACKWATER 1
#define WATERTEMP 2
#define BATTERY1V 3
#define BATTERY2V 4
#define BATTERY1A 5
#define BATTERY2A 6

Sensors sensors[NUM_SENSORS] = {
  { "FreshWater","vessels/self/tanks/freshWater/1/currentLevel", "ratio", 0.0 },
  { "BlackWater","vessels/self/tanks/blackWater/1/currentLevel", "ratio", 0.0 },
  { "WaterTemp","vessels/self/environment/water/temperature", "C", 0.0 },
  { "Battery 1 V","vessels/self/electrical/batteries/1/voltage", "V", 0.0 },
  { "Battery 2 V","vessels/self/electrical/batteries/2/voltage", "V", 0.0 },
  { "Battery 1 A","vessels/self/electrical/batteries/1/current", "A", 0.0},
  { "Battery 2 A","vessels/self/electrical/batteries/2/current", "A", 0.0 },
};

// Temp sensors
OneWire  dstempwater(4);  // on pin 10 (a 4.7K resistor is necessary)
DallasTemperature tempsensors(&dstempwater);

Adafruit_ADS1115 adsbattery ;

void SetFreshWater(){
    int WaterValue = analogRead(34);
    //int outputWater = map(WaterValue, 1870, 1955, 0, 100);
    int outputWater = map(WaterValue, 0, 4096, 0, 100);
    sensors[FRESHWATER].value = double(outputWater);
}

void SetBlackWater(){
    int WaterValue = analogRead(35);
    //int outputWater = map(WaterValue, 1870, 1955, 0, 100);
    int outputWater = map(WaterValue, 0, 4096, 0, 100);e
    sensors[BLACKWATER].value = double(outputWater);
}
// Read water temp via one wire 
void SetWaterTemp(){
   tempsensors.requestTemperatures();
   // Set to Kelvin
  float wt=tempsensors.getTempCByIndex(0) ;
  double watertemp = double(wt) +273.15;
   sensors[WATERTEMP].value = double(watertemp);
}

void SetBattery(){
  int16_t adc0, adc1, adc2, adc3;
  //Read battery 1
  adc0 = adsbattery.readADC_SingleEnded(0);
  sensors[BATTERY1V].value = double(adc0);
  adc1 = adsbattery.readADC_SingleEnded(1);
   sensors[BATTERY1A].value = double(adc1);
}

