# Arduino audio_switch

**Background:** *I wanted to connect both of my Alexa device's audio output and my TV's audio output to a good pair of studio monitor speakers, which employed a single input plus they lacked remote on/off capabilities. Instead of buying an audio mixer, which is expensive, I built this cheap (about 8-10$) device with an Arduino nano.*

**audio_switch** detects audio signals coming from 2 independent stereo audio inputs and switches the audio output to the currently active one. Input signal sensitivity can be adjusted by a variable resistor. A LED indicator indicates volume level activity/sensitivity. Can optionally connect a second SPDT relay or MOSFET to Arduino digital out pin 12 (as indicated but not fully shown in schematic) to further control the speakers AC power supply when sound input is detected. The device will signal to turn of the speakers when 3 minutes of audio input inactivity is detected on any input channel.
