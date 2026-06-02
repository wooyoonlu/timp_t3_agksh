# Variant 3 TCP Server

Qt TCP server for a programming technology course assignment. The server
listens on TCP port `33333`, parses text requests, stores processed requests in
SQLite, and implements the algorithms assigned to variant 3.

## Database singleton

`Database::instance()` returns the only `Database` object in the application.
The object opens `server_database.db` next to `echoServer.pro` and creates the
`requests` table. Each processed request is stored with a UTC timestamp and its
response. The server searches parent directories for the project file, so the
database stays next to the source files when the executable is launched from a
build directory.

For tests, the default location can be changed with `SERVER_DATABASE_PATH`.
For example, `SERVER_DATABASE_PATH=:memory:` uses a temporary in-memory SQLite
database.

## Request protocol

Send one UTF-8 request per line. A request consists of a command and arguments
separated with `|`.

| Request | Description |
| --- | --- |
| `encrypt|LEMON|Attack at dawn` | Vigenere encryption |
| `decrypt|LEMON|Lxfopv ef rnhr` | Vigenere decryption |
| `hash|hello` | SHA-512 digest |
| `bisection|1|2|0.000001` | Root of `x^3 - x - 2` on an interval |
| `shortest_path|A|D|A,B,1;B,D,2;A,D,10` | Dijkstra shortest path |
| `show_db` | Formatted rows from the SQLite request journal |
| `db_info` | SQLite database path used by the server |

For compatibility with simple clients, a request without a newline is
processed after a short delay. Newline-delimited requests are recommended.

## Build

Run qmake and then the make tool installed with Qt:

```text
qmake6 echoServer.pro
mingw32-make
```

For a standalone Windows build, deploy the Qt DLL files and plugins:

```text
windeployqt6 release/echoServer.exe
```

## Doxygen

Generate HTML documentation from the documented source code:

```text
doxygen Doxyfile
```

Open `docs/html/index.html` after generation.

## Unit tests

The `UnitTest` directory contains Qt Test checks for the variant 3 algorithms
and the database singleton. Build and run them separately:

```text
powershell -ExecutionPolicy Bypass -File UnitTest/run_unit_tests.ps1
```

The script stores the latest text report in `UnitTest/UnitTest_Result.txt`.
