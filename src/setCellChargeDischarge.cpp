#include "config_atmega.h"
#include "functionPrototype.h"
#include "config_const.h"

void setCellChargeDischarge(unsigned char cell_id, bool status)
{
    // connect the cell to either discharger or charger
    // status - relay_cell_charge | relay_cell_discharge
    controlRelay(cell_relay_location[cell_id - 1], status);
}