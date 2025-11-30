#include <plog/Init.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>

#include <QApplication>
// clang-format off
#include <winsock2.h>
#include <windows.h>
// clang-format on
#include "src/crashHandler/CrashHandler.h"
#include "src/test/test_tiny_spline.h"
#include "src/ui/CISCamera_imageGrab/cis_camera_image.h"

void initPlog();  // 初始化日志类
int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    CrashHandler::Init(L"data/debug");  // 初始化Mini转储
    initPlog();                         // 初始化日志类
    CISWidget w;
    w.show();

    return a.exec();
}
// 初始化日志类
void initPlog() {
    // 日志信息分类等级: none = 0, fatal = 1, error = 2, warning = 3, info = 4, debug = 5, verbose = 6
    // 设置初始化等级后, 等级『数值大于』设置值的日志信息就会被『忽略』
    plog::init(plog::debug, "./data/log/log.csv", 1000000000, 100);
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::get()->addAppender(&consoleAppender);  // Also add logging to the console.
}
// #include <Eigen/Dense>
// #include <Eigen/SVD>
// #include <iomanip>  // 新增这个头文件
// #include <iostream>
// #include <vector>
// // 定义坐标结构体
// struct Coordinate {
//     double pixel_x;  // 像素坐标x
//     double pixel_y;  // 像素坐标y
//     double world_x;  // 世界坐标x（棋盘格）
//     double world_y;  // 世界坐标y（棋盘格）
// };

// int main() {
//     // 1. 输入数据（您提供的像素坐标到棋盘格坐标映射）
//     std::vector<Coordinate> data = {
//         {0.15888,   0.0607613, 0,  0  },
//         {10.1488,   0.0493199, 10, 0  },
//         {20.138,    0.053679,  20, 0  },
//         {30.142,    0.0416542, 30, 0  },
//         {40.1277,   0.0407815, 40, 0  },
//         {50.1326,   0.0419384, 50, 0  },
//         {60.1369,   0.0358211, 60, 0  },
//         {70.1229,   0.033025,  70, 0  },

//         {0.168691,  10.05,     0,  10 },
//         {10.1653,   10.0435,   10, 10 },
//         {20.148,    10.0415,   20, 10 },
//         {30.1543,   10.0302,   30, 10 },
//         {40.1379,   10.0307,   40, 10 },
//         {50.1451,   10.032,    50, 10 },
//         {60.1473,   10.0228,   60, 10 },
//         {70.1356,   10.0229,   70, 10 },

//         {0.166933,  20.0654,   0,  20 },
//         {10.1614,   20.054,    10, 20 },
//         {20.1487,   20.0569,   20, 20 },
//         {30.1507,   20.047,    30, 20 },
//         {40.1366,   20.0447,   40, 20 },
//         {50.1461,   20.0499,   50, 20 },
//         {60.144,    20.0386,   60, 20 },
//         {70.1324,   20.0354,   70, 20 },

//         {0.140464,  30.0617,   0,  30 },
//         {10.1373,   30.051,    10, 30 },
//         {20.1202,   30.0502,   20, 30 },
//         {30.1245,   30.0388,   30, 30 },
//         {40.1106,   30.0413,   40, 30 },
//         {50.1158,   30.0407,   50, 30 },
//         {60.1204,   30.029,    60, 30 },
//         {70.1089,   30.0291,   70, 30 },

//         {0.106853,  40.0699,   0,  40 },
//         {10.0994,   40.06,     10, 40 },
//         {20.0846,   40.0615,   20, 40 },
//         {30.0934,   40.0491,   30, 40 },
//         {40.0752,   40.0496,   40, 40 },
//         {50.0849,   40.0481,   50, 40 },
//         {60.0852,   40.0393,   60, 40 },
//         {70.0761,   40.0397,   70, 40 },

