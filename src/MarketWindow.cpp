//
//  MarketWindow.cpp
//  moneychanger-qt
//
//  Created by Jonathan Rumion on 9/11/13.
//
//

#include "MarketWindow.h"


MarketWindow::MarketWindow( QWidget *parent )
: QDialog( parent )
{
    QPushButton *ok, *cancel;
    ok = new QPushButton( "OK", this );
    ok->setGeometry( 10,10, 100,30 );
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );
    cancel = new QPushButton( "Cancel", this );
    cancel->setGeometry( 10,60, 100,30 );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
}