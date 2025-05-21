#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits>
#include <ctime>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <netdb.h>  // Para gethostbyname

std::string generate_uuid() {
    uuid_t uuid;
    uuid_generate_random(uuid);
    char uuid_str[37];
    uuid_unparse_lower(uuid, uuid_str);
    return std::string(uuid_str);
}

std::string get_current_time() {
    auto now = std::time(nullptr);
    // Redondeamos al periodo de 30 segundos más cercano para sincronizarnos con el servidor
    now = now - (now % 30);
    
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&now), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

void clearScreen() {
    // Limpiar pantalla (funciona en UNIX/Linux/MacOS y Windows)
    system("clear || cls");
}

void showWelcomeScreen() {
    clearScreen();
    std::cout << "┌──────────────────────────────────────────────────┐" << std::endl;
    std::cout << "│         SISTEMA DE TRANSACCIONES SEGURAS         │" << std::endl;
    std::cout << "└──────────────────────────────────────────────────┘" << std::endl;
    std::cout << std::endl;
    std::cout << "  Este sistema requiere una clave dinámica que cambia" << std::endl;
    std::cout << "  cada 30 segundos y se muestra en el servidor." << std::endl;
    std::cout << std::endl;
    std::cout << "  Usuarios disponibles para pruebas:" << std::endl;
    std::cout << "  - Usuario: user1, Contraseña: pass123" << std::endl;
    std::cout << "  - Usuario: user2, Contraseña: pass456" << std::endl;
    std::cout << std::endl;
    std::cout << "  Presiona ENTER para continuar...";
    std::cin.get();
}

int main() {
    showWelcomeScreen();
    
    std::string username, password, dynamic_key;
    double amount = 1000.50;
    
    while (true) {
        clearScreen();
        std::cout << "┌──────────────────────────────────────────────────┐" << std::endl;
        std::cout << "│         SISTEMA DE TRANSACCIONES SEGURAS         │" << std::endl;
        std::cout << "└──────────────────────────────────────────────────┘" << std::endl;
        std::cout << std::endl;
        
        // Generar ID de transacción único
        std::string transaction_id = generate_uuid();
        
        // Obtener timestamp actual (sincronizado a periodos de 30 segundos)
        std::string timestamp = get_current_time();
        
        // Solicitar información de usuario
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Ingrese usuario: ";
        std::getline(std::cin, username);
        if (username.empty()) continue;
        
        std::cout << "  Ingrese contraseña: ";
        std::getline(std::cin, password);
        if (password.empty()) continue;
        
        std::cout << "  Ingrese monto a transferir: $";
        std::string amount_str;
        std::getline(std::cin, amount_str);
        if (!amount_str.empty()) {
            try {
                amount = std::stod(amount_str);
            } catch (...) {
                std::cout << "\n  Error: Monto inválido. Presione ENTER para continuar.";
                std::cin.get();
                continue;
            }
        }
        
        std::cout << "\n  Ingrese la clave dinámica mostrada en el servidor: ";
        std::getline(std::cin, dynamic_key);
        if (dynamic_key.empty()) {
            std::cout << "\n  Error: Debe ingresar una clave dinámica. Presione ENTER para continuar.";
            std::cin.get();
            continue;
        }
        
        // Conectar al servidor
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            std::cerr << "\n  Error: No se pudo crear el socket. Presione ENTER para continuar.";
            std::cin.get();
            continue;
        }

        struct sockaddr_in server{};
        server.sin_family = AF_INET;
        server.sin_port = htons(8080);

        // Resolver hostname "server"
        struct hostent* host = gethostbyname("server");
        if (host == nullptr) {
            std::cerr << "\n  Error: No se pudo conectar con el servidor. Presione ENTER para continuar.";
            close(sock);
            std::cin.get();
            continue;
        }
        memcpy(&server.sin_addr, host->h_addr_list[0], host->h_length);

        // Intentar conectar con un timeout
        if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
            std::cerr << "\n  Error: No se pudo conectar con el servidor. Presione ENTER para continuar.";
            close(sock);
            std::cin.get();
            continue;
        }

        // Preparar y enviar la transacción
        std::string transaction = transaction_id + "|" + timestamp + "|" + std::to_string(amount) + "|" +
                                username + "|" + password + "|" + dynamic_key;

        if (send(sock, transaction.c_str(), transaction.length(), 0) < 0) {
            std::cerr << "\n  Error: Fallo al enviar la transacción. Presione ENTER para continuar.";
            close(sock);
            std::cin.get();
            continue;
        }

        // Recibir respuesta del servidor
        char buffer[1024] = {0};
        if (recv(sock, buffer, sizeof(buffer), 0) < 0) {
            std::cerr << "\n  Error: Fallo al recibir respuesta del servidor. Presione ENTER para continuar.";
            close(sock);
            std::cin.get();
            continue;
        }

        // Mostrar respuesta
        clearScreen();
        std::string response(buffer);
        
        if (response.find("Acceso exitoso") != std::string::npos) {
            std::cout << "┌──────────────────────────────────────────────────┐" << std::endl;
            std::cout << "│               TRANSACCIÓN EXITOSA                │" << std::endl;
            std::cout << "└──────────────────────────────────────────────────┘" << std::endl;
            std::cout << std::endl;
            std::cout << "  " << response << std::endl;
            std::cout << "  Usuario: " << username << std::endl;
            std::cout << "  Fecha y hora: " << timestamp << std::endl;
            std::cout << std::endl;
        } else {
            std::cout << "┌──────────────────────────────────────────────────┐" << std::endl;
            std::cout << "│              TRANSACCIÓN RECHAZADA               │" << std::endl;
            std::cout << "└──────────────────────────────────────────────────┘" << std::endl;
            std::cout << std::endl;
            std::cout << "  " << response << std::endl;
            std::cout << std::endl;
            std::cout << "  Posibles causas:" << std::endl;
            std::cout << "  - Usuario o contraseña incorrectos" << std::endl;
            std::cout << "  - La clave dinámica ha expirado o es incorrecta" << std::endl;
            std::cout << "  - Error de conexión con el servidor" << std::endl;
            std::cout << std::endl;
        }
        
        close(sock);
        
        std::cout << "  ¿Desea realizar otra transacción? (S/N): ";
        std::string continue_option;
        std::getline(std::cin, continue_option);
        
        if (continue_option != "S" && continue_option != "s") {
            break;
        }
    }
    
    return 0;
}