%% 重投影误差计算与残差可视化（按顺序读取TXT，不指定文件名）
clear; clc; close all;

%% ===================== 1. 基础配置 =====================
root_path = 'D:\Code\CISCamera_DALSA\data\CISCamera_Image\';
dirs = {
    fullfile(root_path, 'mattxt'),    
    fullfile(root_path, 'txt1'),   
    fullfile(root_path, 'txt3')    
};

header_lines = 1;  
x_col = 2;         
y_col = 3;         

config.color1 = [1, 0.5, 0];  
config.color2 = [0, 0.6, 1];  
config.line1 = '-';
config.line2 = '--';
config.legend1 = '重投影1残差';
config.legend2 = '重投影2残差';
config.marker_size = 6;
config.quiver_scale = 1;

%% ===================== 2. 读取并排序TXT文件 =====================
gt_files = get_sorted_txt(dirs{1});
reproj1_files = get_sorted_txt(dirs{2});
reproj2_files = get_sorted_txt(dirs{3});

num_gt = length(gt_files);
num_r1 = length(reproj1_files);
num_r2 = length(reproj2_files);
if num_gt ~= num_r1 || num_gt ~= num_r2
    error('3个文件夹的TXT文件数量不匹配！ txt: %d, txt2: %d, txt3: %d', num_gt, num_r1, num_r2);
end
num_imgs = num_gt;
fprintf('成功读取 %d 幅图像的TXT文件\n', num_imgs);

%% ===================== 3. 逐图计算误差 =====================
err1_per_img = zeros(num_imgs,1);
err2_per_img = zeros(num_imgs,1);
all_gt = cell(num_imgs,1);
all_r1 = cell(num_imgs,1);
all_r2 = cell(num_imgs,1);

for img_idx = 1:num_imgs
    fprintf('\n正在处理第 %d/%d 幅图像...\n', img_idx, num_imgs);
    
    % 原检测点
    gt_path = fullfile(dirs{1}, gt_files(img_idx).name);
    try
        gt_data = readtable(gt_path, 'Delimiter', '\t', 'HeaderLines', header_lines);
        gt_pts = [gt_data{:, x_col}, gt_data{:, y_col}];
    catch
        warning('第%d幅图：原检测点读取失败，跳过！', img_idx);
        continue;
    end
    
    % 第一组重投影点
    r1_path = fullfile(dirs{2}, reproj1_files(img_idx).name);
    try
        r1_data = readtable(r1_path, 'Delimiter', '\t', 'HeaderLines', header_lines);
        r1_pts = [r1_data{:, x_col}, r1_data{:, y_col}];
    catch
        warning('第%d幅图：重投影1读取失败，跳过！', img_idx);
        continue;
    end
    
    % 第二组重投影点
    r2_path = fullfile(dirs{3}, reproj2_files(img_idx).name);
    try
        r2_data = readtable(r2_path, 'Delimiter', '\t', 'HeaderLines', header_lines);
        r2_pts = [r2_data{:, x_col}, r2_data{:, y_col}];
    catch
        warning('第%d幅图：重投影2读取失败，跳过！', img_idx);
        continue;
    end
    
    num_pts = size(gt_pts,1);
    if size(r1_pts,1) ~= num_pts || size(r2_pts,1) ~= num_pts
        warning('第%d幅图：点数不一致，跳过！', img_idx);
        continue;
    end
    
    % 误差
    err1_per_img(img_idx) = mean(sqrt(sum((r1_pts - gt_pts).^2,2)));
    err2_per_img(img_idx) = mean(sqrt(sum((r2_pts - gt_pts).^2,2)));
    
    all_gt{img_idx} = gt_pts;
    all_r1{img_idx} = r1_pts;
    all_r2{img_idx} = r2_pts;
    
    fprintf('第 %d 幅图平均误差：重投影1 = %.6f, 重投影2 = %.6f\n', ...
            img_idx, err1_per_img(img_idx), err2_per_img(img_idx));
end

%% ===================== 4. 总平均误差 =====================
valid_idx = (err1_per_img > 0) & (err2_per_img > 0);
if sum(valid_idx)==0
    error('无有效图像！');
end
total_err1 = mean(err1_per_img(valid_idx));
total_err2 = mean(err2_per_img(valid_idx));

fprintf('\n===================== 总误差汇总 =====================\n');
fprintf('有效图像数量：%d/%d\n', sum(valid_idx), num_imgs);
fprintf('重投影1总平均误差：%.6f\n', total_err1);
fprintf('重投影2总平均误差：%.6f\n', total_err2);
fprintf('=====================================================\n');

