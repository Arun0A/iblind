# iblind

A lightweight screen magnifier for X11.

![](./demo.gif)

---

## Keybindings

#### Persist Mode (`-p`/`--persist`)

In Persist Mode, the magnifier remains always-on-top and does not intercept clicks, allowing you to interact with other windows naturally. Actions require the `Alt` key modifier:

* **`Alt + +`** / **`Alt + =`** / **`Alt + Keypad +`**: Zoom in (step size: `0.25`)
* **`Alt + -`** / **`Alt + Keypad -`**: Zoom out (step size: `0.25`)
* **`Alt + Q`** / **`Alt + Escape`**: Exit the application

#### Grab Mode (`-g` / `--grab`)

In Grab Mode, the pointer is grabbed globally, allowing focus-free mouse operations. Navigational keys can be used to nudge the viewport:

* **`+`** / **`=`** / **`Keypad +`**: Zoom in
* **`-`** / **`Keypad -`**: Zoom out
* **`W`** / **`Up Arrow`**: Scroll viewport Up
* **`S`** / **`Down Arrow`**: Scroll viewport Down
* **`A`** / **`Left Arrow`**: Scroll viewport Left
* **`D`** / **`Right Arrow`**: Scroll viewport Right
* **`Right-Click`**: Move magnifier window to mouse position
* **`Scroll-Wheel Up/Down`**: Quick zoom adjustment
* **`Q`** / **`Escape`** / **`Left-Click`**: Exit the application

---

## Build from source

### Dependencies

Ensure you have `libX11` and its development headers installed.

### For Nix Systems

Start a shell containing the build dependencies, then build:

```bash
nix-shell --run "make"
```

### For Non-Nix Systems

If you have `make` installed:

```bash
make
```

Or, compile directly using `gcc`:

```bash
mkdir -p bin
gcc -O2 -Wall -Isrc src/*.c -o bin/iblind -lX11
```

The compiled binary will be located at `bin/iblind`.

---

## CLI Options

```
Usage: iblind [options]
Options:
  -w, --width <val>   Width of the magnifier box (default: 800)
  -h, --height <val>  Height of the magnifier box (default: 450)
  -x, --xpos <val>    Initial X position of the window
  -y, --ypos <val>    Initial Y position of the window
  -z, --zoom <val>    Default zoom level (default: 2.0)
  -c, --color <hex>   Border color in hex (default: 9C9C9C)
  -b, --no-border     Disable borders completely
  -p, --persist       Persist through clicks (default: enabled)
  -g, --grab          Enable grab mode (modal, exit on click)
  --help              Show this help message
```

---

## Contributing

Contributions are welcome! Feel free to open issues or submit pull requests for bug fixes, performance improvements, or new features.
