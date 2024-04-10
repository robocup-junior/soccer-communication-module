# Communication modules for RCJ Soccer SuperTeams 2024

![modul photos](./.readme_images/modul_2024.png?raw=true)

## What / Why it is?
To make RCJ Soccer SuperTeam games more manageable for referees and to bring a simple and robust way of robot to robot / robot to referee communication, we would like to introduce these modules. This module does not count to the weight limit.

## How to power it?
The required way to power these modules is to connect GND to the negative (-) of your battery and BAT+ to the positive (+) of your battery without having any kind of switch between those connections when battery voltage is between 5.3 V and 25V.

If your battery voltage is not between 5.3 V and 25V or there is some engineering reason why the first option is not realistic, you can use a 3V3 pin to provide 3.3V. The module must be able to draw at least 500mA at all times.
This module needs to be powered at all times during the match, even when the robot is not on the field (out of bounds, damage), in order to be able to keep a stable communication connection.

## How to use it for communication between robots?
The module is equipped with a display that shows a countdown for the duration of the Robot’s penalty. Teams are allowed to put robots back in the game according to rules when penalty time’s up on the display.

## Not working?
If you have problems or questions you can look into and if nobody has posted your problem yet open a [GitHub issue](https://github.com/robocup-junior/soccer-communication-module/issues/new), post on [the forum](https://junior.forum.robocup.org/c/robocupjunior-soccer/5) or ask on the [RoboCupJunior Discord server](https://discord.gg/45pxMQY4nJ)


## What is new from 2023?
* Main change is in using ESP32C6 = improved connectivity/less interference
* Using OLED display = time down counter for penalization time/ connection/ future proof
* D+,D- used for 2 thinks = uploading program easily= firmware updates at home
                                         = choosing communication canal in super team 00,01,10,11  = strategy in super team with communication finally possible
* Using giro/accelerometer = able to measure robot speed, future proof, collision detection (0.25 cents for ocean of possibilities )
* Power would be able to be provided directly from battery (up to 25V ) = no disconnecting during matches
* Also we will provide simple PCBs acting as hubs so easier testing/ connecting modules for everyone
* Multicolour print, pinout = cool, easier pinout reading, less damaged modules, more cool (it is cheap don't worry)
* On competition = everyone will be required to go to technical control of module before actual games so better games
* Still need to work on:
* Make adapter hub for ev3/spike