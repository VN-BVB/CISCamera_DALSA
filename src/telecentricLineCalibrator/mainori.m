% main_LS.m
% 基于最小二乘的远心相机微调优化（theta, k, 内参微调）
clear; clc; close all;

%% ===================== 1. 输入参数配置 =====================
spacingMM = 20.0;          % 标定板点间距（mm）
boardSize = [7, 7];        % 标定板网格尺寸（W×H）
W = boardSize(1);
H = boardSize(2);

% 文件路径（请根据实际路径修改）
imgPointsTxtDir = 'D:\Code\CISCamera_DALSA\data\CISCamera_Image\txt\';
calibDataTxtPath = 'D:\Code\CISCamera_DALSA\data\CISCamera_Image\etxt\before_optimization_calib_data.txt';

%% ===================== 2. 生成世界点 =====================
worldPts = zeros(W*H, 3);
idx = 1;
for r = 0:H-1
    for c = 0:W-1
        worldPts(idx,:) = [c*spacingMM, r*spacingMM, 0];
        idx = idx + 1;
    end
end
N = size(worldPts,1);
ab = worldPts(:,1:2)'; % 2 x N，世界坐标（mm）
fprintf('世界点生成完成：%d 个点\n', N);

%% ===================== 3. 读取初步标定数据 =====================
fid = fopen(calibDataTxtPath, 'r');
if fid == -1
    error('无法打开文件：%s', calibDataTxtPath);
end

initIntrinsics = struct('m', [], 'dx', [], 'dy', [], 'u0', [], 'v0', []);
extrinsics = struct('rvec', {}, 'tvec', {}, 'reprojErr', {});
numImages = 0;

while ~feof(fid)
    line = strtrim(fgetl(fid));
    if isempty(line) || startsWith(line,'#'), continue; end
    if contains(line,'m:')
        tokens = regexp(line, ...
            'm:\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)\s*,?\s*dx:\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)\s*,?\s*dy:\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)\s*,?\s*u0:\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)\s*,?\s*v0:\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)','tokens');
        if ~isempty(tokens)
            nums = tokens{1};
            initIntrinsics.m  = str2double(nums{1});
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
        else
            warning('外参行异常：%s', line);
            numImages = numImages - 1;
        end
    end
end
fclose(fid);
if numImages == 0, error('未解析到任何外参'); end

%% ===================== 4. 读取图像点 =====================
imgTxtFiles = dir(fullfile(imgPointsTxtDir,'*.txt'));
if isempty(imgTxtFiles), error('未找到图像点 txt 文件'); end
[~, idxSort] = sort({imgTxtFiles.name});
imgTxtFiles = imgTxtFiles(idxSort);

allImagePoints = cell(numImages,1);
for i = 1:numImages
    if i > length(imgTxtFiles), break; end
    txtPath = fullfile(imgPointsTxtDir, imgTxtFiles(i).name);
    fid = fopen(txtPath,'r'); if fid==-1, warning('无法打开 %s',txtPath); continue; end
    fgetl(fid); % 跳过标题
    pts = [];
    while ~feof(fid)
        line = fgetl(fid);
        if isempty(line), continue; end
        data = str2num(line); %#ok<ST2NM>
        if length(data)>=3, pts = [pts; data(2), data(3)]; end
    end
    fclose(fid);
    if size(pts,1)~=N, error('第 %d 幅图像点数与世界点数不匹配', i); end
    uv_i = zeros(1,N,2);
    uv_i(1,:,1) = pts(:,1)';
    uv_i(1,:,2) = pts(:,2)';
    allImagePoints{i} = uv_i;
end

%% ===================== 5. 构建初始内外参 =====================
K_init = [initIntrinsics.m/initIntrinsics.dx, 0, initIntrinsics.u0;
          0, 1/initIntrinsics.dy, initIntrinsics.v0/initIntrinsics.m;
          0, 0, 1];
R_init = cell(numImages,1); t_init = cell(numImages,1);
for i = 1:numImages
    R_init{i} = rodrigues_compat(extrinsics(i).rvec(:));
    t_init{i} = extrinsics(i).tvec(:);
end

%% ===================== 5.5 计算优化前重投影误差 =====================
fprintf('\n=== 优化前重投影误差 ===\n');
rmsList = zeros(numImages,1);

