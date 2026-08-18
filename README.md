<p align="center">
  <img src="pictures/Pill_Dispenser_Super.PNG" alt="Pill Dispenser" width="300">
</p>

# Pill Dispenser Super

Pill Dispenser Super is a smart healthcare IoT device designed primarily for elderly care homes, where medication usage is high and human error poses a significant risk. The device automates medication dispensing, tracks system states, and transmits status messages and alerts wirelessly over a LoRaWAN network to an administrative server.

By ensuring the device position and operation parameters are monitored at all times, the system eliminates the need for the patient or caretaker to track dosing schedules manually—requiring only periodic refilling.

---

## Key Features

* **Automated Dispensing:** Uses motor control and calibration routines to ensure precise position tracking and medication delivery.
* **Non-Volatile State Persistence:** System state and total dispensed pill counts are continuously written to EEPROM, allowing the device to recover seamlessly after a power interruption or restart.
* **LoRaWAN Telemetry:** Transmits status updates, fault alerts, and low-inventory notifications to a designated server over a long-range LoRaWAN network.
* **Error Minimization:** Reduces reliance on human memory, mitigating risks associated with manual pill management.
* **Scalability:** Flexible modular design adaptable for personal home use, care facilities, or industrial dosing systems.

---

## Technical Stack

### Software and Protocols
* **Programming Language:** Embedded C
* **Connectivity Protocol:** LoRaWAN
* **Hardware Interfaces:** I2C, GPIO, Hardware Interrupts, Task Queues
* **Data Storage:** EEPROM read/write routines

### Hardware Components
* Embedded microcontroller unit (MCU)
* Motor assembly and calibration logic
* LoRaWAN communication module
* External EEPROM memory chip

<br>

<p align="center">
  <img src="pictures/LoraWAN_Logic.PNG" width="48%" />
  &nbsp;
  <img src="pictures/EEPROM_LOGIC.PNG" width="48%" />
</p>

---

## System Architecture

The firmware is structured modularly across dedicated source files to ensure readability and maintainability.

1. **Initialization:** Configures standard I/O, GPIO pins, I2C bus, system variables, LoRaWAN module, and interrupt queues.
2. **State Restoration:** Reads saved operational parameters from EEPROM. If the EEPROM data is empty or unitialized, default parameters are loaded.
3. **Execution Loop:** Runs the main state machine. Critical state changes trigger an immediate update to EEPROM and dispatch corresponding LoRaWAN messages.

---

By: Sami, Max, Eeli - Metropolia UAS
