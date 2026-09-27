# LoRa-RF-PCB

![License](https://img.shields.io/badge/license-CERN--OHL--S-blue)
![Hack Club Blueprint](https://img.shields.io/badge/Hack%20Club-Blueprint-orange)
![MCU](https://img.shields.io/badge/MCU-RP2040-purple)
![Radio](https://img.shields.io/badge/Radio-SX1262%20LoRa-green)
![PCB](https://img.shields.io/badge/PCB-4--layer-lightgrey)

A custom PCB for long range radio frequency (LoRa) communication, built around an RP2040 and a Semtech SX1262 radio.

![Board](https://cdn.hackclub.com/01a0d8c6-af6b-7915-be3b-160d5cecfcc8/Screenshot-202026-09-25-20183436.png)

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

## Build Journey

This was my first time working with a 4-layer PCB, and it showed me why layer count matters — figuring it out took a while, but I learned a lot along the way. I used **Gemini** for help with the schematics and component routing.

From there, I moved on to placing components and routing the PCB myself. RF signal paths need to be straight and length-matched, which made that part of the routing genuinely tricky — it took about 2 days to fully route the board. Once the hardware was done, I wrote the firmware to bring it to life.

## Getting It Built

The RP2040, SX1262, and flash chip are all fine-pitch QFN parts, so this isn't a hand-soldering job — it needs PCBA (assembled by the fab) rather than a bare PCB.

## AI Use

Being upfront about it: I used Gemini for help with component selection and schematic routing, did the PCB design and routing myself, and used Claude to help me write readme.

## License

Released under [CERN-OHL-S](https://ohwr.org/cern_ohl_s_v2.txt) — feel free to fork it, fab it, and build on it.

Thanks for taking the time to look through this one — it was one of the more challenging boards I've built so far, and I learned a ton doing it. If you spot something worth improving, or you end up building one yourself, I'd genuinely love to hear about it. Feel free to open an issue or reach out.📡