for i = 1:numImages
    % 外参
    R2 = R_init{i}(1:2,1:2); 
    t2 = t_init{i}(1:2);
    
    % 仿射投影
    img_affine = R2*ab + repmat(t2,1,N);
    x_u = img_affine(1,:); 
    y_u = img_affine(2,:);
    
    % 无畸变和倾斜
    x_d = x_u; 
    y_d = y_u;
    
    % 投影到像素坐标
    hat_u = K_init(1,1)*x_d + K_init(1,3);
    hat_v = K_init(2,2)*y_d + K_init(2,3);
    
    actual = squeeze(allImagePoints{i}(1,:,:));
    err = sqrt((hat_u(:)-actual(:,1)).^2 + (hat_v(:)-actual(:,2)).^2);
    rmsList(i) = mean(err); % 平均像素误差
    fprintf('第 %d 幅图: 平均像素误差 = %.4f px\n', i, rmsList(i));
end

fprintf('所有图像平均误差 = %.4f px\n', mean(rmsList));

%% ===================== 6. 最小二乘优化 (核心修正：减少参数冗余) =====================
% 优化参数: [theta, k, m微调, u0微调, v0微调]
% 消除fy冗余，通过theta和m直接计算fx/fy，符合文档公式12
params0 = [0; 0; 0; 0; 0];  % 初始值
lb = [-0.1; -1e-4; -0.01; -10; -10];  % 参数下界（k范围缩小至合理值）
ub = [0.1; 1e-4; 0.01; 10; 10];       % 参数上界
opts = optimoptions('lsqnonlin', ...
    'Display','iter', ...
    'TolFun',1e-12, ...
    'TolX',1e-12, ...
    'MaxFunctionEvaluations',1e5);

% 调用优化函数（传入初始内参而非K_init，避免参数耦合）
[params_opt,resnorm] = lsqnonlin(...
    @(p) reproj_residuals(p, allImagePoints, ab, R_init, t_init, initIntrinsics), ...
    params0, lb, ub, opts);

% 解包优化参数
theta_opt = params_opt(1);       % 倾斜角
k_opt     = params_opt(2);       % 畸变系数
m_opt     = initIntrinsics.m + params_opt(3);  % 优化后的放大倍率
u0_opt    = initIntrinsics.u0 + params_opt(4); % 优化后的u0
v0_opt    = initIntrinsics.v0 + params_opt(5); % 优化后的v0

% 计算优化后的内参矩阵（基于文档公式12）
fx_opt = m_opt / initIntrinsics.dx;
fy_opt = 1 / (initIntrinsics.dy * cos(theta_opt));
K_opt = [fx_opt, -fx_opt*tan(theta_opt), u0_opt;  % 修正theta对u的影响项
         0, fy_opt, v0_opt/m_opt;
         0, 0, 1];

fprintf('\n优化完成: theta=%.6e rad, k=%.6e, m=%.6f\n', theta_opt, k_opt, m_opt);
fprintf('优化后内参: fx=%.6f, fy=%.6f, u0=%.3f, v0=%.3f\n', fx_opt, fy_opt, u0_opt, v0_opt);

%% ===================== 优化后重投影误差（核心修正：删除k_opt=0） =====================
fprintf('\n=== 优化后重投影误差 ===\n');
rmsOptList = zeros(numImages,1);

for i = 1:numImages
    R2 = R_init{i}(1:2,1:2);
    t2 = t_init{i}(1:2);
    img_affine = R2*ab + repmat(t2,1,N);
    x_u = img_affine(1,:);
    y_u = img_affine(2,:);
    
    % 应用畸变（文档公式3）
    r_sq = x_u.^2 + (v0_opt * initIntrinsics.dy)^2;
    delta_x = k_opt * x_u .* r_sq;
    delta_y = -k_opt * v0_opt * initIntrinsics.dy .* r_sq;
    x_d = x_u + delta_x;
    y_d = y_u + delta_y;
    
    % 投影到像素坐标（文档公式12，修正hat_v计算）
    hat_u = fx_opt * x_d - fx_opt * tan(theta_opt) * y_d + u0_opt;
    hat_v = fy_opt * y_d + (v0_opt / m_opt);  % 正确使用m_opt
    
    actual = squeeze(allImagePoints{i}(1,:,:));
    err = sqrt((hat_u(:)-actual(:,1)).^2 + (hat_v(:)-actual(:,2)).^2);
    rmsOptList(i) = mean(err);
    fprintf('第 %d 幅图: 平均像素误差 = %.4f px\n', i, rmsOptList(i));
end

fprintf('所有图像优化后平均误差 = %.4f px\n', mean(rmsOptList));
fprintf('误差改善量 = %.4f px\n', mean(rmsList) - mean(rmsOptList));

