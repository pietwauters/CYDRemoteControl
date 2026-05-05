# CYD Remote Control

**A wireless hardware remote control for the OpenPiste fencing scoring platform**

Part of the [OpenPiste](https://github.com/pietwauters) open source fencing electronics ecosystem.

---

## What is this?

The CYD Remote Control is a dedicated physical remote control for the [OpenPiste ESP32 scoring device](https://github.com/pietwauters/esp32scoringdeviceMqtt). It is built around the **Cheap Yellow Display (CYD)** — an inexpensive ESP32 development board with an integrated 2.8" touchscreen display — combined with a LiPo battery and a single external button. No custom PCB is required.

The device connects over **WiFi** and communicates via **FPA over UDP**, one of the protocols used throughout the OpenPiste platform. Because it uses WiFi rather than infrared, it works from any angle and through obstacles — no line-of-sight issues.

A ready-to-print enclosure is available on **[Printables](https://www.printables.com/model/1599802-remote-control-based-on-cheap-yellow-display)**.

---

## Key features

- **Touchscreen interface** — all standard scoring device functions accessible on-screen
- **WiFi / MQTT communication** — no pairing cables, no line-of-sight requirement
- **Fencer names on screen** — the display shows the names of the current match's fencers, received live from the scoring device
- **Any remote, any device** — each remote can connect to any scoring device on the same network; no fixed pairing needed
- **Minimal hardware** — CYD board + LiPo battery + one external button; no custom PCB
- **3D-printed enclosure** — STL files available on Printables; print-and-assemble in an afternoon
- **Open source** — firmware, enclosure, all of it

---

## Hardware

| Component | Notes |
|-----------|-------|
| Cheap Yellow Display (CYD) | ESP32-2432S028 or compatible |
| LiPo battery | Size to fit your chosen enclosure |
| Push button | External button for primary action |
| 3D-printed enclosure | STLs on Printables *(link above)* |

The CYD board is widely available from the usual suppliers (AliExpress, etc.) for around €10–15. Combined with a LiPo cell and a printed enclosure, the total BOM cost is well under €20.

---

## How it works

1. The remote connects to the local WiFi of the OpenPiste scoring device.
2. The remote displays the information from the scoring device on its touchscreen in real time.
3. Touch controls and the external button send commands to the scoring.

A remote will work with any scoring device — useful at clubs running multiple pistes.

---

## Supported functions

The remote provides access to all standard remote control functions for the scoring device, including:

- Score management (point award, correction, annulation)
- Penalty cards, including P-Cards
- Timer control (start, stop, reset)
- Period management
- Passivity timer
- Match configuration
- Priority

---

## Getting started

### Prerequisites

- PlatformIO (recommended) or Arduino IDE with ESP32 support
- A running OpenPiste scoring device

### Build and flash

```bash
git clone https://github.com/pietwauters/CYDRemoteControl.git
cd CYDRemoteControl
```

Open in PlatformIO, then build and flash to the CYD board.

---

## Part of the OpenPiste platform

| Component | Repository |
|-----------|-----------|
| Scoring device (MQTT) | [esp32scoringdeviceMqtt](https://github.com/pietwauters/esp32scoringdeviceMqtt) |
| Weapon & wire tester | [ImprovedTesterAfterGenova](https://github.com/pietwauters/ImprovedTesterAfterGenova) |
| Hardware remote (this repo) | **CYDRemoteControl** |
| Mobile app remote | *(link to be added)* |
| Piste monitor (browser) | [CyranoPisteMonitor](https://github.com/pietwauters/CyranoPisteMonitor) |
| Electronics designs | *(link to be added)* |

The platform is developed by a member of the **FIE SEMI Commission** (International Fencing Federation) and the **EFC SEMI Commission** (European Fencing Confederation), and has been field-tested at international competition level.

---

## Contributing

Contributions are welcome. Please read [CONTRIBUTING.md](CONTRIBUTING.md) before submitting a pull request.

By contributing, you agree to the terms of the Contributor License Agreement (CLA) described in that document.

---

## License

This project is licensed under the **MIT License** — see [LICENSE](LICENSE) for details.

---

## Acknowledgements

- The [Cheap Yellow Display community](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) for documentation and examples
- The armourers and officials who provided feedback during field testing at the European Fencing Championships, Genova
