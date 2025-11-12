%% ===================== 基础参数定义 =====================
% ------------------ 相机内参 ------------------
m  = 0.9998507744757738;             % 放大倍率（scale）
dx = 0.021148645273721497;           % 像素尺寸 mm/pixel (x方向)
dy = 0.021332260853116576;           % 像素尺寸 mm/pixel (y方向)
u0 = 15551.864283815234;             % 主点 u 坐标
v0 = 8049.999920255876;              % 主点 v 坐标

% 内参矩阵 K (3x3)
K = [47.277296561314515, -0.5909128336820076, 15551.864283815234;
     0.0, 46.88101813138735, 8051.201365000219;
     0.0, 0.0, 1.0];

% 畸变系数 [k1, h1, h2, s1, s2]
coff_dis = [-5.44279886107428e-10, -6.607307708466992e-07, ...
            -3.85955640389808e-07, 2.082696492966601e-06, ...
            -2.0374950491798976e-06];

% ------------------ 外参 ------------------
% 旋转向量 rvec (Rodrigues)
v_rot = [2.08338279543347, 2.0683810278122703, -0.20799695487856212];

% 平移向量 t (只取 X, Y，Z 由远心忽略)
v_trans = [-248.86297225612506, -135.65482873514011, 0.0];
% v_rot = [0, 0, 0];
% v_trans = [0, 0];  % 只取 X,Y 平移 (Z 由远心系统忽略)

% 图像尺寸 (像素)
img_width = 31104;
img_height = 16100;

% 板子尺寸 (mm)
board_width = 190;   % → 向右 (Y方向)
board_length = 325;  % → 向下 (X方向)

% 旋转中心坐标 [X, Y] (世界坐标系，单位 mm)
pts = [
    0,0;
    50,  40;
    100, 40;
    155, 40;
    205, 40;
    285, 60;
    50,  150;
    175, 150;
    245, 150;
    325,190;
];

% 提取世界坐标 (Z=1平面)
X_world = pts(:,1);   % 向下 (世界X轴)
Y_world = pts(:,2);   % 向右 (世界Y轴)
Z_world = ones(size(X_world));  % Z=1 (世界Z轴)

% 判断点是否在板子范围内（世界坐标）
inside = (X_world >= 0) & (X_world <= board_length) & ...
         (Y_world >= 0) & (Y_world <= board_width);

