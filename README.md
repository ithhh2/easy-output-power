# Multi-Output DC Power Supply (v2.1)

STM32F070F6P6 firmware for a multi-rail programmable DC power supply with CH224 PD front-end, relay switching, OLED UI, and ADC measurement.

## Hardware

| Item | Detail |
|------|--------|
| MCU | STM32F070F6P6 (32 KB Flash, 6 KB RAM) |
| PD | CH224 (CFG1/2/3) |
| Outputs | 1.8V / 3.3V / 5.0V via relays |
| Measure | PA0 current, PA5 voltage, VREFINT |
| Display | 128x64 OLED (I2C) |
| Keys | SW1 SET, SW2 ADD, SW3 SUB |

## UI

### Power OFF (setup)

| Row | Content | Highlight |
|-----|---------|-----------|
| 1 | Set Vol | Selected on VOL page |
| 2 | Set Cur | Selected on CUR page |
| 3 | Power OFF | Selected on POWER page |

- **SET**: cycle VOL -> CUR -> POWER
- **ADD/SUB**: adjust voltage tier or current limit; on POWER page toggle output ON

### Power ON (monitor)

| Row | Content | Highlight |
|-----|---------|-----------|
| 1 | Now Vol (2 decimal places) | Normal |
| 2 | Now Cur | Normal |
| 3 | Power ON | Reverse (fixed) |

- **SET**: no effect
- **ADD/SUB**: on POWER page, turn output OFF

To change voltage tier or current limit: turn output OFF, use SET to select row, adjust, then turn ON again.

## Persistence

Voltage tier and current limit are saved to the last Flash page (`0x08007C00`) after changes (2 s debounce) or when output is turned OFF.

## Voltage Tiers

| Display | Relay | CH224 | UV threshold |
|---------|-------|-------|--------------|
| 1.80V | 1.8V | 1.8V profile (PD 5V) | 0.80V |
| 3.30V | 3.3V | 3.3V profile (PD 5V) | 1.80V |
| 5.00V | VBUS | 5V | 3.50V |

Default tier on boot: **5.00V** (or last saved). Current limit: **200-1000 mA**, step 100 mA.

## Fault Codes

| Display | Meaning |
|---------|---------|
| FAULT OC | Over-current (+50 mA hysteresis, latched) |
| FAULT UV | Under-voltage (after 1 s grace) |
| FAULT ADC | ADC sampling failed repeatedly |

OC protection is active even during the 1 s startup grace period; UV is delayed.

## Build and Flash

Use **Release** build (~18-20 KB Flash, 1 KB reserved for config):

```powershell
cmake -S . -B build/Release -DCMAKE_BUILD_TYPE=Release
cmake --build build/Release
```

Flash `build/Release/mult-output-power.elf` (or `.hex`).

## CubeMX

See [docs/CUBEMX.md](docs/CUBEMX.md) before regenerating code from `.ioc`.

## Version Tags

- `v1.0` - Initial stable release
- `v2.0` - DMA ADC, fixed-point measurement, modular app layer
- `v2.1` - Flash persistence, async ADC, UI incremental refresh, protection/UI/hardware enhancements
