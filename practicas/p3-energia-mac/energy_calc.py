#!/usr/bin/env python3
"""
Estima consumo y vida útil de batería a partir de las líneas ENERGEST del log
de Cooja (COOJA.testlog o texto copiado de "Mote output").

Uso:
  python3 energy_calc.py resultados/p3_csma/COOJA.testlog
  python3 energy_calc.py log_csma.txt log_tsch.txt --profile nrf52840
  python3 energy_calc.py log.txt --profile custom --tx 24 --rx 20 --cpu 13 --lpm 0.0013 --battery 2000
  python3 energy_calc.py --list-profiles

Modelo:
  Energest reparte el tiempo total T en fracciones f = t/T por estado.
  La corriente media es la suma de (corriente del estado x fracción de tiempo):
      I = I_tx*f_tx + I_rx*f_rx + I_off*f_off   (radio)
        + I_cpu*f_cpu + I_lpm*f_lpm             (MCU; solo si el simulador la emula)
  y la vida útil  L = C_bateria / I.
  Los Cooja motes NO emulan el modo de bajo consumo de la CPU (cpu = total, lpm = 0),
  por lo que, salvo que se pase --with-cpu, el MCU se modela como "dormido" (I_lpm)
  y solo la radio aporta consumo variable. Con Sky motes (MSPSim) use --with-cpu.
"""
import argparse, re, sys
from collections import OrderedDict

# Perfiles de corriente (mA) tomados de las hojas de datos, a ~3 V. Valores redondeados:
#   tx  : radio transmitiendo (potencia indicada)
#   rx  : radio en escucha / recepción (idle listening cuesta lo mismo que recibir)
#   off : radio apagada con MCU dormido (suelo de consumo del mote)
#   cpu : MCU activo a la frecuencia habitual del port
#   lpm : MCU en modo de bajo consumo con retención de RAM y reloj de 32 kHz
PROFILES = OrderedDict([
    ("cc2650",  dict(desc="TI CC2650 / CC2650 LaunchPad, Contiki-NG cc26x0 (2.4 GHz, TX +5 dBm)",
                     tx=9.1, rx=5.9, off=0.002, cpu=2.9, lpm=0.001)),
    ("cc1352",  dict(desc="TI CC1352P LaunchPad, Contiki-NG simplelink (2.4 GHz, TX +5 dBm)",
                     tx=9.6, rx=6.9, off=0.002, cpu=3.4, lpm=0.001)),
    ("nrf52840", dict(desc="Nordic nRF52840 dongle/DK, Contiki-NG nrf (TX 0 dBm, DC/DC)",
                     tx=4.8, rx=4.6, off=0.002, cpu=3.3, lpm=0.0015)),
    ("cc2538",  dict(desc="TI CC2538 (Zolertia RE-Mote/Firefly, OpenMote), TX 0 dBm",
                     tx=24.0, rx=20.0, off=0.002, cpu=13.0, lpm=0.0013)),
    ("cc2420",  dict(desc="TI CC2420 + MSP430 (Tmote Sky = Cooja 'sky mote'), TX 0 dBm",
                     tx=17.4, rx=18.8, off=0.021, cpu=1.8, lpm=0.0051)),
    ("custom",  dict(desc="valores dados con --tx --rx --off --cpu --lpm", tx=None, rx=None, off=None, cpu=None, lpm=None)),
])

LINE = re.compile(r"ENERGEST node=(\d+) total=(\d+) cpu=(\d+) lpm=(\d+) tx=(\d+) rx=(\d+) off=(\d+) tick_hz=(\d+)")


def analyze(path, prof, battery_mah, with_cpu):
    last = {}
    with open(path, errors="replace") as f:
        for line in f:
            m = LINE.search(line)
            if m:
                last[int(m.group(1))] = [int(x) for x in m.groups()[1:]]
    if not last:
        print(f"{path}: no se encontraron líneas ENERGEST"); return
    print(f"\n=== {path} ===")
    hdr = f"{'nodo':>4} {'t(s)':>7} {'rx%':>6} {'tx%':>6} {'off%':>6} {'I_radio':>8} {'I_mcu':>7} {'I_med(mA)':>10} {'vida(dias)':>11}"
    print(hdr)
    for node in sorted(last):
        total, cpu, lpm, tx, rx, off, hz = last[node]
        secs = total / hz
        f_tx, f_rx, f_off = tx / total, rx / total, off / total
        i_radio = prof["tx"] * f_tx + prof["rx"] * f_rx + prof["off"] * f_off
        if with_cpu:
            i_mcu = prof["cpu"] * (cpu / total) + prof["lpm"] * (lpm / total)
        else:
            i_mcu = prof["lpm"]          # MCU dormido salvo cuando la radio trabaja (ya incluido en tx/rx)
        i_avg = i_radio + i_mcu
        life_days = battery_mah / i_avg / 24 if i_avg > 0 else float("inf")
        print(f"{node:>4} {secs:>7.0f} {100*f_rx:>6.1f} {100*f_tx:>6.1f} {100*f_off:>6.1f} "
              f"{i_radio:>8.3f} {i_mcu:>7.3f} {i_avg:>10.3f} {life_days:>11.1f}")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("logs", nargs="*", help="COOJA.testlog o texto con líneas ENERGEST")
    ap.add_argument("--profile", default="cc2650", choices=list(PROFILES), help="perfil de corriente del mote (por defecto cc2650)")
    ap.add_argument("--tx", type=float); ap.add_argument("--rx", type=float); ap.add_argument("--off", type=float)
    ap.add_argument("--cpu", type=float); ap.add_argument("--lpm", type=float)
    ap.add_argument("--battery", type=float, default=2500.0, help="capacidad de la batería en mAh (2 x AA = 2500)")
    ap.add_argument("--with-cpu", action="store_true", help="incluir cpu/lpm de Energest (solo motes que emulan el MCU, p.ej. Sky)")
    ap.add_argument("--list-profiles", action="store_true")
    args = ap.parse_args()

    if args.list_profiles:
        for k, p in PROFILES.items():
            print(f"{k:>9}: tx={p['tx']} rx={p['rx']} off={p['off']} cpu={p['cpu']} lpm={p['lpm']} mA  - {p['desc']}")
        return

    prof = dict(PROFILES[args.profile])
    for k in ("tx", "rx", "off", "cpu", "lpm"):
        v = getattr(args, k)
        if v is not None:
            prof[k] = v
    if any(prof[k] is None for k in ("tx", "rx", "off", "cpu", "lpm")):
        sys.exit("perfil custom: indique --tx --rx --off --cpu --lpm (mA)")

    print(f"Perfil: {args.profile} ({prof['desc']}) -> tx={prof['tx']} rx={prof['rx']} off={prof['off']} "
          f"cpu={prof['cpu']} lpm={prof['lpm']} mA; batería {args.battery:.0f} mAh; "
          f"MCU {'segun Energest' if args.with_cpu else 'dormido (Cooja mote)'}")
    for p in args.logs or ["COOJA.testlog"]:
        analyze(p, prof, args.battery, args.with_cpu)


if __name__ == "__main__":
    main()
