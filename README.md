# Vinduino

The Vinduino project started from the necessity to manage irrigation in my small Southern California vineyard, by monitoring soil moisture at different depths and at several vineyard locations. 
Soil moisture monitoring systems have been around for decades, but they cost hundreds to thousands of dollars and -because these systems are proprietary- there will be ongoing cost to customize and maintain these systems. As a small vineyard owner, I needed something low cost and flexible.
The open source Arduino platform, together with low cost gypsum soil moisture and salinity sensors, provides all that. While I first envisioned the Vinduino project (Vineyard + Arduino) for my personal interest and needs, but now the scope has broadened to providing easy to use, open source, low cost solutions for agricultural irrigation management.

Saving water is more important now than ever. The drought that is ongoing  in California for four years, made everybody realize the importance of reducing water use. But it is not just California that is plagued by prolonged drought periods. Large agricultural areas in South America, India, China, and Africa suffer from continuing water shortage as well.

To date, the Vinduino project provided the following results, published on the Vinduino blog, Hackaday, and Github:

* DIY calibrated gypsum soil moisture sensors (Watermark SS200 is also supported)
* Hand held sensor reader (soil moisture, soil/water salinity, water pressure)
* Solar powered remote sensor platform (Vinduino R3)
	Options include:
	4 electrically separated inputs for soil moisture sensors
	Wifi (ESP8266) or Globalsat LM-210 LoRa module for long range (6 miles)
	Irrigation valve control, optional pressure sensor for valve operation feedback
	several options for temperature/humidity sensors
	Built in solar battery charger
	Built in real time clock for precise irrigation timing
* Gateway to connect multiple LoRa end nodes to the Internet via Wifi (Vinduino Gateway)

All publicly released rogramming and documentation is available for free under the GNU General Public License 3.0.

Web site: www.vanderleevineyard.com
