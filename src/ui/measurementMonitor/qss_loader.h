#pragma once

#include <QFile>
#include <QStyle>
#include <QString>
#include <QVariant>
#include <QWidget>

namespace qss_loader {

inline QString load(const QString& path) {
    QFile file(path);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        return QString::fromUtf8(file.readAll());
    }
    return QString();
}

inline void setState(QWidget* w, const char* prop, const QVariant& v) {
    w->setProperty(prop, v);
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}

}  // namespace qss_loader
