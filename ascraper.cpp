#include "ascraper.h"
#include "ui_ascraper.h"
#include "QFileDialog"
#include "QInputDialog"
#include "QDir"
#include "QtNetwork/QNetworkAccessManager"
#include "QtNetwork/QNetworkRequest"
#include "QtNetwork/QNetworkReply"
#include "QUrl"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"
#include "QPixmap"
#include "QDebug"

ascraper::ascraper(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ascraper)
{
    manager = new QNetworkAccessManager(this);
    images = new QQueue<QImage*>();
    filenames = new QQueue<QString>();
    current_image = new QImage();
    thread = root = "";
    page = "1";

    ui->setupUi(this);
    page_label = new QLabel("Page " + page + " ", this);
    thread_label = new QLabel("Thread " + thread + " ", this);
    images_label = new QLabel(QString::number(images->size()) + " images ", this);
    ui->statusBar->addWidget(page_label);
    ui->statusBar->addWidget(thread_label);
    ui->statusBar->addWidget(images_label);

    populate_catalog();

    on_actionImage_Root_triggered();
}

ascraper::~ascraper()
{
    delete ui;
}

void ascraper::on_actionQuit_triggered()
{
    exit(0);
}

void ascraper::on_actionImage_Root_triggered()
{
    root = QFileDialog::getExistingDirectory(this,tr("Select Root Folder"));
    if (root != "") {
        ui->root_label->setText("Current Directory: " + root);
        update_folders();
    } else {
        on_actionImage_Root_triggered();
    }
}

void ascraper::on_nf_button_clicked()
{
    QString f = QInputDialog::getText(this,tr("Folder Name"),tr("Input folder name:"));
    if (!QDir(root + "/" + f).exists()) QDir().mkdir(root + "/" + f);
    update_folders();
}

void ascraper::on_cat_button_clicked()
{
    populate_catalog();
}

void ascraper::update_folders() {
    QStringList folders = QDir(root).entryList(QDir::Dirs|QDir::NoDotDot,QDir::NoSort);
    ui->folder_list->clear();
    ui->folder_list->addItems(folders);
}

void ascraper::populate_catalog() {
    QNetworkRequest q;
    //q.setUrl(QUrl("http://a.4cdn.org/a/catalog.json"));
    q.setUrl(QUrl("http://a.4cdn.org/a/" + page + ".json"));
    //q.setHeader(QNetworkRequest::LastModifiedHeader,"Sat, 29 Oct 2014 19:43:31 GMT");
    QNetworkReply *reply = manager->get(q);
    connect(reply, SIGNAL(finished()), this, SLOT(catalog_finished()));

    //http://a.4cdn.org/a/catalog.json
    //http://a.4cdn.org/a/thread/127071203.json
}

void ascraper::populate_thread(QString s) {
    QNetworkRequest q;
    //qDebug() << "http://a.4cdn.org/a/thread/" + s + ".json";
    q.setUrl(QUrl("http://a.4cdn.org/a/thread/" + s + ".json"));
    QNetworkReply *reply = manager->get(q);
    connect(reply, SIGNAL(finished()), this, SLOT(thread_finished()));
}

void ascraper::add_image(QString s) {
    QNetworkRequest q;
    //qDebug() << s;
    q.setUrl(QUrl(s));
    QNetworkReply *reply = manager->get(q);
    connect(reply, SIGNAL(finished()), this, SLOT(image_finished()));
}

void ascraper::catalog_finished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QByteArray qb = reply->readAll();
    QString s = QString::fromUtf8(qb.data(),qb.size());

    //qDebug() << (QString("Length: %1").arg(s.length()));

    QJsonArray jo = QJsonDocument::fromJson(qb).object().value("threads").toArray();

    ui->thread_list->clear();

    foreach (const QJsonValue & v, jo) {
        QJsonObject p = v.toObject().value("posts").toArray()[0].toObject();
        //qDebug() << p.value("no").toInt() << p.value("com").toString();
        QListWidgetItem *ql = new QListWidgetItem(ui->thread_list);
        ql->setData(0,QVariant(p.value("com").toString()));
        ql->setToolTip(QString::number(p.value("no").toInt()));
        ui->thread_list->addItem(ql);
    }
}

