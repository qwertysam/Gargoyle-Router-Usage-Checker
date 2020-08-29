#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include "mainwindow.h"
#include <QDialog>
#include <QTableWidgetItem>

namespace Ui {
class DialogSettings;
}

class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSettings(MainWindow *main);
    ~DialogSettings();

protected:
    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *event);

private slots:
    void on_buttonBox_rejected();

    void on_buttonBox_accepted();

    void on_checkBoxDarkTheme_stateChanged(int arg1);

    void on_horizontalSliderSnap_sliderMoved(int position);

    void on_spinBoxSnap_valueChanged(int arg1);

private:
    Ui::DialogSettings *ui;
    MainWindow *main;

    bool originalDarkTheme;

    void updateGridWidth();
};

#endif // DIALOGSETTINGS_H
