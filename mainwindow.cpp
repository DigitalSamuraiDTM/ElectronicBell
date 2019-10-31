#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "QInputDialog"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    customTemplate = new QStandardItemModel;
    customTemplate->setHorizontalHeaderLabels(QStringList()<<"Имя"<<"Шаблон");
    Settings = new QSettings("settings.ini", QSettings::IniFormat,this);
    AutoSettings = new QSettings("customTemplates.ini", QSettings::IniFormat,this);
    ui->setupUi(this);
    main_player = new QMediaPlayer;
   QString rootAudio = Settings->value("RootAudio","main.mp3").toString();
    int volume = Settings->value("Volume",100).toInt();
    bool auto_mod = Settings->value("Mod", true).toBool();

    main_player->setVolume(volume);
    ui->slider_volume->setValue(volume);

    ui->volume_number->setText(QString::number(volume)+"%");
    ui->route_audio_on_peremena->setText(Settings->value("RootAudioPeremena","main.mp3").toString());
    ui->route_audio->setText(rootAudio);
    timer_now_time = new QTimer(this);
    connect(timer_now_time, SIGNAL(timeout()), this, SLOT(now_time()));

    for (int i=0;i<AutoSettings->allKeys().count();i++) {
        if (AutoSettings->allKeys().at(i) =="LastTemplate")
        {
            continue;
        }
        customTemplate->appendRow(QList<QStandardItem*>()<<new QStandardItem(AutoSettings->allKeys().at(i))
                                  <<new QStandardItem(AutoSettings->value(AutoSettings->allKeys().at(i)).toString()));

    }

    if (auto_mod==true)
    {
        ui->auto_mod->click();
    } else{
        ui->hand_mod->click();
    }
    ui->view_custom_template->setModel(customTemplate);
    ui->view_custom_template->resizeColumnsToContents();
    timer_now_time->start(1000);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::now_time()
{
    ui->now_time->setText(QTime::currentTime().toString());
    if (hand_mod==true)
    {
        return;
    }
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
    } else{
    bool was_update = false;
    QTime out_time = QTime::fromString(ui->for_call->text());



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

        if (next_time.hour()==QTime::currentTime().hour() && next_time.minute()==QTime::currentTime().minute() && next_time.second()==QTime::currentTime().second())
        {
            if (na_peremeny==false)
            {
                //ONLY LINUX
                //file://
                qDebug()<<"НА УРОК";
                main_player->setMedia(QUrl("file://"+Settings->value("RootAudio","main.mp3").toString()));
            } else{
                qDebug()<<"НА ПЕРЕМЕНУ";
                main_player->setMedia(QUrl("file://"+Settings->value("RootAudioPeremena","main.mp3").toString()));
            }
            main_player->play();
            check_next_time_bell();

       }
}

void MainWindow::check_next_time_bell()
{
    QTime current = QTime::currentTime();
    int current_hour = current.hour();
    int current_min = current.minute();
    int ostalos_minut=0;
    int ostalos_hour = 0;
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
    for (int i=0;i<list_timing.count();i++) {
        QTime list_time = QTime::fromString(list_timing[i]);
        if (new_time.hour()==list_time.hour()  && new_time.minute()==list_time.minute())
        {
            int c = i+1;
            if (c==1)
            {
                na_peremeny = false;
                ui->event_label->setText("-");
            } else if (c%2==1)
            {
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
            next_time =QTime::fromString(list_timing[i]);
            ui->next_call->setText(list_timing[i]);
            first_check_ostatok = false;
            return;
        }
    }
    }

}

void MainWindow::on_hand_mod_clicked()
{
    hand_mod = true;
    ui->auto_box->setEnabled(false);
    ui->for_call->hide();
    ui->Send_call->show();
    ui->next_call->setText("-");
    ui->event_label->setText("-");
    ui->main_group->setTitle("Подача звонка");
    Settings->setValue("Mod", false);

}

void MainWindow::on_autoWeek_clicked()
{

}

void MainWindow::on_auto_mod_clicked()
{
    hand_mod = false;
    ui->auto_box->setEnabled(true);
    ui->for_call->show();
    ui->Send_call->hide();
    ui->main_group->setTitle("Осталось до звонка");
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
        on_custom_template_clicked();
        ui->custom_template->setChecked(true);
    }
    }
       Settings->setValue("Mod", true);
}

void MainWindow::on_Send_call_clicked()
{
    lesson_or_turn *turn = new lesson_or_turn;
    if (turn->exec()==QDialog::Accepted)
    {
        bool was_lesson = turn->get_bool();
        if (was_lesson==true)
        {
            main_player->setMedia(QUrl("file://"+Settings->value("RootAudio","main.mp3").toString()));

        } else{
            main_player->setMedia(QUrl("file://"+Settings->value("RootAudioPeremena","main.mp3").toString()));

        }
        delete turn;
        main_player->play();
    }
    //LINUX file://

    main_player->play();
}

void MainWindow::on_select_audio_clicked()
{
    QString rootAudio;
    rootAudio = QFileDialog::getOpenFileName(this,"Выберите mp3","","mp3 (*.mp3)");
    if (rootAudio=="")
    {
        return;
    }


    Settings->setValue("RootAudio",rootAudio);
    ui->route_audio->setText(rootAudio);
}

