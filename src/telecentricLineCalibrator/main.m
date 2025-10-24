% main.m
% 用于读取初步标定数据、图像点，并调用 refine_telecentric_camera 进行非线性优化
clear; clc; close all;

%% ===================== 1. 输入参数配置（请按需修改路径） =====================
spacingMM = 20.0;          % 标定板点间距（mm）
boardSize = [7, 7];        % 标定板网格尺寸（W×H）
W = boardSize(1);
H = boardSize(2);
k_init = 0;       % 初始假设无畸变
theta_init = 0;   % 初始假设无倾斜

% 文件路径（替换为你的实际路径）
imgPointsTxtDir = 'D:\Code\CISCamera_DALSA\data\CISCamera_Image\txt\';
calibDataTxtPath = 'D:\Code\CISCamera_DALSA\data\CISCamera_Image\etxt\before_optimization_calib_data.txt';

%% ===================== 2. 生成世界点（齐次/非齐次） =====================
% 生成平面点（Z=0），为了投影模型我们使用非齐次形式 ab = [Xw; Yw]
worldPts = zeros(W*H, 3);
idx = 1;
for r = 0:H-1
    for c = 0:W-1
        worldPts(idx,:) = [c*spacingMM, r*spacingMM, 0];  % Z=0
        idx = idx + 1;
    end
end
N = size(worldPts, 1);
ab = worldPts(:,1:2)'; % 2 x N (Xw; Yw)
fprintf('世界点生成完成：%d 个点 (2xN 格式)\n', N);

%% ===================== 3. 读取初步标定数据（保持你原有文件读取逻辑） =====================
fid = fopen(calibDataTxtPath, 'r');
if fid == -1
    error('无法打开初步标定数据文件：%s', calibDataTxtPath);
end

initIntrinsics = struct('m', [], 'dx', [], 'dy', [], 'u0', [], 'v0', []);
extrinsics = struct('rvec', {}, 'tvec', {}, 'reprojErr', {});
numImages = 0;

while ~feof(fid)
    line = strtrim(fgetl(fid));
    if isempty(line) || startsWith(line, '#')
        continue;
    end
    if contains(line, 'm:')
        tokens = regexp(line, ...
            'm:\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)\s*,?\s*dx:\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)\s*,?\s*dy:\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)\s*,?\s*u0:\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)\s*,?\s*v0:\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)', ...
            'tokens');
        if ~isempty(tokens)
            nums = tokens{1};
            initIntrinsics.m = str2double(nums{1});
            initIntrinsics.dx = str2double(nums{2});
            initIntrinsics.dy = str2double(nums{3});
            initIntrinsics.u0 = str2double(nums{4});
            initIntrinsics.v0 = str2double(nums{5});
            fprintf('解析内参: m=%.6f dx=%.6f dy=%.6f u0=%.3f v0=%.6f\n', ...
                initIntrinsics.m, initIntrinsics.dx, initIntrinsics.dy, initIntrinsics.u0, initIntrinsics.v0);
        end
    end
    if ~isempty(regexp(line, '^\d+\s', 'once'))
        numImages = numImages + 1;
        data = str2num(line); %#ok<ST2NM>
        if length(data) == 8
            extrinsics(numImages).rvec = data(2:4);
            extrinsics(numImages).tvec = data(5:7);
            extrinsics(numImages).reprojErr = data(8);
            fprintf('解析外参[%d]: rvec=[%.5f %.5f %.5f] t=[%.3f %.3f %.3f] err=%.4f\n', ...
                numImages, data(2), data(3), data(4), data(5), data(6), data(7), data(8));
        else
            warning('外参行长度异常，跳过： %s', line);
            numImages = numImages - 1;
        end
    end
end
fclose(fid);

if numImages == 0
    error('未解析到任何外参，请检查 before_optimization_calib_data.txt');
end

%% ===================== 4. 读取图像点（C++ 输出 txt 格式） =====================
imgTxtFiles = dir(fullfile(imgPointsTxtDir, '*.txt'));
if isempty(imgTxtFiles)
    error('在 %s 未找到图像点 txt 文件', imgPointsTxtDir);
end
[~, idxSort] = sort({imgTxtFiles.name});
imgTxtFiles = imgTxtFiles(idxSort);

