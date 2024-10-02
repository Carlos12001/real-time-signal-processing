[![Open in Visual Studio Code](https://classroom.github.com/assets/open-in-vscode-718a45dd9cf7e7f842a935f5ebbe5719a5e09af4491e668f4dbf3b35d5cca122.svg)](https://classroom.github.com/online_ide?assignment_repo_id=11659015&assignment_repo_type=AssignmentRepo)
# Ejemplo base de Jack en C++

Este ejemplo construye una aplicación muy sencilla de "pass-through"
usando Jack, como punto de partida para los proyectos del curso.

## Dependencias

Requiere C++ en su estándar del 2020 (g++ 12, clang 14).

En derivados de debian (ubuntu, etc):

```bash
     sudo apt install jackd2 libjack-jackd2-dev qjackctl build-essential meson ninja-build
```
 
Jack requiere que su usuario pertenezca al grupo audio, o de otro modo
no tendrá privilegios para el procesamiento demandante en tiempo
real...

```bash
     sudo usermod -aG audio $USER
```

## Construcción

Para construir los ejemplos la primera vez utilice

```bash
     meson setup build
     cd build
     ninja
```

Si requiere reconstruir todo, utilice

```
     meson setup --wipe build
     cd build
     ninja
```

## Latencia y tamaño de bloque

Para reducir la latencia por medio del tamaño del "periodo" (esto es,
el número de "frames" que cada ciclo de procesamiento recibe, en
QjackCtl, en Settings, se indica en Frames/Period.  Eso es un
parámetro del servidor de Jack y no lo puede controlar la aplicación
como tal.

circular_buffer not queue


