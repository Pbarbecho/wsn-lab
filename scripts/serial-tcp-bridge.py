#!/usr/bin/env python3
"""Puente puerto serie <-> TCP para Mac y Windows (Docker Desktop no pasa USB al contenedor).

Se ejecuta en SU máquina (no en el contenedor). Requiere:  pip install pyserial

  Mac:      python3 scripts/serial-tcp-bridge.py /dev/tty.usbmodem1101 60002
  Windows:  python scripts\\serial-tcp-bridge.py COM5 60002

Argumentos: <puerto_serie> [puerto_tcp=60002] [baudios=115200] [ip=127.0.0.1]
Desde el contenedor se alcanza como host.docker.internal:<puerto_tcp>, por ejemplo:
  HOST=host.docker.internal PORT=60002 ./scripts/connect-router-hw.sh   (border router, práctica 8 ruta A)
  Zigbee2MQTT usa tcp://host.docker.internal:6638                         (práctica 8 ruta C)
"""
import socket
import sys
import threading

try:
    import serial  # pyserial
except ImportError:
    sys.exit("Falta pyserial:  pip install pyserial")


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    dev = sys.argv[1]
    tcp_port = int(sys.argv[2]) if len(sys.argv) > 2 else 60002
    baud = int(sys.argv[3]) if len(sys.argv) > 3 else 115200
    bind_ip = sys.argv[4] if len(sys.argv) > 4 else "127.0.0.1"

    ser = serial.Serial(dev, baud, timeout=0.05)
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind((bind_ip, tcp_port))
    srv.listen(1)
    print(f"[puente] {dev} @ {baud} <-> tcp://{bind_ip}:{tcp_port}  (Ctrl+C para salir)")

    try:
        while True:
            conn, addr = srv.accept()
            conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            print(f"[puente] cliente conectado desde {addr[0]}:{addr[1]}")
            stop = threading.Event()

            def serial_to_tcp():
                while not stop.is_set():
                    try:
                        data = ser.read(ser.in_waiting or 1)
                        if data:
                            conn.sendall(data)
                    except (OSError, serial.SerialException):
                        break
                stop.set()

            t = threading.Thread(target=serial_to_tcp, daemon=True)
            t.start()
            try:
                while not stop.is_set():
                    data = conn.recv(4096)
                    if not data:
                        break
                    ser.write(data)
            except OSError:
                pass
            stop.set()
            conn.close()
            t.join(1)
            print("[puente] cliente desconectado; esperando otro...")
    except KeyboardInterrupt:
        print("\n[puente] fin")
    finally:
        srv.close()
        ser.close()


if __name__ == "__main__":
    main()