%% ===================== 5. 可视化所有有效图像 =====================

% img_path = '';  
img_path = 'D:\Code\CISCamera_DALSA\data\CISCamera_Image\qpg\Splice_20251108_160910966.bmp';  % 若你提供路径，在图像上作画

for img_idx = find(valid_idx)'
    gt_vis = all_gt{img_idx};
    r1_vis = all_r1{img_idx};
    r2_vis = all_r2{img_idx};
    
    res1 = r1_vis - gt_vis;
    res2 = r2_vis - gt_vis;

    % ==================== 新功能：如果提供了图片路径 → 在图片上作画 ====================
    if ~isempty(img_path) && exist(img_path,'file')

        img = imread(img_path);

        figure('Name', sprintf('残差图 %d (原图叠加)', img_idx), ...
               'NumberTitle','off', 'Position',[100,100,1200,800]);

        imshow(img); hold on;

        title(sprintf('第 %d 幅图像重投影可视化（叠加原图）', img_idx));

        plot(gt_vis(:,1), gt_vis(:,2), 'ko', 'MarkerSize', config.marker_size, 'LineWidth',1.2);
        plot(r1_vis(:,1), r1_vis(:,2), 'o', 'Color', config.color1, ...
             'MarkerSize', config.marker_size-1, 'LineWidth',1.2);
        plot(r2_vis(:,1), r2_vis(:,2), 's', 'Color', config.color2, ...
             'MarkerSize', config.marker_size-1, 'LineWidth',1.2);

        quiver(gt_vis(:,1), gt_vis(:,2), res1(:,1), res1(:,2), 0, ...
               'Color', config.color1, 'LineStyle', config.line1, 'LineWidth', 1.2);
        quiver(gt_vis(:,1), gt_vis(:,2), res2(:,1), res2(:,2), 0, ...
               'Color', config.color2, 'LineStyle', config.line2, 'LineWidth', 1.2);

        legend('原检测点', config.legend1, config.legend2, 'Location','southeast');
        continue;  % ★ 不走下面的原图逻辑，进入下一个循环
    end
    % ==========================================================================


    % ==================== 原来的纯点云绘图逻辑（保留） ====================
    figure('Name', sprintf('残差图 %d', img_idx), ...
           'NumberTitle','off', 'Position',[100,100,1200,800]);
    hold on; grid on; axis equal;

    xlabel('X 坐标（像素）'); ylabel('Y 坐标（像素）');
    title(sprintf('第 %d 幅图像重投影残差可视化（err1=%.4f px, err2=%.4f px）', ...
          img_idx, err1_per_img(img_idx), err2_per_img(img_idx)));

    plot(gt_vis(:,1), gt_vis(:,2), 'ko', 'MarkerSize', config.marker_size);
    plot(r1_vis(:,1), r1_vis(:,2), 'o', 'Color', config.color1, ...
         'MarkerSize', config.marker_size-1);
    plot(r2_vis(:,1), r2_vis(:,2), 's', 'Color', config.color2, ...
         'MarkerSize', config.marker_size-1);

    quiver(gt_vis(:,1), gt_vis(:,2), res1(:,1), res1(:,2), config.quiver_scale, ...
           'Color', config.color1, 'LineStyle', config.line1, 'LineWidth', 1.2);
    quiver(gt_vis(:,1), gt_vis(:,2), res2(:,1), res2(:,2), config.quiver_scale, ...
           'Color', config.color2, 'LineStyle', config.line2, 'LineWidth', 1.2);

    legend('原检测点', config.legend1, config.legend2, 'Location','northeastoutside');
end
%% ===================== 6. 辅助函数 =====================
function sorted_files = get_sorted_txt(dir_path)
    dir_files = dir(fullfile(dir_path,'*.txt'));
    if isempty(dir_files)
        sorted_files = [];
        return;
    end
    [~, sort_idx] = natsort({dir_files.name});
    sorted_files = dir_files(sort_idx);
end

function [sorted_str, sort_idx] = natsort(str_list)
    if ~iscell(str_list)
        str_list = {str_list};
    end
    n = length(str_list);
    nums = zeros(n,1);
    for i = 1:n
        str = str_list{i};
        num_str = regexp(str,'\d+','match');
        if ~isempty(num_str)
            nums(i) = str2double(num_str{1});
        else
            nums(i) = 0;
        end
    end
    [~, sort_idx] = sort(nums);
    sorted_str = str_list(sort_idx);
end
