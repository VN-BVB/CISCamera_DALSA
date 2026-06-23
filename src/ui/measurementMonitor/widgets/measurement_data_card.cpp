#include "measurement_data_card.h"

#include <QBoxLayout>

MeasurementDataCard::MeasurementDataCard(const QString &title, const QString &unit, QWidget *parent)
    : QFrame(parent)
    , m_title(title)
    , m_unit(unit)
    , m_value(0.0)
    , m_isValid(false)
    , m_titleLabel(nullptr)
    , m_valueLabel(nullptr)
    , m_unitLabel(nullptr)
    , m_statusIcon(nullptr)
{
    setupUi();
}

void MeasurementDataCard::setupUi() {
    setFixedHeight(80);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 10, 15, 10);
    mainLayout->setSpacing(10);

    QWidget* leftWidget = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(5);

    m_titleLabel = new QLabel(m_title, leftWidget);
    m_titleLabel->setStyleSheet("font-size: 12px; color: #AAAAAA; background: transparent;");
    leftLayout->addWidget(m_titleLabel);

    QWidget* valueWidget = new QWidget(leftWidget);
    QHBoxLayout* valueLayout = new QHBoxLayout(valueWidget);
    valueLayout->setContentsMargins(0, 0, 0, 0);
    valueLayout->setSpacing(5);

    m_valueLabel = new QLabel("--", valueWidget);
    m_valueLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #4CAF50; background: transparent;");
    valueLayout->addWidget(m_valueLabel);

    m_unitLabel = new QLabel(m_unit, valueWidget);
    m_unitLabel->setStyleSheet("font-size: 12px; color: #AAAAAA; background: transparent;");
    valueLayout->addWidget(m_unitLabel);
    valueLayout->addStretch();

    leftLayout->addWidget(valueWidget);
    mainLayout->addWidget(leftWidget, 1);

    m_statusIcon = new QLabel(this);
    m_statusIcon->setFixedSize(24, 24);
    m_statusIcon->setText(QString::fromUtf8("○"));
    m_statusIcon->setStyleSheet("font-size: 20px; color: #808080; background: transparent;");
    m_statusIcon->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusIcon);

    updateStyle();
}

void MeasurementDataCard::setValue(double value, bool valid) {
    m_value = value;
    m_isValid = valid;

    if (valid) {
        m_valueLabel->setText(QString::number(value, 'f', 3));
        m_valueLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #4CAF50; background: transparent;");
        m_statusIcon->setText(QString::fromUtf8("✓"));
        m_statusIcon->setStyleSheet("font-size: 18px; color: #4CAF50; background: transparent;");
    } else {
        m_valueLabel->setText("--");
        m_valueLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #808080; background: transparent;");
        m_statusIcon->setText(QString::fromUtf8("○"));
        m_statusIcon->setStyleSheet("font-size: 20px; color: #808080; background: transparent;");
    }
}

void MeasurementDataCard::setTitle(const QString &title) {
    m_title = title;
    m_titleLabel->setText(title);
}

void MeasurementDataCard::updateStyle() {
    setStyleSheet(R"(
        MeasurementDataCard {
            background-color: #2D2D2D;
            border: 1px solid #3D3D3D;
            border-radius: 6px;
        }
    )");
}
