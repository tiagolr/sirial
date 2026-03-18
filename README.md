<h1 align="center">
  <!-- <img src="doc/logo.png" width="200" style="padding: 5px;" /> -->
  Sirial
  <br>
</h1>
<div align="center">

[![Windows Support](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)](https://github.com/tiagolr/sirial/releases)
[![Ubuntu Support](https://img.shields.io/badge/Linux-E95420?style=for-the-badge&logo=linux&logoColor=white)](https://github.com/tiagolr/sirial/releases)
[![Mac Support](https://img.shields.io/badge/MACOS-adb8c5?style=for-the-badge&logo=macos&logoColor=white)](https://github.com/tiagolr/sirial/releases)

</div>
<div align="center">

[![GitHub package.json version](https://img.shields.io/github/v/release/tiagolr/sirial?color=%40&label=latest)](https://github.com/tiagolr/sirial/releases/latest)
![GitHub issues](https://img.shields.io/github/issues-raw/tiagolr/sirial)
![GitHub all releases](https://img.shields.io/github/downloads/tiagolr/sirial/total)
![Github license](https://img.shields.io/github/license/tiagolr/sirial)

</div>

**Sirial** is a Rhythmic Delay where each tap can be placed and configured with different amplitudes and feedback giving total control on how the delay responds and the patterns it creates.

It is loosely based on EchoBoy Tap mode, with the novelty that it uses _serial delay lines_ instead of delay taps, this hybrid approach enables the versatility of multi-tap delays with the natural decay (optional) and coloring of standard delays, producing a more pleasant and realistic sound.

The main advantage of using serial delay lines is that any effects on the feedback path, like damping, are applied on each tap like normal delays, it also enables natural decay over the taps and more importantly allows for classic modes like Ping-Pong or cross feedback. This comes at a cost of complexity and CPU usage, not that the serial delay lines are expensive its just that multiple taps using a single delay line are extremely cheap.

This plug-in doesn't include many effects since applying them on each tap can be prohibitively costly, for example feedback pitch-shift or saturation would be computed each sample for 16 delay lines * 2 channels. The effects included are only pre or post delay, any pre or post FX can be added outside the plug-in in any DAW by using FX buses. If you are looking for a typical delay with more FXs and modes checkout [QDelay](https://github.com/tiagolr/qdelay).

<div align="center">

![Screenshot](./doc/sirial.png)

</div>

## Features

  * **Stereo Dual Delay** with independent delay times and multiple modes.
  * **Ping-Pong mode** with feedback width control.
  * **Tap mode** with offset time (tap) and delay time.
  * **Swing and Feel** offset.
  * **Accent** odd or even taps.
  * **Reverse** delay.
  * **Parametric EQ** on feedback and input signal.
  * **Diffusion** on pre or post delay signal.
  * **Modulation** of delay line time.
  * **Pitch Shifter** on the feedback or post delay signal.
  * **Saturation** on pre and post delay signal (optionally on Feedback path).
  * **Color, Bias and Dynamics** controls for saturation.
  * **Ducking** to muffle the delayed signal on input.
  * **Tape wow and flutter** to add tone variation.
  * **Taps preview** display.

## Download

* [Download latest release](https://github.com/tiagolr/sirial/releases)
* Current builds include VST3 for Windows, VST3 and LV2 for Linux and AU and VST3 for macOS.

## Tips



## MacOS

Because the builds are unsigned you may have to run the following commands:

```bash
sudo xattr -dr com.apple.quarantine /path/to/your/plugins/sirial.component
sudo xattr -dr com.apple.quarantine /path/to/your/plugins/sirial.vst3
sudo xattr -dr com.apple.quarantine /path/to/your/plugins/sirial.lv2
```

The commands above will recursively remove the quarantine flag from the plug-ins.

## Build

```bash
git clone --recurse-submodules https://github.com/tiagolr/sirial.git

# windows
cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release -S . -B ./build

# linux
sudo apt update
sudo apt-get install libx11-dev libfreetype-dev libfontconfig1-dev libasound2-dev libxrandr-dev libxinerama-dev libxcursor-dev
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -S . -B ./build
cmake --build ./build --config Release

# macOS
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" -S . -B ./build
cmake --build ./build --config Release
```
