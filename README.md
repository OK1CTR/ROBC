# PilsenCUBE COM/OBC

A base of firmware of small, compact COM + OBC subsystem for 1U CubeSat student satellite. It was originally designed for PilsenCube project on University of West Bohemia in Pilsen between years 2010 - 2020.

* MCU: STM32F100
* TRX: AX5034
* Frequency band: 435 MHz (70 cm)
* Modulation: CW Morse, FM Morse, FM Voice, FM Test Signals, AX.25 AFSK 1200 Bd, AX.25 GMSK 1200 - 19200 Bd, AX.25 customized (HDLC)

## WARNING

The HW and FW of COM/OBC is a work from my student's years. It is written by unexperienced way and my contain lot of errors. It is pretty obsolete project now. The MCU is prehistoric and almost all other chips on board were disappeared during the Silicon Crisis era. The reason, why I put it on GitHUB today is because some of my colegues had expressed some interest to reproduce similar vintage COM subsystem today. The code might be useful as a set of routines co command the single chip transceiver and a guide to make workarounds to some special transceiver chip features and bugs.

