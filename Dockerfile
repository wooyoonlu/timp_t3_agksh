FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-sqlite \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN qmake6 echoServer.pro && make

EXPOSE 33333

CMD ["./echoServer"]
