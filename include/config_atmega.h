#ifndef CONFIG_ATMEGA
#define CONFIG_ATMEGA

#define ISLOGENABLED true

#define adc0 0
#define adc1 1
#define adc2 2
#define adc3 3

// Mux Pins
#define S0 47
#define S1 43
#define S2 41
#define S3 39

// temp sensor
#define maxNoOfTempSensorPerCell 6

// DHT sensor
#define DHTPIN_1 29
#define DHTPIN_2 27
#define DHTPIN_3 25
#define DHTPIN_4 23
#define DHTTYPE_1 DHT22
#define DHTTYPE_2 DHT22
#define DHTTYPE_3 DHT22
#define DHTTYPE_4 DHT22

// change it according to the relay configuration
// what signal level it is required to keep the relay on? is it high(true) or low(false)
#define relay_on false // active low pin
#define relay_off true // active low pin

// change it accordingly
// whether the charging/discharging is connected on NC/NO of relay
#define relay_cell_charge relay_on
#define relay_cell_discharge relay_off

#define SD_card_module_cs 53

// charging-discharging method
#define ConstantCurrentCharge 1
#define ConstantCurrentDischarge 2
#define ConstantResistanceCharge 3
#define ConstantResistanceDischarge 4
#define ConstantPowerCharge 5
#define ConstantPowerDischarge 6
#define DriveCycle 7
#define Rest 8
#define Hold 9

#define N_CELL_CAPABLE 6

// drive cycle
#define DriveCycleBatchSize 20

#define MAX_EXP_NAME_LENGTH 10

// exp status
#define EXP_NOT_STARTED 0
#define EXP_RUNNING 1
#define EXP_STOPPED 2
#define EXP_FINISHED 3

#endif // CONFIG_ATMEGA