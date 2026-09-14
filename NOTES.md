# SG-1000 sobre FabGL

## Hardware emulado

- Z80 a 3.579545 MHz (NTSC).
- TMS9918A con 16 KiB de VRAM.
- SN76489.
- ROM de cartucho en `$0000-$BFFF` (hasta 48 KiB).
- 1 KiB de RAM, espejada desde `$C000` hasta `$FFFF`.
- Mandos en `$DC` y `$DD`; VDP en `$BE/$BF`; PSG en `$7E/$7F`.

## Entrada de anfitrión

- Mando 1: cursores y `Z`/`X`.
- Mando 2: `W`/`A`/`S`/`D` y `F`/`G`.

El proyecto no implementa PPI, teclado matricial, BASIC, casete ni RAM
ampliada: son periféricos del ordenador SC-3000, no de la consola SG-1000.