allImagePoints = cell(numImages,1);
for i = 1:numImages
    if i > length(imgTxtFiles)
        warning('图像点文件数量少于外参数量，停止读取');
        break;
    end
    txtPath = fullfile(imgPointsTxtDir, imgTxtFiles(i).name);
    fid = fopen(txtPath,'r');
    if fid == -1
        warning('无法打开 %s，跳过', txtPath);
        continue;
    end
    fgetl(fid); % 跳过标题行
    pts = [];
    while ~feof(fid)
        line = fgetl(fid);
        if isempty(line), continue; end
        data = str2num(line); %#ok<ST2NM>
        if length(data) >= 3
            pts = [pts; data(2), data(3)];
        end
    end
    fclose(fid);
    if size(pts,1) ~= N
        error('第 %d 幅图像点数与世界点数不匹配：期望 %d 实际 %d', i, N, size(pts,1));
    end
    % 按 refine_telecentric_camera 的输入约定：每幅图为 1 x N x 2
    uv_i = zeros(1, N, 2);
    uv_i(1,:,1) = pts(:,1)'; % u
    uv_i(1,:,2) = pts(:,2)'; % v
    allImagePoints{i} = uv_i;
    fprintf('读取第 %d 幅图像点 (%s)，点数=%d\n', i, imgTxtFiles(i).name, N);
end

%% ===================== 5. 构建初始内外参（在进入优化前需准备好，用于初步重投影验证） =====================
% 初步内参矩阵 K_init (按你的公式12 简化无 theta,k)
K_init = [initIntrinsics.m / initIntrinsics.dx, 0, initIntrinsics.u0;
          0, 1 / initIntrinsics.dy, initIntrinsics.v0 / initIntrinsics.m;
          0, 0, 1];

% 初始旋转矩阵和 t 列表（将 rvec -> R）
R_init = cell(numImages,1);
t_init = cell(numImages,1);
for i = 1:numImages
    R_init{i} = rodrigues_compat(extrinsics(i).rvec(:));
    t_init{i} = extrinsics(i).tvec(:);
end

%% ===================== 6. 初步重投影误差验证（对应C++ computeReprojectionError） =====================
fprintf('\n========= 初步标定结果重投影误差验证 =========\n');
reprojErr_init = zeros(numImages, 1);

for i = 1:numImages
    % ---- 当前姿态 ----
    R2 = R_init{i}(1:2, 1:2);
    t2 = t_init{i}(1:2);

    % ---- 世界坐标 (2xN) ----
    xy = ab;  % 2 x N

    % ---- 仿射成像模型 [x; y] = R2*[Xw; Yw] + t2 ----
    img_affine = R2 * xy + repmat(t2, 1, N);
    img_affine = [img_affine; ones(1, N)]; % 扩展为3×N，方便投影

    % ---- 像素坐标预测（不含 theta,k）----
    uvw = K_init * img_affine;
    hat_u = uvw(1, :) ./ uvw(3, :);
    hat_v = uvw(2, :) ./ uvw(3, :);

    % ---- 实际像素 ----
    actual_u = squeeze(allImagePoints{i}(1,:,1));
    actual_v = squeeze(allImagePoints{i}(1,:,2));

    % ---- 计算误差 ----
    err_sq = (actual_u - hat_u).^2 + (actual_v - hat_v).^2;
    reprojErr_init(i) = sqrt(mean(err_sq));
    fprintf('图像 %02d: 重投影误差 = %.4f 像素\n', i, reprojErr_init(i));
end

avgReprojErr_init = mean(reprojErr_init);
fprintf('? 初步标定平均重投影误差 = %.4f 像素\n', avgReprojErr_init);

%% ===================== 7. 调用非线性LM优化 =====================
fprintf('\n========= 开始非线性LM优化 =========\n');
[K_opt, R_opt, t_opt, theta_opt, k_opt, final_MSE] = refine_telecentric_camera(...
    allImagePoints, ab, K_init, R_init, t_init, initIntrinsics.dx, initIntrinsics.dy);

fprintf('\n优化完成： theta=%.6e rad (%.4f deg), k=%.6e, final_MSE=%.6f\n', theta_opt, theta_opt*180/pi, k_opt, final_MSE);

%% ===================== 8. 保存并简单展示（可视化第一幅图） =====================
save('telecentric_calib_result.mat','K_opt','R_opt','t_opt','theta_opt','k_opt','final_MSE');
fprintf('结果已保存到 telecentric_calib_result.mat\n');

% 可视化第一幅图（实际点 vs 优化后重投影）
figure;
actual = squeeze(allImagePoints{1}(1,:,1:2));

% 计算第1幅优化后重投影
R2 = R_opt{1}(1:2,1:2);
t2 = t_opt{1}(1:2);
img_affine = R2 * ab + repmat(t2,1,N);
x_u = img_affine(1,:);
y_u = img_affine(2,:);
r_sq = x_u.^2 + (initIntrinsics.v0 * initIntrinsics.dy).^2;
delta_x = k_opt * x_u .* r_sq;
delta_y = -k_opt * initIntrinsics.v0 * initIntrinsics.dy .* r_sq;
x_d = x_u + delta_x;
y_d = y_u + delta_y;
hat_u = (initIntrinsics.m / initIntrinsics.dx) * x_d - (initIntrinsics.m * tan(theta_opt) / initIntrinsics.dx) * y_d + initIntrinsics.u0;
hat_v = (1 / (initIntrinsics.dy * cos(theta_opt))) * y_d + (initIntrinsics.v0 / initIntrinsics.m);
reproj = [hat_u(:), hat_v(:)];

