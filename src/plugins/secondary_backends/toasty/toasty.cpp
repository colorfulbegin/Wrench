/*
    SnoreNotify is a Notification Framework based on Qt
    Copyright (C) 2014-2015  Patrick von Reth <vonreth@kde.org>

    SnoreNotify is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SnoreNotify is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with SnoreNotify.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "toasty.h"
#include "toastysettings.h"
#include "libsnore/utils.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QFile>

using namespace Snore;

void Toasty::slotNotify(Notification notification)
{
    QString key = settingsValue(QLatin1String("DeviceID")).toString();
    if (key.isEmpty()) {
        return;
    }
    QNetworkRequest request(QUrl::fromUserInput(QLatin1String("http://api.supertoasty.com/notify/") + key));
    QHttpMultiPart *mp = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart title;
    title.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QLatin1String("form-data; name=\"title\"")));
    title.setBody(notification.title().toUtf8().constData());
    mp->append(title);

    QHttpPart text;
    text.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QLatin1String("form-data; name=\"text\"")));
    text.setBody(notification.text().toUtf8().constData());
    mp->append(text);

    QHttpPart app;
    app.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QLatin1String("form-data; name=\"sender\"")));
    app.setBody(notification.application().name().toUtf8().constData());
    mp->append(app);

    QHttpPart icon;

    Icon sIcon = notification.icon();
    QSize iconSize = notification.icon().image().size();
    if (iconSize.height() > 128 || iconSize.width() > 128) {
        sIcon = sIcon.scaled(QSize(128, 128));
    }
    icon.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QLatin1String("form-data; name=\"image\"; filename=\"") + sIcon.localUrl() + QLatin1Char('"')));
    icon.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(QLatin1String("image/png")));
    QFile *file = new QFile(sIcon.localUrl());
    file->open(QIODevice::ReadOnly);
    icon.setBodyDevice(file);
    mp->append(icon);

    QNetworkReply *reply =  m_manager.post(request, mp);
    mp->setParent(reply);
    file->setParent(reply);

    connect(reply, &QNetworkReply::finished, [reply]() {
        snoreDebug(SNORE_DEBUG) << reply->error();
        snoreDebug(SNORE_DEBUG) << reply->readAll();
        reply->close();
        reply->deleteLater();
    });

}

void Toasty::slotInitialize()
{
    emit initialisationFinished(true);
}

PluginSettingsWidget *Toasty::settingsWidget()
{
    return new ToastySettings(this);
}

void Toasty::setDefaultSettings()
{
    setDefaultSettingsValue(QLatin1String("DeviceID"), QString());
    SnoreSecondaryBackend::setDefaultSettings();
}
