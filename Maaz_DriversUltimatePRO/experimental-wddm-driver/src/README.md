# Experimental kernel skeleton

`MaazXGPU_experimental.c` is an inert WDM entry-point sample for WDK compilation tests. It deliberately returns `STATUS_NOT_SUPPORTED` and does not implement a display miniport.

It must not be installed, assigned to a PCI device, or used as a replacement for the Intel display driver.

A future real implementation would need separate, reviewed code for PnP/power callbacks, memory management, command submission, synchronization, interrupts, TDR recovery, and WDDM user-mode interfaces. Those parts are intentionally absent.
