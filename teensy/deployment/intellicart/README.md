# Intellicart — Teensy 4.1 deployment

This directory contains the Intellicart Teensy 4.1 sketch and its flashable
firmware artifact: `intellicart-teensy41.hex`.

## Build and upload from Arduino IDE or Arduino CLI

From the repository root:

```sh
arduino-cli compile --fqbn teensy:avr:teensy41 \
  teensy/deployment/intellicart
```

## Flash a Teensy 4.1

Connect USB directly to the Teensy, discover its port with
`arduino-cli board list`, then run:

```sh
arduino-cli compile --upload --fqbn teensy:avr:teensy41 \
  --port <teensy-port> \
  teensy/deployment/intellicart
```

Press the Teensy's Program button once only if it needs to enter the
bootloader. Do not hold it for 13--17 seconds.
