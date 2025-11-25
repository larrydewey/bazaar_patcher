# Bazaar Theme Patcher

A surgical binary patcher that removes LGBT pride theme options from the [Bazaar](https://github.com/kolunmi/bazaar) GNOME application store.

## What It Does

Bazaar includes 11 pride flag themes for its progress bar (rainbow, lesbian, transgender, nonbinary, bisexual, asexual, pansexual, aromantic, genderfluid, polysexual, and omnisexual). This tool removes all of them from the compiled binary, leaving only the default "Accent Color" theme.

## Features

- **No recompilation needed** - patches the installed binary directly
- **High performance** - uses memory-mapped I/O for efficient patching
- **No backups** - permanently removes themes from your system
- **Auto-detection** - finds Bazaar installation automatically (Flatpak or system)

## Requirements

- Linux system with Bazaar installed
- GCC compiler
- Root/sudo access (to modify system binaries)

## Build

```bash
gcc -O3 -march=native -flto -Wall -Wextra -o bazaar_patcher bazaar_patcher.c
```

## Usage

```bash
# Auto-detect Bazaar installation and patch
sudo ./bazaar_patcher

# Or specify path manually
sudo ./bazaar_patcher /usr/local/bin/bazaar

# For Flatpak installations
sudo ./bazaar_patcher /var/lib/flatpak/app/io.github.kolunmi.Bazaar/current/active/files/bin/bazaar
```

**Important**: Close Bazaar before running the patcher.

## How It Works

1. Locates the Bazaar binary (Flatpak or system installation)
2. Memory-maps the binary file for efficient I/O
3. Searches for all pride theme string literals
4. Overwrites them with null bytes (0x00)
5. Syncs changes to disk
6. The UI will only show "Accent Color" as available theme

## Technical Details

The patcher targets hardcoded theme definitions in the binary:

```c
static const BarTheme bar_themes[] = {
  {       "accent-color",  "accent-color-theme",             N_ ("Accent Color") },
  { "pride-rainbow-flag", "pride-rainbow-theme",             N_ ("Pride Colors") },
  // ... etc
};
```

It nulls out all non-default theme strings, effectively removing them from the array without corrupting the binary structure.

## Limitations

- You'll need to re-run this after any Bazaar updates
- The patched binary is permanent (no backups created)
- May not work if Bazaar changes its internal structure in future versions

## Why C?

Honestly, because C is pretty great.

## Disclaimer

This modifies application binaries. Use at your own risk. If you break your Bazaar installation, just reinstall it.
