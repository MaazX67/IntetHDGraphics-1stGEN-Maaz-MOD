# MaazXGPU Experimental WDDM Driver

This directory is reserved for a future **experimental Intel HD Graphics WDDM driver research project**. It is intentionally not a fake or installable `.sys` driver.

## Important limitations

- The existing MaazXGPU project is a user-mode performance layer and remains separate.
- A functional replacement for the Intel driver requires Intel hardware documentation, register specifications, firmware details, WDDM knowledge, and extensive testing.
- An unsigned or incorrectly implemented display driver can cause boot failures, black screens, TDR loops, or data loss.
- This project must not claim to support `PCI\\VEN_8086&DEV_0046` until a real miniport implementation has been tested on matching hardware.

## Planned components

```text
experimental-wddm-driver/
  README.md
  design/
    architecture.md
  src/
    README.md
  inf/
    README.md
```

The first milestone is documentation and a non-hardware test harness. Only after that should a kernel-mode miniport skeleton be considered.

## Development policy

1. Never replace or disable the working Intel driver during development.
2. Test only inside a disposable Windows VM or with a recovery path.
3. Use Windows Driver Kit (WDK) and Visual Studio for kernel builds.
4. Use test-signing only on a development machine; production installation requires appropriate Microsoft/Windows signing.
5. Do not install an INF until a real `.sys`, matching device IDs, service section, and test certificate exist.

## Performance expectation

A new driver will not automatically be faster. A correct driver may initially perform worse than Intel's mature driver. The current user-mode performance layer is the practical path for Minecraft, Roblox, and DX12 tuning.
