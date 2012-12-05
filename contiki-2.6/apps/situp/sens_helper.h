#ifndef _SENS_HELPER_H
#define _SENS_HELPER_H

float getGyroSensitivity(int sens);
float getAccelSensitivity(int sens);
float convert_to_GG(int16_t data, int sens);
float convert_to_AG(int16_t data, int sens);



#endif
