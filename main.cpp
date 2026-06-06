#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>

// Estructura para registrar los comandos del jugador en cada instante de tiempo (Tick)
struct PlayerInput {
    uint32_t tick;
    float move_input; // -1.0 Izquierda, 1.0 Derecha
};

// Estado del personaje (Posición y velocidad en el mundo)
struct State {
    uint32_t tick;
    float position_x;
    float velocity_x;
};

// ========================================================
// EL MICRO-MOTOR DE RED (CLIENT-SIDE PREDICTION ENGINE)
// ========================================================
class NetcodeEngine {
private:
    float speed = 5.0f; // Velocidad del personaje
    std::vector<PlayerInput> input_history; // Historial de botones para reconciliación
    std::vector<State> state_history;       // Historial de posiciones locales
    
    State current_local_state;

public:
    NetcodeEngine() {
        current_local_state = {0, 0.0f, 0.0f};
        state_history.push_back(current_local_state);
    }

    // 1. PREDICCIÓN LOCAL: Mueve al jugador al instante sin esperar al servidor (0 Lag percibido)
    void predict_local_movement(uint32_t tick, float input_direction, float dt) {
        // Guardar el input en el historial
        input_history.push_back({tick, input_direction});

        // Aplicar física inmediata en el cliente
        current_local_state.tick = tick;
        current_local_state.velocity_x = input_direction * speed;
        current_local_state.position_x += current_local_state.velocity_x * dt;

        // Guardar la posición predicha por si hay que corregir luego
        state_history.push_back(current_local_state);

        std::cout << "[CLIENTE] Tick " << tick << " | Posición Predicha: " << current_local_state.position_x << " m\n";
    }

    // 2. RECONCILIACIÓN DEL SERVIDOR: El servidor manda la verdad absoluta con retraso.
    // Si el servidor no coincide con el cliente, el motor rebobina el tiempo y corrige la física.
    void receive_server_authoritative_state(State server_state, float dt) {
        std::cout << "\n[SERVIDOR] -> Llegó paquete retrasado del Tick " << server_state.tick 
                  << " | Posición Real del Servidor: " << server_state.position_x << " m\n";

        // Buscar el estado local que guardamos en ese mismo tick
        for (size_t i = 0; i < state_history.size(); ++i) {
            if (state_history[i].tick == server_state.tick) {
                
                // Calcular el error entre lo que predijo el cliente y lo que dice el servidor
                float error = std::abs(state_history[i].position_x - server_state.position_x);
                
                if (error > 0.01f) {
                    std::cout << "[ALERTA NETCODE] ¡Desincronización detectada! Error: " << error << " m. Reconciliando...\n";
                    
                    // CORRECCIÓN: Forzamos el estado del cliente al del servidor
                    current_local_state = server_state;

                    // REBOBINADO (Resimulación): Volvemos a aplicar todos los inputs desde ese tick hasta el actual
                    for (size_t j = i + 1; j < input_history.size(); ++j) {
                        current_local_state.tick = input_history[j].tick;
                        current_local_state.velocity_x = input_history[j].move_input * speed;
                        current_local_state.position_x += current_local_state.velocity_x * dt;
                        
                        // Actualizar el historial corregido
                        state_history[j] = current_local_state;
                    }
                    std::cout << "[ALERTA NETCODE] Reconciliación completada. Nueva posición corregida: " << current_local_state.position_x << " m\n\n";
                } else {
                    std::cout << "[NETCODE] Tick " << server_state.tick << " verificado correctamente. Predicción perfecta.\n\n";
                }
                break;
            }
        }
    }
};

int main() {
    std::cout << "=========================================================\n";
    std::cout << "  NEXUS-NETCODE: MOTOR DE PREDICCIÓN Y RECONCILIACIÓN   \n";
    std::cout << "=========================================================\n\n";

    NetcodeEngine net_engine;
    float dt = 0.016f; // Un tick cada 16ms (60 Ticks por segundo)

    // Simulamos 3 frames donde el jugador se mueve a la derecha de forma fluida en su pantalla
    net_engine.predict_local_movement(1, 1.0f, dt);
    net_engine.predict_local_movement(2, 1.0f, dt);
    net_engine.predict_local_movement(3, 1.0f, dt);

    // ESCENARIO DE ERROR: El servidor procesa el Tick 1, pero debido al lag de internet,
    // un paquete se perdió o se retrasó, y dice que en el Tick 1 el jugador chocó con una pared
    // y su posición real debería ser 0.05 en lugar de 0.08.
    State fake_server_packet = {1, 0.05f, 0.0f}; 

    // El motor recibe el paquete del servidor, detecta el fallo, viaja al pasado y corrige los frames 2 y 3 automáticamente
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Simulando el lag de la red de medio segundo
    net_engine.receive_server_authoritative_state(fake_server_packet, dt);

    return 0;
}
