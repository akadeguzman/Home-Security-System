# Home-Security-System
The project integrates both hardware and software components with the hardware consisting of an embedded system based on Arduino microcontroller (ATmega328) and it will utilize basic home security system that identify hazards like force entry, fire, and flood. This will be all implemented using cost-efficient sensors available in the market.

The smart home security system will consist of **three sub-parts**, the detection of intruder with smart locks and keyless entry, fire detection, and flood detection. 
In detection intruder and keyless entry, it composes of smart locks and motion sensors. This function will also give the user the ability to have keyless entry due to its feature of using PIN code and unique card/tag for entry access. The sensors like the PIR sensors and Reed switch are only enabled if the system is activated. The smart door lock will always be enabled, with or without the system being activated.

Other smart features like fire detection and flood detection are also included in the features of this project. The functions of these sensors are straight forward, they detect fire and water in the area. These sensors are always activated whether the system is enabled or disabled. The system can just be disabled by entering the correct pin.

&nbsp;

## Block Diagram:

![image](https://github.com/user-attachments/assets/b5a31a42-2ef7-47b3-945c-a5196ac75f8f)

&nbsp;
## System Process Flow:

![image](https://github.com/user-attachments/assets/06ece413-77bf-4980-8fcf-6b5c5e1f6950)

![image](https://github.com/user-attachments/assets/08cceff9-9aa3-489f-b9fe-4f39db0eddbe)

&nbsp;
The system will be only activated through a correct input pin which will be created by the user. This pin can also deactivate the system.  Once the system is activated, all the PIR sensor, Reed Switch, Smoke sensor, Flame sensor, Water sensor will be activated. The lock in the doors is always activated whether the system is activated or deactivated. To access the door, there are two options, whether you will use the RFID tag or the pin code that will be entered in the keypad. Once it is verified, it will trigger the servo motor and will unlock the door latch. The green led will then light up and the buzzer will be triggered. The action will then be displayed in the LCD.
The LCD will be located inside the house and will have dedicated keypad on it. It will be called the “Control Hub” of the system. It basically consists of LCD for output and a keypad for input. This will be used for enabling and disabling the security system. This will also display the status of the sensors when they are triggered. 

In the security system, the components that can only be activated and deactivated are the following:
• PIR Sensor
• Reed Switch
• Flame Sensor
• Smoke Sensor
• Water Sensor

The LEDs and the Buzzers can also be turned off through the keypad with LCD if the system is triggered.
	
Every successful input to the door lock or from the control hub will trigger green LEDs and a distinctive buzzer sound for identifying that the entered data is correct.  It is also the same if the entered data is incorrect, it will also produce a distinct error buzzer sound and a red LED will light up.

## Findings, Recommendations and Conclusions:
The system has a decent number of sensors and is very effective, but it can still be improved by integrating an IoT system or a GSM module; it will make it more convenient when accessing the system without manually interacting with it. The recommended inclusion to the system is by integrating wireless communication with the system. It can also benefit the user if they can still be
