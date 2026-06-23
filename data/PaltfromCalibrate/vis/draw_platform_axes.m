function draw_platform_axes_matlab(platform_idx, bmp_path, json_calib, json_pose, varargin)
% draw_platform_axes_matlab  在标定原图上画对位平台坐标系
%
% 用法:
%   % 画单个平台
%   draw_platform_axes_matlab(5, 'Splice_xxx.bmp', ...
%       'optimized_calib_data.json', 'platform_pose.json')
%
%   % 画所有有效平台
%   draw_platform_axes_matlab([], 'Splice_xxx.bmp', ...
%       'optimized_calib_data.json', 'platform_pose.json')
%
% 可选 Name-Value:
%     'axisLen'    : 坐标轴长度 mm (默认 50)
%     'downsample' : 全图降采样比例 (默认 0.04)
%     'savePath'   : 输出图片路径

p = inputParser;
p.addParameter('axisLen', 50, @isnumeric);
p.addParameter('downsample', 0.04, @isnumeric);
p.addParameter('savePath', '', @ischar);
p.parse(varargin{:});
opt = p.Results;

% ═══════════════════════════════════════════════════════════
% 1. 加载 JSON
% ═══════════════════════════════════════════════════════════
calib = jsondecode(fileread(json_calib));
pose  = jsondecode(fileread(json_pose));

if isfield(calib, 'value0'), calib = calib.value0; end
if isfield(pose, 'PlatformPoseData'), pose = pose.PlatformPoseData; end

% K 矩阵
K = reshape(calib.K.val, [calib.K.cols, calib.K.rows])';

% 外参
allRot  = pose.allRotVecs;
allTrans = pose.allTransVecs;

% ═══════════════════════════════════════════════════════════
% 2. 确定要画的平台
% ═══════════════════════════════════════════════════════════
if isempty(platform_idx)
    platforms = 0:length(allRot)-1;
else
    platforms = platform_idx;
end

% 预计算每个平台的像素坐标
Np = length(platforms);
px_data = cell(Np, 1);
cam2px = @(xc, yc) [K(1,1)*xc + K(1,2)*yc + K(1,3), ...
                     K(2,2)*yc + K(2,3)];

for k = 1:Np
    p = platforms(k) + 1;  % 1-based index

    if isstruct(allRot)
        rv = [allRot(p).value0, allRot(p).value1, allRot(p).value2];
        tv = [allTrans(p).value0, allTrans(p).value1, allTrans(p).value2];
    else
        rv = allRot(p, :);
        tv = allTrans(p, :);
    end

    if all(abs(tv) < 1e-6)
        fprintf('平台 %d 位姿为空，跳过\n', platforms(k));
        px_data{k} = [];
        continue;
    end

    R = rodrigues(rv);
    R2 = R(1:2, 1:2);
    tx = tv(1);  ty = tv(2);

    center_px = cam2px(tx, ty);
    L = opt.axisLen;
    x_tip_px = cam2px(tx + L*R2(1,1), ty + L*R2(2,1));
    y_tip_px = cam2px(tx + L*R2(1,2), ty + L*R2(2,2));

    px_data{k} = struct('p', platforms(k), 'center', center_px, ...
                        'x_tip', x_tip_px, 'y_tip', y_tip_px, ...
                        'tx', tx, 'ty', ty, 'R2', R2);
    fprintf('平台 %d: t=[%.1f, %.1f]mm  pixel=[%.1f, %.1f]  theta=%.1f deg\n', ...
            platforms(k), tx, ty, center_px, atan2d(R2(2,1), R2(1,1)));
end

% ═══════════════════════════════════════════════════════════
% 3. 读图
% ═══════════════════════════════════════════════════════════
fprintf('读取图像: %s\n', bmp_path);
img = imread(bmp_path);
if size(img, 3) == 3, img = rgb2gray(img); end
img = double(img) / 255;
[H, W] = size(img);

ds = opt.downsample;
img_small = imresize(img, ds, 'bilinear');

% ═══════════════════════════════════════════════════════════
% 4. 画图
% ═══════════════════════════════════════════════════════════
figure('Position', [100, 100, 1200, 900], 'Name', 'Platform Axes');
imshow(img_small);  hold on;
title(sprintf('Platform Coordinate Systems  (scale %.0f%%)', ds*100));

% 颜色表 (区分不同平台)
colors = lines(Np);

for k = 1:Np
    d = px_data{k};
    if isempty(d), continue; end

    c = colors(k, :);
    cs = d.center * ds;
    xs = d.x_tip * ds;
    ys = d.y_tip * ds;

    % 旋转中心: 实心圆 + 黑边
    plot(cs(1), cs(2), 'o', 'MarkerSize', 7, ...
         'MarkerFaceColor', c, 'MarkerEdgeColor', 'k', 'LineWidth', 1.5);

    % X 轴
    quiver(cs(1), cs(2), xs(1)-cs(1), xs(2)-cs(2), 0, ...
           'Color', [1 0.2 0.2], 'LineWidth', 2, 'MaxHeadSize', 10);
    % Y 轴
    quiver(cs(1), cs(2), ys(1)-cs(1), ys(2)-cs(2), 0, ...
           'Color', [0.2 0.7 0.2], 'LineWidth', 2, 'MaxHeadSize', 10);

    text(xs(1)+4, xs(2)-4, 'X', 'Color', [1 0.2 0.2], ...
         'FontSize', 12, 'FontWeight', 'bold');
    text(ys(1)+4, ys(2)-4, 'Y', 'Color', [0.2 0.7 0.2], ...
         'FontSize', 12, 'FontWeight', 'bold');

    % 平台标签
    text(cs(1)+10, cs(2)-8, sprintf('P%d', d.p), ...
         'Color', 'w', 'FontSize', 10, 'FontWeight', 'bold', ...
         'BackgroundColor', [0 0 0 0.6]);
end

% 图例
hold off;
fprintf('Done.\n');

% ═══════════════════════════════════════════════════════════
% 5. 保存
% ═══════════════════════════════════════════════════════════
if ~isempty(opt.savePath)
    exportgraphics(gcf, opt.savePath, 'Resolution', 150);
    fprintf('已保存: %s\n', opt.savePath);
end
end

% ═══════════════════════════════════════════════════════════
function R = rodrigues(r)
    theta = norm(r);
    if theta < 1e-12, R = eye(3); return; end
    k = r / theta;
    K = [0 -k(3) k(2); k(3) 0 -k(1); -k(2) k(1) 0];
    R = eye(3) + sin(theta)*K + (1-cos(theta))*K*K;
end
