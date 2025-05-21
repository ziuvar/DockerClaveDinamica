// server.cpp
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <thread>
#include <mutex>
#include <chrono>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

// Variables globales
std::string current_dynamic_key;
std::string current_timestamp;
std::mutex key_mutex;
const std::string SECRET = "clave_secreta_123";

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(str);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string generate_timestamp() {
    auto now = std::time(nullptr);
    now = now - (now % 30); // redondeo

    std::stringstream ss;
    ss << std::put_time(std::gmtime(&now), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

std::string generate_dynamic_key(const std::string& secret, const std::string& data) {
    unsigned int len = SHA256_DIGEST_LENGTH;
    unsigned char* result = HMAC(EVP_sha256(), secret.c_str(), secret.length(),
                                 reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), nullptr, nullptr);
    if (!result) return "00000000";  // fallback si falla HMAC

    char output[2 * SHA256_DIGEST_LENGTH + 1] = {0};
    for (unsigned int i = 0; i < len; i++) {
        snprintf(&output[i * 2], 3, "%02x", result[i]);
    }
    return std::string(output).substr(0, 8);
}

void clear_console() {
#if defined(_WIN32)
    system("cls");
#else
    system("clear");
#endif
}

void update_dynamic_key() {
    while (true) {
        std::string new_timestamp = generate_timestamp();
        std::string new_key = generate_dynamic_key(SECRET, new_timestamp);

        {
            std::lock_guard<std::mutex> lock(key_mutex);
            current_timestamp = new_timestamp;
            current_dynamic_key = new_key;
        }

        clear_console();
        std::cout << "=====================================================" << std::endl;
        std::cout << "       SERVIDOR DE AUTENTICACIÓN - CLAVE DINÁMICA    " << std::endl;
        std::cout << "=====================================================" << std::endl;
        std::cout << "  CLAVE ACTUAL: " << new_key << std::endl;
        std::cout << "  Timestamp:    " << new_timestamp << std::endl;
        std::cout << "  Válida por:   " << 30 - (std::time(nullptr) % 30) << " segundos" << std::endl;
        std::cout << "=====================================================" << std::endl;
        std::cout << "  Esperando conexiones en puerto 8080..." << std::endl;
        std::cout << "=====================================================" << std::endl;

        auto now = std::chrono::system_clock::now();
        auto next_period = std::chrono::system_clock::from_time_t(
            (std::chrono::system_clock::to_time_t(now) / 30 + 1) * 30
        );
        std::this_thread::sleep_until(next_period);
    }
}

int main() {
    std::map<std::string, std::string> users = {
        {"user1", "pass123"},
        {"user2", "pass456"}
    };

    current_timestamp = generate_timestamp();
    current_dynamic_key = generate_dynamic_key(SECRET, current_timestamp);

    std::thread key_update_thread(update_dynamic_key);
    key_update_thread.detach();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Error al crear el socket" << std::endl;
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error en setsockopt" << std::endl;
        return 1;
    }

    struct sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Error al bindear el socket" << std::endl;
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Error al escuchar" << std::endl;
        return 1;
    }

    while (true) {
        int client_socket;
        struct sockaddr_in client_address;
        socklen_t addrlen = sizeof(client_address);

        client_socket = accept(server_fd, (struct sockaddr*)&client_address, &addrlen);
        if (client_socket < 0) {
            std::cerr << "Error al aceptar la conexión" << std::endl;
            continue;
        }

        char buffer[1024] = {0};
        ssize_t recv_len = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (recv_len <= 0) {
            std::cerr << "Error al recibir la transacción" << std::endl;
            close(client_socket);
            continue;
        }
        buffer[recv_len] = '\0';

        std::string transaction(buffer);
        std::vector<std::string> parts = split(transaction, '|');
        if (parts.size() != 6) {
            std::string response = "Error: Formato de transacción inválido";
            send(client_socket, response.c_str(), response.length(), 0);
            close(client_socket);
            continue;
        }

        std::string transaction_id = parts[0];
        std::string timestamp = parts[1];
        std::string amount = parts[2];
        std::string username = parts[3];
        std::string password = parts[4];
        std::string received_key = parts[5];

        std::string response;
        if (users.find(username) == users.end() || users[username] != password) {
            response = "Error: Usuario o contraseña incorrectos";
            send(client_socket, response.c_str(), response.length(), 0);
            close(client_socket);
            continue;
        }

        std::string expected_key, current_ts;
        {
            std::lock_guard<std::mutex> lock(key_mutex);
            expected_key = current_dynamic_key;
            current_ts = current_timestamp;
        }

        bool auth_success = false;
        if (received_key == expected_key ||
            received_key == generate_dynamic_key(SECRET, timestamp)) {
            auth_success = true;
        }

        if (auth_success) {
            response = "Acceso exitoso: ID=" + transaction_id + ", Monto=" + amount;
            std::cout << "\n  NUEVA TRANSACCIÓN AUTORIZADA:" << std::endl;
            std::cout << "  Usuario: " << username << std::endl;
            std::cout << "  Monto: $" << amount << std::endl;
            std::cout << "  ID: " << transaction_id << std::endl;
            std::cout << "=====================================================" << std::endl;
        } else {
            response = "Error: Clave dinámica inválida";
            std::cout << "\n  TRANSACCIÓN RECHAZADA:" << std::endl;
            std::cout << "  Usuario: " << username << std::endl;
            std::cout << "  Clave recibida: " << received_key << std::endl;
            std::cout << "  Clave esperada: " << expected_key << std::endl;
            std::cout << "=====================================================" << std::endl;
        }

        send(client_socket, response.c_str(), response.length(), 0);
        close(client_socket);
    }

    close(server_fd);
    return 0;
}
