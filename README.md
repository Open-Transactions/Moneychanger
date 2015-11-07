Moneychanger (System Tray Oriented)
============
An intuitive QT/C++ system tray client for Open-Transactions.


### Development (and Manual) Build Instuctions

**Please refer to the platform specific build notes located in the "doc" directory for errata and other information.**

Download the latest moneychanger code (If you are planning to contribute please contact the devs in #opentransactions @ freenode.net irc chat)
```
git clone https://github.com/Open-Transactions/Moneychanger.git
```

#### IDE

Download QT tools (Qt Core Version 5.4.0+; 64bit compiling support Recomended for OT)

Open the moneychanger.pro file and press run.

First make sure you have installed the [Open Transactions](https://github.com/Open-Transactions/opentxs) library.

#### Manual

```
cd Moneychanger
git submodule init && git submodule update
cd project
qmake && make -j4
```

(NB: qmake will be qmake-qt4 or qmake-qt5 on linux RH, Fedora)


#### Dependencies
(Only on Unix systems)

Install the following packages with your systems packagmanager (e.g. sudo apt-get install):
```
libidn11
libidn-devel
libcurl4
libcurl-devel
opentxs
protobuf
openssl > 1.0.0 (== 1.0.1h on OSX)
libzmq
libsodium
```
Note: qjsonrpc will be installed automatically as part of the 'make' command

### Menu workaround for some window managers

Some window managers do not show the MoneyChanger icon in the
systray. If Moneychanger starts and there's no tray menu icon, the
following workaround will show the main menu in a separate window.

`touch ~/.ot/knotworkpigeons`

Then restart Moneychanger. 

### Contributing

All development goes in develop branch - please don't submit pull requests to master.

Please do *NOT* use an editor that automatically reformats.

