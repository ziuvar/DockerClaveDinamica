FROM debian:bullseye-slim

RUN apt-get update && apt-get install -y \
    g++ \
    libssl-dev \
    uuid-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY client.cpp .

RUN g++ -o client client.cpp -lssl -lcrypto -luuid

EXPOSE 8080

CMD ["./client"]