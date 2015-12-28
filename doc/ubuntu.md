# Build from Source on Ubuntu

This document has been written for ubuntu 15.10 but it should work on debian and give a good indication for other systems

## Open Transactions Library

You must have the [open-transactions library installed](https://github.com/Open-Transactions/opentxs-notary/blob/develop/docs/INSTALL-Debian_Ubuntu.txt)

Use the `develop` branch of the MoneyChanger and of the Open Transactions library. The `master` branch should not be used.

## Building MoneyChanger

### QT5 and other libraries
 
     sudo apt-get install libboost1.54-all-dev libxmlrpc-c++8-dev libqwt5-qt4 libqwt5-qt4-dev libqwtmathml-qt5-6 libqwtmathml-qt5-dev qt5-default:amd64 qt5-qmake:amd64 qtbase5-dev:amd64 qtbase5-dev-tools qtchooser qttranslations5-l10n

#### clone and submodules

Do not forget to add your public key to your github account in order for the clone to work seamlessly.

    git clone git@github.com:Open-Transactions/Moneychanger.git 
    cd Moneychanger
    git checkout -b develop remotes/origin/develop
    git submodule init
    git submodule update
  
### build

To build the app 

    cd project
    qmake
    make
  
if all went well you should have an executable file `moneychanger-qt/moneychanger-qt` 
