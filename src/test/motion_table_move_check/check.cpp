#include "motion_controller.h"

#include <QtWidgets>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "motion_widget.h"
#include "plc_addr.h"

// Exercise the real UI, controller thread and register serialization without a PLC.
static uint16_t registers[65536] = {};
static QVector<int> locatedPlatforms;
static int writeCount = 0;

extern "C" {
modbus_t *modbus_new_tcp(const char *, int) { return reinterpret_cast<modbus_t *>(1); }
int modbus_set_response_timeout(modbus_t *, uint32_t, uint32_t) { return 0; }
int modbus_connect(modbus_t *) { return 0; }
void modbus_close(modbus_t *) {}
void modbus_free(modbus_t *) {}
int modbus_read_registers(modbus_t *, int address, int count, uint16_t *destination) {
    std::copy(registers + address, registers + address + count, destination);
    return count;
}
int modbus_read_bits(modbus_t *, int, int count, uint8_t *destination) {
    std::fill(destination, destination + count, 0);
    return count;
}
int modbus_write_bit(modbus_t *, int, int) { ++writeCount; return 1; }
int modbus_write_registers(modbus_t *, int address, int count, const uint16_t *data) {
    ++writeCount;
    for (int platform = 0; platform < 7; ++platform) {
        if (address == plc::HDAddr + plc::HD_Plt_0_Location + platform * 300 &&
            count == 1 && (data[0] & 1))
            locatedPlatforms.append(platform);
    }
    std::copy(data, data + count, registers + address);
    return count;
}
}

// MotionWidget already exposes its controller to CISWidget via friendship.
class CISWidget {
public:
    static MotionController *controller(MotionWidget &widget) { return widget.controller_; }
};

static void check(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}

static void flushCommands(MotionController *controller) {
    check(QMetaObject::invokeMethod(controller, [] {}, Qt::BlockingQueuedConnection),
          "controller thread barrier failed");
}

static void clearRegisters() {
    std::fill(std::begin(registers), std::end(registers), 0);
    locatedPlatforms.clear();
    writeCount = 0;
}

static double position(int platform, int axis) {
    if (platform == 0 && axis == 0) return 0.0;
    return (axis == 1 ? -1.0 : 1.0) * (platform * 10.0 + axis + 0.125);
}

static double readDouble(int address) {
    double value;
    std::memcpy(&value, registers + plc::HDAddr + address, sizeof(value));
    return value;
}

class WarningCloser : public QObject {
public:
    int count = 0;

    bool eventFilter(QObject *object, QEvent *event) override {
        if (event->type() == QEvent::Show) {
            if (auto *warning = qobject_cast<QMessageBox *>(object)) {
                ++count;
                QTimer::singleShot(0, warning, &QDialog::accept);
                // The offscreen plugin has no native parent window for QMessageBox::showEvent.
                return true;
            }
        }
        return false;
    }
};

static void checkPlatform(int platform) {
    const int offset = platform * 300;
    const int addresses[] = {plc::HD_0_XAxis1MOVERPosParams, plc::HD_0_YAxis1MOVERPosParams,
                             plc::HD_0_RotAxis1MOVERPosParams};
    for (int axis = 0; axis < 3; ++axis)
        check(readDouble(addresses[axis] + offset) == position(platform, axis),
              "wrong row, column or displacement written to PLC registers");
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    WarningCloser warnings;
    app.installEventFilter(&warnings);
    try {
        MotionWidget widget;
        auto *controller = CISWidget::controller(widget);
        auto *selector = widget.findChild<QComboBox *>("combo_singlepltchoose");
        auto *table = widget.findChild<QTableWidget *>("table_axispos");
        auto *button = widget.findChild<QPushButton *>("plt_btn_singlepltmover");
        check(selector && table && button, "motion UI controls missing");
        check(selector->count() == 8 && table->rowCount() == 7, "unexpected platform options");
        QMetaObject::invokeMethod(controller, [controller] {
            controller->connectPlc("in-memory-no-socket", 502);
            for (auto *timer : controller->findChildren<QTimer *>()) timer->stop();
        }, Qt::BlockingQueuedConnection);
        for (int platform = 0; platform < 7; ++platform)
            for (int axis = 0; axis < 3; ++axis)
                table->item(platform, axis)->setText(QString::number(position(platform, axis)));

        clearRegisters();
        selector->setCurrentIndex(0);
        button->click();
        flushCommands(controller);
        check(locatedPlatforms == QVector<int>({0, 1, 2, 3, 4, 5, 6}),
              "all-platform selection must send exactly one command per platform");
        for (int platform = 0; platform < 7; ++platform) checkPlatform(platform);
        std::cout << "PASS: all 21 displacements reach the correct platform/axis registers\n";

        for (int platform = 0; platform < 7; ++platform) {
            clearRegisters();
            selector->setCurrentIndex(platform + 1);
            button->click();
            flushCommands(controller);
            check(locatedPlatforms == QVector<int>{platform}, "single-platform selection shifted");
            checkPlatform(platform);
        }
        std::cout << "PASS: all seven individual selections map to their original platforms\n";

        selector->setCurrentIndex(0);
        for (const auto &invalid : {QString(""), QString("invalid"), QString("nan"),
                                    QString("inf"), QString("1e309")}) {
            clearRegisters();
            table->item(6, 2)->setText(invalid);
            const int warningsBefore = warnings.count;
            button->click();
            flushCommands(controller);
            check(warnings.count == warningsBefore + 1 && writeCount == 0,
                  "invalid final cell must prevent every PLC write");
        }
        std::cout << "PASS: blank, invalid, non-finite and overflow values reject the whole batch\n";

        clearRegisters();
        selector->setCurrentIndex(1);
        button->click();
        flushCommands(controller);
        check(locatedPlatforms == QVector<int>{0}, "unselected rows must not affect single-platform moves");
        clearRegisters();
        selector->setCurrentIndex(-1);
        button->click();
        flushCommands(controller);
        check(writeCount == 0, "missing selection must not send commands");
        std::cout << "PASS: unselected invalid rows and missing selection handled correctly\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
