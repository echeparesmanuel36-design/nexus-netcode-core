# 🌐 Nexus Netcode Core: Client-Side Prediction & Server Reconciliation

A high-performance, lightweight network physics synchronization micro-engine written in pure C++17. Designed to mitigate network latency (lag) in fast-paced competitive multiplayer games.

## ⚡ The Problem vs. The Solution

In high-latency environments, waiting for the server to validate player movement causes noticeable stuttering and input lag. 

This micro-engine implements **Client-Side Prediction** combined with **Server Authoritative Reconciliation**:
1. **Prediction:** The client executes physics inputs instantly ($O(1)$ latency perception).
2. **History Buffering:** The engine buffers state and inputs for every processed network tick.
3. **Reconciliation:** When a delayed authoritative server packet arrives, the engine verifies the states. If a discrepancy (desync) is found, it automatically rewinds time to the point of failure, corrects the initial state, and resimulates all subsequent ticks instantly.

## 🚀 Key Architectural Features

* **Zero-Allocation History Arrays:** Fast sequential memory vectors storing input and state frames for ultra-quick rollbacks.
* **Deterministic Resimulation Loop:** Re-applies physics steps precisely to guarantee client-server convergence after a correction.
* **Standalone Architecture:** Zero dependencies, making it perfectly suitable to be wrapped as a custom low-level C++ subsystem or plugin inside Unreal Engine or custom engines.

## 🛠️ Quick Compilation

Ensure you have a C++17 compliant compiler and CMake installed.

```bash
mkdir build && cd build
```
```
cmake .. -DCMAKE_BUILD_TYPE=Release
```bash
cmake --build .
```
```bash
./netcode_core
```
## 📊 Sample Execution Log
​When executed, the engine simulates a local player moving smoothly ahead of the server, detects a fake network desync (simulating a high ping spike or collision mismatch), rollbacks the timeline, and fixes the position seamlessly:

```bash
[CLIENTE] Tick 1 | Posición Predicha: 0.08 m
[CLIENTE] Tick 2 | Posición Predicha: 0.16 m
[CLIENTE] Tick 3 | Posición Predicha: 0.24 m
[SERVIDOR] -> Llegó paquete retrasado del Tick 1 | Posición Real del Servidor: 0.05 m
[ALERTA NETCODE] ¡Desincronización detectada! Error: 0.03 m. Reconciliando...
[ALERTA NETCODE] Reconciliación completada. Nueva posición corregida: 0.21 m
```

### 📄 License
​This project is open-source and available under the MIT License.