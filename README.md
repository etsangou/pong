# My Pixelated Pong (reupload)

This is my small Pong game in C++ using SDL2. It's intentionally simple: a low logical resolution is scaled up by drawing rectangles to get a crunchy pixel look (no shaders).

I wrote it as a tiny project you can build and play quickly.

What you get
- Play vs AI or 1 vs 1 (choose from a menu at start)
- A simple menu (native dialog) with a Quit button
- First to 3 points wins — a native dialog announces the winner, then you can pick mode again
- Minimal, easy-to-read single-file C++ code

## Quick start (Debian / Ubuntu)

Install the dependencies:

```bash
  sudo apt update
  sudo apt install -y build-essential libsdl2-dev
```

Build and run:

```bash
  make
  ./pong
```

If you want to compile manually:
```bash
  g++ -std=c++17 -O2 src/main.cpp -o pong `sdl2-config --cflags --libs`
```

Controls
- Left player: Z = up, S = down
- Right player: Up / Down arrows (only used in 1 vs 1 mode)
- Press ESC during play to go back to the mode menu (or close the window to quit)

Notes on gameplay
- At start a native dialog appears to choose mode: "Play vs AI", "Play 1 vs 1", or "Quit".
- In 1 vs 1 mode the AI is disabled and both players use the keyboard.
- In vs AI mode the right paddle is controlled by a simple AI; right-player keys are ignored.
- First to 3 points wins (change WIN_SCORE in the source if you want a different target).

Where to look in the code
- src/main.cpp: the whole program. Look near the top for the configuration constants (LOGICAL_WIDTH, PIXEL_SCALE, speeds and WIN_SCORE).
- Makefile: simple build helper.

Ideas for small improvements
- Add sound effects (SDL_mixer)
- Add a basic text score using an embedded bitmap font
- Make speeds and WIN_SCORE command-line options