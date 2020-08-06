// Setup sensors
typedef struct
  {
      String name;
      String path;
      String unit;
      double value;
      double oldValue;
      bool isActive;
  } Sensors;

#define NUM_SENSORS 10

#define FRESHWATER 0
#define BLACKWATER 1
#define WATERTEMP 2
#define AIRTEMP 3
#define BATTERY1V 4
#define BATTERY2V 5
#define BATTERY1A 6
#define BATTERY2A 7
#define FRESHWATERRAW 8
#define BLACKWATERRAW 9

Sensors sensors[NUM_SENSORS] = {
  { "FreshWater","vessels/self/tanks/freshWater/1/currentLevel", "ratio", 0.0, 0.0, true },
  { "BlackWater","vessels/self/tanks/blackWater/1/currentLevel", "ratio", 0.0, 0.0, true },
  { "WaterTemp","vessels/self/environment/water/temperature", "C", 0.0, 0.0, false   },
  { "AirTemp","vessels/self/environment/outside/temperature", "C", 0.0, 0.0, true   },
  { "Battery 1 V","vessels/self/electrical/batteries/0/voltage", "V", 0.0, 0.0, true   },
  { "Battery 2 V","vessels/self/electrical/batteries/1/voltage", "V", 0.0, 0.0, true   },
  { "Battery 1 A","vessels/self/electrical/batteries/0/current", "A", 0.0, 0.0, false  },
  { "Battery 2 A","vessels/self/electrical/batteries/1/current", "A", 0.0, 0.0, false},
  { "FreshWaterRaw","vessels/self/sensor/SensorStern/freshWater/1/currentLevel", "ratio", 0.0, 0.0, true   },
  { "BlackWaterRaw","vessels/self/sensor/SensorStern/blackWater/1/currentLevel", "ratio", 0.0, 0.0, true   },
};

// Temp sensors
OneWire  dstempwater(WaterTempPin);  // on pin 10 (a 4.7K resistor is necessary)
DallasTemperature tempsensors(&dstempwater);

//Filters
ExponentialFilter<float> FreshWaterFilter(15, 0);
ExponentialFilter<float> BlackWaterFilter(15, 0);
//ExponentialFilter<float> DieselFilter(20, 0);

Adafruit_ADS1115 adsbattery(0x48) ;
Adafruit_ADS1115 adstank(0x49) ;

// Method to setup sensors, set filters etc
void setupSensors() {
  tempsensors.begin();
  adsbattery.begin();
  adstank.begin();
  adstank.setGain(GAIN_FOUR);

  // To make filters work correctly, and not start at zero
  int WaterValue=0;

  for (size_t i = 0; i < 40 ; i++){
     WaterValue = analogRead(FreshWaterPin);
     FreshWaterFilter.Filter(WaterValue);
     WaterValue = analogRead(BlackWaterPin);
     BlackWaterFilter.Filter(WaterValue);
  }

  Serial.println("Tempsensor :");
  DeviceAddress sensoradress;
  for (size_t i = 0; i < tempsensors.getDeviceCount(); i++)
  {
   Serial.print("TempSensor ");
   Serial.print(i);
   tempsensors.getAddress(sensoradress,i);
   Serial.println();
  }
  
}

void SetTankLevel(){
  int16_t adc0, adc1;
  int water,septi;
  adc0 = adstank.readADC_SingleEnded(0);
  adc1 = adstank.readADC_SingleEnded(1);
  
  water = map(adc0, 50, 7300, 100, 0);
  septi = map(adc1, 12, 7300, 100, 0);

  sensors[FRESHWATER].value = double(water);
  sensors[FRESHWATERRAW].value = double(adc0);

  sensors[BLACKWATER].value = double(septi);
  sensors[BLACKWATERRAW].value = double(adc1);
}

void SetFreshWater(){
    int WaterValue = analogRead(FreshWaterPin);
  FreshWaterFilter.Filter(WaterValue);
   WaterValue = FreshWaterFilter.Current();
    int outputWater = map(WaterValue, 1910, 2150, 0, 100);
    //int outputWater = map(WaterValue, 1910, 2070, 0, 100);
    //int outputWater = map(WaterValue, 0, 4096, 0, 100);
    //int outputWater = WaterValue ;
    if(outputWater<0){
      outputWater=0;
    }else if (outputWater>100){
      outputWater=100;
    }
    sensors[FRESHWATER].value = double(outputWater);
    sensors[FRESHWATERRAW].value = double(WaterValue);
}

void SetBlackWater(){
    int WaterValue = analogRead(BlackWaterPin);
    BlackWaterFilter.Filter(WaterValue);
   WaterValue = BlackWaterFilter.Current();
    int outputWater = map(WaterValue, 1927, 2092, 0, 100);
    //int outputWater = map(WaterValue, 0, 4096, 0, 100);
    //int outputWater = WaterValue ;
    if(outputWater<0){
      outputWater=0;
    }else if (outputWater>100){
      outputWater=100;
    }
    // Turn blackwater
    //outputWater = 100 - outputWater;
    sensors[BLACKWATER].value = double(outputWater);
    sensors[BLACKWATERRAW].value = double(WaterValue);
}

// Read water temp via one wire 
void SetWaterTemp(){
  double watertemp =0.00;
  //if(tempsensors.isConnected(0)){
  tempsensors.requestTemperatures();
   // Set to Kelvin
  float wt=tempsensors.getTempCByIndex(0) ;
//  if (wt =! -127.00){
    watertemp = double(wt) +273.15;
  //}
  //else{
    //Error in getting value, set to 0
   // watertemp = 273.15;
  //}
  //sensors[WATERTEMP].value = double(watertemp);
/*
  wt=tempsensors.getTempCByIndex(1) ;
  if (wt =! -127.00){
    watertemp = double(wt) +273.15;
  }
  else{
    //Error in getting value, set to 0
    watertemp = 273.15;
  }
*/
  sensors[AIRTEMP].value = double(watertemp);
}

float fmap(float x, float a, float b, float c, float d)
{
      float f=x/(b-a)*(d-c)+c;
      return f;
}

void SetBattery(){
  int16_t adc0, adc1 , adc2, adc3;
  //Read battery 1
  adc0 = adsbattery.readADC_SingleEnded(0); 
  if(adc0<0) adc0=0;
  float output = fmap(adc0, 0, 10053, 0, 20);
  sensors[BATTERY1V].value = double(output);

  adc1 = adsbattery.readADC_SingleEnded(1);
  // Middle 8760 , +20A 4430
  //float output = float(map(adc1, 8750, 4330, 0, 2000)/100);
  output = fmap(adc1, 4330, 8750, 0, 20);
  
  sensors[BATTERY1A].value = 0;
  adc2 = adsbattery.readADC_SingleEnded(2);
  if(adc2<0) adc2=0;
  output = fmap(adc2, 0, 9782, 0, 20);
  sensors[BATTERY2V].value = double(output);
  adc3 = adsbattery.readADC_SingleEnded(3);
  output = fmap(adc1, 8750, 4330, 0, 20);
  sensors[BATTERY2A].value = 0;
  Serial.printf ("ADC0: %d ADC1: %d ADC2: %d ADC3: %d", adc0, adc1, adc2,adc3);
  Serial.println();
  // Temporärt disablad

}

