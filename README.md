# Communication modules for RCJ Soccer SuperTeams 2024

![modul photos](./.readme_images/rcjv3_dimensions.png?raw=true)

## What / Why it is?
To make RCJ Soccer SuperTeam games more manageable for referees and to bring a simple and robust way of robot to robot / robot to referee communication, we would like to introduce these modules. This module does not count to the weight limit.

## How to power it?
The required way to power these modules is to connect GND to the negative (-) of your battery and BAT+ to the positive (+) of your battery without having any kind of switch between those connections when battery voltage is between 5.3 V and 25V.

If your battery voltage is not between 5.3 V and 25V or there is some engineering reason why the first option is not realistic, you can use a 3V3 pin to provide 3.3V. The module must be able to draw at least 500mA at all times.
This module needs to be powered at all times during the match, even when the robot is not on the field (out of bounds, damage), in order to be able to keep a stable communication connection.

## How to read the start/stop signal?
To get start/stop information one can easily read from pins OUT1 or OUT2 where 3.3V = GO
and 0V = STOP. The robot is required to respond to this stop/go information at all times for
the duration of the game.

## How to use it for communication between robots?
You can use RX, TX for wireless communication between robots using UART. Voltage of
UART logic can be chosen using LOGV pin by connecting required voltage (3.3V - 5.5 V).
You can also choose a communication channel by using A0,A1 pins. Overall, 4 channels are
available (00, 01, 10, 11). Those pins can accept both 3.3V and 5V logic voltag

## How to put robots back in the game?
The module is equipped with a display that shows a countdown for the duration of the
Robot’s penalty. Teams are allowed to put robots back in the game according to rules when
penalty time’s up on the display.

## How to mount it?
Module must be mounted on the robot so that it can be easily connected/disconnected and the display must be visible all the time. 
We recommend mounting the module on the top of your robot and making some hub for it either directly on your PCB or using some protoboards, breadboard and so on.
We also can provide a limited number of hub boards that we can give to make it easier for you so you can directly permanently attach this hub to your robot.

![hub photo](./.readme_images/hub_image.png?raw=true)

## Not working?
If you have problems or questions you can look into and if nobody has posted your problem yet open a [GitHub issue](https://github.com/robocup-junior/soccer-communication-module/issues/new), post on [the forum](https://junior.forum.robocup.org/c/robocupjunior-soccer/5) or ask on the [RoboCupJunior Discord server](https://discord.gg/45pxMQY4nJ)


## How to control app
For actions use double clicks (for example double click to robot buton for starting/stoping/penalty robot)
For seting hold buton(for example hold robot buton for conections setings)

For resetinng all timers start and stop all robots

For adding score double click on it
For -1 score hold score number



## Could use some help with:
  * Make adapter hub for ev3/spike
  * Adding more libraries with footprint/schematic/3D for Altium, Eagle, KiKad, EasyEDA, OrcaCAD and more ...
  * Reporting errors/testing
    
## Currently working on:
   * Making a new mobile app 
   * Enabling giro/accelerometer

# Hall of fame
* App by Mato Faltus
* Altium lib by Youssef Shaalan