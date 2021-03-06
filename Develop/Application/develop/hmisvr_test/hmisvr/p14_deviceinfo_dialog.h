#ifndef P14_DEVICEINFO_DIALOG_H
#define P14_DEVICEINFO_DIALOG_H

#include <QDialog>

namespace Ui {
class p14_deviceinfo_dialog;
}

class p14_deviceinfo_dialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit p14_deviceinfo_dialog(QWidget *parent = 0);
    ~p14_deviceinfo_dialog();
    
private:
    Ui::p14_deviceinfo_dialog *ui;

private:
        QTimer *timer;

private:
    void update_page_info(void);
    void update_hmi_page_index(void);

signals:
    void signal_exit_p14dlg();
    
private slots:
        void init_page_dlg(void);
        void init_page_val(void);
        void start_timer(void);
        void timer_handle(void);
        void exit_page_dlg(void);
        void on_ptn_return_clicked();
};

#endif // P14_DEVICEINFO_DIALOG_H
