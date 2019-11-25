#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "QTimer"
#include "QDebug"
#include "QMediaPlayer"
#include "QTime"
#include "QSignalBlocker"
#include "QSettings"
#include "QCloseEvent"
#include "QDialog"
#include "qdialogtime.h"
#include "lesson_or_turn.h"
#include "alternativeinput.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void now_time();

    void check_next_time_bell();

    void on_hand_mod_clicked();

    void on_auto_mod_clicked();

    void on_Send_call_clicked();

    void on_select_audio_clicked();

    void on_slider_volume_valueChanged(int value);

    void closeEvent(QCloseEvent *event);

    void on_lessons_45_minuts_clicked();

    void on_lessons_40_minuts_clicked();

    void check_ostatok();

    void on_custom_template_clicked();

    void on_add_template_clicked();

    void on_delete_template_clicked();

    void on_select_template_clicked();


    void on_selected_audio_peremena_clicked();

    void on_Exit_clicked();

    void on_autoWeek_clicked();

    void on_change_bell_in_autoWeek_clicked();

    void hand_mod_now_time();

    void AutoWeek_now_time();

    void bell_is_off();
private:
    Ui::MainWindow *ui;
    QTimer *timer_now_time;
    QTimer *hand_mod_timer;
    QTimer *AutoWeekTimer;
    QTimer *bell_off;

    QMediaPlayer *main_player;
    QStringList list_timing;
    QTime next_time;

    QSettings *Settings;

    QSettings *AutoSettings;
    int repeat_checking = 0;
    bool na_peremeny = false;
    bool is_it_auto = false;
    QString hand_mod;
    bool first_check_ostatok=false;

    QStandardItemModel *AutoWeek;
    QStandardItemModel *customTemplate;

    alternativeInput *input;
};

#endif // MAINWINDOW_H
