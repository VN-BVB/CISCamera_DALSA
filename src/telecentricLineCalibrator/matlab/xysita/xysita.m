%% 主程序入口
clear; clc; close all;

% ========== 初始化平台 ==========
[X_world, Y_world, Z_world, inside, board_length, board_width, ...
          world_pts3, platform_pts3, chess_pts3] = init_alignment_platform();

% ========== 执行相机模型投影 ==========
[pixel_u, pixel_v, in_image, u0, v0, img_width, img_height, inside] = ...
    camera_model_projection(X_world, Y_world, Z_world, inside);

% 小平台角点投影
for i = 1:length(platform_pts3)
    [u_square, v_square, in_img_square] = camera_model_projection( ...
        platform_pts3{i}(1,:), platform_pts3{i}(2,:), platform_pts3{i}(3,:), true(size(platform_pts3{i},2),1));
end


%% ========== 棋盘格角点投影（适配 cell） ==========
for i = 1:length(chess_pts3)

    if isempty(chess_pts3{i})
        continue;  % 该平台没有棋盘格
    end

    for k = 1:length(chess_pts3{i})   % 平台 i 的第 k 个棋盘格
        pts = chess_pts3{i}{k};       % Nx3

        [u_chess, v_chess, in_img_chess] = camera_model_projection( ...
            pts(1,:), pts(2,:), pts(3,:), true(size(pts,2),1));

        % (此处你可以继续处理 u_chess/v_chess，不做修改)
    end
end


%% ========== 保存棋盘格三维角点（适配 cell） ==========
for i = 1:length(chess_pts3)
    if isempty(chess_pts3{i})
        fprintf('平台 %d 没有棋盘格角点，跳过保存\n', i);
        continue;
    end

    cell_arr = chess_pts3{i};   % 平台 i 的棋盘格 cell 数组
    
    fprintf("平台 %d 棋盘格数量 = %d\n", i, length(cell_arr));

    for k = 1:length(cell_arr)
        pts = cell_arr{k};   % Nx3
        
        fprintf("平台 %d 第 %d 个棋盘格 pts 大小 = [%d, %d]\n", ...
            i, k, size(pts,1), size(pts,2));

        filename = sprintf('chessboard_platform%d_grid%d.txt', i-1, k-1);

        fid = fopen(filename, 'w');
        fprintf(fid, '# Index\tX\tY\n');

        for j = 1:size(pts, 2)
            fprintf(fid, '%d\t%.6f\t%.6f\n', j-1, pts(1,j), pts(2,j));
        end

        fclose(fid);
        fprintf('平台 %d 第 %d 个棋盘格三维角点已保存到 %s\n', i, k, filename);
    end
end


%% ===================== 可视化 =====================
figure('Color','w', 'Position', [100, 100, 1600, 800]);

% ---------- 子图1：世界坐标 ----------
subplot(1, 2, 1);
cla; hold on; axis equal; view(45, 30);
title('对位平台世界坐标系');
xlabel('X (mm) ←'); ylabel('Y (mm) →'); zlabel('Z (mm) ↑');
grid on; box on; rotate3d on;

% 绘制主平台底板
boardX = [0, board_length, board_length, 0, 0];
boardY = [0, 0, board_width, board_width, 0];
boardZ = [1, 1, 1, 1, 1];
patch('XData', boardX, 'YData', boardY, 'ZData', boardZ, ...
      'FaceColor', [0.9, 0.9, 0.9], 'EdgeColor', 'k', 'FaceAlpha', 0.6);

% 绘制主点
z_off = 0.08;
scatter3(X_world(inside), Y_world(inside), Z_world(inside)+z_off, ...
    100, 'bo', 'filled', 'LineWidth', 1.2);
scatter3(X_world(~inside), Y_world(~inside), Z_world(~inside)+z_off, ...
    120, 'rx', 'LineWidth', 2);

% 绘制每个小平台矩形
for i = 1:length(platform_pts3)
    p = platform_pts3{i};
    patch('XData', p(1,:), 'YData', p(2,:), 'ZData', p(3,:), ...
          'FaceColor', [0.6,0.9,0.6], 'EdgeColor', 'g', 'FaceAlpha', 0.4, 'LineWidth', 1.2);
end

% 绘制棋盘格 (cell 结构)
for i = 1:length(chess_pts3)
    if isempty(chess_pts3{i}), continue; end

    for k = 1:length(chess_pts3{i})
        c = chess_pts3{i}{k};
        scatter3(c(1,:), c(2,:), c(3,:)+0.2, 25, 'm', 'filled');
    end
end


%% ---------- 子图2：像素坐标 ----------
subplot(1, 2, 2);
cla; hold on; set(gca, 'YDir', 'reverse'); axis equal;
title('像素坐标系示意');
xlabel('U (像素)'); ylabel('V (像素)');
rectangle('Position', [0, 0, img_width, img_height], 'EdgeColor', 'k', 'LineWidth', 2);
grid on; box on;

% 绘制主点像素
scatter(pixel_u, pixel_v, 100, 'b', 'filled');

% 绘制棋盘格角点像素（cell 结构）
for i = 1:length(chess_pts3)
    if isempty(chess_pts3{i}), continue; end

    for k = 1:length(chess_pts3{i})
        pts = chess_pts3{i}{k};
        [u_chess, v_chess, ~] = camera_model_projection( ...
            pts(1,:), pts(2,:), pts(3,:), true(size(pts,2),1));

        scatter(u_chess, v_chess, 10, 'm', 'filled');
    end
end

% 主点中心
plot(u0, v0, 'ko', 'MarkerSize', 10, 'LineWidth', 2, 'MarkerFaceColor', 'yellow');

xlim([0, img_width]); ylim([0, img_height]);
sgtitle('世界坐标系与像素坐标系可视化', 'FontSize', 14, 'FontWeight', 'bold');


%% ========== 输出像素坐标表 ==========
fprintf('\n===== 像素坐标系下的点 =====\n');
fprintf('点编号\tU坐标\tV坐标\t是否在图像内\n');
fprintf('---------------------------------------------\n');
for i = 1:length(pixel_u)
    fprintf('%d\t%.2f\t%.2f\t%s\n', i, pixel_u(i), pixel_v(i), ...
        ternary(in_image(i),'是','否'));
end
fprintf('---------------------------------------------\n');
fprintf('总点数: %d\n', length(pixel_u));
fprintf('在图像内: %d\n', sum(in_image));
fprintf('在图像外: %d\n', sum(~in_image));


%% 小工具函数
function out = ternary(cond, a, b)
if cond, out = a; else, out = b; end
end
