MaazXGPU - Pure Performance Driver for Vostro 3300

Files Maded for Dell Vostro 3300 / Intel HD Graphics (PCI\VEN_8086&DEV_0046).
This project is a user-mode safe compatibility layer. It does not include or install a kernel display driver.
The existing Intel driver must remain installed. No build was executed.

To compile later, from this folder run:
cmake -B Build -G "Visual Studio 17 2022" -A x64
cmake --build Build --config Release

The generated binaries are placed in the Build output directory by CMake.
