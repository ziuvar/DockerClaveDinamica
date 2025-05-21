FROM debian:bullseye-slim

RUN apt-get update && apt-get install -y \
    g++ \
    libssl-dev \
    uuid-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY server.cpp .

RUN g++ -o server server.cpp -lssl -lcrypto -luuid -lpthread

EXPOSE 8080

CMD ["./server"]

