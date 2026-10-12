// The kernel's audio thread calls XSDKAUD_KernelEntry, which keeps the registers the kernel's code relies on.
#include "xsdk/kernel_entry.inc"

	XSDK_KERNEL_ENTRY XSDKAUD_KernelEntry, XSDKAUD_KernelCallback
