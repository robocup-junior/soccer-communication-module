# Communication modules for RCJ Soccer SuperTeams

![cartoon sad kitty](./.readme_images/module_pinout.png?raw=true)

## What / Why it is?
To make RCJ Soccer SuperTeam games more manageable for judges and to bring a simple and robust
way of robot to robot / robot to judge communication, we would like to introduce these modules.

## How does it work?
These modules use Bluetooth to connect to the judge's phone, which can send start/stop signals to the
module. So when judges want to stop a specific robot for some reason (out of bounds, start/stop game,
damage...), they can do it easily and instantly. Also, it ensures a more precise timing of penalty time.

## How do I use it?
It is really simple to connect this module. First, you need to supply power to this module, so connect the
GND pin (written on the bottom side of modules – see picture above) to the GND of your robot.

Then for power, you have two options:
1. Connect voltage ranging from 4.8V to 20V to VCC pin.
2. Connect 3.3V directly to 3.3V pin
To get an actual stop/go signal, you also have multiple options:
1. Read voltage on pin 19 or 18 (3.3V = GO , 0V = STOP) (the recommended way)
2. Connect UART (RX, TX) ("G" = GO, "S" = STOP), default voltage of UART is 3.3V. If you need
something different, connect that specific voltage to LOG V pin (for example, 5V, 1.8V...).

## How do I test / where will I get this module?
Our aim is to play a full module-enabled SuperTeam game on Saturday. We hence ask you to validate that
your robots can work with them until the end of day on Friday. To test your robot, simply come to the Soccer
part of the OC Office and ask for a Communication Module test – We would be happy to help!

## What happens if I destroy this module?
We will be very sad, and this cat may cry.

<img src="https://github.com/robocup-junior/soccer-communication-module/raw/master/.readme_images/cat.png?raw=true" width="200" />

## Not working?
In case of any questions, do not hesitate to ask the judges for help. Or if there are some more unexpected
problems, you can text on whatsapp / call me at +421917888170 we will figure something out (: .
