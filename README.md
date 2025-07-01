# SSDS V1-V2 Test Scripts

## Description

This repository contains a set of data-logging and hardware-test scripts developed for the Space Systems Design Studio (SSDS) Version 1 and Version 2 Chipsat platforms. It includes routines for voltage/current logging, power characterization, multi-sensor logging, LoRa/T-Beam communication tests, and version-specific prototype tests, along with the RF calibration files.

## Repository Structure

The repository is organized as follows: 
- **Multi_V_I_Sensor_Logger/**  
  Scripts for simultaneously logging multiple voltage and current sensor channels.

- **Power_Testing/**  
  Test routines for characterizing power systems under various load conditions.

- **T_Beam_Scripts/**  
  LoRa “T-Beam” module test scripts for recieving 915MHz LoRa Signals from V2.1.X Chipsats. 

- **V1_Breadboard_Test_Scripts/**  
  Breadboard-based test scripts for V1.9.1 Chipsat. Includes testing for Atmega, GPS, IMU, LoRa, and BME280.

- **V2_1_0_Test_Scripts/**  
  Test scripts for V2.1.0 Chipsat. Includes testing for Wio E-5 (MCU), GPS, IMU, LoRa, and BME280.

- **V2_1_1_Test_Scripts/**  
  Updated test scripts for V2.1.1 Chipsat. Includes testing for Wio E-5 (MCU), GPS, IMU, LoRa, and BME280.
  
- **voltage_current_logger/**  
  Continuous voltage and current logging routines, suitable for long-duration data collection.

- **Descent_V2_GPS_1575_600_20.cal**  
  VNA Calibration data for the Version 2 GPS module at 1575 MHz, 600 MHz Span, 20 Segements

- **Descent_V2_LoRa_915_400_10.cal**  
  VNA Calibration data for the Version 2 LoRa module at 915 MHz, 400 MHz Span, 10 Segments

