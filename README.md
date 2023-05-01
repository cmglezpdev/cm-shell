# CMSH Shell
> Proyecto de Sistema Operativo (Ciencias de la Computación)
> Carlos Manuel González Peña

Este proyecto tiene como objetivo la creación de un shell para Linux en c.

## Dependencias
El proyecto se encuentra desarrollado en C, por lo que tienes que tener C instalado

## Ejecutar el Proyecto

Solo debe ejecutar el siguente comando

```bash
make
```

o si no te funciona puedes poner el comando directamente

```bash
gcc main.c builtin.c parser.c execute.c utils.c list.c -o shell && ./shell
```

## Funcionalidades:

- `basic:` funcionalidades básicas.
- `pipes:` implementación de múltiples tuberías.
- `background:` permite correr procesos en el background.
- `spaces:` los comandos pueden estar separados por cualquier cantidad de espacios.
- `history:` se almacena un historial de comandos.
- `ctrl+c`: finaliza el proceso actual.
- `chain:` permite ejecutar múltiples comandos en una sola línea y comandos de forma condicional.
- `conditional:` permite ejecutar comandos de forma condicional.
- `variables:` permite almacenar variables.

## Comandos:

- `cd:` cambia de directorio.
- `exit:` finaliza la ejecución del shell.
- `fg:` trae hacia el foreground el último proceso enviado al background.
- `jobs:` lista todos los procesos en el background.
- `history:` muestra el historial de comandos.
- `again:` ejecuta un comando almacenado en el historial.
- `true:` representa un comando que siempre se ejecuta con éxito.
- `false:` representa un comando que nunca se ejecuta con éxito.
- `get:` muestra el valor de las variables.
- `set:` modifica el valor de una variable.
- `unset:` elimina una variable.

Los detalles de las funcionalidades y la ejecución se detallan dentro del propio programa mediante el comando help.