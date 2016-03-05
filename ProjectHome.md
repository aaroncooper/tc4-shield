<h1>TC4 Digital Thermometer and Temperature Controller</h1>

<h3>Version 6.00 of the TC4 shield and version 1.33 of the standalone TC4C are the current versions.</h3>

<h3>01-July-2015: <a href='https://tc4-shield.googlecode.com/svn/applications/Artisan/aArtisan/tags/REL-310/aArtisan-V310.zip'>Click here</a> for release 3.10 of aArtisan (same as beta version)<br>
<br>
<h3>25-Nov-2014: <a href='https://tc4-shield.googlecode.com/svn/applications/RoastLoggerTC4/tags/REL-3.1/RoastLoggerTC4_REL-310.zip'>Click here</a> for version 3.10 of RoastLoggerTC4<br>
<br>
16-April-2014:  <a href='http://tc4-shield.googlecode.com/svn/applications/Artisan/aArtisan/tags/REL-300/aArtisan-V300.zip'>Click here</a> for version 3.00 of aArtisan<h3>

<a href='http://www.homeroasters.org/php/news.php'>Homeroasters.org</a> project to develop hardware and software for roast temperature monitoring and control.<br>
<br>
<a href='http://www.mlgp-llc.com/arduino/public/arduino-pcb.html'>Click here</a> for information about ordering TC4 system circuit boards.  PCB's with all surface mount chips pre-soldered, DIY kits, and ready to run TC4C boards are available.<br>
<br>
<h1>TC4C Version 1.33 shown below</h1>
<img src='http://www.mlgp-llc.com/arduino/public/tc4c133-800.jpg' />

<b>Version 1.33 of the TC4C standalone board includes:</b>

<ol><li>High quality ENIG gold surface finish and purple solder mask color<br>
</li><li>Resistor arrays replace individual 1K's (reduced chip count)<br>
</li><li>Eliminated diode that had restricted OT1 and OT2 output voltage<br>
</li><li>Provided 3.3V power source for external components if needed<br>
</li><li>Changed from 1206 to 0805 chips (for consistency)</li></ol>

<b>Version 1.32 of the TC4C standalone board includes:</b>

<ol><li>Reduced base resistor value to increase output capacity on OT1 and OT2<br>
</li><li>Surface mount option for reset diode</li></ol>

<b>Version 1.30 of the TC4C standalone board includes:</b>

<ol><li>One-piece green terminal block for thermocouple connections for greater strength<br>
</li><li>Pads for optional resistors to help deal with ground loop problems<br>
</li><li>Power for OT1 and OT2 is now provided by the TC4C's 5V PSU (no longer selectable)<br>
</li><li>Board and solder mask are now blue<br>
</li><li>Improved sensing of external power with addition of new 3.3V PSU<br>
</li><li>Diode added to address rare reset problems associated with voltage spikes<br>
</li><li>Second I2C header added to allow connection of multiple I2C adapters</li></ol>

<b>Version 1.10 of the TC4C standalone featured:</b>

<ol><li>Fully compatible with TC4 application software (have not tested Kona)<br>
</li><li>Uses the common FTDI chipset for USB-serial conversion.  No compatibility issues with Ubuntu, MacOS, etc.<br>
</li><li>Has the latest optiboot bootloader installed, making it Uno-compatible<br>
</li><li>Implements all features of the TC4 Version 5.20 shield, plus an additional I/O port<br>
</li><li>Sockets provided for connecting a Roving Networks Bluetooth adapter<br>
</li><li>Provision for optional panel-mounted USB connector.<br>
</li><li>Ships pre-programmed with choice of application.<br>
</li><li>Open source hardware and software.</li></ol>

<hr />
<h1>TC4 Shield Version 5.30 shown below</h1>
<img src='http://www.mlgp-llc.com/arduino/public/TC4-530-shield.jpg' />

<b>Version 5.31 of the TC4 shield includes:</b>

<ol><li>Reduced base resistor value to increase output capacity on OT1 and OT2</li></ol>

<b>Version 5.30 of the TC4 shield includes:</b>

<ol><li>One-piece green terminal block for thermocouple connections for greater strength<br>
</li><li>Pads for optional resistors to help deal with ground loop problems<br>
</li><li>Power for OT1 and OT2 is now provided by the Arduino's 5V PSU (no longer selectable)<br>
</li><li>Board and solder mask are now blue to match the Arduino boards</li></ol>

<b>Version 5.20 of the TC4 shield had these improvements:</b>

<ol><li>All electronic components are now surface mount.<br>
</li><li>Through-hole soldering is now required only for headers, switch, and terminals.<br>
</li><li>Power for OT1 and OT2 is now jumper-selectable, 5V or VIN (5V is now default)<br>
</li><li>An 0805 LED power indicator has been added (green)<br>
</li><li>Indicators (red 0805 LED's) have been added for OT1 and OT2<br>
</li><li>I/O2 port has been added (for zero-cross detection and phase angle control)<br>
</li><li>The 8-pin header for parallel LCD interface has been removed (use I2C instead)<br>
</li><li>Boards ordered pre-populated with SMD's are tested and initialized</li></ol>

<hr />
Probably best to think of the TC4 system as programmable interface hardware for multiple thermocouple sensors.<br>
<br>
Currently available Arduino-based applications that use the interface:<br>
<br>
<ol><li><b>RoastLoggerTC4</b> - roast monitor and controller, compatible with RoastLogger software<br>
</li><li><b>aArtisan</b> - interface for the Artisan Visual Scope application<br>
</li><li><b>Bourbon</b> - roast monitor<br>
</li><li><b>Catuai</b> - roast monitor and manual controller</li></ol>

Each of these programs support both standalone operation using an LCD display and/or data logger operation if the TC4 is attached via USB cable to a PC.<br>
<hr />
<b>RoastLoggerTC4</b> is a program that offers the ability to both monitor and control your roaster, using Tom Coxon's powerful RoastLogger software.  When not connected to a computer, the RoastLoggerTC4 program also offers a nice standalone interface which optionally uses a 4-button interface to control heater and fan output on roasters.<br>
<hr />
The <b>aArtisan</b> application is written specifically to allow the TC4 to be used as the datalogger for the Artisan Visual Scope for Coffee Roasters application <a href='http://code.google.com/p/artisan/'>http://code.google.com/p/artisan/</a>.<br>
<hr />
<b>Bourbon</b> is a straightforward monitoring and logging application. When used as part of a standalone system it displays elapsed time, BT, ET, and BT-RoR on a 2-line LCD. Since Bourbon creates and sends a continuous data stream over the USB serial interface, realtime graphic display of roast history as well as CSV format data logging can be done if you connect the TC4 to your computer.<br>
<hr />
<b>Catuai</b> is very similar to Bourbon. In addition to the above, by connecting 1 or 2 potentiometers to the analog input 3-pin headers provided on the TC4 board, you can manually control the heater and/or fan on your roaster.<br>
<br>
Bourbon and Catuai also support an optional 4-button interface for marking roast events (bean drop, first crack, second crack, and eject).<br>
<hr />

Each of the above communicate over a serial connection to a PC, and optionally support a standalone LCD display.  A special I2C LCD board (the <b>LCDapterC</b>) has been developed as part of the project and is supported by each of the applications.<br>
<a href='http://www.mlgp-llc.com/arduino/public/LCDapterCVersion100.pdf'>http://www.mlgp-llc.com/arduino/public/LCDapterCVersion100.pdf</a>