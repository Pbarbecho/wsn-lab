#!/usr/bin/env python3
"""
Práctica 7 - Consolida los COOJA.testlog de resultados/p7_n<N>_s<seed>/ en un
CSV y grafica PDR y saltos medios en función del tamaño de la red.

  python3 parse_results.py                # busca en ../../resultados
  python3 parse_results.py --dir otra/ruta --out p7.csv --plot p7.png
"""
import argparse, csv, glob, os, re, statistics as st

SUMMARY = re.compile(r"SUMMARY node=(\d+) tx=(\d+) rx=(\d+) pdr=([\d.]+) hops=([\d.]+)")
TOTAL = re.compile(r"TOTAL tx=(\d+) rx=(\d+) pdr=([\d.]+)")
DIRNAME = re.compile(r"p7_n(\d+)_s(\d+)")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dir", default=os.path.join(os.path.dirname(__file__), "..", "..", "resultados"))
    ap.add_argument("--out", default="p7_resultados.csv")
    ap.add_argument("--plot", default="p7_resultados.png")
    args = ap.parse_args()

    rows = []
    for log in sorted(glob.glob(os.path.join(args.dir, "p7_n*_s*", "COOJA.testlog"))):
        m = DIRNAME.search(log)
        if not m:
            continue
        n, seed = int(m.group(1)), int(m.group(2))
        hops, total = [], None
        with open(log, errors="replace") as f:
            for line in f:
                s = SUMMARY.search(line)
                if s and int(s.group(3)) > 0:
                    hops.append(float(s.group(5)))
                t = TOTAL.search(line)
                if t:
                    total = t
        if total:
            rows.append({"nodos": n, "semilla": seed, "tx": int(total.group(1)),
                         "rx": int(total.group(2)), "pdr": float(total.group(3)),
                         "saltos_medios": round(st.mean(hops), 2) if hops else 0.0})
    if not rows:
        print("No se encontraron resultados (¿ejecutó run_experiment.sh?)"); return

    with open(args.out, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=rows[0].keys()); w.writeheader(); w.writerows(rows)
    print(f"{len(rows)} corridas -> {args.out}")

    by_n = {}
    for r in rows:
        by_n.setdefault(r["nodos"], []).append(r)
    print(f"{'N':>5} {'PDR medio %':>12} {'desv':>6} {'saltos':>7}")
    for n in sorted(by_n):
        p = [r["pdr"] for r in by_n[n]]
        h = [r["saltos_medios"] for r in by_n[n]]
        print(f"{n:>5} {st.mean(p):>12.1f} {(st.pstdev(p) if len(p) > 1 else 0):>6.1f} {st.mean(h):>7.2f}")

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        ns = sorted(by_n)
        fig, ax = plt.subplots(1, 2, figsize=(9, 3.5))
        ax[0].errorbar(ns, [st.mean([r["pdr"] for r in by_n[n]]) for n in ns],
                       yerr=[(st.pstdev([r["pdr"] for r in by_n[n]]) if len(by_n[n]) > 1 else 0) for n in ns],
                       marker="o", capsize=3)
        ax[0].set_xlabel("Número de nodos"); ax[0].set_ylabel("PDR (%)"); ax[0].set_ylim(0, 105); ax[0].grid(alpha=.3)
        ax[1].plot(ns, [st.mean([r["saltos_medios"] for r in by_n[n]]) for n in ns], marker="s")
        ax[1].set_xlabel("Número de nodos"); ax[1].set_ylabel("Saltos medios"); ax[1].grid(alpha=.3)
        fig.tight_layout(); fig.savefig(args.plot, dpi=150)
        print(f"Gráfica -> {args.plot}")
    except ImportError:
        print("matplotlib no disponible: se omite la gráfica")


if __name__ == "__main__":
    main()
