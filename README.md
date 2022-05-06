# Wayland Tools
A collection of tools for inspecting and debugging wayland compositors. Currently, there are only two programs, but feel free to open a PR or suggest more :)

# Table of Contents
- [Wayland Tools](#wayland-tools)
  * [How to build](#how-to-build)
    + [How to install](#how-to-install)
    + [How to uninstall](#how-to-uninstall)
    + [I am having trouble installing/building/uninstalling](#i-am-having-trouble)
  * [Wayland globals (wayland-globals.c)](#wayland-globals)
  * [Wayland outputs (wayland-outputs.c)](#wayland-outputs)



## How to build
You'll need: `meson`, `ninja` and `wayland-client`, then (assuming you're not familiar with `meson`):

 1. Clone the repo
 2. Inside the repo folder, run `meson build` (here "build" could be anything, it doesn't matter)
 3. Run `ninja -C build` (assuming you used "build" in the previous step).
 
 Bash script for the lazy:
 ```bash
 git clone https://github.com/Diegovsky/wayland-tools;
 cd wayland-tools;
 meson build;
 ninja -C build
 ```
 
 You'll find the executables inside the `build/` folder (again, assuming you used "build" in step 2).
 
 ### How to install
 Basically, the same thing as building, except the `ninja` subcommand is `install` and you need write permissions to where you want the files to be installed (by default, it's `/usr/local/bin`).
 
 ```bash
 sudo ninja -C build install
 ```
 
 ### How to uninstall
 Also the same thing as installing, except the subcommand is `uninstall`:
 ```bash
 sudo ninja -C build uninstall
 ```
 
 ### I am having trouble
 Don't be afraid to open an issue so I can leave more user friendly instructions. Though, I believe it's already pretty clear.

## Wayland Globals
This executable connects to the compositor and lists all supported globals.
![image](https://user-images.githubusercontent.com/46163903/159488429-ba15e570-3d6b-4ce2-87c3-c993470ad78c.png)

## Wayland Outputs
This executable lists all outputs detected. If the compositor supports `xdg_output` protocol, it shows aditional information.
![image](https://user-images.githubusercontent.com/46163903/167226836-e3468d79-5e8c-4b81-8f8e-6245b7063b67.png)

