# Build from Source on Ubuntu

This document has been written for ubuntu 15.10 but it should work on debian and give a good indication for other systems

## Open Transactions Library

You must have the [open-transactions library installed](https://github.com/Open-Transactions/opentxs-notary/blob/develop/docs/INSTALL-Debian_Ubuntu.txt)

It must be the same branch as MoneyChanger: if you are building the `develop` branch f the MoneyChaner, that is the branch you want to checkout for the Open Transactions library as well
moneychanger-/moneychanger-qt` 
## Building MoneyChanger

You can try to build the `master` branch first. If that does not succeed, try the `develop` branch instead.

### QT5 and other libraries
 
  suddo apt-get install libxmlrpc-c++8-dev libqwt-qt5-6 libqwt-qt5-dev libqwtmathml-qt5-6 libqwtmathml-qt5-devqt5-default:amd64 qt5-qmake:amd64 qtbase5-dev:amd64 qtbase5-dev-tools qtchooser qttranslations5-l10n

#### clone and submodules

Do not forget to add you public key to your github account in order for the clone to work seamlessly.

  git clone git@github.com:Open-Transactions/Moneychanger.git 
  cd Moneychanger
  git submodule init
  git submodule update
  
### build

To build the app 

  cd project
  qmake
  make
  
if all went well you should have an executable file `moneychanger-qt/moneychanger-qt` 

### switching branches 

to switch branches use

    git checkout -b develop remotes/origin/develop
  
don't forget to switch branches on the open transactions library and rebuild

    cd {$PATH_TO_OT_CHECKOUT}/build
    sudo make uninstall
    git checkout -b develop remotes/origin/develop
    cmake .. && make && sudo make install
    
    
