/* Cuenta TX/RX unicast en el sumidero durante 5 min simulados */
TIMEOUT(300000);
var tx = 0, rx = 0;
timeout_function = function() {
  log.log("SUMMARY tx=" + tx + " rx=" + rx + " pdr=" + (tx > 0 ? (100.0 * rx / tx).toFixed(1) : 0) + "\n");
  log.testOK();
};
while (true) {
  YIELD();
  log.log(time + "\t" + id + "\t" + msg + "\n");
  if (msg.indexOf("TX unicast") >= 0) tx++;
  if (id == 1 && msg.indexOf("RX node=") >= 0) rx++;
}
