#ifndef ASCRAPER_H
#define ASCRAPER_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QListWidgetItem>
#include <QQueue>
#include <QImage>
#include <QLabel>

namespace Ui {
class ascraper;
}

class ascraper : public QMainWindow
{
    Q_OBJECT

public:
    explicit ascraper(QWidget *parent = 0);
    ~ascraper();

private slots:
    void on_actionQuit_triggered();
    void on_actionImage_Root_triggered();
    void on_nf_button_clicked();
    void on_cat_button_clicked();
    void update_folders();
    void populate_catalog();
    void populate_thread(QString s);
    void add_image(QString s);
    void catalog_finished();
    void thread_finished();
    void image_finished();
    void on_thread_list_itemDoubleClicked(QListWidgetItem *item);
    void on_folder_list_itemDoubleClicked(QListWidgetItem *item);

    void on_skip_button_clicked();

private:
    Ui::ascraper *ui;
    QString root, thread, page;
    QImage *current_image;
    QLabel *page_label, *thread_label, *images_label;
    QQueue<QImage*> *images;
    QQueue<QString> *filenames;
    QNetworkAccessManager *manager;
};

#endif // ASCRAPER_H