%% ===================== 7. 可视化第一幅图 =====================
figure;
actual = squeeze(allImagePoints{1}(1,:,:));
R2 = R_init{1}(1:2,1:2); t2 = t_init{1}(1:2);
img_affine = R2*ab + repmat(t2,1,N);
x_u = img_affine(1,:); y_u = img_affine(2,:);

% 计算重投影点
r_sq = x_u.^2 + (v0_opt*initIntrinsics.dy)^2;
delta_x = k_opt*x_u.*r_sq; delta_y = -k_opt*v0_opt*initIntrinsics.dy.*r_sq;
x_d = x_u + delta_x; y_d = y_u + delta_y;
hat_u = fx_opt*x_d - fx_opt*tan(theta_opt)*y_d + u0_opt;
hat_v = fy_opt*y_d + (v0_opt / m_opt);
reproj = [hat_u(:), hat_v(:)];

% 绘图
plot(actual(:,1), actual(:,2),'ro','MarkerSize',6,'DisplayName','实际点'); hold on;
plot(reproj(:,1), reproj(:,2),'b+','MarkerSize',6,'DisplayName','重投影点');
dx = reproj(:,1)-actual(:,1); dy = reproj(:,2)-actual(:,2);
quiver(actual(:,1), actual(:,2), dx, dy, 0,'Color',[0,1,0],'DisplayName','误差向量');
legend('Location','best'); axis equal; grid on;
title(sprintf('第1幅图优化结果 (平均误差: %.4f px)', rmsOptList(1)));
xlabel('u / px'); ylabel('v / px');

%% ===================== 8. 保存结果 =====================
save('telecentric_calib_LS_result.mat',...
    'K_opt','theta_opt','k_opt','m_opt','fx_opt','fy_opt','u0_opt','v0_opt',...
    'rmsList','rmsOptList');
fprintf('优化结果已保存到 telecentric_calib_LS_result.mat\n');

%% ===================== 9. 辅助函数: Rodrigues变换 =====================
function R = rodrigues_compat(rvec)
    theta = norm(rvec);
    if theta < 1e-12
        R = eye(3); return;
    end
    k = rvec/theta;
    K = [0,-k(3),k(2); k(3),0,-k(1); -k(2),k(1),0];
    R = eye(3) + sin(theta)*K + (1-cos(theta))*K*K;
end

%% ===================== 10. 重投影残差函数（核心修正：公式对齐） =====================
function residuals = reproj_residuals(params, allImagePoints, ab, R_init, t_init, initIntrinsics)
    % 解析参数
    theta = params(1);
    k     = params(2);
    m_inc = params(3);  % m的微调量
    u0_inc = params(4); % u0的微调量
    v0_inc = params(5); % v0的微调量

    % 计算实际参数
    m = initIntrinsics.m + m_inc;
    u0 = initIntrinsics.u0 + u0_inc;
    v0 = initIntrinsics.v0 + v0_inc;
    dx = initIntrinsics.dx;
    dy = initIntrinsics.dy;
    fx = m / dx;                  % 文档公式12的fx
    fy = 1 / (dy * cos(theta));   % 文档公式12的fy（无冗余）

    residuals = [];
    N = size(ab,2);
    for i = 1:length(allImagePoints)
        % 外参投影（世界坐标→相机平面坐标）
        R2 = R_init{i}(1:2,1:2);
        t2 = t_init{i}(1:2);
        img_affine = R2 * ab + repmat(t2, 1, N);
        x_u = img_affine(1,:);
        y_u = img_affine(2,:);

        % 径向畸变（文档公式3）
        r_sq = x_u.^2 + (v0 * dy)^2;  % 正确计算r?
        delta_x = k * x_u .* r_sq;
        delta_y = -k * v0 * dy .* r_sq;
        x_d = x_u + delta_x;
        y_d = y_u + delta_y;

        % 重投影像素坐标（文档公式12，修正两处错误）
        hat_u = fx * x_d - fx * tan(theta) * y_d + u0;  % 包含theta对u的影响
        hat_v = fy * y_d + (v0 / m);                    % 正确使用m计算v0项

        % 计算残差（实际点 - 重投影点）
        actual = squeeze(allImagePoints{i}(1,:,:));
        res_u = hat_u(:) - actual(:,1);
        res_v = hat_v(:) - actual(:,2);
        residuals = [residuals; res_u; res_v];
    end
end