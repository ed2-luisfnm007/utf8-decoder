# UTF-8 Decoder - Luis Noriega

## Instrucciones de Compilación y Ejecución

Este proyecto requiere un compilador compatible con C++20.

### Opción 1: Compilar directamente con g++

Desde la raíz del proyecto:

```bash
g++ -std=c++20 src/main.cpp src/decoder.cpp -o utf8-decoder
```

Luego, ejecutar:

```bash
./utf8-decoder ruta_del_archivo.txt
```

### Opción 2: Compilar mediante CMake

Desde la raíz del proyecto:

```bash
cmake -S . -B build
cmake --build build
```

Luego, ejecutar:

```bash
./build/utf8-decoder ruta_del_archivo.txt
```
