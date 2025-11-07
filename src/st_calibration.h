#ifndef ST_CALIBRATION_H
#define ST_CALIBRATION_H

// Calibration functions
void saveCalibrationData(int yaw, int tilt);
void loadCalibrationData(int &yaw, int &tilt);
void startCalibration();

#endif