void MainWindow::on_slider_volume_valueChanged(int value)
{
    ui->volume_number->setText(QString::number(value)+"%");
    Settings->setValue("Volume",ui->slider_volume->value());
    main_player->setVolume(value);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void MainWindow::on_lessons_45_minuts_clicked()
{
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
    ui->view_custom_template->setEnabled(false);
    ui->delete_template->setEnabled(false);
    ui->add_template->setEnabled(false);
    ui->select_template->setEnabled(false);
    list_timing= {QStringList()<<"08:30"<<"09:10"<<"09:20"<<"10:00"<<"10:15"<<"10:55"<<"11:10"<<"11:50"<<"12:05"<<"12:45"<<"12:55"<<"13:35"};
    check_next_time_bell();
    Settings->setValue("Template",40);
}

void MainWindow::check_ostatok()
{
    QTime current = QTime::currentTime();
    int current_hour = current.hour();
    int current_min = current.minute();
    QTime next = QTime::fromString(ui->next_call->text());
    int ostalos_minut=0;
    int ostalos_hour = 0;
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

            ui->for_call->setText(to_set.toString());
            return;
        }
    }
}
}

void MainWindow::on_custom_template_clicked()
{
    ui->view_custom_template->setEnabled(true);
    ui->delete_template->setEnabled(true);
    ui->add_template->setEnabled(true);
    ui->select_template->setEnabled(true);
    QString last_template = AutoSettings->value("LastTemplate","None").toString();
    if (last_template!="None")
    {
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

void MainWindow::on_add_template_clicked()
{
    QMessageBox::StandardButton batya = QMessageBox::question(this,"Создаение нового шаблона","Начать создание нового шаблона?", QMessageBox::Yes | QMessageBox::No);

    QStringList helpList;
    QString name;
    if (batya == QMessageBox::No)
    {
        return;
    } else if (batya == QMessageBox::Yes) {


        name =QInputDialog::getText(this,"Название шаблона","Введите название шаблона");
        QString customTemplate;
        QTime time = QTime(8,30);
        int i=1;
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
        }
    AutoSettings->setValue(name,helpList.join("/"));
    customTemplate->appendRow(QList<QStandardItem*>()<<new QStandardItem(name)<< new QStandardItem(helpList.join("/")));
    ui->view_custom_template->resizeColumnsToContents();
    }

void MainWindow::on_delete_template_clicked()
{
    if (ui->view_custom_template->model()==nullptr)
    {
        return;
    }
    QList<QModelIndex> index = ui->view_custom_template->selectionModel()->selectedRows();
    int count_selected_rows = ui->view_custom_template->selectionModel()->selectedRows().count();
    QString last_templ = AutoSettings->value("LastTemplate").toString();
    for (int i=0;i<count_selected_rows;i++) {
        QString name = ui->view_custom_template->model()->data(ui->view_custom_template->model()->index(index.at(i).row(),0)).toString();
        AutoSettings->remove(name);
        ui->view_custom_template->model()->removeRow(ui->view_custom_template->selectionModel()->selectedRows().at(0).row(), QModelIndex());
        if (name==last_templ)
        {
            ui->selected_template->setText("Выбранный шаблон отсутствует");
            AutoSettings->setValue("LastTemplate","None");
            QMessageBox::critical(this,"Шаблон","Вы удалили шаблон, который сейчас использовался для подачи звонков.");
            on_custom_template_clicked();
        }
    }
}

void MainWindow::on_select_template_clicked()
{
     int count_selected_rows = ui->view_custom_template->selectionModel()->selectedRows().count();
     if (count_selected_rows>1)
     {
         QMessageBox::warning(this,"Шаблон","Выберите один шаблон");
         return;
     }
     int row = ui->view_custom_template->selectionModel()->selectedRows().at(0).row();
     QString name = ui->view_custom_template->model()->data(ui->view_custom_template->model()->index(row,0)).toString();
     ui->selected_template->setText(name);
     AutoSettings->setValue("LastTemplate",name);
     list_timing =  ui->view_custom_template->model()->data(ui->view_custom_template->model()->index(row,1)).toString().split("/");
     check_next_time_bell();
     QMessageBox::information(this,"Шаблон","Вы выбрали шаблон: "+name+"\nВремя будет автоматически перестроено под выбранный шаблон");
}

void MainWindow::on_selected_audio_peremena_clicked()
{
    QString rootAudio;
    rootAudio = QFileDialog::getOpenFileName(this,"Выберите mp3","","mp3 (*.mp3)");
    if (rootAudio=="")
    {
        return;
    }

    Settings->setValue("RootAudioPeremena",rootAudio);
    ui->route_audio_on_peremena->setText(rootAudio);
}

void MainWindow::on_Exit_clicked()
{
    QMessageBox::StandardButton bat = QMessageBox::question(this, "Закрыть приложение?","Закрытие приложения приведет к тому, что звонки не будут подаваться ни в каком его режиме.\nЗакрыть приложение?", QMessageBox::Yes | QMessageBox::No);
    if (bat==QMessageBox::Yes)
    {
        this->close();
    } else if(bat==QMessageBox::No)
    {
        return;
    }
}

