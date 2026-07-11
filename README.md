# SN74HC595 as a Pin Multiplier

Using a single 8-bit shift register (SN74HC595) to turn **1 MCU pin into 9 usable outputs**.

## Pinout Reference

| Pin | Name | Function |
|-----|------|----------|
| 8   | GND  | Ground |
| 16  | VCC  | Supply |
| 14  | SER  | Serial data in |
| 11  | SRCLK | Shift register clock (shifts bit in on rising edge) |
| 12  | RCLK | Storage register clock / latch (moves shift reg → outputs on rising edge) |
| 13  | \~OE | Output enable, active low → tie to GND |
| 10  | \~SRCLR | Shift register clear, active low → tie to VCC |
| 15,1-7 | QA-QH | Parallel outputs |
| 9   | QH'  | Serial out (cascade to next 595) |

---

## Variant 1 — One Pin, Filter Resistor (SRCLK + RCLK tied together; resistor to SER)

Only **one MCU pin** drives everything. A resistor filters SER input so the previous average value is shifted in by SRCLK:

```
                     ┌────────────────────────────┐
                     │        SN74HC595           │
   MCU PIN ──┬─[ R ]─┤ SER   (14)                 │
             ├───────┤ SRCLK (11)                 │
             └───────┤ RCLK  (12)                 │
                     │                            │
             VCC ────┤ ~SRCLR (10)                │
             GND ────┤ ~OE    (13)                │
                     │                            │
                     │  QA..QH (15,1-7) ──► 8 outputs
                     │  QH'  (9) ──► cascade (optional)
                     └────────────────────────────┘
```

**Bonus 9th bit:** since the MCU pin's *own* resting logic level (before/after the
clocking pulses) is controlled by firmware, that same pin can be used directly
as a 9th output — e.g. to drive an LED or logic input in parallel with the 595, since it
can be set to a defined HIGH or LOW between shift operations.

Trade-off: this relies on matching the RC time constant of pin and board capacitance to MCU_PIN timing.

---

## Variant 2 — Glitch-Free Latch (extra resistor or extra pin for RCLK)

To avoid any risk of the outputs flickering mid-shift (since RCLK is tied to the same
node as SRCLK/SER), give RCLK its own, larger-value resistor (or its own dedicated MCU
pin) so it lags further behind and only latches once the full byte has finished shifting in.

```
                      ┌────────────────────────────┐
                      │        SN74HC595           │
   MCU PIN ──┬─[ R1 ]─┤ SER   (14)                 │
             ├────────┤ SRCLK (11)                 │
             └─[ R2 ]─┤ RCLK  (12)                 │
                      │                            │
                      │                            │
              VCC ────┤ ~SRCLR (10)                │
              GND ────┤ ~OE    (13)                │
                      │                            │
                      │  QA..QH ──► 8 outputs      │
                      └────────────────────────────┘
```

*Alternative:* use a **second MCU pin** for RCLK instead of R2, and pulse it explicitly
after the 8 SRCLK pulses complete — removes analog timing dependence entirely while
still saving a pin versus a fully separate SER/SRCLK/RCLK wiring.

---

## Variant 3 — Two Pins, No Resistors (fast/simple timing)

Drop the resistor trick altogether: dedicate one pin to **SER** (data) and one to
**SRCLK** (clock), driven with standard bit-bang or hardware SPI timing. RCLK can still
share a pin with one of these via a resistor (Variant 2 style) or use a third pin for
fully deterministic, glitch-free, high-speed operation.

```
   MCU SER   ──────┤ SER   (14)
   MCU SRCLK ──────┤ SRCLK (11)
   MCU RCLK   ─────┤ RCLK  (12)   (separate pin, or resistor-delayed from SRCLK)

            VCC ───┤ ~SRCLR (10)
            GND ───┤ ~OE    (13)
```

This is the simplest to get right and the fastest to clock (no RC settling time to wait
out), at the cost of 2–3 MCU pins instead of 1.

---

## Summary

| Variant | MCU pins used | Timing | Notes |
|---|---|---|---|
| 1. Single pin | 1 | Requires accurate timing | Also gives a free 9th output bit |
| 2. Single pin + glitch-free latch | 1 (+ larger R) or 2 | Safer latch timing | Second R or second pin isolates RCLK |
| 3. Separate SER/SRCLK | 2–3 | Fast, deterministic | No resistor, standard shift-register timing |