% 世界点 3xN
world_pts3 = [X_world'; Y_world'; Z_world'];   % 3xN

%% ========== Rodrigues：旋转向量 -> 旋转矩阵 ==========
theta = norm(v_rot);
if theta < 1e-12
    R_full = eye(3);
else
    k_axis = v_rot(:) / theta;  % 单位旋转轴
    Kskew = [   0       -k_axis(3)  k_axis(2);
             k_axis(3)    0       -k_axis(1);
            -k_axis(2)  k_axis(1)     0    ];
    R_full = eye(3) + sin(theta)*Kskew + (1 - cos(theta))*(Kskew*Kskew);
end

% 只保留前 2x2
R_2d = R_full(1:2,1:2);

% 构造等效的 "rt_matri" （3x3 形式）
R_ortho = eye(3);
R_ortho(1:2,1:2) = R_2d;
R_ortho(1:2,3)   = v_trans(1:2);  % 只用2D平移

disp('Effective 2D rotation matrix (R_2d):');
disp(R_2d);
disp('Translation (v_trans):');
disp(v_trans);

%% ========== 世界 -> 相机坐标变换 (正交投影模型) ==========
% 世界点：3×N
world_pts3 = [X_world'; Y_world'; ones(1, numel(X_world))];

% 相机坐标只考虑平面旋转和平移
camera_xy = R_ortho(1:2,:) * world_pts3;   % 2×N

% 扩展成 3×N，Z 恒为 1（远心相机无透视）
camera_points = [camera_xy; ones(1, size(camera_xy,2))];

% fprintf('\n===== 世界坐标系下的点 =====\n');
% fprintf('点编号\tX(世界)\tY(世界)\n');
% fprintf('-----------------------------\n');
% for i = 1:numel(X_world)
%     fprintf('%d\t%.4f\t%.4f\n', i, X_world(i), Y_world(i));
% end
% 
% fprintf('\n===== 相机坐标系下的点 =====\n');
% fprintf('点编号\tXc\t\tYc\t\tZc\n');
% fprintf('---------------------------------\n');
% for i = 1:size(camera_points,2)
%     fprintf('%d\t%.4f\t%.4f\t%.4f\n', i, camera_points(1,i), camera_points(2,i), camera_points(3,i));
% end


%% ========== 归一化坐标 ==========
% 对远心相机来说，不需要除以 Z
normalized_x = camera_points(1,:);  % Xc
normalized_y = camera_points(2,:);  % Yc

% fprintf('\n===== 归一化坐标 (畸变前) =====\n');
% fprintf('点编号\tXn\t\tYn\n');
% fprintf('--------------------------\n');
% for i = 1:numel(normalized_x)
%     fprintf('%d\t%.6f\t%.6f\n', i, normalized_x(i), normalized_y(i));
% end


%% ========== 应用畸变 ==========
[k1, h1, h2, s1, s2] = deal(coff_dis(1),coff_dis(2),coff_dis(3),coff_dis(4),coff_dis(5));
r_sq = normalized_x.^2 + normalized_y.^2;  % 径向距离平方

deltaX = k1 .* normalized_x .* r_sq + ...
         h1 .* (3*normalized_x.^2 + normalized_y.^2) + ...
         2*h2 .* normalized_x .* normalized_y + ...
         s1 .* r_sq;
deltaY = k1 .* normalized_y .* r_sq + ...
         2*h1 .* normalized_x .* normalized_y + ...
         h2 .* (normalized_x.^2 + 3*normalized_y.^2) + ...
         s2 .* r_sq;

distorted_x = normalized_x + deltaX;
distorted_y = normalized_y + deltaY;

% fprintf('\n===== 畸变后坐标 =====\n');
% fprintf('点编号\tXd\t\tYd\n');
% fprintf('--------------------------\n');
% for i = 1:numel(distorted_x)
%     fprintf('%d\t%.6f\t%.6f\n', i, distorted_x(i), distorted_y(i));
% end


%% ========== 畸变坐标 -> 像素坐标 ==========
distorted_homo = [distorted_x; distorted_y; ones(1, numel(distorted_x))]; % 3xN
disp('===== 内参矩阵 K =====');
disp(K);
pixel_coords = K * distorted_homo;  % 3xN
pixel_u = pixel_coords(1,:);  % 像素U坐标
pixel_v = pixel_coords(2,:);  % 像素V坐标

% fprintf('\n===== 像素坐标系下的点 =====\n');
% fprintf('点编号\tU(像素)\t\tV(像素)\n');
% fprintf('----------------------------------\n');
% for i = 1:numel(pixel_u)
%     fprintf('%d\t%.3f\t\t%.3f\n', i, pixel_u(i), pixel_v(i));
% end
%% ===================== 可视化 (保留原3D图 + 新增像素图) =====================
figure('Color','w', 'Position', [100, 100, 1600, 800]);

% ---------- 子图1：原有3D世界坐标系示意 ----------
subplot(1, 2, 1);
cla;
hold on;
axis equal;
view(45, 30);

% 绘制板子 (放在 z=1)
boardX = [0, board_length, board_length, 0, 0];
boardY = [0, 0, board_width, board_width, 0];
boardZ = [1, 1, 1, 1, 1];
patch('XData', boardX, 'YData', boardY, 'ZData', boardZ, ...
      'FaceColor', [0.9, 0.9, 0.9], 'EdgeColor', 'k', 'FaceAlpha', 0.6);

% 绘制旋转中心（板内蓝点，板外红点）
z_off = 0.08;
scatter3(X_world(inside), Y_world(inside), Z_world(inside)+z_off, 100, 'bo', 'filled', 'LineWidth', 1.2);
scatter3(X_world(~inside), Y_world(~inside), Z_world(~inside)+z_off, 120, 'rx', 'LineWidth', 2);

% 标注编号与坐标（把文本放高一些，避免遮挡）
for i = 1:length(X_world)
    tx = X_world(i);
    ty = Y_world(i);
    tz = Z_world(i) + 6;
    txt = sprintf('%d (%.0f,%.0f)', i, X_world(i), Y_world(i));
    text(tx, ty, tz, txt, 'FontSize', 10, 'HorizontalAlignment', 'center', 'BackgroundColor', 'w');
end

% 添加坐标轴箭头
quiver3(0,0,1, 40,0,0, 'r', 'LineWidth', 2, 'MaxHeadSize', 0.6);
text(40,0,1, ' X (down)','Color','r','FontWeight','bold');
quiver3(0,0,1, 0,40,0, 'g', 'LineWidth', 2, 'MaxHeadSize', 0.6);
text(0,40,1, ' Y (right)','Color','g','FontWeight','bold');
quiver3(0,0,1, 0,0,30, 'b', 'LineWidth', 2, 'MaxHeadSize', 0.6);
text(0,0,32, ' Z (up)','Color','b','FontWeight','bold');

% 轴设置
xlabel('X (mm) ↓ 向下');
ylabel('Y (mm) → 向右');
zlabel('Z (mm) ↑ 向上');
title('XYSita 对位平台旋转中心在板面 (Z=1) 上的三维示意');
xlim([-10, board_length + 10]);
ylim([-10, board_width + 40]);
zlim([0, 50]);
grid on;
box on;
rotate3d on;

%% ========= 子图2：像素坐标系示意 (新增) ----------
subplot(1, 2, 2);
cla;
hold on;
% 使用图像坐标系惯例：原点左上，y 向下
set(gca, 'YDir', 'reverse');
axis equal;  % 保持像素比例一致
% 不用 axis equal（像素比例按图像）
title('转换后的像素坐标系示意 (远心镜头+畸变)');
xlabel('U (像素)');
ylabel('V (像素)');

% 绘制图像框（用 patch/rectangle with Position）
rectangle('Position', [0, 0, img_width, img_height], 'EdgeColor', 'k', 'LineWidth', 2, 'FaceColor', 'none');

% 判断哪些像素点落在图像范围
in_image = (pixel_u >= 0) & (pixel_u <= img_width) & (pixel_v >=0) & (pixel_v <= img_height);

% 绘制像素点（板内且在图像内：蓝；板内但图像外：黄色边圈；板外：红叉）
% blue filled for points that are both inside board and in image
% 保证逻辑向量尺寸一致
inside = inside(:)';   
in_image = in_image(:)'; 
% 蓝色实心圆：在图像内且在板内
scatter(pixel_u(inside & in_image), pixel_v(inside & in_image), 100, 'b', 'filled');

% 黄色实心圆：在板内但不在图像内
scatter(pixel_u(inside & ~in_image), pixel_v(inside & ~in_image), 100, 'o', ...
        'MarkerEdgeColor', [0.85,0.65,0], 'MarkerFaceColor', [0.85,0.65,0], 'LineWidth',1.5);

% 红色实心叉：板外（投影到任意位置）
scatter(pixel_u(~inside), pixel_v(~inside), 250, 'x', 'MarkerEdgeColor', 'r', 'LineWidth', 2);

% 标注像素坐标（文本略微偏移，避免遮挡）
for i = 1:length(pixel_u)
%     txt = sprintf('%d (%.0f,%.0f)', i, pixel_u(i), pixel_v(i));
    txt = sprintf('%d', i);  % ? 只显示序号
    % 小心文本别超出图像：垂直偏移 -300 像素，更靠近点，但若超出图像会被剪裁（正常）
    text(pixel_u(i), pixel_v(i) - 1000, txt, 'FontSize', 8, ...
         'HorizontalAlignment', 'center', 'BackgroundColor', 'w');
end

% 标记主点（相机内参主点）
plot(u0, v0, 'ko', 'MarkerSize', 10, 'LineWidth', 2, 'MarkerFaceColor', 'yellow');
text(u0, v0 - 800, '主点 (u0,v0)', 'FontSize', 11, ...
     'HorizontalAlignment', 'center', 'FontWeight', 'bold', 'BackgroundColor','w');

% 轴设置与边界
xlim([0, img_width]);
ylim([0, img_height]);
grid on;
box on;

% 如果有点投影在图像外，提示数量
n_out = sum(~in_image);
if n_out > 0
    warning_msg = sprintf('%d 个点投影在图像外 (超出 %d x %d).', n_out, img_width, img_height);
    annotation('textbox',[0.62,0.02,0.36,0.05],'String',warning_msg,'EdgeColor','none','Color','r','FontSize',10,'FontWeight','bold');
end

% 总标题
sgtitle('世界坐标系3D示意 + 像素坐标系转换结果', 'FontSize', 14, 'FontWeight', 'bold');

hold off;

%% ========== 创建像素坐标表格 ==========
idx = (1:length(pixel_u))';
pixel_points = table(idx, pixel_u', pixel_v', in_image', ...
    'VariableNames', {'Index', 'U', 'V', 'InImage'});

% ========== 打印像素坐标系下的点 ==========
fprintf('\n===== 像素坐标系下的点 =====\n');
fprintf('点编号\tU坐标(像素)\tV坐标(像素)\t是否在图像内\n');
fprintf('---------------------------------------------\n');

for i = 1:height(pixel_points)
    if pixel_points.InImage(i)
        status = '是';
    else
        status = '否';
    end
    fprintf('%d\t%.2f\t%.2f\t%s\n', ...
        pixel_points.Index(i), pixel_points.U(i), pixel_points.V(i), status);
end

% 显示统计信息
fprintf('---------------------------------------------\n');
fprintf('总点数: %d\n', length(pixel_u));
fprintf('在图像内的点数: %d\n', sum(in_image));
fprintf('在图像外的点数: %d\n\n', sum(~in_image));