function [X_world, Y_world, Z_world, inside, board_length, board_width, ...
          world_pts3, platform_pts3, chess_pts3] = init_alignment_platform()
%% ===================== 对位平台初始化 =====================

% 主平台尺寸 (mm)
board_width  = 190;   % → Y方向
board_length = 325;   % → X方向

% 主点（平台中心点）坐标 (mm)
pts = [
%     0,0;
    50,  40;
    100, 40;
    155, 40;
    205, 40;
    285, 60;
    50,  150;
    175, 150;
    245, 150;
%     325,190;
];

% 主平台点（世界坐标）
X_world = pts(:,1);
Y_world = pts(:,2);
Z_world = ones(size(X_world));

inside = (X_world >= 0) & (X_world <= board_length) & ...
         (Y_world >= 0) & (Y_world <= board_width);
world_pts3 = [X_world'; Y_world'; Z_world'];

%% ===================== 局部对位小平台生成 =====================
sub_size = 50;   % 每个小平台边长 (mm)
half = sub_size/2;

% 定义小平台角点（局部坐标，Z=1）
local_square = [
    -half, -half;
     half, -half;
     half,  half;
    -half,  half;
    -half, -half
];

platform_pts3 = cell(length(X_world), 1);
for i = 1:length(X_world)
    % 平移到主平台点
    local = local_square + [X_world(i), Y_world(i)];
    platform_pts3{i} = [local, ones(size(local,1),1)]';  % 3xN
end

%% ===================== 棋盘格角点生成 =====================
% 假设主平台有 10 个点
platforms_with_chess = [1];         % 第2和第5个平台生成棋盘格
offsets = [-25, 25];             % 第2个平台偏移(2,-1)，第5个(-3,0)
rotations = [-pi/4];           % 第2个平台旋转30度，第5个旋转-15度

chess_pts3 = generate_chess_on_platform2(X_world, Y_world, ...
                                        platforms_with_chess, ...
                                        7, 3, offsets, rotations);
                                    
                                    
end
%% ===================== 增强棋盘格角点生成 原地+平移=====================
function chess_pts3 = generate_chess_on_platform(X_world, Y_world, ...
                                                 platforms_with_chess, ...
                                                 grid_n, grid_d, ...
                                                 offsets, rotations)
% X_world, Y_world: 主平台坐标向量
% platforms_with_chess: 放棋盘格的平台编号数组，例如 [2,5,7]
% grid_n: 棋盘格角点数 (nxn)
% grid_d: 棋盘格间距 (mm)
% offsets: Nx2 偏移向量 [dx, dy]，每个平台对应一行
% rotations: N x 1 旋转角度 (rad)，每个平台对应一行，绕平台中心旋转

half_grid = (grid_n - 1) * grid_d / 2;
[xx, yy] = meshgrid(-half_grid:grid_d:half_grid, -half_grid:grid_d:half_grid);
local_chess = [xx(:), yy(:)];  % nxn x 2

chess_pts3 = cell(length(X_world), 1);  % 初始化

for idx = 1:length(platforms_with_chess)
    i = platforms_with_chess(idx);          % 平台编号
    dx = offsets(idx, 1); dy = offsets(idx, 2);  % 偏移
    theta = rotations(idx);                  % 旋转角度 (弧度)

    % 旋转矩阵
    R = [cos(theta), -sin(theta);
         sin(theta),  cos(theta)];

    % 平移 + 旋转
    transformed = (R * local_chess')';       % 旋转
    transformed = transformed + [X_world(i)+dx, Y_world(i)+dy]; % 平移到平台中心 + 偏移

    % 保存
    chess_pts3{i} = [transformed, ones(size(transformed,1),1)]';  % 3xN
end

% 其他平台为空
for i = 1:length(X_world)
    if isempty(chess_pts3{i})
        chess_pts3{i} = [];
    end
end
end
%% ===================== 增强棋盘格角点生成 平移+原地=====================
function chess_pts3 = generate_chess_on_platform2(X_world, Y_world, ...
                                                 platforms_with_chess, ...
                                                 grid_n, grid_d, ...
                                                 offsets, rotations)
% X_world, Y_world: 主平台坐标向量
% platforms_with_chess: 放棋盘格的平台编号数组，例如 [2,5,7]
% grid_n: 棋盘格角点数 (nxn)
% grid_d: 棋盘格间距 (mm)
% offsets: Nx2 偏移向量 [dx, dy]，每个平台对应一行
% rotations: N x 1 旋转角度 (rad)，每个平台对应一行
%            绕平台中心旋转

half_grid = (grid_n - 1) * grid_d / 2;
[yy, xx] = meshgrid(-half_grid:grid_d:half_grid, -half_grid:grid_d:half_grid);
local_chess = [xx(:), yy(:)];  % nxn x 2

chess_pts3 = cell(length(X_world), 1);

for idx = 1:length(platforms_with_chess)
    i = platforms_with_chess(idx);  % 平台编号
    dx = offsets(idx, 1);
    dy = offsets(idx, 2);
    theta = rotations(idx);

    % 旋转矩阵
    R = [cos(theta), -sin(theta);
         sin(theta),  cos(theta)];

    % 平台中心
    C = [X_world(i), Y_world(i)];

    % -------------------------------
    % 1. 平移棋盘格中心
    % 2. 绕平台中心旋转
    % -------------------------------
    transformed = (R * (local_chess' + [dx; dy]))';  % 平移后绕原点旋转
    transformed = transformed + C;                   % 放回平台中心位置

    % 保存结果
    chess_pts3{i} = [transformed, ones(size(transformed,1),1)]';
end

% 保证所有单元非空
for i = 1:length(X_world)
    if isempty(chess_pts3{i})
        chess_pts3{i} = [];
    end
end

end
