#include "lesson_or_turn.h"


lesson_or_turn::lesson_or_turn()
{
    setFont(QFont("Calibri",12,2,false));
    on_lesson = new QPushButton("На УРОК");
    on_turn = new QPushButton("На ПЕРЕМЕНУ");
    QLabel *label = new QLabel("На урок или на перемену?");
    connect(on_lesson, SIGNAL(clicked()),this, SLOT(set_lesson()));
    connect(on_turn, SIGNAL(clicked()),this, SLOT(set_turn()));

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(label,1,1);
    grid->addWidget(on_lesson,2,0);
    grid->addWidget(on_turn,2,2);
    setLayout(grid);

}
void lesson_or_turn::set_lesson()
{
    on_lesson_bool = true;
    accept();
}

void lesson_or_turn::set_turn()
{
    on_lesson_bool = false;
    accept();
}

bool lesson_or_turn::get_bool() const
{
    return on_lesson_bool;
}
