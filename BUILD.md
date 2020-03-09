- [dependencies](#sec-1)
  - [install python2.7](#sec-1-1)
  - [install and build boost<sub>python</sub> agains python2.7](#sec-1-2)
- [make based build system](#sec-2)
  - [copy ThirdParty under src/ directory](#sec-2-1)
  - [edit *src/make/head.mk*, change **boost<sub>DIR</sub>** and **PYTHON<sub>BIN</sub>** accordingly](#sec-2-2)
  - [invoke *make* command](#sec-2-3)
    - [a new *release* directory will be created with all targets inside](#sec-2-3-1)
- [Caveants](#sec-3)
  - [change libev.so.4.0.0 to libev.so](#sec-3-1)
    - [only dynamic linkage success right now, to be investigated later](#sec-3-1-1)

# dependencies<a id="sec-1"></a>

## install python2.7<a id="sec-1-1"></a>

## install and build boost<sub>python</sub> agains python2.7<a id="sec-1-2"></a>

# make based build system<a id="sec-2"></a>

## copy ThirdParty under src/ directory<a id="sec-2-1"></a>

## edit *src/make/head.mk*, change **boost<sub>DIR</sub>** and **PYTHON<sub>BIN</sub>** accordingly<a id="sec-2-2"></a>

## invoke *make* command<a id="sec-2-3"></a>

### a new *release* directory will be created with all targets inside<a id="sec-2-3-1"></a>

# Caveants<a id="sec-3"></a>

## change libev.so.4.0.0 to libev.so<a id="sec-3-1"></a>

### only dynamic linkage success right now, to be investigated later<a id="sec-3-1-1"></a>
