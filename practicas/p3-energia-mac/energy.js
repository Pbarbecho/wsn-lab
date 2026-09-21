/* Registra la salida serie durante 10 min simulados y termina.
 * Las líneas ENERGEST quedan en COOJA.testlog; analícelas con energy_calc.py */
TIMEOUT(600000);
timeout_function = function() { log.log("FIN\n"); log.testOK(); };
while (true) {
  YIELD();
  log.log(time + "\t" + id + "\t" + msg + "\n");
}
