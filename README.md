BalooTextSearch - KRunner plugin for search in text files using baloo.
======================================================================


This is a copy of the original baloo runner present in the 5.8 branch
of the plasma source code and it was changed to search for text files
instead of documents, images, etc... so source code files can be found
using krunner.


Install
-------

To install the plugin you must install the dependencies firt. In a
debian system you can use:

```
aptitude install cmake extra-cmake-modules build-essential libkf5runner-dev \
	         libkf5textwidgets-dev kio-dev baloo-kf5-dev
```

After that clone the source code

```
$ git clone
```

Enter in the code dir and create a build directory:

```
$ mkdir build && cd build
```

Now, build the software

```
$ cmake .. -DCMAKE_INSTALL_PREFIX=`kf5-config --prefix` -DQT_PLUGIN_INSTALL_DIR=`kf5-config --qt-plugins`
$ make
```

As root, install it:

```
# make install
```

And finally, as a normal user again, restart Krunner

```
$ kquitapp5 krunner 2> /dev/null
$ kstart5 --windowclass krunner krunner 2> /dev/null
```

That's it!