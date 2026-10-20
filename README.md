# AUTOMATIC-LIGHT-POST-HEALING-SYSTEM

# Self-Healing Street Light System Using Fault Detection and Automatic Recovery

An ESP-based embedded prototype that detects a failed street light pole by monitoring its current, **automatically isolates it with a relay**, and keeps the remaining poles running without human intervention. Most existing IoT street lighting systems stop at sending an alert. This one goes a step further: detect, isolate, report, and sustain.

> **Course:** BEEE213L – Embedded Systems Design, School of Electrical Engineering, VIT Chennai (April 2026)
> **Guide:** Sumathi V, Professor, SELECT

## Table of Contents

- [Overview](#overview)
- [Problem Statement](#problem-statement)
- [Objectives](#objectives)
- [Research Gap and Novelty](#research-gap-and-novelty)
- [How It Works](#how-it-works)
- [Hardware Components](#hardware-components)
- [Testing and Results](#testing-and-results)
- [Limitations](#limitations)
- [Future Scope](#future-scope)
- [Repository Contents](#repository-contents)
- [Team](#team)
- [References](#references)
- [License](#license)

## Overview

Street lighting is critical for urban safety, yet conventional systems can't detect or respond to lamp failures on their own. Repairs usually wait for citizen complaints or scheduled inspections, so faults can stay unresolved for days.

In this project each light pole acts as a monitored node. A current sensor watches the load current of each pole, and an ESP microcontroller compares it against a preset threshold. When a pole's current drops (burnt-out bulb or wiring break), the ESP opens that pole's relay, reports the pole number, and keeps monitoring the healthy poles.

## Problem Statement

- Traditional street lighting lacks fault intelligence.
- Manual fault detection causes delayed maintenance.
- Downtime increases public safety risks.
- There is a need for autonomous fault detection and recovery.

## Objectives

1. **Fault detection:** a current sensor detects a failed pole when its load current drops below a preset threshold.
2. **Automatic isolation:** the faulty pole's relay is opened immediately so it does not affect the other poles.
3. **Pole identification:** the faulty pole is reported through the ESP so maintenance teams know where to go.
4. **Continuity:** the remaining poles stay lit, with the segment running at reduced coverage until repair.

## Research Gap and Novelty

Existing IoT street light systems (using LDR, PIR and current sensors with platforms like Blynk, ThingSpeak and TagoIO, and some with GPS-tagged fault maps) handle energy saving and fault **detection** well. But each one stops at notification and waits for maintenance staff.

This project adds:

| Contribution | Description |
|---|---|
| Relay isolation | The ESP electrically disconnects the faulty pole |
| Recovery state | Healthy relays stay closed; the segment keeps operating at reduced coverage |
| Active post-fault control | The ESP keeps monitoring the healthy poles; the system is never idle between detection and repair |
| Single-board integration | One ESP, relay modules and a current sensor, with no secondary processor or cloud required |

## How It Works

1. The ESP continuously reads the current sensor on each pole's load circuit.
2. Under normal operation every pole draws a measurable current.
3. If a pole's current drops below the preset threshold, the ESP flags it as faulty.
4. The ESP **opens that pole's relay**, leaving all other relays closed.
5. The faulty pole number is recorded and reported (over serial in the current prototype).
6. The healthy poles continue to operate until a technician repairs the fault.

An LDR (light sensor) switches the lights on only when ambient light is low. The fault detection logic runs independently of the LDR, so a daytime fault is still detected and isolated.

## Hardware Components

| Component | Purpose |
|---|---|
| ESP microcontroller (ESP32) | Reads sensors, runs detection and isolation logic, reports the pole number |
| Current sensor (ACS712 / INA219) | Measures each pole's load current |
| Voltage sensor module | Voltage monitoring |
| Relay / MOSFET switching circuit | Isolates the faulty pole |
| LED modules (street light models) | Act as the poles |
| LDR | Ambient-light-based switching |
| Backup battery | Backup supply |
| Wi-Fi module (optional) | Optional alerts / dashboard |

## Testing and Results

**Setup:** bench prototype with two LED loads acting as poles, each with its own relay channel and an ACS712 current sensor in series. Faults were introduced while the system was running.

**Test plan:**
- Normal operation
- Single-node failure simulation
- Multiple-failure testing
- Measurement of detection and recovery time
- Live healing demonstration

**Findings:**
- The relay opened on the pole where the fault was introduced.
- Healthy poles kept running without interruption.
- Faults were detected and isolated within about three seconds, with the correct pole number reported.

<img width="1280" height="720" alt="HW_project" src="https://github.com/user-attachments/assets/a9b41352-9c23-4939-adb5-0c993753a34a" />
<img width="1280" height="720" alt="HW_OP" src="https://github.com/user-attachments/assets/2ebf0490-e3d5-44b3-8740-2047885afe9f" />


## Limitations

- Fault location is reported over **serial output only**, with no wireless or GPS reporting.
- The current threshold is **fixed at compile time**, so lamp or supply variations can cause false alarms.
- The logic handles **one fault at a time** per segment.
- Lights run at **fixed brightness** (no dimming).
- Tested at **bench scale** with LED loads, not real street light voltages and currents.
- Powered from a bench supply, not solar or a battery-backed field supply.

## Future Scope

- Wireless fault reporting (GSM / LoRa) with GPS location
- Predictive fault detection using machine learning
- Solar power and battery autonomy
- PIR motion sensing with PWM dimming
- Multi-segment networks with a central controller (e.g., LoRaWAN)
- Mobile app / web dashboard
- Weatherproofing and field trials

## Repository Contents

```
.
├── README.md
├── firmware/                    # ESP code (add your sketch here)
├── docs/
│   ├── 23BEE1018_23BEE1084_Report.docx
│   ├── 23BEE1018_23BEE1084_Paper.docx
│   ├── 23BEE1018_23BEE1084_PPT.pptx
│   └── images/                  # Block diagram, circuit diagram, hardware photos
└── LICENSE
```

Links: [Project report](docs/23BEE1018_23BEE1084_Report.docx) · [Paper](docs/23BEE1018_23BEE1084_Paper.docx) · [Presentation](docs/23BEE1018_23BEE1084_PPT.pptx)

## Team

| Name | Registration No. |
|---|---|
| Ritvik Anish | 23BEE1018 |
| Akshaya N | 23BEE1084 |

**Guide:** Sumathi V, Professor, SELECT

## References

Key related work reviewed in the project (full list in the report and paper):

- Taye et al., *IoT Based Automatic Damaged Street Light Fault Detection and Monitoring System*, IJSRET, 2025
- Anchitaalagammai et al., *IoT Based Automated Street Light Control with Fault Detection and Reporting System*, IEEE ICIRCA, 2023
- Sudha et al., *IoT-Based Smart Street Light Fault Detection and Real-Time Fault Location Identification Using WSN and GPS*, IJLTEMAS, 2025
- Jabbar et al., *Optimising Urban Lighting Efficiency with IoT and LoRaWAN Integration in Smart Street Lighting Systems*, Discover Internet of Things (Springer), 2025

