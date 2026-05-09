
# Renderizador por raytracing



## Preparación del entorno


### 1. Clonar el repositorio RECURSIVAMENTE

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
  build-essential \
  cmake \
  clang \
  clangd \
  ninja-build \
  git \
  curl \
  zip unzip \
  pkg-config \
  python3 \
  autoconf autoconf-archive automake libtool \
  libx11-dev libxft-dev libxext-dev \
  libwayland-dev libxkbcommon-dev libegl1-mesa-dev \
  libibus-1.0-dev
```

### 4. Extensiones VSCode

Instalar estas extensiones:

* CMake Tools de Microsoft
* C/C++ de Microsoft
* clangd de LLVM


### 5. Cómo desarrollar con VSCode

1. Abrir el directorio que contiene el proyecto con VSCode.
2. Presionar `Ctrl+Shift+p` y buscar el comando "Select Configure Preset" y seleccionar "Debug" (*).
3. Presionar `Ctrl+Shift+p` y buscar el comando "Set Build Target" y seleccionar "raytracer" (*).

Ya con esto, puede debuggearse y ejecutarse el programa con los botones de debug y run en la barra inferior de VSCode (`F5` y `Ctrl+F5` no funcionan). Deberían funcionar bien los breakpoints y la ejecución paso a paso (si se seleccionó "Debug" como configuración).

(*) Están definidas dos configuraciones distintas para compilar el proyecto: "Debug" y "Release".

*  La opción "Debug" compila el proyecto, incluyendo símbolos de debugging para que podamos ejecutar paso a paso, colocar breakpoints, etc. Esta configuración desactiva todas las optimizaciones que puede hacer el compilador, por lo que va a andar bastante más lento.

* La configuración "Release" optimiza al máximo el código, haciendo que vaya mucho más rápido. Pero como el código generado es muy distinto al fuente, se hace imposible debuggear paso a paso, colocar breakpoints, etc. Sirve más que nada para el ejecutable final, y para ir viendo qué tan rápido funciona el raytracer.
