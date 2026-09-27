# LoRa-RF-PCB

![License](https://img.shields.io/badge/license-CERN--OHL--S-blue)
![Hack Club Forge](https://img.shields.io/badge/Hack%20Club-Forge-orange)
![MCU](https://img.shields.io/badge/MCU-RP2040-purple)
![Radio](https://img.shields.io/badge/Radio-SX1262%20LoRa-green)
![PCB](https://img.shields.io/badge/PCB-4--layer-lightgrey)

A custom PCB for long range radio frequency (LoRa) communication, built around an RP2040 and a Semtech SX1262 radio.
<img width="587" height="657" alt="Screenshot 2026-09-21 153059" src="https://github.com/user-attachments/assets/bb423f98-a75b-4b43-9cc4-c00d4f99078e" />



## Overview

I saw a guideline for a LoRa RF devboard in Hack Club Blueprint and decided to make my own. This is a self-contained board — MCU, radio, and flash all on one PCB, powered and programmed over USB-C.

## Schematics Breakdown

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

This was my first time working with a 4-layer PCB, and it showed me why layer count matters — figuring it out took a while, but I learned a lot along the way. I used **Gemini** for help with the schematics and component routing.

From there, I moved on to placing components and routing the PCB myself. 
<img width="681" height="667" alt="Screenshot 2026-03-30 202426" src="https://github.com/user-attachments/assets/05cb9698-a56c-4158-9b60-93e01ff10650" />

RF signal paths need to be straight and length-matched, which made that part of the routing genuinely tricky — it took about 2 days to fully route the board. Once the hardware was done, I wrote the firmware to bring it to life.
<img width="657" height="627" alt="image" src="https://github.com/user-attachments/assets/9edbfd49-4605-4430-aa8a-c78076ee55db" />

## Getting It Built

The RP2040, SX1262, and flash chip are all fine-pitch QFN parts, so this isn't a hand-soldering job — it needs PCBA (assembled by the fab) rather than a bare PCB.

## AI Use

Being upfront about it: I used Gemini for help with component selection and schematic routing, did the PCB design and routing myself, and used Claude to help me write readme.

## License

Released under [CERN-OHL-S](https://ohwr.org/cern_ohl_s_v2.txt) — feel free to fork it, fab it, and build on it.

Thanks for taking the time to look through this one — it was one of the more challenging boards I've built so far, and I learned a ton doing it. If you spot something worth improving, or you end up building one yourself, I'd genuinely love to hear about it. Feel free to open an issue or reach out.📡
