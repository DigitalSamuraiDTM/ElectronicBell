#include "qdialogtime.h"

QDialogTime::QDialogTime(QTime time,QString name,QWidget *pwgt /*=0*/) : QDialog (pwgt, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    setFont(QFont("Calibri",15,2,false));
    edit = new QTimeEdit(time);
    QPushButton *ok = new QPushButton("Далее");
    ok->setFont(QFont("Calibri",14));
    connect(ok,SIGNAL(clicked()), SLOT(accept()));
    QLabel *sel_time = new QLabel("№ "+name);
    sel_time->setFont(QFont("Calibri",14));

    QPushButton *decline = new QPushButton("Закончить");
    decline->setFont(QFont("Calibri",14));
    connect(decline,SIGNAL(clicked()), SLOT(reject()));

    QGridLayout *Grid = new QGridLayout;

    Grid->addWidget(ok,2,0);
    Grid->addWidget(decline,2,1);
    Grid->addWidget(sel_time,1,0);
    Grid->addWidget(edit,1,1);


    setLayout(Grid);
}

QTime QDialogTime::GetTime() const
{
    return edit->time();
}



