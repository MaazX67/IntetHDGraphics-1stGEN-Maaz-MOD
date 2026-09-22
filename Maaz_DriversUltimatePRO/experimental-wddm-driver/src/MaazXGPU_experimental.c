#include <ntddk.h>

// MaazXGPU experimental kernel entry point.
// This is intentionally inert: it does not register a device, bind to PCI,
// expose WDDM callbacks, touch hardware registers, or replace Intel's driver.

static VOID MaazXgpuUnload(_In_ PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
}

_Use_decl_annotations_
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    if (DriverObject == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    DriverObject->DriverUnload = MaazXgpuUnload;

    // Deliberately refuse loading until a real, tested WDDM miniport exists.
    return STATUS_NOT_SUPPORTED;
}
