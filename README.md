# Cobalt Strike BOFs

A collection of **Beacon Object Files (BOFs)** authored for offensive red team and adversary-simulation engagements. Each BOF is a small, position-independent C program that runs inside a Cobalt Strike beacon process via `inline-execute`, without spawning new processes or dropping binaries to disk.

Every BOF lives in its own directory with:
- Source (`<name>.c`)
- Per-BOF `Makefile` (mingw-w64 cross-compile)
- Pre-built `<name>.x64.o`
- Dedicated `README.md` with usage and example output

## BOFs

| BOF | Path | Purpose |
| --- | --- | --- |
| Sticky Grabber | [Sticky_Grabber/](Sticky_Grabber/) | Reads the current user's Microsoft Sticky Notes WAL file and returns its contents through the beacon - quick win for credentials and secrets users have pasted into Sticky Notes. |

## Building

All BOFs cross-compile from Linux / macOS using **mingw-w64**.

```bash
# install (Debian / Ubuntu)
sudo apt install -y gcc-mingw-w64

# install (macOS, via Homebrew)
brew install mingw-w64
```

Then inside the BOF directory:

```bash
cd Sticky_Grabber
make            # produces sticky_grabber.x64.o
make clean
```

Each `Makefile` expects a shared `beacon.h` at `../../common/beacon.h` (the Cobalt Strike Arsenal Kit header). Drop your copy there or adjust `COMINCLUDE` inside the `Makefile`.

## Loading in Cobalt Strike

```
beacon> inline-execute /path/to/Sticky_Grabber/sticky_grabber.x64.o
```

For repeatable use, wrap it in an Aggressor script:

```aggressor
alias sticky_grabber {
    local('$bof_path');
    $bof_path = script_resource("Sticky_Grabber/sticky_grabber.x64.o");
    bof_load($1, $bof_path);
}
```

## OPSEC notes

- BOFs run inside the beacon process. No new process is spawned, no PE is loaded by Windows. Detection surfaces are limited to whatever the host process is already allowed to do (file I/O, registry, named pipes, etc.) and to memory-based EDR hooks.
- Each BOF in this repo uses dynamic Win32 imports through Beacon's `LIBRARY$Function` syntax, so they do **not** pull additional imports into the beacon process's IAT.
- Always run a BOF against a known target first - they are minimal by design and most do not gracefully handle missing files or permission errors.

## Requirements

| Component | Notes |
| --- | --- |
| Cobalt Strike 4.x | `inline-execute` support and the Arsenal Kit `beacon.h` header |
| mingw-w64 (`x86_64-w64-mingw32-gcc`) | Cross-compilation toolchain |
| GNU make | Driven by the per-BOF `Makefile` |

## Repository layout

```
Cobalt_Strike_BOFs/
|-- README.md                          # this file
`-- Sticky_Grabber/
    |-- README.md                      # BOF-specific docs
    |-- Makefile                       # mingw build
    |-- sticky_grabber.c               # source
    `-- sticky_grabber.x64.o           # prebuilt object
```

## Author

[@redroot97](https://github.com/redroot97) - offensive security engineer.
Issues and pull requests welcome.

## License

Released under the MIT License unless stated otherwise inside a BOF directory.
