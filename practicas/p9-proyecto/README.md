# Práctica 9 – Proyecto integrador

Diseñe, simule y evalúe una red de sensores completa para un caso de uso real
(monitoreo agrícola, calidad de aire urbana, edificio inteligente, etc.).

## Requisitos mínimos

- Al menos 15 nodos sensores con topología multisalto (RPL) y un border router.
- Los nodos deben dormir la mayor parte del tiempo (TSCH o CSMA con envío poco frecuente)
  y reportar el consumo estimado con Energest (práctica 3).
- Publicación de datos a un broker MQTT o exposición mediante CoAP (práctica 6) y un
  panel en Node-RED.
- Evaluación con al menos tres tamaños de red o tres configuraciones, ejecutada sin GUI
  y con varias semillas (práctica 7).

## Entregables

1. Código fuente de los nodos (`nodo/`), simulaciones `.csc` y scripts de experimento.
2. Informe (plantilla en `informe/plantilla-informe.tex`) con: descripción del escenario,
   arquitectura (diagrama de bloques), decisiones de diseño (MAC, periodo de envío,
   topología), resultados (PDR, latencia, saltos, vida útil estimada) y conclusiones.
3. Demostración en vivo de 10 minutos.

## Rúbrica orientativa

| Criterio | Peso |
|---|---|
| Funcionamiento de la red (unión al DODAG, entrega de datos, panel) | 30 % |
| Diseño energético justificado con mediciones | 20 % |
| Rigor experimental (semillas, repeticiones, gráficas) | 20 % |
| Calidad del código y de la documentación | 15 % |
| Presentación y defensa | 15 % |
