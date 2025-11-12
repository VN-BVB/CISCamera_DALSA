%% 主程序入口
clear; clc; close all;

% ========== 初始化平台 ==========
[X_world, Y_world, Z_world, inside, board_length, board_width, world_pts3] = init_alignment_platform();

% ========== 执行相机模型投影 ==========
[pixel_u, pixel_v, in_image, u0, v0, img_width, img_height, inside] = ...
    camera_model_projection(X_world, Y_world, Z_world, inside);

%% ===================== 可视化 =====================
figure('Color','w', 'Position', [100, 100, 1600, 800]);

% ---------- 子图1：世界坐标 ----------
subplot(1, 2, 1);
cla; hold on; axis equal; view(45, 30);

boardX = [0, board_length, board_length, 0, 0];
boardY = [0, 0, board_width, board_width, 0];
boardZ = [1, 1, 1, 1, 1];
patch('XData', boardX, 'YData', boardY, 'ZData', boardZ, ...
      'FaceColor', [0.9, 0.9, 0.9], 'EdgeColor', 'k', 'FaceAlpha', 0.6);

z_off = 0.08;
scatter3(X_world(inside), Y_world(inside), Z_world(inside)+z_off, 100, 'bo', 'filled', 'LineWidth', 1.2);
scatter3(X_world(~inside), Y_world(~inside), Z_world(~inside)+z_off, 120, 'rx', 'LineWidth', 2);

for i = 1:length(X_world)
    tx = X_world(i); ty = Y_world(i); tz = Z_world(i) + 6;
   txt = sprintf('%d (%.0f,%.0f)', i, X_world(i), Y_world(i));
    text(tx, ty, tz, txt, 'FontSize', 12, 'FontWeight','bold', ...
         'HorizontalAlignment', 'center', 'BackgroundColor', 'w');
end

xlabel('X (mm) ↓'); ylabel('Y (mm) →'); zlabel('Z (mm) ↑');
title('对位平台世界坐标系');
grid on; box on; rotate3d on;

% ---------- 子图2：像素坐标 ----------
subplot(1, 2, 2);
cla; hold on; set(gca, 'YDir', 'reverse'); axis equal;
title('像素坐标系示意'); xlabel('U (像素)'); ylabel('V (像素)');
rectangle('Position', [0, 0, img_width, img_height], 'EdgeColor', 'k', 'LineWidth', 2);
inside = inside(:);       % 变成列向量
in_image = in_image(:);   % 变成列向量
scatter(pixel_u(inside & in_image), pixel_v(inside & in_image), 100, 'b', 'filled');
scatter(pixel_u(inside & ~in_image), pixel_v(inside & ~in_image), 100, 'o', ...
        'MarkerEdgeColor', [0.85,0.65,0], 'MarkerFaceColor', [0.85,0.65,0], 'LineWidth',1.5);
scatter(pixel_u(~inside), pixel_v(~inside), 250, 'x', 'MarkerEdgeColor', 'r', 'LineWidth', 2);

for i = 1:length(pixel_u)
    txt = sprintf('%d', i);
    text(pixel_u(i) + 300, pixel_v(i) - 300, txt, ...
         'FontSize', 12, 'FontWeight','bold', ...
         'HorizontalAlignment', 'center', 'BackgroundColor', 'w');
end

plot(u0, v0, 'ko', 'MarkerSize', 10, 'LineWidth', 2, 'MarkerFaceColor', 'yellow');
text(u0, v0 - 800, '主点 (u0,v0)', 'FontSize', 11, ...
     'HorizontalAlignment', 'center', 'FontWeight', 'bold', 'BackgroundColor','w');

xlim([0, img_width]); ylim([0, img_height]); grid on; box on;
sgtitle('世界坐标系与像素坐标系可视化', 'FontSize', 14, 'FontWeight', 'bold');

% ========== 输出像素坐标表 ==========
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
