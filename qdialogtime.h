#ifndef QDIALOGTIME_H
#define QDIALOGTIME_H

#include "QDialog"
#include "QInputDialog"
#include "QTimeEdit"
#include "QLabel"
#include "QGridLayout"
#include "QPushButton"
class QTimeEdit;
class QDialogTime : public QDialog
{
    //Q_OBJECT
private:
    QTimeEdit *edit;
public:
    QDialogTime(QTime,QString,QWidget *pwgt=0);

    QTime GetTime() const;


};

#endif // QDIALOGTIME_H
