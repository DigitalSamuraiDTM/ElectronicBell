#ifndef LESSON_OR_TURN_H
#define LESSON_OR_TURN_H

#include "QPushButton"
#include "QGridLayout"
#include "QLabel"
#include "QDialog"
#include "QObject"
class lesson_or_turn : public QDialog
{
private:
    Q_OBJECT
    QPushButton *on_lesson;
    QPushButton *on_turn;
private slots:
    void set_lesson();
    void set_turn();
public slots:
    bool get_bool() const;
public:
    bool on_lesson_bool = false;
    lesson_or_turn();
};

#endif // LESSON_OR_TURN_H
