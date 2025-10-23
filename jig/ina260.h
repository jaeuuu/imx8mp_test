#ifndef INA260_H_
#define INA260_H_

void ina260_init(void);
float ina260_get_current(void);
float ina260_get_voltage(void);
float ina260_get_power(void);
int ina260_get_manufacturer_id(void);
int ina260_get_die_id(void);
int ina260_set_curr_limit_over(int limit_mA);
int ina260_set_curr_limit_under(int limit_mA);
int ina260_set_volt_limit_over(int limit_mV);
int ina260_set_volt_limit_under(int limit_mV);
int ina260_set_power_limit_over(int limit_mW);

#endif