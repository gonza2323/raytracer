
# Renderizador por raytracing



## Preparación del entorno


### 1. Clonar el repositorio recursivamente

```bash
git clone --recurse-submodules git@github.com:gonza2323/raytracer.git
```

Necesitamos clonar recursivamente para que se instale vcpkg, el manejador de paquetes de C++, que está incluido en el proyecto como un submódulo de git.


### 2. Configurar vcpkg (el manejador de paquetes para C++)

Entramos al directorio del proyecto y ejecutamos el script de configuración de vcpkg.

```
cd raytracer
./vcpkg/bootstrap-vcpkg.sh
```

### 3. Instalar estas dependencias

```bash
sudo apt install -y \
  build-essential git make clang clangd \
  pkg-config cmake ninja-build gnome-desktop-testing libasound2-dev libpulse-dev \
  libaudio-dev libfribidi-dev libjack-dev libsndio-dev libx11-dev libxext-dev \
  libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev \
  libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev \
  libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev libthai-dev \
  libpipewire-0.3-dev libwayland-dev libdecor-0-dev liburing-dev
```


### 4. Configuración de VSCode

#### 4.1 Instalación de extensiones

Instalar estas extensiones:

* CMake Tools de Microsoft
* C/C++ de Microsoft
* clangd de LLVM


#### 4.2 Seleccionar configuración de compilación

1. Abrir el directorio que contiene el proyecto con VSCode.
2. Al abrir el proyecto, debería preguntarnos qué configuración para compilar queremos usar (debug o release). Seleccionamos "release". Si no ocurre, presionar `Ctrl+Shift+p` y buscar el comando "Select Configure Preset" y seleccionar "Debug". Tarda bastante, así que paciencia. (*)
3. Presionar `Ctrl+Shift+p` y buscar el comando "Set Build Target" y seleccionar "raytracer". No debería hacer falta este paso, pero por las dudas.

Ya con esto, puede debuggearse y ejecutarse el programa con los botones de debug y run que se encuentran la barra inferior de VSCode (`F5` y `Ctrl+F5` no funcionan). Deberían funcionar bien los breakpoints y la ejecución paso a paso (si se seleccionó "Debug" como configuración). Si se quiere compilar sin ejecutar, hay que apretar el botón "Build"

(*) Están definidas dos configuraciones distintas para compilar el proyecto: "Debug" y "Release".

*  Debug: si compilamos con esta configuración, se incluyen símbolos de debugging para que podamos ejecutar paso a paso, colocar breakpoints, etc. Esta configuración desactiva todas las optimizaciones que puede hacer el compilador, por lo que va a andar bastante más lento.

* Release: optimiza al máximo el código, haciendo que vaya mucho más rápido. Pero como el código generado es muy distinto al fuente, se hace imposible debuggear paso a paso, colocar breakpoints, etc. Sirve más que nada para crear el ejecutable final, y para ir viendo qué tan rápido funciona verdaderamente el renderizador.


#### 4.3 Configurar Intellisense

Por defecto, el intellisense no funciona bien para las dependencias. Puede arreglarse de la siguiente forma.

1. Crear, si no existe, `./.vscode/settings.json` dentro del directorio del proyecto.

2. Incluir adentro del archivo el siguiente código:

    ```json
    {
      "C_Cpp.intelliSenseEngine": "disabled",
      "clangd.arguments": [
        "--compile-commands-dir=build/debug"
      ]
    }
    ```

3. Reiniciar VSCode.


### 5. Binarios

Cuando se compila el proyecto, el ejecutable final se encuentra en `build/debug/raytracer` o en `build/release/raytracer` dependiendo de la configuración seleccionada.

Para compilar desde consola y no a través de VSCode, se puede ejecutar:

```bash
cmake --preset debug
cmake --build build/debug
```

o con "release" respectivamente.