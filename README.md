# darev

Nothing to see here at the moment unless you're interested in PS2 programming.

This is a fledgling re-implementation of parts of the Baldurs Gate Dark Alliance engine for the PS2.
You may ask what is the point ... and there really is no point other then curiosity and to gain an understanding of
how games drove the PS2 over 20 years ago.

This isn't a decompilation project - the code here will be different than what is in the original game. I'm aiming for the output
to look the same as the original as far as possible.

There is nothing in this repository that is copied from elsewhere. All data is read at run-time from the original game.
The code is not clean room - I've looked at the disassembly to understand how to parse some of the data files.
I don't think it harms the original developer - you can only use this if you own the original game.

## Building

To build you'll need a working PS2 toolchain. Look at the ps2dev project on github. If you can't build and run the samples from that,
you're not ready for this yet. you'll also need mkisofs from cdrtools to build the ISO.

You'll need the PS2 PAL version of Baldurs Gate Dark Alliance (BGDA). The US version is unlikely to work because the code uses some resources from the elf and I guess these have a different offset in the US version.
You need to copy the files from the CD into the fs directory.
Right now you only need:

    BG/DATA/FX.LMP
    BG/DATA/HUD.LMP
    BG/DATA/LANGMENU.LMP
    SLES_506.72

once that's done, running make should build you an ISO that can be run on an emulator or read hardware via OPL.

## Tests

There are some unit test in the tests directory. Those run on a host system and are built using meson. See the readme in that directory.

## Status

The language select menu works. That's it so far. The DMA handling code is quite interesting though.
