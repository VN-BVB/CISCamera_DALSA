

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

#include <opencv2/opencv.hpp>

#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/board_energy.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/boards_from_corners.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/config.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/filter_board.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/grow_board.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/init_board.h"

namespace cbdetect {

void debug_grow_process(const cv::Mat& img, const Corner& corners, const Board& board,
                        const std::vector<cv::Point2i>& proposal, int direction, bool type) {
  cv::Mat img_show;
  if(img.channels() != 3) {
#if CV_VERSION_MAJOR >= 4
    cv::cvtColor(img, img_show, cv::COLOR_GRAY2BGR);
#else
    cv::cvtColor(img, img_show, CV_GRAY2BGR);
#endif
  } else {
    img_show = img.clone();
  }

  cv::Point2d mean(0.0, 0.0);
  for(int i = 0; i < board.idx.size(); ++i) {
    for(int j = 0; j < board.idx[i].size(); ++j) {
      if(board.idx[i][j] < 0) {
        continue;
      }
      cv::circle(img_show, corners.p[board.idx[i][j]], 4, cv::Scalar(255, 0, 0), -1);
      cv::putText(img_show, std::to_string(board.idx[i][j]),
                  cv::Point2i(corners.p[board.idx[i][j]].x - 12, corners.p[board.idx[i][j]].y - 6),
                  cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
      mean += corners.p[board.idx[i][j]];
    }
  }
  mean /= (double)(board.num);
  mean.x -= 10;
  mean.y += 10;
  cv::putText(img_show, std::to_string(direction), mean,
              cv::FONT_HERSHEY_SIMPLEX, 1.3, cv::Scalar(196, 196, 0), 2);

  for(const auto& i : proposal) {
    if(board.idx[i.y][i.x] < 0) {
      continue;
    }
    if(type) {
      cv::circle(img_show, corners.p[board.idx[i.y][i.x]], 4, cv::Scalar(0, 255, 0), -1);
    } else {
      cv::circle(img_show, corners.p[board.idx[i.y][i.x]], 4, cv::Scalar(0, 0, 255), -1);
    }
  }

  cv::imshow("grow_process", img_show);
  cv::waitKey();
}

void boards_from_corners(const cv::Mat& img, const Corner& corners, std::vector<Board>& boards, const Params& params) {
  // intialize boards
  boards.clear();
  Board board;
  std::vector<int> used(corners.p.size(), 0);

  int start = 0;
  if(!params.overlay) {
    //如果不使用叠加模式（overlay），就随机选择一个角点作为生长起点，增加多样性，避免总是从第一个角点开始。
    std::default_random_engine e;
    auto time = std::chrono::system_clock::now().time_since_epoch();
    e.seed(static_cast<unsigned long>(time.count()));
    start = e() % corners.p.size();
  }
  //从所有角点尝试作为起点生成棋盘
  int n = 0;
  while(n++ < corners.p.size()) {
    int i = (n + start) % corners.p.size();// 顺序选取角点索引，带随机偏移
    if(used[i] == 1 || !init_board(corners, used, board, i)) {
      continue; // 如果这个角点已经被用过或无法初始化一个有效的3x3棋盘，就跳过
    }

     // 找到棋盘能量矩阵中的最大能量位置
    cv::Point3i maxE_pos = board_energy(corners, board, params);
    double energy        = board.energy[maxE_pos.y][maxE_pos.x][maxE_pos.z];
    // 如果棋盘质量不好（能量值大于阈值），回退并释放角点的“已使用”标记
    if(energy > -6.0) {
      for(int jj = 0; jj < 3; ++jj) {
        for(int ii = 0; ii < 3; ++ii) {
          used[board.idx[jj][ii]] = 0;
        }
      }
      continue;
    }

    // grow boards
    while(1) {
      int num_corners = board.num;

      for(int j = 0; j < (params.corner_type == MonkeySaddlePoint ? 6 : 4); ++j) {
        std::vector<cv::Point2i> proposal;
        GrowType grow_type = grow_board(corners, used, board, proposal, j, params);
        if(grow_type == GrowType_Failure) {
          continue;
        }

        if(params.show_grow_processing) {
          for(int ii = 0; ii < board.idx.size(); ++ii) {
            for(int jj = 0; jj < board.idx[ii].size(); ++jj) {
              std::cout << board.idx[ii][jj] << " ";
            }
            std::cout << "\n";
          }
          std::cout << "\n";
          debug_grow_process(img, corners, board, proposal, j, false);
        }

        filter_board(corners, used, board, proposal, energy, params);

        if(params.show_grow_processing) {
          for(int ii = 0; ii < board.idx.size(); ++ii) {
            for(int jj = 0; jj < board.idx[ii].size(); ++jj) {
              std::cout << board.idx[ii][jj] << " ";
            }
            std::cout << "\n";
          }
          std::cout << "\n";
          debug_grow_process(img, corners, board, proposal, j, true);
        }

        if(grow_type == GrowType_Inside) {
          --j;
        }
      }

      // exit loop
      if(board.num == num_corners) {
        break;
      }
    }

    if(!params.overlay) {
      boards.emplace_back(board);
      continue;
    }

    std::vector<std::pair<int, double>> overlap;
    for(int j = 0; j < boards.size(); ++j) {
      // check if new chessboard proposal overlaps with existing chessboards
      for(int k1 = 0; k1 < board.idx.size(); ++k1) {
        for(int k2 = 0; k2 < board.idx[0].size(); ++k2) {
          for(int l1 = 0; l1 < boards[j].idx.size(); ++l1) {
            for(int l2 = 0; l2 < boards[j].idx[0].size(); ++l2) {
              if(board.idx[k1][k2] != -1 && board.idx[k1][k2] != -2 && board.idx[k1][k2] == boards[j].idx[l1][l2]) {
                cv::Point3i maxE_pos_tmp = board_energy(corners, boards[j], params);
                overlap.emplace_back(std::make_pair(j, boards[j].energy[maxE_pos_tmp.y][maxE_pos_tmp.x][maxE_pos_tmp.z]));
                goto GOTO_BREAK;
              }
            }
          }
        }
      }
    }
  GOTO_BREAK:;

    if(overlap.empty()) {
      boards.emplace_back(board);
    } else {
      bool is_better = true;
      for(int j = 0; j < overlap.size(); ++j) {
        if(overlap[j].second <= energy) {
          is_better = false;
          break;
        }
      }
      if(is_better) {
        std::vector<Board> tmp;
        for(int j = 0, k = 0; j < boards.size(); ++j) {
          if(overlap[k].first == j) {
            continue;
            ++k;
          }
          tmp.emplace_back(boards[j]);
        }
        std::swap(tmp, boards);
        boards.emplace_back(board);
      }
    }
    std::fill(used.begin(), used.end(), 0);
    n += 2;
  }
}

} // namespace cbdetect
