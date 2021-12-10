Fabla - OpenAV Productions
==========================

Official page: http://openavproductions.com/fabla

Documentation: http://openavproductions.com/doc/fabla.html

This is the repository of a sampler LV2 plugin called Fabla.

![screenshot](https://raw.githubusercontent.com/openAVproductions/openAV-Fabla/master/gui/fabla.png "Fabla 1.3 Screenshot")

Dependencies
------------
```
ntk  (git clone git://git.tuxfamily.org/gitroot/non/fltk.git ntk)
 or  (git clone https://git.kx.studio/non/ntk ntk)
cairo
cairomm-1.0
sndfile
lv2
```

Install
-------
Once deps are satisfied, building and installing into ~/.lv2/ is easy,
just run CMake as usual:
```
mkdir build
cd build
cmake ..
make
make install
```

Running
-------
After the `Install` step Ardour3, QTractor, and any other LV2 host should
automatically find Fabla, and be able to use it.

If you have the JALV LV2 host installed, the "run.sh" script can be used to
launch Fabla as a standalone JACK client:
```
$ ./run.sh
```

Midi Mapping
------
Fabla responds to midi notes on all midi channels.
The pads **1-16** map to midi notes **36-52**, anything outside that range
is clamped to the closest value, *midi note 0-36 triggers pad 1*, *midi note 52-127 triggers pad 16*.

Contact
-------
If you have a particular question, email me!
```
harryhaaren@gmail.com
```

Cheers, -Harry
