#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/plot_boards.h"

#include <opencv2/opencv.hpp>
#include <vector>

#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/config.h"

namespace cbdetect {

void plot_boards(const cv::Mat& img, const Corner& corners, const std::vector<Board>& boards, const Params& params) {
    cv::Mat img_show;
    if (img.channels() != 3) {
#if CV_VERSION_MAJOR >= 4
        cv::cvtColor(img, img_show, cv::COLOR_GRAY2BGR);
#else
        cv::cvtColor(img, img_show, CV_GRAY2BGR);
#endif
    } else {
        img_show = img.clone();
    }
    // 绘制每一个棋盘板
    for (int n = 0; n < boards.size(); ++n) {
        const auto& board = boards[n];
        // 绘制棋盘网格线
        for (int i = 1; i < board.idx.size() - 1; ++i) {
            for (int j = 1; j < board.idx[i].size() - 1; ++j) {
                if (board.idx[i][j] < 0) {
                    continue;
                }
                // 红线（粗线）绘制邻接格子
                // 如果右边的点存在，画一条红色线（BGR颜色为(0, 0, 255)）表示横向相邻。
                if (board.idx[i][j + 1] >= 0) {
                    cv::line(img_show, corners.p[board.idx[i][j]], corners.p[board.idx[i][j + 1]], cv::Scalar(0, 0, 255), 3,
                             cv::LINE_AA);
                }
                if (params.corner_type == MonkeySaddlePoint && board.idx[i + 1][j + 1] >= 0) {
                    cv::line(img_show, corners.p[board.idx[i][j]], corners.p[board.idx[i + 1][j + 1]], cv::Scalar(0, 0, 255), 3,
                             cv::LINE_AA);
                }
                if (board.idx[i + 1][j] >= 0) {
                    cv::line(img_show, corners.p[board.idx[i][j]], corners.p[board.idx[i + 1][j]], cv::Scalar(0, 0, 255), 3,
                             cv::LINE_AA);
                }

                // 绘制白色细线
                if (board.idx[i][j + 1] >= 0) {
                    cv::line(img_show, corners.p[board.idx[i][j]], corners.p[board.idx[i][j + 1]], cv::Scalar(255, 255, 255), 1,
                             cv::LINE_AA);
                }
                if (params.corner_type == MonkeySaddlePoint && board.idx[i + 1][j + 1] >= 0) {
                    cv::line(img_show, corners.p[board.idx[i][j]], corners.p[board.idx[i + 1][j + 1]], cv::Scalar(255, 255, 255),
                             1, cv::LINE_AA);
                }
                if (board.idx[i + 1][j] >= 0) {
                    cv::line(img_show, corners.p[board.idx[i][j]], corners.p[board.idx[i + 1][j]], cv::Scalar(255, 255, 255), 1,
                             cv::LINE_AA);
                }
            }
        }

        // 绘制坐标系
        for (int i = 1; i < board.idx.size() * board.idx[0].size(); ++i) {
            int row = i / board.idx[0].size();
            int col = i % board.idx[0].size();
            if (board.idx[row][col] < 0 || col == board.idx[0].size() - 1 || board.idx[row][col + 1] < 0 ||
                board.idx[row + 1][col] < 0) {
                continue;
            }
            cv::line(img_show, corners.p[board.idx[row][col]], corners.p[board.idx[row][col + 1]], cv::Scalar(255, 0, 0), 3,
                     cv::LINE_AA);
            cv::line(img_show, corners.p[board.idx[row][col]], corners.p[board.idx[row + 1][col]], cv::Scalar(0, 255, 0), 3,
                     cv::LINE_AA);
            break;
        }

        // 绘制棋盘编号
        cv::Point2d mean(0.0, 0.0);
        for (int i = 1; i < board.idx.size() - 1; ++i) {
            for (int j = 1; j < board.idx[i].size() - 1; ++j) {
                if (board.idx[i][j] < 0) {
                    continue;
                }
                mean += corners.p[board.idx[i][j]];
            }
        }
        mean /= (double)(board.num);
        mean.x -= 10;
        mean.y += 10;
        cv::putText(img_show, std::to_string(n), mean, cv::FONT_HERSHEY_SIMPLEX, 1.3, cv::Scalar(196, 196, 0), 2);
    }

    cv::namedWindow("boards_img", cv::WINDOW_NORMAL);
    cv::imshow("boards_img", img_show);
    cv::imwrite("./example_data/ecorners_img.png", img_show);
    cv::waitKey();
}
void plot_board_points(const cv::Mat& img, const Corner& corners, const std::vector<Board>& boards,
                       std::vector<std::vector<cv::Point2d>>& board_points_sorted) {
    cv::Mat img_show;
    if (img.channels() != 3) {
#if CV_VERSION_MAJOR >= 4
        cv::cvtColor(img, img_show, cv::COLOR_GRAY2BGR);
#else
        cv::cvtColor(img, img_show, CV_GRAY2BGR);
#endif
    } else {
        img_show = img.clone();
    }

    board_points_sorted.clear();  // 清空旧数据

    for (int n = 0; n < boards.size(); ++n) {
        const auto& board = boards[n];
        std::vector<cv::Point2d> current_board_points;
        cv::Point2d mean(0.0, 0.0);
        int count = 0;
        for (int i = 1; i < board.idx.size() - 1; ++i) {  // 行优先
            for (int j = 1; j < board.idx[i].size() - 1; ++j) {
                int idx = board.idx[i][j];
                if (idx < 0) continue;

                const cv::Point2d& pt = corners.p[idx];
                current_board_points.push_back(pt);
                mean += pt;
                count++;
            }
        }
        // for (int j = 1; j < board.idx[0].size() - 1; ++j) {   // 列优先
        //     for (int i = 1; i < board.idx.size() - 1; ++i) {  // 行在内层
        //         int idx = board.idx[i][j];
        //         if (idx < 0) continue;
        //         current_board_points.push_back(corners.p[idx]);
        //     }
        // }
        board_points_sorted.push_back(current_board_points);

        // 绘制角点和编号
        int pt_counter = 0;
        for (const auto& pt : current_board_points) {
            cv::drawMarker(img_show, pt, cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 2, 1, cv::LINE_AA);
            cv::putText(img_show, std::to_string(pt_counter++), pt + cv::Point2d(3, -3), cv::FONT_HERSHEY_PLAIN, 1.0,
                        cv::Scalar(0, 0, 255), 2);
        }

        // 棋盘编号
        if (count > 0) {
            mean /= count;
            mean.x -= 10;
            mean.y += 10;
            cv::putText(img_show, std::to_string(n), mean, cv::FONT_HERSHEY_SIMPLEX, 1.3, cv::Scalar(0, 255, 0), 2);
        }
    }

    cv::namedWindow("board_points", cv::WINDOW_NORMAL);
    cv::imshow("board_points", img_show);
    static int a = 0;
    cv::imwrite("./data/CISCamera_Image/libdetect/calibdect" + std::to_string(a++) + ".bmp", img_show);
    cv::waitKey(5);
}
cv::Mat warpToPlane(const cv::Mat& img,                       // 输入原图
                    const std::vector<cv::Point2f>& img_pts,  // 图像角点（检测到的）
                    int board_width, int board_height, float square_size,
                    double scale  // mm -> px 比例
) {
    // Step1: 构造棋盘格物理坐标 (mm)
    std::vector<cv::Point2f> obj_points;
    for (int j = 0; j < board_height; ++j) {
        for (int i = 0; i < board_width; ++i) {
            obj_points.emplace_back(i * square_size, j * square_size);
        }
    }

    // Step2: 计算单应矩阵 (img -> world)
    cv::Mat H = cv::findHomography(img_pts, obj_points, cv::RANSAC);
    std::cout << "Homography Matrix H:\n" << H << std::endl;

    // Step3: 自适应输出范围
    std::vector<cv::Point2f> corners = {cv::Point2f(0, 0), cv::Point2f(img.cols, 0), cv::Point2f(img.cols, img.rows),
                                        cv::Point2f(0, img.rows)};

    std::vector<cv::Point2f> warped_corners;
    cv::perspectiveTransform(corners, warped_corners, H);

    float min_x = warped_corners[0].x, max_x = warped_corners[0].x;
    float min_y = warped_corners[0].y, max_y = warped_corners[0].y;
    for (auto& pt : warped_corners) {
        min_x = std::min(min_x, pt.x);
        max_x = std::max(max_x, pt.x);
        min_y = std::min(min_y, pt.y);
        max_y = std::max(max_y, pt.y);
    }

    int out_w = int((max_x - min_x) * scale);
    int out_h = int((max_y - min_y) * scale);

    // Step4: 缩放 + 平移矩阵
    cv::Mat S = (cv::Mat_<double>(3, 3) << scale, 0, 0, 0, scale, 0, 0, 0, 1);
    cv::Mat T = (cv::Mat_<double>(3, 3) << 1, 0, -min_x, 0, 1, -min_y, 0, 0, 1);
    cv::Mat H_final = S * T * H;

    // Step5: 透视变换
    cv::Mat img_warped;
    cv::warpPerspective(img, img_warped, H_final, cv::Size(out_w, out_h));

    // Step6: 在校正图上画棋盘点
    std::vector<cv::Point2f> warped_points;
    cv::perspectiveTransform(img_pts, warped_points, H_final);
    for (size_t i = 0; i < warped_points.size(); i++) {
        cv::circle(img_warped, warped_points[i], 5, cv::Scalar(0, 0, 255), -1);
        std::string text = "(" + std::to_string((int)warped_points[i].x) + ", " + std::to_string((int)warped_points[i].y) + ")";
        cv::putText(img_warped, text, warped_points[i] + cv::Point2f(10, -10), cv::FONT_HERSHEY_SIMPLEX, 0.5,
                    cv::Scalar(255, 255, 0), 1);
    }

    return img_warped;
}
// void plot_board_points(const cv::Mat& img, const Corner& corners,
//                        const std::vector<Board>& boards,
//                        std::vector<std::vector<cv::Point2d>>& board_points_sorted) {
//     cv::Mat img_show;
//     if (img.channels() != 3) {
// #if CV_VERSION_MAJOR >= 4
//         cv::cvtColor(img, img_show, cv::COLOR_GRAY2BGR);
// #else
//         cv::cvtColor(img, img_show, CV_GRAY2BGR);
// #endif
//     } else {
//         img_show = img.clone();
//     }

//     board_points_sorted.clear();  // 清空旧数据

//     for (int n = 0; n < boards.size(); ++n) {
//         const auto& board = boards[n];
//         std::vector<cv::Point2d> current_board_points;

//         cv::Point2d mean(0.0, 0.0);
//         int count = 0;
//         int pt_counter = 0; // 每个棋盘点计数器

//         for (int i = 1; i < board.idx.size() - 1; ++i) {
//             for (int j = 1; j < board.idx[i].size() - 1; ++j) {
//                 int idx = board.idx[i][j];
//                 if (idx < 0) continue;

//                 const cv::Point2d& pt = corners.p[idx];
//                 current_board_points.push_back(pt);
//                 mean += pt;
//                 count++;

//                 // 画角点
//                 cv::circle(img_show, pt, 1, cv::Scalar(0, 0, 255), -1, cv::LINE_AA);

//                 // 在角点旁边标注编号（可选：使用 pt_counter，也可以用 i,j 组合）
//                 std::string pt_label = std::to_string(pt_counter++);
//                 cv::putText(img_show, pt_label, pt + cv::Point2d(3, -3),
//                             cv::FONT_HERSHEY_PLAIN, 0.8, cv::Scalar(255, 0, 0), 1);
//             }
//         }

//         // 保存当前棋盘格点（按行排序）
//         std::sort(current_board_points.begin(), current_board_points.end(),
//                   [](const cv::Point2d& a, const cv::Point2d& b) {
//                       if (std::abs(a.y - b.y) > 5)  // y方向排序，容忍小误差
//                           return a.y < b.y;
//                       return a.x < b.x;
//                   });

//         board_points_sorted.push_back(current_board_points);

//         // 绘制编号
//         if (count > 0) {
//             mean /= count;
//             mean.x -= 10;
//             mean.y += 10;
//             cv::putText(img_show, std::to_string(n), mean,
//                         cv::FONT_HERSHEY_SIMPLEX, 1.3, cv::Scalar(0, 255, 0), 2);
//         }
//     }

//     cv::namedWindow("board_points", cv::WINDOW_NORMAL);
//     cv::imshow("board_points", img_show);
//     cv::imwrite("./result/calibdect.bmp",img_show);
//     cv::waitKey();
// }

}  // namespace cbdetect
