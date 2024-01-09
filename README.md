# ps2-unpacker

PS2-Unpacker version 0.1.2
==========================

Overview
--------

  This (very early, highly alpha) software is designed to unpack PS2 elf. It
uses lots of pcsx2's internal code ( http://pcsx2.net ). It will load your elf,
and start emulating it, until it think it has reached the entry point.


Changelog
---------

  2004/08/14: release of version 0.1, first version.
  2004/08/16: addded trace option
              release of version 0.1.1
  2005/04/19: added more raw options per popular request.
              release of version 0.1.2


Todo
----

  -) Add full COP0 support maybe ?
  -) Add multiple section dump ?
  -) Add other syscalls supports ?
  -) Better bss detection ?
  -) Improve code in general :p


Source code and legal stuff
---------------------------

  This code is covered by GPL. Actually, I don't give a shit about licenses
and stuff. If you don't like it covered by GPL, just ask me, and we'll change
it. The only problem is it uses modified version of a lot of GPLed code,
especially pcsx2, so...

  Beeing a ps2dev.org developper got me banned from ps2ownz.com's website. Thus,
as a special exception to the GPL, since I can not go on their forums, and react
and help people about that software, I to NOT give them the autorization to
mirror these packages on their website, only to link to the homepage of this
project, that is, http://www.nobis-crew.org/ps2-unpacker nor are they authorized
to support their users on their forums on questions about that software.

  If you want to reach me, and find support about ps2-unpacker, either ask me
directly, by mail, or by reaching me on IRC (channel #ps2dev on EfNet), or ask
your questions on ps2dev.org's forums and ps2-scene's forums.

  I actually know they won't give a shit about these restrictions, but I felt
like proceeding so. If you want real and *legit* ps2 development, go on the
genuine ps2dev website, that is, http://ps2dev.org


How it works
------------

  Usage: ps2-unpacker [-t] [-l lo] [-i hi] [-a align] <in_elf> <out_elf>

  Options description:
    -t             activate output trace.
    -l low         sets the low address of the final dumped binary. Otherwise,
                      it is auto detected.
    -i high        sets the high address of the final dumper binary. Otherwise,
                      it is auto detected.
    -a align       sets section alignment. 16 by default. Any value accepted.
                      However, here is a list of alignments to know:
		1 - no alignment, risky, shouldn't work.
		4 - 32-bits alignment, tricky, should on certain loaders.
	       16 - 128-bits alignment, normal, should work everywhere.
	      128 - 128-bytes alignment, dma-safe.
	     4096 - supra-safe, this is the default alignment of binutils.

  Now, you have to understand the mechanisms.

  The emulation core will run, and tag the memory during the long run. When the
execution reaches the ExecPS2 syscall, or a jump to a written part of the
memory, it will consider that the job is done. The bloc of non-zero written
memory around the detected entry point will be dumped. The low address will be
aligned to the alignment specified.

  If needed, you can force the low and high dumping addresses, if they were not
detected correctly.


Bugs and limitations
--------------------

-) It's poorly coded :-P
-) Some of the packed elf I tested just went wrong.


How to compile
--------------

  A simple "make" should do the trick in order to compile everything.

  Don't look at the 'dist' target in the Makefile, it's only for me to build
the various packages.


Author
------

  Nicolas "Pixel" Noble <pixel@nobis-crew.org> - http://www.nobis-crew.org


Where to find
-------------

  The "official" webpage for this tool is at on my personal webspace:

    http://www.nobis-crew.org/ps2-unpacker/

  However, you can find the latests CVS changes into ps2dev's CVS:

    http://cvs.ps2dev.org/ps2-unpacker/

  For more informations about it, feel free to go on ps2dev's website located
at http://ps2dev.org/ and be sure to drop by #ps2dev in EfNet.


Thanks and greetings
--------------------

  They go to adresd, blackd_wd, drakonite, emoon, gorim, hiryu, herben, jenova,
linuzapp, oobles, oopo, mrbrown, nagra, neov, nik, t0mb0la, tyranid

and maybe a few other people I forgot at #ps2dev :)
