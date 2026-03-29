# 🛸 Flying Fan

**Flying Fan** is an intelligent ventilation system powered by the **Atmega328p** microcontroller, designed to redefine human-machine interaction through touchless control. This project was developed for the **Embedded Systems** course at the Faculty of Automation, Computers and Electronics, University of Craiova.

The name is a playful nod to the **"Flying Fish"** IR proximity sensors used for gesture detection, suggesting the ease with which a user can "direct" the airflow with a simple hand wave.

---

## 🚀 Key Features

* **Zero Arduino Libraries**: The system utilizes direct register manipulation for maximum control and efficiency.
* **Gesture Recognition**: Interfaces with 4 IR sensors to interpret spatial movements (Up, Down, Left, Right).
* **I2C Display Integration**: Uses a 1602 LCD with an I2C adapter (PCF8574) for real-time status and speed feedback.
* **Hardware PWM Control**: Precise fan speed regulation using Timer1/Timer2 PWM generation.
* **Modular Driver Architecture**: Features documented and reusable drivers for GPIO, I2C, PWM, and Timers.

---

## 🛠️ Project Structure

The repository follows a structured layout to separate hardware abstraction from application logic:

```text
├── bsp/            # Board Support Package (pin mappings for Nano/Uno)
├── drivers/        # Hardware Abstraction Layer
│   ├── i2c/        # TWI driver for LCD communication
│   ├── gpio/       # Sensor input and digital control
│   ├── pwm/        # Fan speed control via PWM
│   └── timer/      # 1ms System Tick (Millis) for gesture timing
├── src/            # Application source code (main.c)
├── utils/          # Bit manipulation helper macros
└── Makefile        # Robust build system for compilation and flashing