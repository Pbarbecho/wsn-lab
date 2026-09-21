#!/usr/bin/env python3
"""
Generador de simulaciones Cooja (.csc) para el laboratorio WSN.

Crea un archivo .csc con uno o más tipos de mote (Cooja motes, compilados a
partir de un .c), N motes por tipo colocados según una disposición
(estrella, línea, malla, aleatoria), los plugins habituales de la GUI y,
opcionalmente, un ScriptRunner para ejecuciones sin GUI.

Ejemplos:
  # 1 raíz + 9 clientes en disposición aleatoria de 120x120 m, alcance 50 m
  python3 gen_csc.py -o sim.csc --type raiz:udp-server.c:1 --type nodo:udp-client.c:9 \
        --layout random --area 120 --range 50

  # Cadena de 5 nodos separados 40 m (multisalto), con script y timeout de 10 min
  python3 gen_csc.py -o cadena.csc --type raiz:udp-server.c:1 --type nodo:udp-client.c:4 \
        --layout line --spacing 40 --script metrics.js --timeout 600000

  # Border router con Serial Socket en el puerto 60001
  python3 gen_csc.py -o br.csc --type br:[CONTIKI_DIR]/examples/rpl-border-router/border-router.c:1 \
        --type nodo:sensor-node.c:3 --layout star --serial-socket

Las rutas de --type son relativas al directorio del .csc ([CONFIG_DIR]) salvo
que empiecen por "[" (p. ej. [CONTIKI_DIR]/...).
"""
import argparse
import math
import os
import random
import sys
from xml.sax.saxutils import escape

INTERFACES = [
    "org.contikios.cooja.interfaces.Position",
    "org.contikios.cooja.interfaces.Battery",
    "org.contikios.cooja.contikimote.interfaces.ContikiVib",
    "org.contikios.cooja.contikimote.interfaces.ContikiMoteID",
    "org.contikios.cooja.contikimote.interfaces.ContikiRS232",
    "org.contikios.cooja.contikimote.interfaces.ContikiBeeper",
    "org.contikios.cooja.interfaces.RimeAddress",
    "org.contikios.cooja.contikimote.interfaces.ContikiIPAddress",
    "org.contikios.cooja.contikimote.interfaces.ContikiRadio",
    "org.contikios.cooja.contikimote.interfaces.ContikiButton",
    "org.contikios.cooja.contikimote.interfaces.ContikiPIR",
    "org.contikios.cooja.contikimote.interfaces.ContikiClock",
    "org.contikios.cooja.contikimote.interfaces.ContikiLED",
    "org.contikios.cooja.contikimote.interfaces.ContikiCFS",
    "org.contikios.cooja.contikimote.interfaces.ContikiEEPROM",
    "org.contikios.cooja.interfaces.Mote2MoteRelations",
    "org.contikios.cooja.interfaces.MoteAttributes",
]


def parse_type(spec):
    parts = spec.split(":")
    if len(parts) != 3:
        sys.exit(f"--type debe ser nombre:archivo.c:cantidad, recibido '{spec}'")
    name, src, count = parts
    if not src.startswith("["):
        src = "[CONFIG_DIR]/" + src
    return {"name": name, "source": src, "count": int(count),
            "target": os.path.basename(src)[:-2]}


