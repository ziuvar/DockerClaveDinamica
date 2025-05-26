Sistema de Autenticación con Clave Dinámica 🔐
Un sistema de autenticación seguro implementado en C++ que utiliza claves dinámicas que cambian cada 30 segundos, desplegado con Docker para demostrar conceptos avanzados de seguridad en aplicaciones cliente-servidor.

Características Principales

Autenticación Multi-Factor: Combina credenciales tradicionales con claves dinámicas basadas en tiempo (TOTP-like)
Claves Rotativas: Las claves se regeneran automáticamente cada 30 segundos usando HMAC-SHA256
Arquitectura Cliente-Servidor: Comunicación segura via sockets TCP
Containerización: Desplegado completamente en Docker para portabilidad
Sincronización Temporal: Cliente y servidor sincronizan períodos de tiempo para validación de claves
Interfaz Intuitiva: Terminal con interfaz de usuario clara y mensajes informativos

Arquitectura del Sistema
┌─────────────────┐         ┌─────────────────┐
│     Cliente     │   TCP   │    Servidor     │
│   (Terminal)    │◄──────►│  (Generador)    │
│                 │  :8080  │                 │
│ • Auth Usuario  │         │ • Clave TOTP    │
│ • Input Clave   │         │ • Validación    │
│ • Transacciones │         │ • Logs          │
└─────────────────┘         └─────────────────┘
🔧 Tecnologías Utilizadas

Lenguaje: C++17
Criptografía: OpenSSL (HMAC-SHA256)
Red: Sockets BSD
Identificadores: UUID v4
Contenedores: Docker & Docker Compose
OS Base: Debian Bullseye

Instalación y Uso
Prerrequisitos

Docker Engine 20.10+
Docker Compose v2+

Método 1: Usando Docker Compose (Recomendado)
bash# Clonar el repositorio
git clone <repository-url>
cd clave-dinamica-docker

# Levantar los servicios
docker-compose up -d

# Ver la clave dinámica en el servidor
docker logs -f clavedinamicadocker-server

# En otra terminal, ejecutar el cliente
docker exec -it clavedinamicadocker-client ./client
Método 2: Construcción Manual
bash# Construir las imágenes
docker build -t clave-server ./server
docker build -t clave-client ./client

# Crear red personalizada
docker network create clave-network

# Ejecutar servidor
docker run -d --name server --network clave-network -p 8080:8080 clave-server

# Ejecutar cliente
docker run -it --name client --network clave-network clave-client
🎮 Guía de Uso
1. Iniciar el Sistema
Después de ejecutar docker-compose up -d, verás en el servidor:
=====================================================
       SERVIDOR DE AUTENTICACIÓN - CLAVE DINÁMICA    
=====================================================
  CLAVE ACTUAL: a1b2c3d4
  Timestamp:    2025-05-26T10:30:00Z
  Válida por:   25 segundos
=====================================================
  Esperando conexiones en puerto 8080...
=====================================================
2. Usar el Cliente
Ejecuta el cliente y sigue las instrucciones:
bashdocker exec -it clavedinamicadocker-client ./client
3. Credenciales de Prueba
El sistema incluye usuarios predefinidos:

Usuario: user1 | Contraseña: pass123
Usuario: user2 | Contraseña: pass456

4. Realizar una Transacción

Ingresa las credenciales del usuario
Especifica el monto a transferir
Copia la clave dinámica del servidor (se muestra en tiempo real)
Pégala en el campo "clave dinámica" del cliente
La transacción se procesa instantáneamente

Aspectos de Seguridad
Algoritmo de Generación de Claves
cpp// Pseudocódigo del algoritmo
timestamp = current_time_truncated_to_30s()
secret_key = "clave_secreta_123"
dynamic_key = HMAC_SHA256(secret_key, timestamp)[0:8]
Ventajas de Seguridad

Tiempo Limitado: Las claves expiran cada 30 segundos
No Reutilizable: Cada clave es válida solo una vez por período
Resistente a Replay: Los timestamps previenen ataques de repetición
Autenticación Fuerte: Requiere algo que sabes (password) + algo que tienes (clave dinámica)

Consideraciones de Producción
⚠️ Importante: Este es un proyecto educativo. Para uso en producción considera:

Usar secretos más robustos y rotativos
Implementar TLS/SSL para cifrado de transporte
Añadir rate limiting y protección contra brute force
Usar bases de datos seguras para almacenar usuarios
Implementar logging y auditoria completa

Estructura del Proyecto
clave-dinamica-docker/
├── README.md
├── docker-compose.yml
├── server/
│   ├── Dockerfile
│   └── server.cpp
└── client/
    ├── Dockerfile
    └── client.cpp
🐛 Resolución de Problemas
El cliente no puede conectar al servidor
bash# Verificar que el servidor esté corriendo
docker ps

# Ver logs del servidor
docker logs clavedinamicadocker-server

# Reiniciar los servicios
docker-compose restart
Error de "Clave dinámica inválida"

Asegúrate de copiar la clave exactamente como aparece en el servidor
Verifica que no haya espacios adicionales
La clave debe usarse dentro de su ventana de validez (30 segundos)

Problemas de compilación
bash# Limpiar y reconstruir
docker-compose down
docker-compose build --no-cache
docker-compose up -d
🧪 Casos de Prueba
Prueba 1: Autenticación Exitosa

Usuario: user1, Password: pass123
Clave dinámica válida del servidor
Resultado esperado: ✅ "Transacción exitosa"

Prueba 2: Credenciales Incorrectas

Usuario: wronguser, Password: wrongpass
Resultado esperado: ❌ "Usuario o contraseña incorrectos"

Prueba 3: Clave Expirada

Usar una clave de un período anterior
Resultado esperado: ❌ "Clave dinámica inválida"

