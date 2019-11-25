#ifndef ALTERNATIVEINPUT_H
#define ALTERNATIVEINPUT_H

#include <QDialog>

namespace Ui {
class alternativeInput;
}

class alternativeInput : public QDialog
{
    Q_OBJECT

public:
    explicit alternativeInput(QWidget *parent = nullptr);
    ~alternativeInput();

private slots:

public slots:
    QStringList take_data();

private:
    Ui::alternativeInput *ui;
};

#endif // ALTERNATIVEINPUT_H