def positions(n, layout, area, spacing, rng):
    """Devuelve n posiciones (x, y). El primer mote (id 1) queda en el centro/inicio."""
    pts = []
    if layout == "line":
        for i in range(n):
            pts.append((i * spacing, 0.0))
    elif layout == "grid":
        cols = max(1, int(math.ceil(math.sqrt(n))))
        for i in range(n):
            pts.append(((i % cols) * spacing, (i // cols) * spacing))
    elif layout == "star":
        pts.append((area / 2, area / 2))
        r = spacing
        for i in range(1, n):
            ang = 2 * math.pi * (i - 1) / max(1, n - 1)
            pts.append((area / 2 + r * math.cos(ang), area / 2 + r * math.sin(ang)))
    else:  # random
        pts.append((area / 2, area / 2))
        for i in range(1, n):
            pts.append((rng.uniform(0, area), rng.uniform(0, area)))
    return pts


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-o", "--out", required=True, help="archivo .csc de salida")
    ap.add_argument("--title", default="Simulacion WSN")
    ap.add_argument("--type", action="append", required=True,
                    help="nombre:archivo.c:cantidad (repetible; el primero contiene al mote id 1)")
    ap.add_argument("--layout", choices=["random", "grid", "line", "star"], default="random")
    ap.add_argument("--area", type=float, default=100.0, help="lado del área (m) para random/star")
    ap.add_argument("--spacing", type=float, default=30.0, help="separación (m) para line/grid/star")
    ap.add_argument("--range", type=float, default=50.0, help="alcance de transmisión UDGM (m)")
    ap.add_argument("--interference", type=float, default=100.0, help="alcance de interferencia (m)")
    ap.add_argument("--tx-ratio", type=float, default=1.0, help="prob. éxito TX (0-1)")
    ap.add_argument("--rx-ratio", type=float, default=1.0, help="prob. éxito RX (0-1)")
    ap.add_argument("--seed", type=int, default=123456, help="semilla de la simulación")
    ap.add_argument("--pos-seed", type=int, default=1, help="semilla de las posiciones")
    ap.add_argument("--make-args", default="", help="argumentos extra para make (p.ej. MAKE_MAC=MAKE_MAC_TSCH)")
    ap.add_argument("--clean", action="store_true", help="hacer 'make clean' antes de compilar")
    ap.add_argument("--script", help="archivo .js para el ScriptRunner (relativo al .csc)")
    ap.add_argument("--timeout", type=int, default=0, help="TIMEOUT(ms) si se genera script embebido")
    ap.add_argument("--serial-socket", action="store_true", help="Serial Socket (SERVER) en el mote 1, puerto 60001")
    ap.add_argument("--no-gui-plugins", action="store_true", help="omitir Visualizer/LogListener/TimeLine")
    ap.add_argument("--pcap", metavar="ARCHIVO", help="Radio messages guarda todas las tramas 802.15.4 en este .pcap (relativo al .csc); ábralo con Wireshark")
    args = ap.parse_args()

    types = [parse_type(t) for t in args.type]
    total = sum(t["count"] for t in types)
    rng = random.Random(args.pos_seed)
    pts = positions(total, args.layout, args.area, args.spacing, rng)

    out = []
    w = out.append
    w('<?xml version="1.0" encoding="UTF-8"?>')
    w('<simconf version="2023090101">')
    w('  <simulation>')
    w(f'    <title>{escape(args.title)}</title>')
    w(f'    <randomseed>{args.seed}</randomseed>')
    w('    <motedelay_us>1000000</motedelay_us>')
    w('    <radiomedium>')
    w('      org.contikios.cooja.radiomediums.UDGM')
    w(f'      <transmitting_range>{args.range}</transmitting_range>')
    w(f'      <interference_range>{args.interference}</interference_range>')
    w(f'      <success_ratio_tx>{args.tx_ratio}</success_ratio_tx>')
    w(f'      <success_ratio_rx>{args.rx_ratio}</success_ratio_rx>')
    w('    </radiomedium>')
    w('    <events>')
    w('      <logoutput>40000</logoutput>')
    w('    </events>')

    mote_id = 1
    for i, t in enumerate(types):
        cmds = []
        if args.clean:
            cmds.append("$(MAKE) TARGET=cooja clean")
        cmds.append(f"$(MAKE) -j$(CPUS) {t['target']}.cooja TARGET=cooja {args.make_args}".rstrip())
        w('    <motetype>')
        w('      org.contikios.cooja.contikimote.ContikiMoteType')
        w(f'      <identifier>mtype{i + 1}</identifier>')
        w(f'      <description>{escape(t["name"])}</description>')
        w(f'      <source>{escape(t["source"])}</source>')
        w(f'      <commands>{escape(chr(10).join(cmds))}</commands>')
        for itf in INTERFACES:
            w(f'      <moteinterface>{itf}</moteinterface>')
        for _ in range(t["count"]):
            x, y = pts[mote_id - 1]
            w('      <mote>')
            w('        <interface_config>')
            w('          org.contikios.cooja.interfaces.Position')
            w(f'          <pos x="{x:.3f}" y="{y:.3f}" />')
            w('        </interface_config>')
            w('        <interface_config>')
            w('          org.contikios.cooja.contikimote.interfaces.ContikiMoteID')
            w(f'          <id>{mote_id}</id>')
            w('        </interface_config>')
            w('        <interface_config>')
            w('          org.contikios.cooja.contikimote.interfaces.ContikiRadio')
            w('          <bitrate>250.0</bitrate>')
            w('        </interface_config>')
            w('      </mote>')
            mote_id += 1
        w('    </motetype>')
    w('  </simulation>')

    if not args.no_gui_plugins:
        w('  <plugin>')
        w('    org.contikios.cooja.plugins.Visualizer')
        w('    <plugin_config>')
        w('      <moterelations>true</moterelations>')
        w('      <skin>org.contikios.cooja.plugins.skins.IDVisualizerSkin</skin>')
        w('      <skin>org.contikios.cooja.plugins.skins.UDGMVisualizerSkin</skin>')
        w('      <skin>org.contikios.cooja.plugins.skins.MoteTypeVisualizerSkin</skin>')
        w('      <viewport>2.5 0.0 0.0 2.5 50.0 50.0</viewport>')
        w('    </plugin_config>')
        w('    <bounds x="1" y="1" height="500" width="520" z="3" />')
        w('  </plugin>')
        w('  <plugin>')
        w('    org.contikios.cooja.plugins.LogListener')
        w('    <plugin_config>')
        w('      <filter />')
        w('      <formatted_time />')
        w('      <coloring />')
        w('    </plugin_config>')
        w('    <bounds x="525" y="1" height="500" width="900" z="2" />')
        w('  </plugin>')
        w('  <plugin>')
        w('    org.contikios.cooja.plugins.TimeLine')
        w('    <plugin_config>')
        for m in range(total):
            w(f'      <mote>{m}</mote>')
        w('      <showRadioRXTX />')
        w('      <showRadioHW />')
        w('      <showLEDs />')
        w('      <zoomfactor>500.0</zoomfactor>')
        w('    </plugin_config>')
        w('    <bounds x="1" y="505" height="220" width="1424" z="1" />')
        w('  </plugin>')
        w('  <plugin>')
        w('    org.contikios.cooja.plugins.RadioLogger')
        w('    <plugin_config>')
        if args.pcap:
            pcap = args.pcap if args.pcap.startswith("[") else "[CONFIG_DIR]/" + args.pcap
            w(f'      <pcap_file>{escape(pcap)}</pcap_file>')
            w('      <analyzers name="6lowpan-pcap" />')
        w('      <split>150</split>')
        w('      <formatted_time />')
        w('      <showdups>false</showdups>')
        w('      <hidenodests>false</hidenodests>')
        w('    </plugin_config>')
        w('    <bounds x="1" y="730" height="260" width="1424" z="0" />')
        w('  </plugin>')

    if args.serial_socket:
        w('  <plugin>')
        w('    org.contikios.cooja.serialsocket.SerialSocketServer')
        w('    <mote_arg>0</mote_arg>')
        w('    <plugin_config>')
        w('      <port>60001</port>')
        w('      <bound>true</bound>')
        w('    </plugin_config>')
        w('    <bounds x="1430" y="1" height="120" width="360" z="4" />')
        w('  </plugin>')

    if args.script or args.timeout:
        w('  <plugin>')
        w('    org.contikios.cooja.plugins.ScriptRunner')
        w('    <plugin_config>')
        if args.script:
            w(f'      <scriptfile>[CONFIG_DIR]/{escape(args.script)}</scriptfile>')
        else:
            w(f'      <script>TIMEOUT({args.timeout});\ntimeout_function = function() {{ log.testOK(); }};\nwhile(true) {{ YIELD(); log.log(time + "\\t" + id + "\\t" + msg + "\\n"); }}</script>')
        w('      <active>true</active>')
        w('    </plugin_config>')
        w('    <bounds x="1430" y="130" height="500" width="500" z="4" />')
        w('  </plugin>')

    w('</simconf>')
    with open(args.out, "w") as f:
        f.write("\n".join(out) + "\n")
    extra = f", pcap={args.pcap}" if args.pcap and not args.no_gui_plugins else ""
    print(f"{args.out}: {total} motes, {len(types)} tipos, layout={args.layout}, range={args.range} m{extra}")


if __name__ == "__main__":
    main()
