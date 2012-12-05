float getGyroSensitivity(int sens){
  if (sens == 0){
    return 131.0;
  }else if (sens == 1){
    return 65.5;
  }else if (sens == 2){
    return 32.8;
  }else{
    return 16.4;
  }
}

float getAccelSensitivity(int sens){
  if (sens == 0){
    return 16384.0;
  }
  else if (sens == 1){
    return 8192.0;
  }
  else if (sens == 2){
    return 4096.0;
  }
  else{
    return 2048.0;
  }
}

float convert_to_GG(int16_t data, int sens);{
  float sensitivity = getGyroSensitivity(sens);

  return ((float) data)/sensitivity;
}

float convert_to_AG(int16_t data, int sens);{
  float sensitivity = getAccelSensitivity(sens);

  return ((float) data)/sensitivity;
}
