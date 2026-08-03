# Vesper Image Generator Pro

Local image generation that runs entirely on your own machine. Download a model once, then generate offline — no API keys, no account, no network calls.

Built on [stable-diffusion.cpp](https://github.com/leejet/stable-diffusion.cpp) with a Qt6 Quick frontend, so there is no Python runtime to install and nothing phoning home.

## Status

Early but working. The app builds and runs on Windows, downloads and verifies models, generates images through the bundled engine, and exports compositions. Verified so far on Windows with MSVC and Qt 6.9.3: full build, five passing test targets, a real SD1.5 generation, and a composition exported at 4x the background resolution.

The macOS and Linux build paths are wired up in CI but have not been run on real hardware yet, and no release has been published. Treat those two platforms as untested.

## What it does

Two things that usually need separate tools.

**Generation.** txt2img and img2img against SD1.5, SDXL, or Flux models, with the controls you would expect: prompt, negative prompt, steps, CFG scale, sampler, resolution, and a seed you can lock or randomize. Step progress streams into the UI while the model samples.

**Composition.** Diffusion models are bad at text. Rather than asking one to draw a letter and getting garbled handwriting back, Vesper prompts for a background with deliberate negative space and then lays real type on top of it. The title and body copy stay live, editable text after generation — move them, resize them, change the font, export at any resolution. You can save the background and the text layers separately and reopen the composition later instead of being stuck with a flattened PNG.

## Requirements

- Qt 6.9 or newer (the UI uses `QtQuick.Effects`, added in 6.5)
- CMake 3.21+ and Ninja
- A C++20 compiler: MSVC 19.3x, Clang 15+, or GCC 12+
- Around 15 GB free disk space per large model

GPU acceleration is optional. Windows and Linux use Vulkan when a device is available, macOS uses Metal, and everything falls back to CPU.

## Building

```
git clone --recurse-submodules https://github.com/AroseEditor/Vesper-Image-Generator-Pro.git
cd Vesper-Image-Generator-Pro
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

If you already cloned without the submodules:

```
git submodule update --init --recursive
```

Pass `-DCMAKE_PREFIX_PATH=/path/to/Qt/6.9.3/<compiler>` if Qt is not on the default search path.

GPU backends are off by default. Add `-DSD_VULKAN=ON` on Windows or Linux, or `-DSD_METAL=ON` on macOS. Vulkan needs the LunarG SDK installed at configure time.

Run the tests with:

```
ctest --test-dir build --output-on-failure
```

## Models

No weights are bundled. The Models screen reads `resources/models.json` and downloads on request, resuming interrupted transfers with range requests and verifying SHA-256 before marking anything installed.

| Model | Family | Download | License |
|---|---|---|---|
| Stable Diffusion 1.5 | SD1.x | 4.27 GB | CreativeML OpenRAIL-M |
| SDXL Base 1.0 | SDXL | 6.94 GB | CreativeML OpenRAIL++-M |
| FLUX.1 schnell (Q4_K_S) | Flux | 12.3 GB across four files | Apache-2.0 |

Flux arrives as four files because the transformer, both text encoders, and the VAE ship separately. The manifest handles that internally; the UI shows one combined size and one progress bar.

Every entry links its license, and that license has to be acknowledged before the download button unlocks. Some of these models carry real usage restrictions and are worth reading before you agree to them.

Weights live outside the repository:

| Platform | Location |
|---|---|
| Windows | `%APPDATA%\Vesper\VesperImageGenerator\models` |
| macOS | `~/Library/Application Support/Vesper Image Generator Pro/models` |
| Linux | `~/.local/share/VesperImageGenerator/models` |

Delete them from Settings, which shows exactly how much space each one reclaims before you confirm.

## How generation works

Vesper bundles `sd-cli` from stable-diffusion.cpp and runs it as a subprocess, parsing its progress output and reading back the finished PNG. Shelling out keeps the engine and the UI independent, and it makes per-platform packaging far simpler than linking the library directly. Linking is a reasonable optimization later, once this path is solid.

Every result is written with a sidecar JSON holding the prompt, model, seed, sampler, and step count, so any image can be reproduced exactly.

The engine prints sampling progress to stdout as carriage-return delimited lines like `| 5/20 - 1.23s/it`, which the bridge parses into the progress bar. Model loading lines report `MB/s` instead and are ignored.

## Compositions

Text layers are stored in normalized coordinates, so export resolution is independent of the editing viewport. Export runs through `QPainter` in C++ rather than a screen grab, which is what makes a 4K export from a 900px editor possible with type that stays sharp.

Saving a composition writes two files next to each other, `name.png` and `name.composition.json`. Reopening the JSON restores every layer for further editing.

## Privacy

Once a model is on disk, Vesper makes no network requests at all. No telemetry, no crash reporting, no update check, no analytics. Generation happens on your hardware and the output never leaves it.

## License

The application license is not finalized yet. Model weights are covered by their own licenses, linked above and surfaced in the app before download — those apply independently of whatever this repository settles on.