void ascraper::thread_finished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QByteArray qb = reply->readAll();
    QString s = QString::fromUtf8(qb.data(),qb.size());
    QJsonArray jo = QJsonDocument::fromJson(qb).object().value("posts").toArray();
    //ui->images;

    while (!images->empty()) images->dequeue();
    while (!filenames->empty()) filenames->dequeue();
    current_image = new QImage();
    ui->current_image->clear();

    ui->image_list->clear();
    foreach (const QJsonValue & v, jo) {
        if (v.toObject().value("tim") != QJsonValue::Undefined) {
            //http://i.4cdn.org/a/tim.ext
            if (v.toObject().value("ext").toString() != QString(".jpg")
                    && v.toObject().value("ext").toString() != QString(".png")
                    && v.toObject().value("ext").toString() != QString(".gif")
                    && v.toObject().value("ext").toString() != QString(".jpeg")) continue;
            QString id = QString::number(v.toObject().value("tim").toVariant().toLongLong());
            //qDebug() << "http://i.4cdn.org/a/" + id + v.toObject().value("ext").toString();
            add_image("http://i.4cdn.org/a/" + id + v.toObject().value("ext").toString());
        }
    }
}

void ascraper::image_finished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QByteArray qb = reply->readAll();
    QImage* qi = new QImage();
    qi->loadFromData(qb);
    QString f = reply->url().path();
    f = f.remove(0, f.indexOf("/",1)+1);
    filenames->enqueue(f);
    //qDebug() << f;

    if (!current_image->isNull()) {
        images->enqueue(qi);
        QListWidgetItem *ql = new QListWidgetItem(ui->image_list);
        QPixmap qp = QPixmap::fromImage(*qi);
        ql->setData(Qt::DecorationRole, qp.scaledToHeight(100));
        ui->image_list->addItem(ql);
    } else {
        current_image = qi;
        QPixmap qp = QPixmap::fromImage(*current_image);
        ui->current_image->setPixmap(qp.scaledToHeight(ui->current_image->height())); // sometimes is bad
    }
    images_label->setText(QString::number(filenames->size()) + " images ");
}

void ascraper::on_thread_list_itemDoubleClicked(QListWidgetItem *item) {
    //http://a.4cdn.org/a/thread/127071203.json
    //qDebug() << item->toolTip();
    thread = item->toolTip();
    thread_label->setText("Thread " + thread + " ");
    populate_thread(thread);
}

void ascraper::on_folder_list_itemDoubleClicked(QListWidgetItem *item) {
    if (current_image->isNull()) return;
    current_image->save(root + "/" + item->text() + "/" + filenames->dequeue());
    if (images->size() == 0) {
        current_image = new QImage();
    } else {
        current_image = images->dequeue();
        delete ui->image_list->item(0);
    }
    QPixmap qp = QPixmap::fromImage(*current_image);
    ui->current_image->setPixmap(qp.scaledToHeight(ui->current_image->height()));
    images_label->setText(QString::number(filenames->size()) + " images ");
}

void ascraper::on_skip_button_clicked() {
    if (current_image->isNull()) return;
    filenames->dequeue();
    images_label->setText(QString::number(filenames->size()) + " images ");
    if (images->size() == 0) {
        current_image = new QImage();
        ui->current_image->clear();
    } else {
        current_image = images->dequeue();
        delete ui->image_list->item(0);
        QPixmap qp = QPixmap::fromImage(*current_image);
        ui->current_image->setPixmap(qp.scaledToHeight(ui->current_image->height()));
    }
}
