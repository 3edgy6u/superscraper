#include "downloader.h"
#include "QMessageBox"

Downloader::Downloader(QObject *parent) :
    QObject(parent)
{
}

void Downloader::do_download()
{
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), SLOT(reply_finished(QNetworkReply*)));
    QNetworkRequest q;
    q.setUrl(QUrl("http://a.4cdn.org/a/catalog.json"));
    manager->get(q);
}

void Downloader::reply_finished(QNetworkReply *reply)
{
    QMessageBox messageBox;
    messageBox.critical(0,"Error","An error has occured !");
    messageBox.setFixedSize(500,200);
    if (reply->error()) {
        qDebug() << "ERROR!" << reply->errorString();
    } else {
        qDebug() << reply->header(QNetworkRequest::ContentTypeHeader).toString();
        qDebug() << reply->header(QNetworkRequest::LastModifiedHeader).toDateTime().toString();
        qDebug() << reply->header(QNetworkRequest::ContentLengthHeader).toULongLong();
        qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();

        QFile *file = new QFile("C:\\Users\\Jason\\Documents\\Programming\\qt\\downloaded.txt");
        if(file->open(QFile::Append))
        {
            file->write(reply->readAll());
            file->flush();
            file->close();
        }
        delete file;

        QMessageBox messageBox;
        messageBox.critical(0,"Error",reply->readAll());
        messageBox.setFixedSize(500,200);
    }
    reply->deleteLater();
}
