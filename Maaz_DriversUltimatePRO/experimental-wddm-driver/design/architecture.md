# Experimental architecture

A real WDDM display stack would need at least:

- Kernel-mode display miniport (`.sys`)
- Direct3D user-mode display driver
- DXGI integration
- Memory manager integration
- Command submission and synchronization
- Interrupt and fence handling
- TDR/recovery support
- Display/output and power management
- Correct INF and signed package metadata

The current repository does not contain enough hardware-level code to implement these components. Do not create placeholder functions that pretend to submit commands to Intel hardware.

## Milestones

### M0: research

- Identify the exact GPU model and PCI device ID.
- Record Windows version, existing Intel driver version, WDDM version, RAM, and BIOS version.
- Collect public documentation and build a read-only hardware information tool.

### M1: isolated kernel skeleton

- Build a WDK sample in a VM.
- Validate signing, installation, service loading, and recovery.
- Do not bind it to the physical Intel device.

### M2: hardware interface research

- Implement only documented probing and validation.
- No display output, memory allocation, or command submission until hardware behavior is known.

### M3: conformance and performance

- Run Driver Verifier only in a disposable environment.
- Test suspend/resume, display mode changes, process termination, and TDR.
- Compare against the Intel driver before considering replacement.
