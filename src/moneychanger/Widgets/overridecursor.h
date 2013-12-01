#ifndef OVERRIDECURSOR_H
#define OVERRIDECURSOR_H

#include <QObject>

class MTOverrideCursor : public QObject
{
    Q_OBJECT
public:
    explicit MTOverrideCursor(QObject *parent = 0);
    virtual ~MTOverrideCursor();

signals:

public slots:

protected:
    bool eventFilter( QObject *obj, QEvent *event );
};

#endif // OVERRIDECURSOR_H


