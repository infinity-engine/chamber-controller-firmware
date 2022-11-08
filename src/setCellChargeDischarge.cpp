#include "config_atmega.h"
#ifndef PROTOTYPE
#include "functionPrototype.h"
#endif

#ifndef CONFIG_CONST
#define CONFIG_CONST
#include "config_const.h"
#endif

void setCellChargeDischarge(unsigned char cell_id, bool status)
{
    // connect the cell to either discharger or charger
    // status - relay_cell_charge | relay_cell_discharge
    controlRelay(cell_relay_location[cell_id - 1], status);
}