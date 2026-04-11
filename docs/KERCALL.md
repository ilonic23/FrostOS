# Introduction to FrostOS API via KERCALLs
---

## Basic usage:
1. Set EAX register to the function we want to use.
2. Call ``int 99h``.

## Example:
```asm
mov eax, 3003 ; Select the legacy reboot function
int 99h
```

> [!note]
> The value in eax is decimal.

## Legacy functions
The current system API state is in development, that's the reason there are only
legacy functions for now.

| **EAX** | EBX | ECX | EDX | Return Values | Description |
| ------- | --- | --- | --- | ------------- | ----------- |
| 3000    |  Low ms amount  |  High ms amount  |  -  |      -        | **DO NOT USE! SYSTEM WILL BE STUCK**. Sleep for a specified ms amount. |
| 3001    | - | - | - | EAX - How many ms elapsed from system boot (low). EBX - How many ms elapsed from system boot (high). | Returns the amount of ms elapsed from system boot as a 64-bit value. |
| 3002    | - | - | - | EAX - pointer to a `datetime_t` structure with current time. | Returns a pointer to current time. |
| 3003    | - | - | - | - | Reboots the machine. |
| 3004    | - | - | - | - | Halts the system. Means you can safely turn it off. |
| 3005    | - | - | - | - | Sets the frequency of the PC speaker to 440Hz |
| 3006    | Desired frequency | - | - | - | Sets the frequency of the PC speaker to a specific Hz value. |
| 3007    | - | - | - | - | Stops the PC speaker. |
| 3008    | - | - | - | EAX - Current frequency in Hz | Returns the current frequency of the PC speaker. |
| 3009    | - | - | - | - | Reserved. |
| 3010    | - | - | - | - | Clears the display with current background color. |
| 3011    | R value | G value | B value | - | Sets current display's foreground color to an RGB value specified. |
| 3012    | R value | G value | B value | - | Sets current display's background color to an RGB value specified. |
| 3013    | Character | - | - | - | Prints an ASCII character EBX at current position. |
| 3014    | Pointer to a null terminated string. | - | - | - | Prints an ASCII string pointed by EBX at current position. |
| 3015    | Cursor X | Cursor Y | - | - | Sets current cursor position. |

## Datetime:
The `datetime_t` structure looks like this:
```c
typedef struct {
	unsigned int year : 12;
	unsigned int month : 4;
	unsigned int day : 5;
	unsigned int hours : 5;
	unsigned int minutes: 6;
	unsigned int seconds : 6;
	unsigned int milliseconds : 10;
} __attribute__ ((packed)) datetime_t;
```
Sooner the Datetime API will be reworked for more easy usage.

## PC Speaker:
> [!warning]
> The speaker may break on high frequencies, up to 2000Hz is probably okay.

## Display:
The cursor position is set by pixels when using Framebuffer mode, but in characters
when using VGA text mode (80x25).