% ---------- 绘制部分 ----------
figure;
plot(actual(:,1), actual(:,2), 'ro'); hold on;
plot(reproj(:,1), reproj(:,2), 'b+');

% 添加绿色箭头（表示误差向量）
dx = reproj(:,1) - actual(:,1);
dy = reproj(:,2) - actual(:,2);
quiver(actual(:,1), actual(:,2), dx, dy, 0, 'Color', [0,1,0], 'LineWidth', 1.2);

legend('实际','优化后重投影','误差方向');
axis equal; grid on;
title(sprintf('第1幅图：优化后重投影 (MSE=%.6f)', final_MSE));
xlabel('u / px'); ylabel('v / px');
%% ===================== 9. 优化总结报告 =====================
fprintf('\n================= 优化前后参数与误差对比报告 =================\n');

% 内参矩阵对比
fprintf('\n--- 内参矩阵 K ---\n');
fprintf('K_init =\n'); disp(K_init);
fprintf('K_opt  =\n'); disp(K_opt);
fprintf('变化 ΔK =\n'); disp(K_opt - K_init);

% 畸变参数和倾斜角
k_init = 0;   % 初始假设无畸变
theta_init = 0; % 初始假设无倾斜
fprintf('\n--- 畸变系数与倾斜角 ---\n');
fprintf('theta: %.6e rad → %.6e rad (%.4f deg → %.4f deg, Δ %.6e rad)\n', ...
    theta_init, theta_opt, theta_init*180/pi, theta_opt*180/pi, theta_opt - theta_init);
fprintf('k: %.6e → %.6e (Δ %.6e)\n', k_init, k_opt, k_opt - k_init);

% 外参变化（旋转+平移）
fprintf('\n--- 外参变化（前3幅图像示例） ---\n');
numShow = min(3, numImages);
for i = 1:numShow
    fprintf('图像 %d:\n', i);
    fprintf('R_init =\n'); disp(R_init{i});
    fprintf('R_opt  =\n'); disp(R_opt{i});
    fprintf('ΔR     =\n'); disp(R_opt{i} - R_init{i});
    
    fprintf('t_init = [%.6f %.6f %.6f]\n', t_init{i});
    fprintf('t_opt  = [%.6f %.6f %.6f]\n', t_opt{i});
    fprintf('Δt     = [%.6f %.6f %.6f]\n\n', t_opt{i} - t_init{i});
end

% 重投影误差
fprintf('\n--- 重投影误差变化 ---\n');
reprojErr_opt = zeros(numImages,1);
for i = 1:numImages
    % 计算优化后重投影
    uv_proj = zeros(2, N);
    xy = ab;
    R2 = R_opt{i}(1:2,1:2);
    t2 = t_opt{i}(1:2);
    img_affine = R2 * xy + repmat(t2,1,N);
    x_u = img_affine(1,:);
    y_u = img_affine(2,:);
    r_sq = x_u.^2 + (initIntrinsics.v0 * initIntrinsics.dy)^2;
    delta_x = k_opt * x_u .* r_sq;
    delta_y = -k_opt * initIntrinsics.v0 * initIntrinsics.dy .* r_sq;
    x_d = x_u + delta_x;
    y_d = y_u + delta_y;
    hat_u = (initIntrinsics.m / initIntrinsics.dx) * x_d - (initIntrinsics.m * tan(theta_opt) / initIntrinsics.dx) * y_d + initIntrinsics.u0;
    hat_v = (1 / (initIntrinsics.dy * cos(theta_opt))) * y_d + (initIntrinsics.v0 / initIntrinsics.m);
    uv_proj(1,:) = hat_u;
    uv_proj(2,:) = hat_v;

    actual = squeeze(allImagePoints{i}(1,:,1:2))';
    diff = uv_proj - actual;
    err_sq = sum(diff.^2,1);
    reprojErr_opt(i) = sqrt(mean(err_sq(:)));
    
    fprintf('图像 %02d: %.4f px → %.4f px (Δ %.4f px)\n', ...
        i, reprojErr_init(i), reprojErr_opt(i), reprojErr_opt(i)-reprojErr_init(i));
end

fprintf('平均重投影误差: %.4f px → %.4f px (Δ %.4f px)\n', ...
    mean(reprojErr_init), mean(reprojErr_opt), mean(reprojErr_opt)-mean(reprojErr_init));

fprintf('================= 报告结束 =================\n');
%% ===================== 辅助：Rodrigues (rvec -> R) =====================
function R = rodrigues_compat(rvec)
    theta = norm(rvec);
    if theta < 1e-12
        R = eye(3);
        return;
    end
    k = rvec / theta;
    K = [0, -k(3), k(2); k(3), 0, -k(1); -k(2), k(1), 0];
    R = eye(3) + sin(theta)*K + (1 - cos(theta))*(K*K);
end
