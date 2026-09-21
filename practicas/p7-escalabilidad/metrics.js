/*
 * Script de Cooja (ScriptRunner) para las prácticas 4 y 7.
 * Registra toda la salida serie en COOJA.testlog y, al vencer el TIMEOUT,
 * escribe un resumen por nodo: paquetes TX, RX en la raíz, PDR y saltos medios.
 *
 * Variables disponibles en cada YIELD(): time (us), id (mote), msg (línea), mote, sim, log
 */
TIMEOUT(600000);   /* 10 minutos de tiempo simulado */

var tx = {};
var rx = {};
var hops = {};
var tx_re = /TX node=(\d+) seq=(\d+)/;
var rx_re = /RX node=(\d+) seq=(\d+) temp=(-?\d+) hops=(\d+)/;

function summary() {
  log.log("==== RESUMEN (" + (time / 1000000) + " s simulados, " +
          sim.getMotesCount() + " motes) ====\n");
  var nodes = Object.keys(tx).sort(function(a, b) { return a - b; });
  var totTx = 0, totRx = 0;
  for (var i = 0; i < nodes.length; i++) {
    var n = nodes[i];
    var t = tx[n] || 0, r = rx[n] || 0;
    var pdr = t > 0 ? (100.0 * r / t) : 0;
    var h = r > 0 ? (hops[n] / r) : 0;
    totTx += t; totRx += r;
    log.log("SUMMARY node=" + n + " tx=" + t + " rx=" + r +
            " pdr=" + pdr.toFixed(1) + " hops=" + h.toFixed(2) + "\n");
  }
  log.log("TOTAL tx=" + totTx + " rx=" + totRx + " pdr=" +
          (totTx > 0 ? (100.0 * totRx / totTx).toFixed(1) : 0) + "\n");
}

timeout_function = function() {
  summary();
  log.testOK();
};

while (true) {
  YIELD();
  log.log(time + "\t" + id + "\t" + msg + "\n");

  var m = tx_re.exec(msg);
  if (m) {
    tx[m[1]] = (tx[m[1]] || 0) + 1;
  }
  m = rx_re.exec(msg);
  if (m) {
    rx[m[1]] = (rx[m[1]] || 0) + 1;
    hops[m[1]] = (hops[m[1]] || 0) + parseInt(m[4]);
  }
}
