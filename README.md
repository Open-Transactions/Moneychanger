Moneychanger (System Tray Oriented)
============
An intuitive QT/C++ system tray client for Open-Transactions.


## Development (and Manual) Build Instuctions

Download the latest moneychanger code (If you are planning to contribute please contact the devs in #opentransactions @ freenode.net irc chat)

git clone https://github.com/Open-Transactions/Moneychanger.git

#### IDE

Download QT tools (Qt Core Version 5.1.0+; 64bit compiling support Recomended for OT)

Open the moneychanger.pro file and press run. First make sure you have installed OT (https://github.com/Open-Transactions/ or at https://github.com/FellowTraveler/Open-Transactions)

#### Manual

cd Moneychanger/project

qmake

make

(NB: qmake will be qmake-qt4 or qmake-qt5 on linux RH, Fedora)
