DROD: Gunthro and the Epic Blunder
(c) Caravel Games 2005, 2006, 2007, 2008, 2011, 2012
----------------------------
Source Module
Version 4.0.1.110
6 April 2012



Building the application
----------------------------
This is the DROD:GatEB source.
It may be used under the terms of the MPL and other licenses,
described in the Licenses subdirectory.
Windows, Linux and Mac compilations are supported.
Workspace files are included for Microsoft Visual Studio 6.0,
Visual Studio 2002, Visual Studio 2005, and Visual Studio 2008 in the Master subdirectory.

Linux builds
---------------------------
scons and makefile files for Linux are included in Master/Linux.

cd to Master/Linux and run 'scons -h' to see build options.
Options are passed to scons as 'option=value' (without the quotes), separated by spaces.
For example, if you wanted to build DROD with FMOD audio for amd64/x86-64
(which wouldn't work since FMOD 3.x doesn't exist for amd64, but hey,
let's ignore such trifling details), you would do:

scons audio=fmod arch=amd64

The dist option should be left at the default (none).

Mac builds
---------------------------
Makefiles for Mac are included in Master/Darwin.

You can attempt to build either:
* a PowerPC build, by uncommenting PPC_BUILD=1
* an Intel 32-bit build, by uncommenting arch=i386
* an Intel 64-bit build, by leaving arch=x86_64 uncommented

To use SDL_mixer instead of FMOD:
---------------------------
Add -lSDL_mixer to the link flags (LDFLAGS_* in Master/Linux/Config)
and either (1) add -DUSE_SDL_MIXER flag to the C++ flags (i.e., CXXFLAGS_* in Config) -- this will require a clean recompile, since the dependency system doesn't detect command-line changes yet; or (2) add #define USE_SDL_MIXER to the top of FrontEndLib/Sound.h -- this will make the dependency system pick up the change, so a normal recompile is sufficient.

Including content media
----------------------------
A fresh Data/drod4_0.dat file is provided, but you will need to use the drod4_0.dat file provided in an official Caravel installation of DROD (demo or full) to make this compiled source run.  Note that files obtained from a Caravel installation are protected under the DROD copyright and are not for distribution, public or private.

Graphics/styles:
Stub and mod versions of the tile and styles graphics are available,
and can be found on the Development Board on http://forum.caravelgames.com
Add these graphics files to the Data/Bitmaps directory for in-game use.
To avoid running in fullscreen mode, run the application with the "nofullscreen" command line parameter.

Music:
The music engine supports sampled sound formats (e.g. Ogg Vorbis, wave, mp3).
Add music files to Data/Music for in-game use.
Modify the [Music] section of Data/drod.ini to apply your selection of music files.
To avoid running with any sound or music,
run the application with the "nosound" command line parameter.
