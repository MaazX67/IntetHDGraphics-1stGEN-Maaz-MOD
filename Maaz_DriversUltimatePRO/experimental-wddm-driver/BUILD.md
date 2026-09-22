# Experimental WDDM research skeleton

This folder contains a deliberately non-functional kernel-mode research skeleton. It does not bind to Intel PCI devices, expose a display adapter, submit GPU commands, or replace the installed Intel driver.

## Build prerequisites

- Windows 10/11 development VM or disposable test machine
- Visual Studio with Desktop C++ tools
- Windows Driver Kit (WDK) matching the target Windows SDK
- Administrator access only for development setup

## Build

Open an **x64 Native Tools Command Prompt for VS** with the WDK environment loaded, then compile the source as a WDM test driver using the WDK build tools or a WDK Visual Studio project. This repository intentionally does not include an INF or installation target.

The source is expected to compile into a test `.sys` only after it is placed in a WDK project. It returns `STATUS_NOT_SUPPORTED` from `DriverEntry`, so it must not be loaded as a display driver.

## Next safe milestones

1. Add a WDK project with no hardware IDs and verify compilation.
2. Add Driver Verifier testing in a disposable VM.
3. Study WDDM samples and documentation before implementing any PnP or display callbacks.
4. Keep the production Intel driver active throughout research.

Do not add `PCI\\VEN_8086&DEV_0046` to an INF until a complete, tested miniport exists.
