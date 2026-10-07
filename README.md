# LoRa-RF-PCB

![License](https://img.shields.io/badge/license-CERN--OHL--S-blue)
![Hack Club Forge](https://img.shields.io/badge/Hack%20Club-Forge-orange)
![MCU](https://img.shields.io/badge/MCU-RP2040-purple)
![Radio](https://img.shields.io/badge/Radio-SX1262%20LoRa-green)
![PCB](https://img.shields.io/badge/PCB-4--layer-lightgrey)

A custom PCB for long range radio frequency (LoRa) communication, built around an RP2040 and a Semtech SX1262 radio.
<img width="587" height="657" alt="Screenshot 2026-09-21 153059" src="https://github.com/user-attachments/assets/bb423f98-a75b-4b43-9cc4-c00d4f99078e" />



## Overview

I saw a guideline for a LoRa RF devboard in Forge Blueprint and decided to make my own. I did made schematics with help of AI and then designed and routed the PCB myself!!

###This PCB contains following components###
|MCU, radio, and flash all on one PCB, powered and programmed over USB-C.|

## Schematics Components 

| Part | Role |
|---|---|
| RP2040 | Microcontroller — runs everything |
| SX1262 | LoRa radio chip — handles the actual RF signal |
| 16MB flash | Stores the firmware |
| USB-C | Power and programming |
| RF switch + filter + antenna | Gets the signal in and out cleanly |
| Crystal oscillators | Reference clocks for the MCU and radio |
| LDO regulator | Power supply |
| Boot / Reset buttons | Standard flashing controls |

<img width="1283" height="836" alt="586520549-c6413717-07d1-4046-96aa-68a0806cc8d6" src="https://github.com/user-attachments/assets/c0fb06eb-0b1e-4000-94c9-bd98ddb23361" />


## Build Journey

This was my first time working with a 4-layer PCB, and it showed me why layer count matters — figuring it out took a while, but I learned a lot along the way. I used **Gemini** for help with the schematics.

From there, I moved on to placing components and routing the PCB myself. 
<img width="681" height="667" alt="Screenshot 2026-03-30 202426" src="https://github.com/user-attachments/assets/05cb9698-a56c-4158-9b60-93e01ff10650" />

RF signal paths need to be straight and length-matched, which made that part of the routing genuinely tricky — it took about 2 days to fully route the board. Once the hardware was done, I wrote the firmware to bring it to life.
<img width="657" height="627" alt="image" src="https://github.com/user-attachments/assets/9edbfd49-4605-4430-aa8a-c78076ee55db" />

## Bill of Materials (BOM)

| Designator(s) | Part | Value / Footprint | Qty |
|---|---|---|---|
| U1 | RP2040 | Microcontroller, QFN-56 | 1 |
| U2 | SX1262IMLTRT | LoRa/FSK Transceiver, QFN-24 | 1 |
| U3 | W25Q128JVS | 16MB SPI Flash, SOIC-8 | 1 |
| U4 | PE4259 | RF Switch, SOT-363 | 1 |
| U5 | AP2112K-3.3 | 3.3V LDO Regulator, SOT-23-5 | 1 |
| USB_C | USB-C Receptacle | USB 2.0, 16-pin | 1 |
| J3 | Pin Socket, 1x04 | For a 0.96" OLED display | 1 |
| AE111 | U.FL Connector | For an external antenna | 1 |
| FL1 | Balun/Filter | Johanson 0900FM15K0039 | 1 |
| Y1 | Crystal | 32MHz (radio ref clock) | 1 |
| Y2 | Crystal | 12MHz (MCU ref clock) | 1 |
| BOOT | Tactile Switch | B3U-1000P | 1 |
| RESET | Tactile Switch | B3U-1000P | 1 |
| LED1 | LED | Blue | 1 |
| L1 | Inductor | 47nH | 1 |
| L2 | Inductor | 9.1nH | 1 |
| L3 | Inductor | 15µH (PA regulator) | 1 |
| R1 | Resistor | 100Ω | 1 |
| R2, R3, R7* | Resistor | 1kΩ | 3 |
| R4, R10 | Resistor | 10kΩ | 2 |
| R5, R6 | Resistor | 5.1kΩ (USB CC) | 2 |
| R8, R9 | Resistor | 27Ω (USB D+/D−) | 2 |
| R11, R12 | Resistor | 4.7kΩ | 2 |
| C1, C8, C9, C13, C14, C17–C23 | Capacitor | 100nF | 12 |
| C11, C12 | Capacitor | 27pF (crystal load) | 2 |
| C15, C16 | Capacitor | 1µF | 2 |
| C24, C25 | Capacitor | 10µF | 2 |
| C2 | Capacitor | 1nF | 1 |
| C3 | Capacitor | 39pF | 1 |
| C4, C5 | Capacitor | 3.3pF (RF matching) | 2 |
| C6 | Capacitor | 47pF | 1 |
| C7 | Capacitor | 47nF | 1 |
| C10 | Capacitor | 470nF (PA regulator) | 1 |

## Getting It Built

The RP2040, SX1262, and flash chip are all fine-pitch QFN parts, so this isn't a hand-soldering job — it needs PCBA (assembled by the fab) rather than a bare PCB.

## AI Use

Being upfront about it: I used Gemini for help with component selection and schematic routing, did the PCB design and routing myself!!.

## License

Released under [CERN-OHL-S](https://ohwr.org/cern_ohl_s_v2.txt) — feel free to fork it, fab it, and build on it.

Thanks for taking the time to look through this one — it was one of the more challenging boards I've built so far, and I learned a ton doing it. If you spot something worth improving, or you end up building one yourself, I'd genuinely love to hear about it. Feel free to open an issue or reach out.📡