//         {0.101373,  50.0755,   0,  50 },
//         {10.0962,   50.067,    10, 50 },
//         {20.0859,   50.0692,   20, 50 },
//         {30.0883,   50.0572,   30, 50 },
//         {40.0723,   50.0559,   40, 50 },
//         {50.0778,   50.0549,   50, 50 },
//         {60.0804,   50.0458,   60, 50 },
//         {70.0691,   50.0464,   70, 50 },

//         {0.124067,  60.0577,   0,  60 },
//         {10.1168,   60.0483,   10, 60 },
//         {20.1045,   60.0464,   20, 60 },
//         {30.1069,   60.0358,   30, 60 },
//         {40.0933,   60.0358,   40, 60 },
//         {50.0988,   60.0344,   50, 60 },
//         {60.1015,   60.0244,   60, 60 },
//         {70.0904,   60.0261,   70, 60 },

//         {0.108772,  70.0541,   0,  70 },
//         {10.1038,   70.0448,   10, 70 },
//         {20.0894,   70.0432,   20, 70 },
//         {30.0963,   70.0354,   30, 70 },
//         {40.0806,   70.0342,   40, 70 },
//         {50.0863,   70.0358,   50, 70 },
//         {60.0891,   70.0242,   60, 70 },
//         {70.0759,   70.0223,   70, 70 },

//         {0.0907781, 80.0434,   0,  80 },
//         {10.0859,   80.0344,   10, 80 },
//         {20.0738,   80.0351,   20, 80 },
//         {30.0765,   80.023,    30, 80 },
//         {40.0631,   80.0246,   40, 80 },
//         {50.0666,   80.0236,   50, 80 },
//         {60.0696,   80.0111,   60, 80 },
//         {70.0609,   80.011,    70, 80 },

//         {0.0876787, 90.0406,   0,  90 },
//         {10.083,    90.0313,   10, 90 },
//         {20.0687,   90.029,    20, 90 },
//         {30.0737,   90.018,    30, 90 },
//         {40.0604,   90.0189,   40, 90 },
//         {50.0685,   90.0195,   50, 90 },
//         {60.0672,   90.0067,   60, 90 },
//         {70.0608,   90.0072,   70, 90 },

//         {0.0884629, 100.043,   0,  100},
//         {10.0839,   100.033,   10, 100},
//         {20.0698,   100.032,   20, 100},
//         {30.0749,   100.02,    30, 100},
//         {40.0617,   100.019,   40, 100},
//         {50.0677,   100.02,    50, 100},
//         {60.0686,   100.008,   60, 100},
//         {70.0579,   100.008,   70, 100}
//     };

//     // 2. 构建最小二乘问题
//     // 转换模型：world = A * pixel + b
//     // 其中A是2x2矩阵，b是2x1向量
//     int n = data.size();
//     Eigen::MatrixXd X(2 * n, 6);  // 扩展矩阵用于求解仿射变换
//     Eigen::VectorXd Y(2 * n);     // 目标世界坐标

//     for (int i = 0; i < n; ++i) {
//         // X分量
//         X(2 * i, 0) = data[i].pixel_x;
//         X(2 * i, 1) = data[i].pixel_y;
//         X(2 * i, 2) = 1;
//         X(2 * i, 3) = 0;
//         X(2 * i, 4) = 0;
//         X(2 * i, 5) = 0;

//         // Y分量
//         X(2 * i + 1, 0) = 0;
//         X(2 * i + 1, 1) = 0;
//         X(2 * i + 1, 2) = 0;
//         X(2 * i + 1, 3) = data[i].pixel_x;
//         X(2 * i + 1, 4) = data[i].pixel_y;
//         X(2 * i + 1, 5) = 1;

//         // 目标值
//         Y(2 * i) = data[i].world_x;
//         Y(2 * i + 1) = data[i].world_y;
//     }

//     // 3. 使用SVD求解最小二乘问题
//     Eigen::VectorXd params = X.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(Y);

