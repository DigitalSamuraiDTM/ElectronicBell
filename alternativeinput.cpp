#include "alternativeinput.h"
#include "ui_alternativeinput.h"
#include "QDebug"
alternativeInput::alternativeInput(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::alternativeInput)
{
    ui->setupUi(this);
}

alternativeInput::~alternativeInput()
{
    delete ui;
}


QStringList alternativeInput::take_data()
{
    //В цикле получаем дату, если она состоит из однозначная, то дописываем ноль слева
    QStringList list;
    QString m;

    QSpinBox *Sp[28];
    Sp[0]=ui->hours_les_1;
    Sp[1]=ui->min_les_1;
    Sp[2]=ui->hours_les_2;
    Sp[3]=ui->min_les_2;
    Sp[4]=ui->hours_les_3;
    Sp[5]=ui->min_les_3;
    Sp[6]=ui->hours_les_4;
    Sp[7]=ui->min_les_4;
    Sp[8]=ui->hours_les_5;
    Sp[9]=ui->min_les_5;
    Sp[10]=ui->hours_les_6;
    Sp[11]=ui->min_les_6;
    Sp[12]=ui->hours_les_7;
    Sp[13]=ui->min_les_7;
    Sp[14]=ui->hours_les_8;
    Sp[15]=ui->min_les_8;
    Sp[16]=ui->hours_les_9;
    Sp[17]=ui->min_les_9;
    Sp[18]=ui->hours_les_10;
    Sp[19]=ui->min_les_10;
    Sp[20]=ui->hours_les_11;
    Sp[21]=ui->min_les_11;
    Sp[22]=ui->hours_les_12;
    Sp[23]=ui->min_les_12;
    Sp[24]=ui->hours_les_13;
    Sp[25]=ui->min_les_13;
    Sp[26]=ui->hours_les_14;
    Sp[27]=ui->min_les_14;
    for (int i = 0;i< 28;i++) {
        if (i%2==0)
        {
           if (Sp[i]->text().length()==1)
           {
               m +="0"+Sp[i]->text()+":";
           }else{
               m+=Sp[i]->text()+":";
           }
        } else if (i%2==1)
        {
            if (Sp[i]->text().length()==1)
            {


            m +="0"+Sp[i]->text();
            list.append(m);
            m.clear();

        } else{
            m+=Sp[i]->text();
            list.append(m);
            m.clear();
        }
        }


    }
    return list;
}
