function [X_world, Y_world, Z_world, inside, board_length, board_width, world_pts3] = init_alignment_platform()
%% ===================== 对位平台初始化 =====================
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
world_pts3 = [X_world'; Y_world'; Z_world'];
end
