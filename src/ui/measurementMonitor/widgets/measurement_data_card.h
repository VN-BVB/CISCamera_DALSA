#ifndef MEASUREMENT_DATA_CARD_H
#define MEASUREMENT_DATA_CARD_H

#include <QFrame>
#include <QLabel>

class MeasurementDataCard : public QFrame {
    Q_OBJECT

public:
    explicit MeasurementDataCard(const QString &title, const QString &unit, QWidget *parent = nullptr);

    void setValue(double value, bool valid = true);
    void setTitle(const QString &title);
    double getValue() const { return m_value; }
    bool isValid() const { return m_isValid; }

private:
    void setupUi();
    void updateStyle();

private:
    QString m_title;
    QString m_unit;
    double m_value;
    bool m_isValid;

    QLabel* m_titleLabel;
    QLabel* m_valueLabel;
    QLabel* m_unitLabel;
    QLabel* m_statusIcon;
};

#endif  // MEASUREMENT_DATA_CARD_H
