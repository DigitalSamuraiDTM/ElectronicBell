/*  В общем это программа настоящего мужчины
 *  Разработка шла и закончена в 2020 году. Разработана программа
 * Нугаевым Андреем Александровичем (e-mail: nugaev.andrei@gmail.com)
 * Для МАОУ "Лицей" города Лесной Свердловской области
 *
 * ССылка на GitHub: https://github.com/DigitalSamuraiDTM/ElectronicBell
 *
 * Предикшн к коду:
 * Код возможно не самый оптимизированный и не самый оптимальный, но это полностью готовое решение.
 * 
 *
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "QInputDialog"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Создаем настройки
    Settings = new QSettings("settings.ini", QSettings::IniFormat,this);

    //получаем настройки расписания недельного режима
    AutoWeek = new QStandardItemModel;
    Settings->beginGroup("AutoWeek");
    AutoWeek->appendRow(QList<QStandardItem*>()<<new QStandardItem("Понедельник")<<new QStandardItem(Settings->value("Monday", "45").toString()));
    AutoWeek->appendRow(QList<QStandardItem*>()<<new QStandardItem("Вторник")<<new QStandardItem(Settings->value("Tuesday", "45").toString()));
    AutoWeek->appendRow(QList<QStandardItem*>()<<new QStandardItem("Среда")<<new QStandardItem(Settings->value("Wednesday", "45").toString()));
    AutoWeek->appendRow(QList<QStandardItem*>()<<new QStandardItem("Четверг")<<new QStandardItem(Settings->value("Thursday", "45").toString()));
    AutoWeek->appendRow(QList<QStandardItem*>()<<new QStandardItem("Пятница")<<new QStandardItem(Settings->value("Friday", "45").toString()));
    AutoWeek->appendRow(QList<QStandardItem*>()<<new QStandardItem("Суббота")<<new QStandardItem(Settings->value("Saturday", "40").toString()));
    AutoWeek->appendRow(QList<QStandardItem*>()<<new QStandardItem("Воскресенье")<<new QStandardItem(Settings->value("Sunday", "-").toString()));
    Settings->endGroup();

    //настройка view и толчок модели во view
    ui->view_autoWeek->verticalHeader()->setVisible(true);
    ui->view_autoWeek->horizontalHeader()->setVisible(true);
    ui->view_autoWeek->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->view_autoWeek->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    AutoWeek->setHorizontalHeaderLabels(QStringList()<<"День"<<"Уроки");
    ui->view_autoWeek->setModel(AutoWeek);
    ui->view_autoWeek->resizeColumnsToContents();

    //Единственный и главный проигрыватель, устанавливаем для него громкость, не забывая про слайдер
    main_player = new QMediaPlayer;
    int volume = Settings->value("Volume",100).toInt();
    main_player->setVolume(volume);
    ui->slider_volume->setValue(volume);
    ui->volume_number->setText(QString::number(volume)+"%");

    //инициализируем кастомные шаблоны, выгружаем из настроек в модель и толкаем модель во view
    customTemplate = new QStandardItemModel;
    customTemplate->setHorizontalHeaderLabels(QStringList()<<"Имя"<<"Шаблон");
    AutoSettings = new QSettings("customTemplates.ini", QSettings::IniFormat,this);
    for (int i=0;i<AutoSettings->allKeys().count();i++) {
        if (AutoSettings->allKeys().at(i) =="LastTemplate")
        {
            continue;
        }
        customTemplate->appendRow(QList<QStandardItem*>()<<new QStandardItem(AutoSettings->allKeys().at(i))
                                  <<new QStandardItem(AutoSettings->value(AutoSettings->allKeys().at(i)).toString()));

    }
    ui->view_custom_template->setModel(customTemplate);
    ui->view_custom_template->resizeColumnsToContents();

    //получаем настройку, какой был последний установленный режим
    QString auto_mod = Settings->value("Mod", "true").toString();

    //получаем настройку было ли включено в последний раз предупреждение за 1 минуту до звонка и совершаем "клик" по нужной нам радио кнопке
    callFrom1Min  = Settings->value("Call1Min",false).toBool();
    if (callFrom1Min==false){
        ui->No_1min->click();
    } else{
        ui->Yes_1min->click();
    }

    //получаем пути до наших аудио и расталкиваем их по label, для удобства пользователя
    ui->route_1min_fromLes->setText(Settings->value("RootAudio1MinFromLess","НЕ ВЫБРАНО").toString());
    ui->route_route_on_1min_fromDelay->setText(Settings->value("RootAudio1MinFromPer","НЕ ВЫБРАНО").toString());
    ui->route_audio_on_peremena->setText(Settings->value("RootAudioPeremena","НЕ ВЫБРАНО").toString());
    ui->route_audio->setText(Settings->value("RootAudio","НЕ ВЫБРАНО").toString());

    //таймер отключенного звонка
    bell_off = new QTimer(this);
    connect(bell_off, SIGNAL(timeout()), this, SLOT(bell_is_off()));

    //автоматический режим
    timer_now_time = new QTimer(this);
    connect(timer_now_time, SIGNAL(timeout()), this, SLOT(now_time()));

    //ручной режим
    hand_mod_timer = new QTimer(this);
    connect(hand_mod_timer, SIGNAL(timeout()), this, SLOT(hand_mod_now_time()));

    //режим авто недели
    AutoWeekTimer = new QTimer(this);
    connect(AutoWeekTimer, SIGNAL(timeout()), this, SLOT(AutoWeek_now_time()));



    //до этого мы получали последний включенный режим, теперь проверяем каким был этот режим и кликаем на нужную радио кнопку в зависимости от режима
    //true - автоматический режим
    //falase - ручной режим
    //week - недельный режим
    if (auto_mod=="true"){
        ui->auto_mod->click();
    } else if (auto_mod=="false"){
        ui->hand_mod->click();
    } else if (auto_mod=="week") {
        ui->autoWeek->click();
}
    //установка шрифта для основных элементов
    setFont(QFont("Calibri",15));
}

MainWindow::~MainWindow()
{
    //Удаление интерфейса, мэээээ
    delete ui;
}


//-----------------------Рассчеты времени-------------------------------------

void MainWindow::check_next_time_bell()
{
    //функция поиска следующего времени для звонка
    //опускаем флаг для того, чтобы в таймере прошел алгоритм рассчета времени сигнала за минуту до звонка
    wasCreate1Min = false;
    QTime current = QTime::currentTime();
    int current_hour = current.hour();
    int current_min = current.minute();
    int ostalos_minut=0;
    int ostalos_hour = 0;
    //ALERT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //Проводим рассчет следующего времени с помощью цикла
    //Почему?
    //Если отталкиваться от ситуации, когда время всегда идет в массиве по порядку от 1 звонка и до последнего
    //то можно было обойтись выборкой следующего времени из массива со звонкам
    //НО так как может быть ситуация, когда время 9:15 стоит в массиве ПЕРЕД временем 8:30 это может сбить программу
    //Поэтому происходит парсинг всего массива от текущего поданного звонка до следующего
    while (1)
    {
        //суть цикла: берется время звонка, которое подалось и прибавляется по 1 минуте до тех пор, пока не найдется следующее БЛИЖАЙШЕЕ время
        ostalos_minut+=1;
        if (ostalos_minut==60)
        {
            ostalos_minut=0;
            ostalos_hour+=1;
        }
    current_min+=1;
    if (current_min==60)
    {
        current_hour+=1;
        current_min=0;
    }
    if (current_hour==24)
    {
        current_hour=0;
    }

    QTime new_time(current_hour,current_min,0);
    for (int i=0;i<list_timing.count();i++) {
        QTime list_time = QTime::fromString(list_timing[i]);

        //Если мы нашли наше следующее время
        if (new_time.hour()==list_time.hour()  && new_time.minute()==list_time.minute())
        {
            int c = i+1;
            if (c==1)
            {
                na_peremeny = false;
                ui->event_label->setText("-");
            } else if (c%2==1)
            {
                //проводим рассчет того, что сейчас идет: перемена или урок?
                int count = 1;
                int row = 3;
                while (c!=row)
                {
                    row+=2;
                    count+=1;
                }
                na_peremeny = false;
                ui->event_label->setText("Перемена "+QString::number(count));
            } else if (c%2==0){
                int count = 1;
                int row = 2;
                while (row!=c)
                {
                    row+=2;
                    count+=1;
                }
                na_peremeny = true;
                ui->event_label->setText("Урок "+QString::number(count));
            }
            //забираем наше время, ставим его в место для следующего звонка, опускаем флаг проверки остатка времени
            next_time =QTime::fromString(list_timing[i]);
            ui->next_call->setText(list_timing[i]);
            first_check_ostatok = false;
            return;
        }
    }
    }

}

void MainWindow::check_ostatok()
{
    //функция поиска остатка времени


    QTime current = QTime::currentTime();
    int current_hour = current.hour();
    int current_min = current.minute();
    QTime next = QTime::fromString(ui->next_call->text());
    int ostalos_minut=0;
    int ostalos_hour = 0;
    //Берем текущее время и начинаем прибавлять по минуте, пока не дойдем до одинакового времени
    //Далее отнимаем одну минуту и берем разность секунд
    //Более оптимального способа я не придумал, придумаете сами - буду рад
    while (1)
    {
        ostalos_minut+=1;
        if (ostalos_minut==60)
        {
            ostalos_minut=0;
            ostalos_hour+=1;
        }
    current_min+=1;
    if (current_min==60)
    {
        current_hour+=1;
        current_min=0;
    }
    if (current_hour==24)
    {
        current_hour=0;
    }
    QTime new_time(current_hour,current_min,0);
    if (new_time.hour()==next.hour() && new_time.minute()==next.minute())
    {
        if (QTime::currentTime().second()==0)
        {
            QTime to_set(ostalos_hour,ostalos_minut,0);
            ui->for_call->setText(to_set.toString());
        } else{
            QTime to_set(ostalos_hour,ostalos_minut-1,60-QTime::currentTime().second());

            //ставим время и завершаем функцию
            ui->for_call->setText(to_set.toString());
            return;
        }
    }
}
}

//----------------------------выбор режима подачи звонка------------------


void MainWindow::on_hand_mod_clicked()
{
    //нажал на ручной режим.

    //останавливаем таймер авто недели, авто режима, а также выключенного звонка, запускаем таймер ручного режима
    //выключаем ненужные элемент, ставим в настройках последний выбранный режим - ручной (false)
    AutoWeekTimer->stop();
    timer_now_time->stop();
    bell_off->stop();
    hand_mod_timer->start(1000);
    is_it_auto = false;
    hand_mod = "true";
    ui->auto_box->setEnabled(false);
    ui->for_call->hide();
    ui->day_of_week->hide();
    ui->Send_call->show();
    ui->next_call->setText("-");
    ui->event_label->setText("-");
    ui->main_group->setTitle("Подача звонка");
    Settings->setValue("Mod", "false");

}

void MainWindow::on_autoWeek_clicked()
{
    //нажал на авто неделю

    //выключаем ненужные таймеры
    //включаем нужные элементы и выключаем не нужные
    ui->select_template->setEnabled(false);
    ui->for_call->show();
    ui->Send_call->hide();
    ui->day_of_week->show();
    hand_mod_timer->stop();
    bell_off->stop();
    AutoWeekTimer->stop();
    timer_now_time->stop();

    ui->box_templates->setEnabled(false);
    ui->auto_box->setEnabled(true);
    is_it_auto = true;
    list_timing.clear();
    //получаем из настроек сегодняшний день недели
    ui->this_day->setText(QDate::currentDate().toString("dddd"));
    QString day = QDate::currentDate().toString("dddd");
    QString type_bell;
    //в зависимости от дня забираем название шаблона, который будет использоваться для подачи звонков
    if (day=="понедельник"){
        type_bell=ui->view_autoWeek->model()->data(ui->view_autoWeek->model()->index(0,1)).toString();
    } else if(day=="вторник"){
        type_bell=ui->view_autoWeek->model()->data(ui->view_autoWeek->model()->index(1,1)).toString();
    } else if(day=="среда"){
        type_bell=ui->view_autoWeek->model()->data(ui->view_autoWeek->model()->index(2,1)).toString();
    } else if (day == "четверг") {
        type_bell=ui->view_autoWeek->model()->data(ui->view_autoWeek->model()->index(3,1)).toString();
} else if (day == "пятница") {
        type_bell=ui->view_autoWeek->model()->data(ui->view_autoWeek->model()->index(4,1)).toString();
} else if(day=="суббота"){
        type_bell=ui->view_autoWeek->model()->data(ui->view_autoWeek->model()->index(5,1)).toString();
    } else if (day=="воскресенье") {
        type_bell=ui->view_autoWeek->model()->data(ui->view_autoWeek->model()->index(6,1)).toString();
}

    //проверяем название шаблона
    if (type_bell=="45") {
        list_timing= {QStringList()<<"08:30"<<"09:15"<<"09:25"<<"10:10"<<"10:25"<<"11:10"<<"11:30"<<"12:15"<<"12:30"<<"13:15"<<"13:30"<<"14:15"<<"14:25"<<"15:05"};
        check_next_time_bell();
        check_ostatok();
        AutoWeekTimer->start(1000);
        hand_mod = "week";
        Settings->setValue("Mod","week");
    } else if (type_bell=="40") {
        list_timing= {QStringList()<<"08:30"<<"09:10"<<"09:20"<<"10:00"<<"10:15"<<"10:55"<<"11:10"<<"11:50"<<"12:05"<<"12:45"<<"12:55"<<"13:35"<<"13:45"<<"14:25"};
        check_next_time_bell();
        check_ostatok();
        AutoWeekTimer->start(1000);
        hand_mod = "week";
        Settings->setValue("Mod","week");
} else if (type_bell=="выключен") {
        ui->next_call->setText("-");
        ui->event_label->setText("-");
        bell_off->start(1000);
        Settings->setValue("Mod","week");

} else{
        //получаем шаблон, парсим его и добавляем в массив
        for (int i=0; i<ui->view_custom_template->model()->rowCount();i++)
        {
            //можно без цикла, если получить номер строки названия шаблона и получить данные из этой же строки из колонки номер 1
            if (type_bell==ui->view_custom_template->model()->data(ui->view_custom_template->model()->index(i,0)).toString())
            {
                //получаем массив времени звонков,  делаем рассчет времени следующего звонка, запускаем таймер
               list_timing = ui->view_custom_template->model()->data(ui->view_custom_template->model()->index(i,1)).toString().split("/");
               check_next_time_bell();
                check_ostatok();
                AutoWeekTimer->start(1000);
                hand_mod = "week";
                //последний использованный режим - неделя
                Settings->setValue("Mod","week");
                return;
            }
        }
        //ситуация, когда в недели используется шаблон, которого нет в созданных шаблонах
        if (list_timing.isEmpty())
        {
            QMessageBox::critical(this,"Ошибка","Программа не смогла распознать расписание уроков: "+type_bell+"\n Программа автоматически перестроится на ручной режим");
            ui->hand_mod->click();
            return;
        }
    }
}

void MainWindow::on_auto_mod_clicked()
{
    //режим автоматического звонка, начало такое же, как и у всех
    ui->select_template->setEnabled(true);
    AutoWeekTimer->stop();
    hand_mod_timer->stop();
    bell_off->stop();
    hand_mod = "false";
    is_it_auto = false;
    ui->auto_box->setEnabled(true);
    ui->box_templates->setEnabled(true);
    ui->for_call->show();
    ui->Send_call->hide();
    ui->day_of_week->hide();
    ui->main_group->setTitle("Осталось до звонка");
    //проверка на последний используемый шаблон из настроек
    int templates = Settings->value("Template",45).toInt();
    switch (templates) {
    case 45:{
        on_lessons_45_minuts_clicked();
        ui->lessons_45_minuts->setChecked(true);
        break;
    }
    case 40:{
        on_lessons_40_minuts_clicked();
        ui->lessons_40_minuts->setChecked(true);
        break;
    }
    case -1:{
        //если шаблон не 40 и не 45, то есть он кастомный, то кликаем, на кастом
        on_custom_template_clicked();
        ui->custom_template->setChecked(true);
    }
    }
    //запуск таймера, последний режим - автомат
        timer_now_time->start(1000);
       Settings->setValue("Mod", "true");
}

//------------Выбран автоматический режим

void MainWindow::on_lessons_45_minuts_clicked()
{
    //выбрал шаблон 45 минут для автоматической подачи
    ui->view_custom_template->setEnabled(false);
    ui->delete_template->setEnabled(false);
    ui->add_template->setEnabled(false);
    ui->select_template->setEnabled(false);
    list_timing= {QStringList()<<"08:30"<<"09:15"<<"09:25"<<"10:10"<<"10:25"<<"11:10"<<"11:30"<<"12:15"<<"12:30"<<"13:15"<<"13:30"<<"14:15"<<"14:25"<<"15:05"};
    check_next_time_bell();
    Settings->setValue("Template",45);
}

void MainWindow::on_lessons_40_minuts_clicked()
{
    //выбрал шаблон 40 минут для автоматической подачи

    ui->view_custom_template->setEnabled(false);
    ui->delete_template->setEnabled(false);
    ui->add_template->setEnabled(false);
    ui->select_template->setEnabled(false);
    list_timing= {QStringList()<<"08:30"<<"09:10"<<"09:20"<<"10:00"<<"10:15"<<"10:55"<<"11:10"<<"11:50"<<"12:05"<<"12:45"<<"12:55"<<"13:35"<<"13:45"<<"14:25"};
    check_next_time_bell();
    Settings->setValue("Template",40);
}

void MainWindow::on_custom_template_clicked()
{
    //функция выбора кастомного шаблона в автоматическом режиме
    ui->view_custom_template->setEnabled(true);
    ui->delete_template->setEnabled(true);
    ui->add_template->setEnabled(true);
    ui->select_template->setEnabled(true);
    QString last_template = AutoSettings->value("LastTemplate","None").toString();
    //проверка, существовал ли выбранный шаблон ранее или такового не было
    if (last_template!="None")
    {
        //получаем массив времени, ищем следующее время и так далее
        list_timing = AutoSettings->value(last_template).toString().split("/");
        check_next_time_bell();
        ui->selected_template->setText(last_template);

    } else{
        QMessageBox::warning(this,"Шаблоны","У вас отсутствует выбранный шаблон. Программа будет перестроена на уроки по 45 минут\nдо ближайшего выбранного вами шаблона.");
        list_timing= {QStringList()<<"08:30"<<"09:15"<<"09:25"<<"10:10"<<"10:25"<<"11:10"<<"11:30"<<"12:15"<<"12:30"<<"13:15"<<"13:30"<<"14:15"<<"14:25"<<"15:05"};
        check_next_time_bell();

    }
        Settings->setValue("Template",-1);
}

//----------------Настройки и остальное-------------------------------------

void MainWindow::call_1Min()
{
    //проверка на ситуацию, когда программа не знает, когда подавать предупреждение
    if (wasCreate1Min==false)
    {
        wasCreate1Min=true;
        //проводим рассчет времени одной минуты до звонка
        int hours = next_time.hour();
        int min = next_time.minute();
        if (min==0)
        {
            call1Min.setHMS(hours-1,59,0);
        } else{
            call1Min.setHMS(hours,min-1,0);
        }

    } else{
        //проверка на совпадение времени для подачи сигнала
        if (call1Min.hour()==QTime::currentTime().hour() && call1Min.minute()==QTime::currentTime().minute() && call1Min.second()==QTime::currentTime().second())
        {
            //классическая проверка на тип звонка (на урок или на перемену?)
            if (na_peremeny==false)
            {
                //ONLY LINUX
                //file://
                qDebug()<<"НА УРОК";
                main_player->setMedia(QUrl(Settings->value("RootAudio1MinFromLess","main.mp3").toString()));
            } else{
                qDebug()<<"НА ПЕРЕМЕНУ";
                main_player->setMedia(QUrl(Settings->value("RootAudio1MinFromPer","main.mp3").toString()));
            }
            main_player->play();
        }
    }
}

void MainWindow::on_Send_call_clicked()
{
    //подать звонок в ручную

    //создаем диалоговую форму, на звонок или на перемену? (см. lesson_or_turn.cpp)
    lesson_or_turn *turn = new lesson_or_turn;
    if (turn->exec()==QDialog::Accepted)
    {
        bool was_lesson = turn->get_bool();
    //LINUX file://
        if (was_lesson==true)
        {
            main_player->setMedia(QUrl(Settings->value("RootAudio","main.mp3").toString()));

        } else{
            main_player->setMedia(QUrl(Settings->value("RootAudioPeremena","main.mp3").toString()));

        }
        delete turn;
        main_player->play();
    }

    main_player->play();
}

void MainWindow::on_slider_volume_valueChanged(int value)
{
    //изменение громкости, не забываем сохранять в настройки
    ui->volume_number->setText(QString::number(value)+"%");
    Settings->setValue("Volume",ui->slider_volume->value());
    main_player->setVolume(value);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //думал что-то будет, но тут ничего нет
    event->accept();
}

void MainWindow::on_change_bell_in_autoWeek_clicked()
{

    //смена шаблона в недельном режиме.

    //получаем какой день нужно изменить, получаем список названий и меняем, всё просто
    QString day = QInputDialog::getItem(this,"День недели", "Выберите день недели для изменения графика",QStringList()<<"Понедельник"<<"Вторник"<<"Среда"<<"Четверг"<<"Пятница"<<"Суббота"<<"Воскресенье",0);


    QStringList listCustomTemplates;
    for (int i=0;i<ui->view_custom_template->model()->rowCount();i++)
    {
        listCustomTemplates.append(ui->view_custom_template->model()->data(ui->view_custom_template->model()->index(i,0)).toString());
    }


    QStringList times = {"45","40","выключен"};
    times.append(listCustomTemplates);
    bool ok;
    QString time = QInputDialog::getItem(this,"Звонок","Выберите шаблон звонков на:\n"+day,times,0,false,&ok);
    if(!ok)
    {
        return;
    }


    int row = 0;
Settings->beginGroup("AutoWeek");
    if (day=="Понедельник"){
        Settings->setValue("Monday",time);
        ui->view_autoWeek->model()->setData(ui->view_autoWeek->model()->index(0,1),time);
        row = 0;
} else if (day=="Вторник") {
        row = 1;
        Settings->setValue("Tuesday",time);
        ui->view_autoWeek->model()->setData(ui->view_autoWeek->model()->index(1,1),time);

}else if (day=="Среда"){
        row = 2;
        Settings->setValue("Wednesday",time);
        ui->view_autoWeek->model()->setData(ui->view_autoWeek->model()->index(2,1),time);

} else if (day=="Четверг") {
        row = 3;
        Settings->setValue("Thursday",time);
        ui->view_autoWeek->model()->setData(ui->view_autoWeek->model()->index(3,1),time);

} else if (day=="Пятница") {
        row = 4;
        Settings->setValue("Friday",time);
        ui->view_autoWeek->model()->setData(ui->view_autoWeek->model()->index(4,1),time);

} else if (day=="Суббота") {
        row = 5;
        Settings->setValue("Saturday",time);
        ui->view_autoWeek->model()->setData(ui->view_autoWeek->model()->index(5,1),time);

} else if (day=="Воскресенье") {
        row = 6;
        Settings->setValue("Sunday",time);
        ui->view_autoWeek->model()->setData(ui->view_autoWeek->model()->index(6,1),time);

}
    Settings->endGroup();

    AutoWeek->setData(AutoWeek->index(row,1),time);
    if (is_it_auto==true)
    {
        ui->autoWeek->click();
    }
}

void MainWindow::on_Exit_clicked()
{
    //выход, с выбором, для защиты от случайного клика по кнопке
    QMessageBox::StandardButton bat = QMessageBox::question(this, "Закрыть приложение?","Закрытие приложения приведет к тому, что звонки не будут подаваться ни в каком его режиме.\nЗакрыть приложение?", QMessageBox::Yes | QMessageBox::No);
    if (bat==QMessageBox::Yes)
    {
        this->close();
    } else if(bat==QMessageBox::No)
    {
        return;
    }
}


//----------------------------настройка кастом шаблонов------------------

void MainWindow::on_add_template_clicked()
{
    //функция добавления нового кастомного шаблона
    QMessageBox::StandardButton batya = QMessageBox::question(this,"Создаение нового шаблона","Начать создание нового шаблона?", QMessageBox::Yes | QMessageBox::No);

    QStringList helpList;
    QString name;
    if (batya == QMessageBox::No)
    {
        return;
    } else if (batya == QMessageBox::Yes) {

        //вводим имя шаблона
        name =QInputDialog::getText(this,"Название шаблона","Введите название шаблона");
        QString customTemplate;
        QTime time = QTime(8,30);
        int i=1;
        int test = QMessageBox::question(this, "режим создания шаблона", "Выберите режим создания шаблона", QObject::tr("Без ограничений"), QObject::tr("7 уроков"));


        if (test==0)
        {
            //был выбран бесконечный режим. Основан на бесконечном создании диалогового окна до тех пор, пока пользователь не нажмет завершить
        while (true) {

            QDialogTime *timer = new QDialogTime(time,QString::number(i));
            if(timer->exec()==QDialog::Accepted)
            {
                time = timer->GetTime();
                QStringList str_time = time.toString().split(":");
                helpList.append(str_time[0]+":"+str_time[1]);
            } else{
                delete timer;
                break;
            }
            i+=1;
            delete timer;
        }
        } else if (test==1) {

            //режим альтернативного ввода (см alternativeinput.cpp)
            input = new alternativeInput;
            if (input->exec()==QDialog::Accepted)
            {

                 helpList = input->take_data();

            } else{
                return;

            }
            delete  input;
        }

    }
    //не забываем добавлять шаблон в настойки и добавлять во view
    AutoSettings->setValue(name,helpList.join("/"));
    customTemplate->appendRow(QList<QStandardItem*>()<<new QStandardItem(name)<< new QStandardItem(helpList.join("/")));
    ui->view_custom_template->resizeColumnsToContents();
    }

void MainWindow::on_delete_template_clicked()
{
   //Удаление шаблона

    int row = ui->view_custom_template->model()->rowCount();
    //получаем список имен шаблонов
    QStringList names;
    for (int i=0;i<row;i++)
    {
        names.append(ui->view_custom_template->model()->data(ui->view_custom_template->model()->index(i,0)).toString());
    }

    bool ok;
    //вызываем диалоговое окно со спинбоксом и получаем то, что надо удалить
    QString name = QInputDialog::getItem(this,"Удаление шаблона","Выберите название шаблона для удаления",names,0,false,&ok);
    if (!ok)
    {
        return;
    }
    //получаем индекс и по индексу вычисляем строку, удаляем из view, а также из настроек
     AutoSettings->remove(name);
    int num_row = names.indexOf(name);
    ui->view_custom_template->model()->removeRow(num_row);

    if (ui->view_custom_template->model()==nullptr)
    {
        return;
    }
    int rowAuto = ui->view_autoWeek->model()->rowCount();
    //проверяем, использовался ли этот шаблон в режиме автоматической НЕДЕЛИ, если да, то предупреждаем пользователя и чиллим
    for (int i=0;i<rowAuto;i++) {
        QString templateBell = ui->view_autoWeek->model()->data(ui->view_autoWeek->model()->index(i,1)).toString();
        if (templateBell==name)
        {

            QString day = ui->view_autoWeek->model()->data(ui->view_autoWeek->model()->index(i,0)).toString();
            QMessageBox::critical(this,"Шаблон","Вы удалили шаблон который использовался в день:\n"+day+"\nИзмените график подачи звонков на этот день, иначе программа будет перестроена в ручной режим в этот день");
        }
    }
    QString last_templ = AutoSettings->value("LastTemplate").toString();
    //проверяем использовался шаблон в автоматике, если да, то предупреждаем пользователя и переключаемся на режим 45 минут
        if (name==last_templ)
        {
            ui->selected_template->setText("Выбранный шаблон отсутствует");
            AutoSettings->setValue("LastTemplate","None");
            QMessageBox::critical(this,"Шаблон","Вы удалили шаблон, который сейчас использовался для подачи звонков в автоматическом режиме");
            on_custom_template_clicked();
        }

}

void MainWindow::on_select_template_clicked()
{
    //функция выбора кастомного шаблона


    //Всё просто, получаем список названий, выкидываем диалоговое окно с gеtItem, находим нашу строку, парсим на массив и ставим
    int strokes = ui->view_custom_template->model()->rowCount();
    QStringList list;
    for (int i = 0;i<strokes;i++)
    {
        list.append(ui->view_custom_template->model()->data(ui->view_custom_template->model()->index(i,0)).toString());
    }
    bool ok;
   QString name = QInputDialog::getItem(this,"Выбор шаблона","Выберите шаблон:",list,0,false,&ok);
   if (!ok)
   {
       return;
   }
   int row = list.indexOf(name);
    ui->selected_template->setText(name);
     AutoSettings->setValue("LastTemplate",name);
     list_timing =  ui->view_custom_template->model()->data(ui->view_custom_template->model()->index(row,1)).toString().split("/");
     check_next_time_bell();
     QMessageBox::information(this,"Шаблон","Вы выбрали шаблон: "+name+"\nВремя будет автоматически перестроено под выбранный шаблон");
}


//-----------------------Настройка аудио звонка-------------------------------


void MainWindow::on_selected_audio_peremena_clicked()
{
    //функция выбора файла для подачи на перемену
    QString rootAudio;
    rootAudio = QFileDialog::getOpenFileName(this,"Выберите mp3","","mp3 (*.mp3)");
    if (rootAudio=="")
    {
        return;
    }

    Settings->setValue("RootAudioPeremena",rootAudio);
    ui->route_audio_on_peremena->setText(rootAudio);
}

void MainWindow::on_selected_route_1minLess_clicked()
{
    //одна минута до звонка на урок
    QString rootAudio;
    rootAudio = QFileDialog::getOpenFileName(this,"Выберите mp3","","mp3 (*.mp3)");
    if (rootAudio=="")
    {
        return;
    }

    Settings->setValue("RootAudio1MinFromLess",rootAudio);
    ui->route_1min_fromLes->setText(rootAudio);
}

void MainWindow::on_selected_route_1minDel_clicked()
{
    //одна минута до звонка на перемену

    QString rootAudio;
    rootAudio = QFileDialog::getOpenFileName(this,"Выберите mp3","","mp3 (*.mp3)");
    if (rootAudio=="")
    {
        return;
    }

    Settings->setValue("RootAudio1MinFromPer",rootAudio);
    ui->route_route_on_1min_fromDelay->setText(rootAudio);
}

void MainWindow::on_select_audio_clicked()
{
    //нажал на выбрать аудио для подачи на урок
    QString rootAudio;
    rootAudio = QFileDialog::getOpenFileName(this,"Выберите mp3","","mp3 (*.mp3)");
    if (rootAudio=="")
    {
        return;
    }


    Settings->setValue("RootAudio",rootAudio);
    ui->route_audio->setText(rootAudio);
}

//-----------------------Функции таймеров---------------------------------------

void MainWindow::hand_mod_now_time()
{
   //таймер ручного режима
    ui->now_time->setText(QTime::currentTime().toString());
}

void MainWindow::bell_is_off()
{

    //таймер выключенного режима

    //не забываем в полночь проверять
    if (QTime::currentTime().hour() == 0 && QTime::currentTime().minute()==0 && QTime::currentTime().second()==0)
    {
        ui->autoWeek->click();
        return;
    }
    ui->for_call->setText("Звонок выключен");
    ui->now_time->setText(QTime::currentTime().toString());
}

void MainWindow::AutoWeek_now_time()
{
    //таймер авто недели


    //проверяем был ли включен режим сигнала за 1 минуту
    ui->now_time->setText(QTime::currentTime().toString());
    if (callFrom1Min==true)
    {
        call_1Min();
    }
    //проверяем совпадает ли "сейчас" с тем, когда нужно подать звонок
    if (next_time.hour()==QTime::currentTime().hour() && next_time.minute()==QTime::currentTime().minute() && next_time.second()==QTime::currentTime().second())
    {
        //проверяем перемена сейчас или урок
        if (na_peremeny==false)
        {
            //ONLY LINUX
            //file://
            qDebug()<<"ЗА МИНУТУ ДО УРОКА";
            main_player->setMedia(QUrl(Settings->value("RootAudio","main.mp3").toString()));
        } else{
            qDebug()<<"ЗА МИНУТУ ДО ПЕРЕМЕНЫ";
            main_player->setMedia(QUrl(Settings->value("RootAudioPeremena","main.mp3").toString()));
        }
        main_player->play();
        check_next_time_bell();
        return;
    }
    //!!!!!!!!!!!!!!!!!!!!
    //ЕСЛИ ПОЛНОЧЬ, ТО ВЫЗЫВАЕМ КЛИК НА АВТО НЕДЕЛЮ ДЛЯ СМЕНЫ ШАБЛОНА
    //! !!!!!!!!!!!!!!!!!!
    if (QTime::currentTime().hour() == 0 && QTime::currentTime().minute()==0 && QTime::currentTime().second()==0)
    {
        ui->autoWeek->click();
        return;
    }

    //ежеминутное обновление остатка для слабых машин
    repeat_checking++;
    if (repeat_checking==60)
    {
        first_check_ostatok=false;
    }
    if (first_check_ostatok==false)
    {
        repeat_checking = 0;
        first_check_ostatok=true;
        check_ostatok();
        return;
    } else{
        bool was_update = false;
        QTime out_time = QTime::fromString(ui->for_call->text());



    //отсчет времени до звонка
        if (out_time.second()==0)
        {
            out_time.setHMS(out_time.hour(),out_time.minute()-1,59);
            was_update=true;
        }
        if (out_time.minute()==0 && out_time.second()==0)
        {
            out_time.setHMS(out_time.hour()-1,59,59);
            was_update=true;
        }
        if (was_update==false)
        {
            out_time.setHMS(out_time.hour(),out_time.minute(),out_time.second()-1);
        }

        ui->for_call->setText(out_time.toString());

}
}

void MainWindow::now_time()
{
    //каждую секунду обновляем текущее время на label
    ui->now_time->setText(QTime::currentTime().toString());

    //проверка на включенность режима подачи сигнала за минуту до звонка
    if (callFrom1Min==true)
    {
        call_1Min();
    }


    //проверка подачи звонка
    if (next_time.hour()==QTime::currentTime().hour() && next_time.minute()==QTime::currentTime().minute() && next_time.second()==QTime::currentTime().second())
    {
        if (na_peremeny==false)
        {
            //ONLY LINUX
            //file://
            qDebug()<<"НА УРОК";
            main_player->setMedia(QUrl(Settings->value("RootAudio","main.mp3").toString()));
        } else{
            qDebug()<<"НА ПЕРЕМЕНУ";
            main_player->setMedia(QUrl(Settings->value("RootAudioPeremena","main.mp3").toString()));
        }
       //включаем плеер и вызываем функцию поиска следующего времени
        main_player->play();
        check_next_time_bell();
        //обязательно делаем return для того, чтобы не продолжался отсчет оставшегося времени
        return;

   }
    //ежеминутное обновление оставшегося времени
    //Сделано для того, что если оставшееся время для подачи звонка собьется, то через минуту оно обновится и будет правильным
    //нужен для слабых станций, которые не успевают рассчитать оставшееся время при поиске следующего времени подачи звонка
    repeat_checking++;
    if (repeat_checking==60)
    {
        first_check_ostatok=false;
    }
    if (first_check_ostatok==false)
    {
        repeat_checking = 0;
        first_check_ostatok=true;
        check_ostatok();
        return;
    } else{
        //если алгоритм обновления оставшегося времени не срабатывает отнимает 1 секунду от нашего оставшегося времени
        bool was_update = false;
        QTime out_time = QTime::fromString(ui->for_call->text());


        //всё просто, если секунды = 0, то ставим секунды 59, а минуты отнимаем от текущих
        if (out_time.second()==0)
        {
            out_time.setHMS(out_time.hour(),out_time.minute()-1,59);
            was_update=true;
        }
        //также делаем для минут и часов
        if (out_time.minute()==0 && out_time.second()==0)
        {
            out_time.setHMS(out_time.hour()-1,59,59);
            was_update=true;
        }
        //Если рассчет не препдполагал обновлять секунды и минуты или минуты и часы, то просто отнимаем секунды
        if (was_update==false)
        {
            out_time.setHMS(out_time.hour(),out_time.minute(),out_time.second()-1);
        }

        //обновляем label
        ui->for_call->setText(out_time.toString());

}

}

//-----------------------Вкл/Выкл сигнала за 1 минуту--------------------------
void MainWindow::on_Yes_1min_clicked()
{
    callFrom1Min = true;
    Settings->setValue("Call1Min",callFrom1Min);
}

void MainWindow::on_No_1min_clicked()
{
    callFrom1Min = false;
    Settings->setValue("Call1Min",callFrom1Min);
}