//     // 4. 提取补偿矩阵和偏移量
//     Eigen::Matrix2d A;
//     Eigen::Vector2d b;
//     A << params[0], params[1], params[3], params[4];
//     b << params[2], params[5];
//     // 5. 输出结果
//     std::cout << "=== 补偿矩阵求解结果 ===" << std::endl;
//     std::cout << "转换矩阵 A (2x2):" << std::endl;
//     std::cout << A << std::endl;
//     std::cout << "\n偏移向量 b (2x1):" << std::endl;
//     std::cout << b << std::endl;

//     // 5.1 计算补偿前（原始）误差
//     double raw_error = 0;
//     for (const auto& p : data) {
//         Eigen::Vector2d raw_pred(p.pixel_x, p.pixel_y);
//         Eigen::Vector2d world_true(p.world_x, p.world_y);
//         double err = (raw_pred - world_true).norm();
//         raw_error += err * err;
//     }

//     // 6. 计算补偿后误差
//     double corrected_error = 0;
//     std::cout << "\n=== 补偿后拟合误差分析 ===" << std::endl;
//     for (const auto& p : data) {
//         Eigen::Vector2d pixel(p.pixel_x, p.pixel_y);
//         Eigen::Vector2d world_pred = A * pixel + b;
//         Eigen::Vector2d world_true(p.world_x, p.world_y);
//         double err = (world_pred - world_true).norm();
//         corrected_error += err * err;

//         if (&p == &data[0] || &p == &data[8] || &p == &data[16]) {
//             std::cout << "像素(" << p.pixel_x << "," << p.pixel_y << ") -> 预测(" << world_pred[0] << "," << world_pred[1]
//                       << ") 真实(" << p.world_x << "," << p.world_y << ") 误差: " << err << std::endl;
//         }
//     }

//     // 7. 输出补偿前/补偿后的对比结果
//     std::cout << "\n=== 原始误差 vs 补偿后误差 ===" << std::endl;
//     std::cout << "原始均方误差 (RMSE): " << sqrt(raw_error / n) << std::endl;
//     std::cout << "补偿后均方误差 (RMSE): " << sqrt(corrected_error / n) << std::endl;

//     // 7. 输出所有补偿后坐标及误差（加入补偿前误差）
//     std::cout << "\n=== 所有坐标补偿结果及误差 ===" << std::endl;
//     std::cout << std::left << std::setw(15) << "像素X" << std::setw(15) << "像素Y" << std::setw(18) << "补偿后世界X"
//               << std::setw(18) << "补偿后世界Y" << std::setw(15) << "真实世界X" << std::setw(15) << "真实世界Y" <<
//               std::setw(15)
//               << "点误差" << std::setw(15) << "原始误差"  //  新增这一列
//               << std::endl;

//     std::cout << std::string(130, '-') << std::endl;

//     for (const auto& p : data) {
//         Eigen::Vector2d pixel(p.pixel_x, p.pixel_y);
//         Eigen::Vector2d world_pred = A * pixel + b;        // 补偿后世界坐标
//         Eigen::Vector2d world_true(p.world_x, p.world_y);  // 真实世界坐标

//         double err_after = (world_pred - world_true).norm();  // 补偿后误差
//         double err_before = (pixel - world_true).norm();      // 补偿前误差

//         std::cout << std::left << std::setw(15) << p.pixel_x << std::setw(15) << p.pixel_y << std::setw(18) << world_pred[0]
//                   << std::setw(18) << world_pred[1] << std::setw(15) << p.world_x << std::setw(15) << p.world_y <<
//                   std::setw(15)
//                   << err_after << std::setw(15) << err_before  // 输出原始误差
//                   << std::endl;
//     }

//     // 输出统计
//     std::cout << "\n=== 整体误差统计 ===" << std::endl;
//     std::cout << "原始均方误差 (RMSE):  " << sqrt(raw_error / n) << std::endl;
//     std::cout << "补偿后均方误差 (RMSE): " << sqrt(corrected_error / n) << std::endl;
//     std::cout << "误差下降比例: " << (1.0 - sqrt(corrected_error / raw_error)) * 100.0 << "%" << std::endl;

//     return 0;
// }
