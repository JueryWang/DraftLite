#pragma once

#include <QLabel>

class ClickableLabel : public QLabel {
    Q_OBJECT
public:
    ClickableLabel(const QString& path = "", int size = 16);
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent* event) override;
};