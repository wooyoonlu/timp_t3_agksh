FROM ubuntu:22.04

# Устанавливаем зависимости
RUN apt-get update && apt-get install -y \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-sqlite \
    build-essential \
    git \
    && rm -rf /var/lib/apt/lists/*

# Устанавливаем qmake6 как qmake (симлинк)
RUN ln -s /usr/lib/qt6/bin/qmake /usr/bin/qmake || true

WORKDIR /app

# Копируем исходники
COPY *.cpp *.h *.pro ./

# Собираем проект
RUN qmake echoServer.pro && make

# Открываем порт
EXPOSE 33333

# Запускаем сервер
CMD ["./echoServer"]
