# MaazXGPU game-focused performance layer

The tray profile targets Minecraft Java/Bedrock, Roblox, and selected DX12 executables. It uses conservative, reversible user-mode changes: `HIGH_PRIORITY_CLASS`, disabling execution-speed throttling when permitted, periodic process detection, and a low-memory VRAM cap.

It does **not** guarantee 30 FPS. Actual FPS depends on the Intel HD Graphics generation, driver, resolution, game settings, thermals, RAM speed, and whether the game supports the API. Do not use `REALTIME_PRIORITY_CLASS`, registry hacks, overclocking, or fake VRAM.

Use the tray menu:
- **FPS Boost**: enables a 5-second scan for supported game processes.
- **Boost Minecraft now**: targets `javaw.exe` candidates.
- **Boost supported games now**: targets Roblox, Minecraft Bedrock, and selected DX12 names.
- **Balance RAM/VRAM**: applies the conservative low-memory profile.

For a 4GB Vostro 3300, start games at 1280x720 or lower, use low settings, disable shaders, cap render distance, close browsers, and use the correct Intel graphics driver. The software layer cannot turn shared RAM into dedicated VRAM or bypass hardware limits.
